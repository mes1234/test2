#include "FreeRTOS.h"

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "../common/definitions.h"
#include "../common/interfaces.h"

#define DEBOUNCE_SIZE 10

void vTaskStartStopDriver(void *pvParameters)
{
    int state[DEBOUNCE_SIZE] = {0};
    bool debounce_ok = false;
    gpio_init(ENABLE_PIN);
    gpio_set_dir(ENABLE_PIN, GPIO_OUT);
    gpio_put(ENABLE_PIN, 0);

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);

    while (1)
    {
        int i = 0;
        while (i < DEBOUNCE_SIZE)
        {
            int status = xQueueReceive(start_stop_queue, &state[i], pdMS_TO_TICKS(1));

            if (status == pdTRUE)
            {
                i++;
            }
        }

        debounce_ok = true;
        for (int i = 0; i <= DEBOUNCE_SIZE - 1; i++)
        {
            if (state[0] != state[i])
            {
                debounce_ok = false;
                break;
            }
        }
        if (debounce_ok)
        {
            gpio_put(ENABLE_PIN, state[0]);
            gpio_put(PICO_DEFAULT_LED_PIN, state[0]);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}