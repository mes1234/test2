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
