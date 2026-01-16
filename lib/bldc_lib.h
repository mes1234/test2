#include <cmath>
#include <cstdint>

#define ANGLE_BUFFER_SIZE 10

#define TWO_PI (2.0 * M_PI)
#define ZERO_CROSSING_THRSHOLD TWO_PI / 3.0
#define ZERO_NOISE 0.005 // ~0.3deg

typedef struct
{
    uint64_t timestamp = 0;
    double angle_rad = 0;
    int revolution = 0;
} AngleInTime;

typedef struct
{
    AngleInTime buffer[ANGLE_BUFFER_SIZE];
    float buffer_mask[ANGLE_BUFFER_SIZE - 1] = {0.0};
    bool buffer_initialized = false;
    float avg_rot_speed = 0.0;
    int buffer_position = 0;
    float buffer_weight_ratio = 0.0;
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

/// @brief Estimate revolution adder based on previous and current angle
/// @return 1,0,-1
int estimate_revolution_adder(float prev, float current);

/// @brief Estimate absolute angle based on definition
/// @param angle_in_time
/// @return
double get_abslute_angle(AngleInTime &angle_in_time);

inline double to_degrees(double radians)
{
    return radians * (180.0 / M_PI);
}

inline double to_radians(double deg)
{
    return deg * (M_PI / 180.0);
}