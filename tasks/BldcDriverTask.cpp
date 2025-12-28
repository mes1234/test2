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

uint16_t duty = 500; // 50% duty cycle
int step = 0;
const float angle_step = 0.02f; // speed control (smaller = slower)

void InitBldcDriver()
{
    init_pwm();
}

void BldcDriverCallback(TimerHandle_t xTimer)
{
    set_phase_pwm(angle_rad + angle_step, duty); // sinusoidal 3-phase output
}