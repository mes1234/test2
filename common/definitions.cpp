#include "FreeRTOS.h"
#include "queue.h"

#include "definitions.h"

QueueHandle_t start_stop_queue;
QueueHandle_t command_queue;

float zero_offset_angle = -1.0f;

float between_phase_angle = -1.0f;

DriveMode system_mode = OFF;