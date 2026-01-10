#include <cmath>
#include "bldc_lib.h"
#include <cstdint>

#define TWO_PI (2.0 * M_PI)

AngleInTime angle_in_time_rad[ANGLE_BUFFER_SIZE] = {0};

float angular_speed[ANGLE_BUFFER_SIZE - 1] = {0};

float avg_rot_speed = 0.0;

int buffer_position = 0;
int prev_buffer_position = ANGLE_BUFFER_SIZE - 1;

double add_angles(double alfa, double beta)
{
    float wrapped = fmodf(alfa + beta, TWO_PI);
    return wrapped < 0 ? wrapped + TWO_PI : wrapped;
}

void update_average_rot_speed()
{
    int i = 0;
    int counter = 0;

    if (buffer_position != ANGLE_BUFFER_SIZE - 1)
    {
        for (i = buffer_position + 1; i < ANGLE_BUFFER_SIZE - 2; i++)
        {
            angular_speed[counter] = (float)(angle_in_time_rad[i + 1].angle_rad - angle_in_time_rad[i].angle_rad) / (float)(angle_in_time_rad[i + 1].timestamp - angle_in_time_rad[i].timestamp);
            counter++;
        }

        angular_speed[counter] = (float)(angle_in_time_rad[0].angle_rad - angle_in_time_rad[ANGLE_BUFFER_SIZE - 1].angle_rad) / (float)(angle_in_time_rad[0].timestamp - angle_in_time_rad[ANGLE_BUFFER_SIZE - 1].timestamp);
        counter++;
    }

    for (i = 0; i < buffer_position; i++)
    {
        angular_speed[counter] = (float)(angle_in_time_rad[i + 1].angle_rad - angle_in_time_rad[i].angle_rad) / (float)(angle_in_time_rad[i + 1].timestamp - angle_in_time_rad[i].timestamp);
        counter++;
    }

    avg_rot_speed = 0;

    for (i = 0; i < ANGLE_BUFFER_SIZE - 1; i++)
    {
        avg_rot_speed = avg_rot_speed + angular_speed[i];
    }

    avg_rot_speed = avg_rot_speed / ANGLE_BUFFER_SIZE - 1;
}

float estimate_angle(uint64_t timestamp)
{
    AngleInTime last_observed = angle_in_time_rad[prev_buffer_position];

    return last_observed.angle_rad + avg_rot_speed * (timestamp - last_observed.timestamp);
}

void add_angle_to_buffer(float angle, uint64_t timestamp)
{
    
    int full_revolutions = (int)(angle_in_time_rad[buffer_position].angle_rad / (2 * M_PI));
    float angle_within_rev = fmod(angle_in_time_rad[buffer_position].angle_rad, 2 * M_PI);

    double angle_to_store = 0;

    if (avg_rot_speed > 0 && angle < angle_within_rev)
    {
        angle_to_store = (full_revolutions + 1) * (2 * M_PI) + angle;
    }

    else if (avg_rot_speed < 0 && angle > angle_within_rev)
    {
        angle_to_store = (full_revolutions - 1) * (2 * M_PI) + angle;
    }

    else
    {
        angle_to_store = (full_revolutions) * (2 * M_PI) + angle;
    }

    angle_in_time_rad[buffer_position] = {
        timestamp,
        angle_to_store,
    };

    update_average_rot_speed();

    if (buffer_position == ANGLE_BUFFER_SIZE - 1)
    {
        buffer_position = 0;
        prev_buffer_position = ANGLE_BUFFER_SIZE - 1;
    }
    else
    {
        buffer_position++;
        prev_buffer_position = buffer_position - 1;
    }
}