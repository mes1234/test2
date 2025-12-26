#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"

#include "pico/stdlib.h"

#include "adder.h"
#include <cmath>
#include <cstdio>

// Define PWM pins for DRV8313 high-side inputs
#define INH_U 16
#define INH_V 18
#define INH_W 20
#define ENABLE_PIN 27

// Define I2C pins
#define SDA_PIN 4
#define SCL_PIN 5

// Gempad definitions
#define SEESAW_GPIO_BASE 0x01
#define SEESAW_GPIO_BULK 0x04
#define SEESAW_I2C_ADDR 0x50
#define SEESAW_GPIO_DIRCLR_BULK 0x03
#define SEESAW_GPIO_PULLENSET 0x0B
#define SEESAW_GPIO_BULK_SET 0x05

// AS6500 definitions
#define AS5600_ADDRESS 0x36
#define AS5600_ANGLE 0x0E

#define BUTTON_X 6
#define BUTTON_Y 2
#define BUTTON_A 5
#define BUTTON_B 1
#define BUTTON_SELECT 0
#define BUTTON_START 16
// Button mask
#define BUTTON_MASK (      \
    (1 << BUTTON_X) |      \
    (1 << BUTTON_Y) |      \
    (1 << BUTTON_A) |      \
    (1 << BUTTON_B) |      \
    (1 << BUTTON_SELECT) | \
    (1 << BUTTON_START))

typedef struct
{
    bool x;
    bool y;
    bool a;
    bool b;
    bool select;
    bool start;
} ButtonStates;

static QueueHandle_t queue;

static QueueHandle_t start_stop_queue;

void vTaskBlink(void *pvParameters)
{
    Adder adder;
    while (1)
    {
        vTaskDelay(5000);

        auto result = adder.add(1, 2);

        xQueueSend(queue, &result, 10);
    }
}

void vTaskBlink2(void *pvParameters)
{
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    int blink_count = 0;
    int i = 0;

    while (1)
    {
        xQueueReceive(queue, (void *)&blink_count, 10);

        for (i = 0; i < blink_count; i++)
        {
            gpio_put(LED_PIN, 1);
            vTaskDelay(50);
            gpio_put(LED_PIN, 0);
            vTaskDelay(50);
        }

        blink_count = 0;
    }
}

ButtonStates digitalReadBulk(i2c_inst_t *i2c)
{

    uint8_t reg[2] = {SEESAW_GPIO_BASE, SEESAW_GPIO_BULK};
    uint8_t buf[4] = {0, 0, 0, 0};
    int ret = -1;

    // Write register address for base and gpio
    ret = i2c_write_blocking(i2c, SEESAW_I2C_ADDR, reg, 2, false);
    if (ret < 0)
    {
        printf("Error writing register address base: %d\n", ret);
        return (ButtonStates){0};
    }

    sleep_ms(1);

    // Read 4 bytes
    ret = i2c_read_blocking(i2c, SEESAW_I2C_ADDR, buf, 4, false);
    if (ret < 0)
    {
        printf("Error reading data: %d\n", ret);
        return (ButtonStates){0};
    }

    // Combine bytes into 32-bit value
    uint32_t ret_val = ((uint32_t)buf[0] << 24) |
                       ((uint32_t)buf[1] << 16) |
                       ((uint32_t)buf[2] << 8) |
                       ((uint32_t)buf[3]);

    ButtonStates states;
    states.x = (ret_val & (1 << BUTTON_X)) ? false : true;
    states.y = (ret_val & (1 << BUTTON_Y)) ? false : true;
    states.a = (ret_val & (1 << BUTTON_A)) ? false : true;
    states.b = (ret_val & (1 << BUTTON_B)) ? false : true;
    states.select = (ret_val & (1 << BUTTON_SELECT)) ? false : true;
    states.start = (ret_val & (1 << BUTTON_START)) ? false : true;

    return states;
}

