#include "queue.h"
#include "timers.h"

// Driver for gamepad which does button interaction
void vTaskGamePadDriver(void *pvParameters);

// Task to interface with driver
void vTaskSystemConfig(void *pvParameters);

// Init BLDC driver
void InitBldcDriver();

// Callback to update BLDC motor setup
void BldcDriverCallback(TimerHandle_t xTimer);

// Callback to read hall sensor
void ReadAngleCallback(TimerHandle_t xTimer);

float angle_rad_to_deg(float rad_angle);

// Driver to enable/disable motor supply voltage
void vTaskStartStopDriver(void *pvParameters);

extern QueueHandle_t start_stop_queue;
extern QueueHandle_t command_queue;

// Single Write, multiple Read Angle
// Circular buffer of last ANGLE_BUFFER_SIZE
extern AngleInTime angle_in_time_rad[ANGLE_BUFFER_SIZE];

// Single Write, offset to set 0 Angle
extern float zero_offset_angle;

// Average angle between phases (mechanical)
extern float between_phase_angle;

extern DriveMode system_mode;