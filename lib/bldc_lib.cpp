#include <cmath>
#include "bldc_lib.h"
#include <cstdint>

void init_buffer_mask(AngleBuffer *buffer, float ratio)
{
    float sum = 0.0;
    for (int i = 0; i < ANGLE_BUFFER_SIZE - 1; i++)
    {
        buffer->buffer_mask[i] = exp((float)(i + 1) * ratio);
        sum = sum + buffer->buffer_mask[i];
    }

    for (int i = 0; i < ANGLE_BUFFER_SIZE - 1; i++)
    {
        buffer->buffer_mask[i] = buffer->buffer_mask[i] / sum;
    }
}

void update_average_rot_speed(AngleBuffer *buffer)
{
    int i = 0;
    int counter = 0;
    float speed = 0.0;

    // Clear speed
    buffer->avg_rot_speed = 0.0;

    for (i = 0; i <= ANGLE_BUFFER_SIZE - 2; i++)
    {
        int item_position = (i + 1 + buffer->buffer_position) % ANGLE_BUFFER_SIZE;
        int next_item_position = (i + 2 + buffer->buffer_position) % ANGLE_BUFFER_SIZE;

        int delta_t = buffer->buffer[next_item_position].timestamp - buffer->buffer[item_position].timestamp;

        if (delta_t != 0)
        {
            speed = (buffer->buffer[next_item_position].angle_rad - buffer->buffer[item_position].angle_rad) / (float)delta_t;

            buffer->avg_rot_speed = buffer->avg_rot_speed + speed * buffer->buffer_mask[i];
        }
    }
}

float estimate_angle(AngleBuffer *buffer, uint64_t timestamp)
{
    AngleInTime last_observed = buffer->buffer[buffer->buffer_position];

    auto dt = (timestamp - last_observed.timestamp);

    auto d_angle = buffer->avg_rot_speed * (timestamp - last_observed.timestamp);

    auto result = last_observed.angle_rad + d_angle;
    return result;
}

float get_current_buffer_value(AngleBuffer *buffer)
{
    return buffer->buffer[buffer->buffer_position].angle_rad;
}

void add_angle_to_buffer(AngleBuffer *buffer, float angle, uint64_t timestamp)
{
    auto angle_wraped = fmod(angle, TWO_PI);

    int i = 0;

    // Handle init state
    if (!buffer->buffer_initialized)
    {
        for (i = 0; i < ANGLE_BUFFER_SIZE; i++)
        {
            buffer->buffer[i] = {
                timestamp - (ANGLE_BUFFER_SIZE - i),
                angle_wraped,
            };
        }
        buffer->buffer_position = ANGLE_BUFFER_SIZE - 1;

        buffer->buffer_initialized = true;

        init_buffer_mask(buffer, buffer->buffer_weight_ratio);

        return;
    }

    // Estimate previous item revolution and angle
    int prev_full_revolutions = (int)(buffer->buffer[buffer->buffer_position].angle_rad / (2 * M_PI));
    float prev_angle_within_rev = fmod(buffer->buffer[buffer->buffer_position].angle_rad, 2 * M_PI);

    // Set next position //

    // Estimate next position, ensure buffer circular
    buffer->buffer_position = (buffer->buffer_position + 1) % ANGLE_BUFFER_SIZE;

    double angle_to_store = 0;

    if (buffer->avg_rot_speed > 0 && angle < prev_angle_within_rev)
    {
        angle_to_store = (prev_full_revolutions + 1) * (2 * M_PI) + angle;
    }
    else if (buffer->avg_rot_speed < 0 && angle > prev_angle_within_rev)
    {
        angle_to_store = (prev_full_revolutions - 1) * (2 * M_PI) + angle;
    }
    else
    {
        angle_to_store = (prev_full_revolutions) * (2 * M_PI) + angle;
    }
    buffer->buffer[buffer->buffer_position] = {
        timestamp,
        angle_to_store,
    };

    update_average_rot_speed(buffer);
}