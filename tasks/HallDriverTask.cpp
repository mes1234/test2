#include "FreeRTOS.h"

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/timer.h"

#include "../common/definitions.h"
#include "../common/interfaces.h"

#include <math.h>
#include <stdio.h>
#include "timers.h"

float raw_angle_to_radians(uint16_t raw_angle)
{
    return (raw_angle * 2.0f * M_PI) / 4096.0f;
}

float angle_rad_to_deg(float rad_angle)
{
    return rad_angle * (180.0f / M_PI);
}

float read_raw_angle(i2c_inst_t *i2c)
{
    uint8_t data[2];
    uint8_t data_address_lsb = AS5600_ANGLE;

    i2c_write_blocking(i2c, AS5600_ADDRESS, &data_address_lsb, 1, true);
    i2c_read_blocking(i2c, AS5600_ADDRESS, data, 2, false);

    uint16_t raw_angle = ((data[0] & 0x0F) << 8) | data[1];
    return raw_angle_to_radians(raw_angle);
}

void ReadAngleCallback(TimerHandle_t xTimer)
{
    uint64_t timestamp = time_us_64();
    float angle_rad = read_raw_angle(i2c_default);
}