#include "FreeRTOS.h"
#include "task.h"

#include "pico/stdlib.h"

#include "adder.h"

void vTaskBlink(void *pvParameters)
{
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    while (1)
    {
        gpio_put(LED_PIN, 1);
        vTaskDelay(500);
        gpio_put(LED_PIN, 0);
        vTaskDelay(500);
        Adder adder;

        auto result = adder.add(1, 2);
    }
}

void vTaskBlink2(void *pvParameters)
{
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    while (1)
    {
        gpio_put(LED_PIN, 1);
        vTaskDelay(50);
        gpio_put(LED_PIN, 0);
        vTaskDelay(50);
    }
}

int main()
{
    xTaskCreate(vTaskBlink, "Blink Task", 256, NULL, 1, NULL);
    xTaskCreate(vTaskBlink2, "Blink Task", 256, NULL, 1, NULL);
    vTaskStartScheduler();
    while (1)
        ;
}