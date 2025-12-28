#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"

#include "pico/stdlib.h"

#include "common/definitions.h"
#include "common/interfaces.h"

#include <math.h>
#include <stdio.h>

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

    vTaskDelay(pdMS_TO_TICKS(10));

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
        }
        if (buttons.b)
        {
            state = 0;
            xQueueSend(start_stop_queue, &state, 10);
        }
        sleep_ms(20);
    }
}