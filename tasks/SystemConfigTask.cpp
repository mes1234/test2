#include "FreeRTOS.h"

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "../common/definitions.h"
#include "../common/interfaces.h"

#include <cstdio>

const char *mode_names[] = {"OFF", "PHASE_A", "PHASE_B", "PHASE_C", "AUTO"};

void vTaskSystemConfig(void *ppvParameters)
{
    int waiter = 0;

    int state = 0;

    while (true)
    {
        int status = xQueueReceive(command_queue, &state, pdMS_TO_TICKS(1));

        if (status == pdTRUE)
        {
            if (state == PHASE_A_SET)
            {
                system_mode = PHASE_A;
            }

            if (state == PHASE_B_SET)
            {
                system_mode = PHASE_B;
            }

            if (state == PHASE_C_SET)
            {
                system_mode = PHASE_C;
            }

            if (state == MOTOR_STOP)
            {
                system_mode = OFF;
            }

            if (state == AUTO_MODE)
            {
                system_mode = AUTO;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));

        waiter++;

        if (waiter > 20)
        {
            printf("Angle (deg): %.2f, MODE: %s\n", angle_rad_to_deg(angle_rad), mode_names[system_mode]);
            waiter = 0;
        }
    }
}