void configureButtons(i2c_inst_t *i2c)
{
    i2c_init(i2c, 100 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    uint8_t dir_buf[6] = {
        SEESAW_GPIO_BASE, SEESAW_GPIO_DIRCLR_BULK, // Register address
        (uint8_t)(BUTTON_MASK >> 24),              // Data (4 bytes)
        (uint8_t)(BUTTON_MASK >> 16),
        (uint8_t)(BUTTON_MASK >> 8),
        (uint8_t)BUTTON_MASK};

    uint8_t pullset_buf[6] = {
        SEESAW_GPIO_BASE, SEESAW_GPIO_PULLENSET, // Register address
        (uint8_t)(BUTTON_MASK >> 24),            // Data (4 bytes)
        (uint8_t)(BUTTON_MASK >> 16),
        (uint8_t)(BUTTON_MASK >> 8),
        (uint8_t)BUTTON_MASK};

    uint8_t bulkset_buf[6] = {
        SEESAW_GPIO_BASE, SEESAW_GPIO_BULK_SET, // Register address
        (uint8_t)(BUTTON_MASK >> 24),           // Data (4 bytes)
        (uint8_t)(BUTTON_MASK >> 16),
        (uint8_t)(BUTTON_MASK >> 8),
        (uint8_t)BUTTON_MASK};

    // Set pins as inputs (DIRCLR)
    int ret = i2c_write_blocking(i2c, SEESAW_I2C_ADDR, dir_buf, 6, false);
    if (ret < 0)
    {
        printf("Error writing DIRCLR: %d\n", ret);
        return;
    }

    // Enable pull-ups (PULLENSET)
    ret = i2c_write_blocking(i2c, SEESAW_I2C_ADDR, pullset_buf, 6, false);
    if (ret < 0)
    {
        printf("Error writing PULLENSET: %d\n", ret);
        return;
    }

    // Set pull-ups high (BULK_SET)
    ret = i2c_write_blocking(i2c, SEESAW_I2C_ADDR, bulkset_buf, 6, false);
    if (ret < 0)
    {
        printf("Error writing BULK_SET: %d\n", ret);
        return;
    }
}

void vTaskGamePadDriver(void *pvParameters)
{
    configureButtons(i2c_default);

    int state = 0;

    while (1)
    {
        ButtonStates buttons = digitalReadBulk(i2c_default);
        if (buttons.a)
        {
            state = 1;
            xQueueSend(start_stop_queue, &state, 10);
            printf("Button A is pressed!\n");
        }
        if (buttons.b)
        {
            state = 0;
            xQueueSend(start_stop_queue, &state, 10);
            printf("Button B is pressed!\n");
        }
        sleep_ms(20);
    }
}

void vTaskStartStopDriver(void *pvParameters)
{
    int state = 0;
    gpio_init(ENABLE_PIN);
    gpio_set_dir(ENABLE_PIN, GPIO_OUT);
    gpio_put(ENABLE_PIN, state);
    while (1)
    {
        xQueueReceive(start_stop_queue, &state, 10);

        gpio_put(ENABLE_PIN, state);
    }
}

float raw_angle_to_degrees(uint16_t raw_angle)
{
    return (raw_angle * 360.0f) / 4096.0f;
}

float read_raw_angle(i2c_inst_t *i2c)
{
    uint8_t data[2];
    uint8_t data_address_lsb = AS5600_ANGLE;

    i2c_write_blocking(i2c, AS5600_ADDRESS, &data_address_lsb, 1, true);
    i2c_read_blocking(i2c, AS5600_ADDRESS, data, 2, false);

    uint16_t raw_angle = ((data[0] & 0x0F) << 8) | data[1];
    return raw_angle_to_degrees(raw_angle);
}

void vTaskReadRawAngle(void *pvParameters)
{
    float angle_deg = 0.0f;
    while (1)
    {
        sleep_ms(500);
        angle_deg = read_raw_angle(i2c_default);
        printf("Angle (deg): %.2f\n", angle_deg);
    }
}

void init_pwm()
{
    // Initialize PWM for the 3 high-side channels
    gpio_set_function(INH_U, GPIO_FUNC_PWM);
    gpio_set_function(INH_V, GPIO_FUNC_PWM);
    gpio_set_function(INH_W, GPIO_FUNC_PWM);

    // Set PWM frequency (e.g., 20 kHz)
    uint slice_num_u = pwm_gpio_to_slice_num(INH_U);
    uint slice_num_v = pwm_gpio_to_slice_num(INH_V);
    uint slice_num_w = pwm_gpio_to_slice_num(INH_W);

    pwm_set_clkdiv(slice_num_u, 1); // 125 MHz / 125 = 1 MHz, /5 = 200 kHz, /10 = 20 kHz
    pwm_set_wrap(slice_num_u, 1000);
    pwm_set_chan_level(slice_num_u, PWM_CHAN_A, 0); // Start with 0% duty cycle
    pwm_set_enabled(slice_num_u, true);

    // Repeat for INH_V and INH_W
    pwm_set_clkdiv(slice_num_v, 1);
    pwm_set_wrap(slice_num_v, 1000);
    pwm_set_chan_level(slice_num_v, PWM_CHAN_A, 0);
    pwm_set_enabled(slice_num_v, true);

    pwm_set_clkdiv(slice_num_w, 1);
    pwm_set_wrap(slice_num_w, 1000);
    pwm_set_chan_level(slice_num_w, PWM_CHAN_A, 0);
    pwm_set_enabled(slice_num_w, true);
}

void set_phase_pwm(float angle, uint16_t amplitude)
{
    uint16_t u = amplitude * (0.5f + 0.5f * sinf(angle));
    uint16_t v = amplitude * (0.5f + 0.5f * sinf(angle - 2.094f));
    uint16_t w = amplitude * (0.5f + 0.5f * sinf(angle + 2.094f));

    pwm_set_chan_level(pwm_gpio_to_slice_num(INH_U), PWM_CHAN_A, u);
    pwm_set_chan_level(pwm_gpio_to_slice_num(INH_V), PWM_CHAN_A, v);
    pwm_set_chan_level(pwm_gpio_to_slice_num(INH_W), PWM_CHAN_A, w);
}

void vTaskBldcDriver(void *pvParameters)
{
    init_pwm();

    uint16_t duty = 500; // 50% duty cycle
    int step = 0;

    float angle = 0.0f;            // electrical angle [rad]
    const float angle_step = 0.2f; // speed control (smaller = slower)

    while (1)
    {
        set_phase_pwm(angle, duty); // sinusoidal 3-phase output
        angle += angle_step;
        if (angle > 2.0f * M_PI)
            angle -= 2.0f * M_PI;

        sleep_us(500);
    }
}

int main()
{
    stdio_init_all();

    queue = xQueueCreate(10, sizeof(int));
    start_stop_queue = xQueueCreate(10, sizeof(int));

    xTaskCreate(vTaskBlink, "Blink Task", 256, NULL, 1, NULL);
    xTaskCreate(vTaskBlink2, "Blink Task", 256, NULL, 1, NULL);
    xTaskCreate(vTaskBldcDriver, "Blink Task", 256, NULL, 1, NULL);
    xTaskCreate(vTaskGamePadDriver, "Blink Task", 256, NULL, 1, NULL);
    xTaskCreate(vTaskReadRawAngle, "Read angle", 256, NULL, 1, NULL);
    xTaskCreate(vTaskStartStopDriver, "Read buttons for start stop", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1)
        ;
}