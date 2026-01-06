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

    pwm_set_clkdiv(slice_num_u, 10); // 125 MHz / 125 = 1 MHz, /5 = 200 kHz, /10 = 20 kHz
    pwm_set_wrap(slice_num_u, 1000);
    pwm_set_chan_level(slice_num_u, PWM_CHAN_A, 0); // Start with 0% duty cycle
    pwm_set_enabled(slice_num_u, true);

    // Repeat for INH_V and INH_W
    pwm_set_clkdiv(slice_num_v, 10);
    pwm_set_wrap(slice_num_v, 1000);
    pwm_set_chan_level(slice_num_v, PWM_CHAN_A, 0);
    pwm_set_enabled(slice_num_v, true);

    pwm_set_clkdiv(slice_num_w, 10);
    pwm_set_wrap(slice_num_w, 1000);
    pwm_set_chan_level(slice_num_w, PWM_CHAN_A, 0);
    pwm_set_enabled(slice_num_w, true);
}

float previous_angle = 0.0f;

float calculate_phase_advance(float current_el_angle, float previous_angle)
{
    float speed_s = BLDC_DRIVER_MS / 1000.f;                         // Time between updates in seconds
    float rot_speed = (current_el_angle - previous_angle) / speed_s; // Electrical angular velocity (rad/s)

    // Define maximum phase advance (e.g., 30° = 0.5236 rad)
    const float max_phase_advance = 0.5236f;
    // Define speed threshold for max advance (e.g., 100 rad/s)
    const float speed_threshold = 100.0f;

    // Calculate phase advance: linear with speed, capped at max_phase_advance
    float phase_advance = rot_speed / speed_threshold * max_phase_advance;
    if (phase_advance > max_phase_advance)
    {
        phase_advance = max_phase_advance;
    }
    else if (phase_advance < 0.0f)
    {
        phase_advance = 0.0f;
    }

    return phase_advance;
}
void inverse_park_transform(float vd, float vq, float theta, float *valpha, float *vbeta)
{
    *valpha = vd * cosf(theta) - vq * sinf(theta);
    *vbeta = vd * sinf(theta) + vq * cosf(theta);
}

void inverse_clarke_transform(float valpha, float vbeta, float *va, float *vb, float *vc)
{
    *va = valpha;
    *vb = -0.5f * valpha + 0.866025f * vbeta;
    *vc = -0.5f * valpha - 0.866025f * vbeta;
}

void set_phase_pwm(uint16_t amplitude)
{
    // Electrical to mechanical angle for 22P/24N -> 11x
    float electrical_angle = (angle_rad - zero_offset_angle + 0.01f) * 11.0f;
    electrical_angle += calculate_phase_advance(electrical_angle, previous_angle);

    electrical_angle = fmodf(electrical_angle, 2.0f * 3.14159f);

    // 2. Set desired voltages (open-loop FOC)
    float vd = 0.0f;                // No d-axis voltage
    float vq = amplitude / 1000.0f; // Normalize amplitude to [0,1]

    // 3. Inverse Park transform
    float valpha, vbeta;
    inverse_park_transform(vd, vq, electrical_angle, &valpha, &vbeta);

    // 4. Inverse Clarke transform
    float va, vb, vc;
    inverse_clarke_transform(valpha, vbeta, &va, &vb, &vc);

    // 4. Scale and apply PWM (va, vb, vc are in [-1,1])
    uint16_t u = (uint16_t)(500.0f * (va + 1.0f)); // Scale to [0,1000]
    uint16_t v = (uint16_t)(500.0f * (vb + 1.0f));
    uint16_t w = (uint16_t)(500.0f * (vc + 1.0f));

    // Clamp to [0,1000] (optional, for safety)
    if (u > 1000)
        u = 1000;
    if (v > 1000)
        v = 1000;
    if (w > 1000)
        w = 1000;

    // uint16_t u = amplitude * (0.5f + 0.5f * sinf(electrical_angle));
    // uint16_t v = amplitude * (0.5f + 0.5f * sinf(electrical_angle - 2.094f));
    // uint16_t w = amplitude * (0.5f + 0.5f * sinf(electrical_angle + 2.094f));

    pwm_set_chan_level(pwm_gpio_to_slice_num(INH_U), PWM_CHAN_A, u);
    pwm_set_chan_level(pwm_gpio_to_slice_num(INH_V), PWM_CHAN_A, v);
    pwm_set_chan_level(pwm_gpio_to_slice_num(INH_W), PWM_CHAN_A, w);

    previous_angle = electrical_angle;
}

void set_phase_a(uint16_t amplitude)
{
    pwm_set_chan_level(pwm_gpio_to_slice_num(INH_U), PWM_CHAN_A, amplitude);
    pwm_set_chan_level(pwm_gpio_to_slice_num(INH_V), PWM_CHAN_A, 0);
    pwm_set_chan_level(pwm_gpio_to_slice_num(INH_W), PWM_CHAN_A, 0);
}

void set_phase_b(uint16_t amplitude)
{
    pwm_set_chan_level(pwm_gpio_to_slice_num(INH_U), PWM_CHAN_A, 0);
    pwm_set_chan_level(pwm_gpio_to_slice_num(INH_V), PWM_CHAN_A, amplitude);
    pwm_set_chan_level(pwm_gpio_to_slice_num(INH_W), PWM_CHAN_A, 0);
}

void set_phase_c(uint16_t amplitude)
{
    pwm_set_chan_level(pwm_gpio_to_slice_num(INH_U), PWM_CHAN_A, 0);
    pwm_set_chan_level(pwm_gpio_to_slice_num(INH_V), PWM_CHAN_A, 0);
    pwm_set_chan_level(pwm_gpio_to_slice_num(INH_W), PWM_CHAN_A, amplitude);
}

uint16_t duty = 500; // 50% duty cycle
int step = 0;
const float angle_step = 0.02f; // speed control (smaller = slower)

void InitBldcDriver()
{
    init_pwm();
}

void BldcDriverCallback(TimerHandle_t xTimer)
{
    switch (system_mode)
    {
    case OFF:
        break;
    case PHASE_A:
        set_phase_a(duty);
        break;
    case PHASE_B:
        set_phase_b(duty);
        break;
    case PHASE_C:
        set_phase_c(duty);
        break;
    case AUTO:
        set_phase_pwm(duty); // sinusoidal 3-phase output
        break;
    default:
        break;
    }
}