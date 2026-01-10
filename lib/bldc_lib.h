#include <cmath>
#include <cstdint>

typedef struct
{
    uint64_t timestamp;
    double angle_rad;
} AngleInTime;

#define ANGLE_BUFFER_SIZE 10

/// @brief Add two angles
/// @param alfa
/// @param beta
/// @return return angle
double add_angles(double alfa, double beta);

/// @brief Add to angle buffer another sensor read
/// @param angle angle to add
/// @param timestamp timestamp of measurement
void add_angle_to_buffer(float angle, uint64_t timestamp);

/// @brief estimate angle based on provided timestamp
/// @param timestamp timestamp in question
/// @return
float estimate_angle(uint64_t timestamp);