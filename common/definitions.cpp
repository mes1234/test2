#include "FreeRTOS.h"
#include "queue.h"

#include "definitions.h"

QueueHandle_t start_stop_queue;
QueueHandle_t command_queue;

float angle_rad = 0.0f;

float zero_offset_angle = -1.0f;

float between_phase_angle = -1.0f;

long time_absolute_ticks_last_angle_read = 0;

DriveMode system_mode = OFF;