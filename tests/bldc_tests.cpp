#include <gtest/gtest.h>
#include <cmath>
#include "../lib/bldc_lib.h"

/// @brief Check if buffer is init correctly
TEST(BldcLibSuit, Buffer_Init_Correctness)
{

    // Arrange
    AngleBuffer buffer = {0};

    auto angle = 5 * M_PI_2; // It is actually absoulte PI/2

    // Act
    // Timestamp should be at least ANGLE_BUFFER_SIZE
    add_angle_to_buffer(&buffer, angle, ANGLE_BUFFER_SIZE);

    // Assert
    auto result = get_current_buffer_value(&buffer);
    EXPECT_FLOAT_EQ(result, M_PI_2);
}

/// @brief Check if buffer is filed correctly
TEST(BldcLibSuit, Buffer_Filling_Correctness)
{

    // Arrange
    AngleBuffer buffer = {0};

    auto angle = 5 * M_PI_2; // It is actually absoulte PI/2

    // Act
    // Timestamp should be at least ANGLE_BUFFER_SIZE
    add_angle_to_buffer(&buffer, angle, ANGLE_BUFFER_SIZE);

    auto init_angle = get_current_buffer_value(&buffer);

    int i = 0;
    for (i = 0; i < 100; i++)
    {
        auto angle = fmod(i * M_PI_2 + init_angle, 2 * M_PI);
        add_angle_to_buffer(&buffer, angle, i + ANGLE_BUFFER_SIZE);
    }

    // Assert
    auto result = get_current_buffer_value(&buffer);
    EXPECT_FLOAT_EQ(result, M_PI_2);
}

/// @brief  Check if ratio 0.0 gives uniform mask distribution
TEST(BldcLibSuit, Mask_Uniform_Test)
{
    // Arrange
    AngleBuffer buffer = {0};

    // Act
    init_buffer_mask(&buffer, 0.0);

    // Assert

    float buffer_sum = 0.0;

    for (int i = 0; i < ANGLE_BUFFER_SIZE - 1; i++)
    {
        EXPECT_FLOAT_EQ(buffer.buffer_mask[i], 1.0 / (ANGLE_BUFFER_SIZE - 1));

        buffer_sum = buffer_sum + buffer.buffer_mask[i];
    }

    EXPECT_FLOAT_EQ(1.0, buffer_sum);
}

/// @brief  Check if ratio 0.4 gives mask sum = 1.0
TEST(BldcLibSuit, Mask_Non_Uniform_Sum_Test)
{
    // Arrange
    AngleBuffer buffer = {0};

    // Act
    init_buffer_mask(&buffer, 0.4);

    // Assert

    float buffer_sum = 0.0;

    for (int i = 0; i < ANGLE_BUFFER_SIZE - 1; i++)
    {
        buffer_sum = buffer_sum + buffer.buffer_mask[i];
    }

    EXPECT_FLOAT_EQ(1.0, buffer_sum);
}

// /// @brief Test storing a buffer
// TEST(BldcLibSuit, Angular_Speed)
// {
//     int i = 0;
//     for (i = 0; i < 100; i++)
//     {
//         auto angle = fmod(i * M_PI_2, 2 * M_PI);
//         add_angle_to_buffer(angle, i);
//     }

//     auto result = estimate_angle(i - 1);
//     auto expected = fmod((i - 1) * M_PI_2, 2 * M_PI);

//     EXPECT_FLOAT_EQ(expected, result);
// }

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
