#include "FreeRTOS.h"

#include "pico/stdlib.h"
#include "queue.h"
#include "hardware/i2c.h"
#include "../common/definitions.h"
#include "../common/interfaces.h"

#include <cstdio>

const char *mode_names[] = {"OFF", "PHASE_A", "PHASE_B", "PHASE_C", "AUTO"};

float phase_a_angle = -1.0f;
float phase_b_angle = -1.0f;
float phase_c_angle = -1.0f;

void start_sequence()
{
    // Start motor
    int state = MOTOR_ENABLED;
    for (int i = 0; i < DEBOUNCE_SIZE * 10; i++)
    {
        xQueueSend(start_stop_queue, &state, 10);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
    printf("\nNEW RUN\n");
    printf("START SEQ: START MOTOR: DONE\n");
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Set A phase
    system_mode = PHASE_A;
    vTaskDelay(pdMS_TO_TICKS(1000));
    phase_a_angle = angle_rad;
    printf("START SEQ: PHASE A ON: DONE, angle A: %.2f\n", angle_rad_to_deg(phase_a_angle));

    // Set B phase
    system_mode = PHASE_B;
    vTaskDelay(pdMS_TO_TICKS(1000));
    phase_b_angle = angle_rad;
    printf("START SEQ: PHASE B ON: DONE, angle B: %.2f\n", angle_rad_to_deg(phase_b_angle));

    // Set C phase
    system_mode = PHASE_C;
    vTaskDelay(pdMS_TO_TICKS(1000));
    phase_c_angle = angle_rad;
    printf("START SEQ: PHASE C ON: DONE, angle C: %.2f\n", angle_rad_to_deg(phase_c_angle));

    // Set B phase
    system_mode = PHASE_B;
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Set A phase
    system_mode = PHASE_A;
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Stop motor
    state = MOTOR_STOP;
    for (int i = 0; i < DEBOUNCE_SIZE * 10; i++)
    {
        xQueueSend(start_stop_queue, &state, 10);
    }
    printf("START SEQ: STOP MOTOR: DONE\n");
    system_mode = OFF;

    between_phase_angle = ((phase_b_angle - phase_a_angle) + (phase_c_angle - phase_b_angle)) / 2.0f;

    zero_offset_angle = phase_a_angle;

    printf("CALIBRATION ZERO OFFSET : %.2f, average phase angle: %.2f\n", angle_rad_to_deg(zero_offset_angle), angle_rad_to_deg(between_phase_angle));
}

void vTaskSystemConfig(void *ppvParameters)
{
    int waiter = 0;

    int state = 0;

    start_sequence();

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