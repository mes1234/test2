#include "queue.h"
#include "timers.h"

// Driver for gamepad which does button interaction
void vTaskGamePadDriver(void *pvParameters);

// Init BLDC driver
void InitBldcDriver();

// Callback to update BLDC motor setup
void BldcDriverCallback(TimerHandle_t xTimer);

// Callback to read hall sensor
void ReadAngleCallback(TimerHandle_t xTimer);

// Driver to enable/disable motor supply voltage
void vTaskStartStopDriver(void *pvParameters);

extern QueueHandle_t start_stop_queue;

// Single Write, multiple Read Angle
extern float angle_rad;

// Time from start of service
extern long time_absolute_ticks_last_angle_read;