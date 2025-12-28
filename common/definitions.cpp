#include "FreeRTOS.h"
#include "queue.h"

#include "definitions.h"

QueueHandle_t start_stop_queue;

float angle_rad = 0.0f;

long time_absolute_ticks_last_angle_read = 0;