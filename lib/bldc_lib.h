#include <cmath>
#include <cstdint>

#define ANGLE_BUFFER_SIZE 10

#define TWO_PI (2.0 * M_PI)

typedef struct
{
    uint64_t timestamp;
    double angle_rad;
} AngleInTime;

typedef struct
{
    AngleInTime buffer[ANGLE_BUFFER_SIZE];
    float buffer_mask[ANGLE_BUFFER_SIZE - 1] = {0.0};
    bool buffer_initialized = false;
    float avg_rot_speed = 0.0;
    int buffer_position = 0;
} AngleBuffer;

/// @brief Init buffer mask
/// @param buffer buffer
/// @param ratio exponent ratio
void init_buffer_mask(AngleBuffer *buffer, float ratio);

/// @brief Init buffer mask so last item is taken with ratio and rest is adjusted using linear ramp
/// @param buffer
/// @param ratio
void init_buffer_mask(AngleBuffer *buffer, float ratio);

/// @brief Add to angle buffer another sensor read
/// @param angle angle to add
/// @param timestamp timestamp of measurement
void add_angle_to_buffer(AngleBuffer *buffer, float angle, uint64_t timestamp);

/// @brief estimate angle based on provided timestamp
/// @param timestamp timestamp in question
/// @return
float estimate_angle(AngleBuffer *buffer, uint64_t timestamp);

/// @brief Get last item in buffer
/// @return
float get_current_buffer_value(AngleBuffer *buffer);