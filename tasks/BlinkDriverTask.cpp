#include "FreeRTOS.h"

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "../common/definitions.h" 
#include "../common/interfaces.h" 

void vTaskBlinkAtZero(void *pvParameters)
{
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    int blink_count = 0;
    int i = 0;

    while (1)
    {
        xQueueReceive(queue_zero_crossing, (void *)&blink_count, 10);

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