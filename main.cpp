#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"

#include "pico/stdlib.h"

#include "common/definitions.h"
#include "common/interfaces.h"

#include <cmath>
#include <cstdio>

int main()
{
    stdio_init_all();

    start_stop_queue = xQueueCreate(10, sizeof(int));
    command_queue = xQueueCreate(10, sizeof(int));

    xTaskCreate(vTaskGamePadDriver, "Blink Task", 256, NULL, 1, NULL);
    xTaskCreate(vTaskStartStopDriver, "Read buttons for start stop", 256, NULL, 1, NULL);

    TimerHandle_t hallSensorTimer = xTimerCreate("Timer Hall sensor", pdMS_TO_TICKS(HALL_READ_PERIOD_MS), pdTRUE, NULL, ReadAngleCallback);
    xTimerStart(hallSensorTimer, 0);

    InitBldcDriver();

    TimerHandle_t bldcDriverTimer = xTimerCreate("Timer BLDC driver", pdMS_TO_TICKS(BLDC_DRIVER_MS), pdTRUE, NULL, BldcDriverCallback);

    xTimerStart(hallSensorTimer, 0);
    xTimerStart(bldcDriverTimer, 0);

    vTaskStartScheduler();

    while (1)
        ;
}