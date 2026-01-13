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

    // Act
    // Timestamp should be at least ANGLE_BUFFER_SIZE
    add_angle_to_buffer(&buffer, M_PI_2, ANGLE_BUFFER_SIZE);         //  90deg Rev 0 <- init
    add_angle_to_buffer(&buffer, M_PI_2 * 2, ANGLE_BUFFER_SIZE + 1); // 180deg Rev 0
    add_angle_to_buffer(&buffer, M_PI_2 * 3, ANGLE_BUFFER_SIZE + 2); // 270deg Rev 0
    add_angle_to_buffer(&buffer, M_PI_2 * 4, ANGLE_BUFFER_SIZE + 3); // 360deg Rev 1
    add_angle_to_buffer(&buffer, M_PI_2, ANGLE_BUFFER_SIZE + 4);     // 90deg Rev 1 -> absolute 450deg

    // Assert
    auto result = get_current_buffer_value(&buffer);
    auto expected = to_radians(450.0);
    EXPECT_FLOAT_EQ(result, expected);
}

/// @brief Check if angle tracking is getting back to 0
TEST(BldcLibSuit, Angle_tracking_back_to_zero)
{
    // Arrange
    AngleBuffer buffer = {0};

    // Act
    // Timestamp should be at least ANGLE_BUFFER_SIZE
    add_angle_to_buffer(&buffer, M_PI_2, ANGLE_BUFFER_SIZE);         //  90deg Rev 0 <- init
    add_angle_to_buffer(&buffer, M_PI_2 * 2, ANGLE_BUFFER_SIZE + 1); // 180deg Rev 0
    add_angle_to_buffer(&buffer, M_PI_2 * 3, ANGLE_BUFFER_SIZE + 2); // 270deg Rev 0
    add_angle_to_buffer(&buffer, M_PI_2 * 4, ANGLE_BUFFER_SIZE + 3); // 360deg Rev 1

    add_angle_to_buffer(&buffer, M_PI_2 * 3, ANGLE_BUFFER_SIZE + 4); // 270deg Rev 0
    add_angle_to_buffer(&buffer, M_PI_2 * 2, ANGLE_BUFFER_SIZE + 5); // 180deg Rev 0
    add_angle_to_buffer(&buffer, M_PI_2 * 1, ANGLE_BUFFER_SIZE + 6); // 180deg Rev 0
    add_angle_to_buffer(&buffer, M_PI_2 * 0, ANGLE_BUFFER_SIZE + 7); // 180deg Rev 0

    // Assert
    auto result = get_current_buffer_value(&buffer);
    EXPECT_FLOAT_EQ(result, 0.0);
}
/// TODO
/// WRAP AROUND 0deg + rot - rot -rot +rot -> 0deg

/// @brief Check if angle estimate is correct
TEST(BldcLibSuit, Constant_speed)
{
    // Arrange
    AngleBuffer buffer = {0};

    // Act
    // Timestamp should be at least ANGLE_BUFFER_SIZE
    add_angle_to_buffer(&buffer, M_PI_2, ANGLE_BUFFER_SIZE);         //  90deg Rev 0 <- init
    add_angle_to_buffer(&buffer, M_PI_2 * 2, ANGLE_BUFFER_SIZE);     // 180deg Rev 0
    add_angle_to_buffer(&buffer, M_PI_2 * 3, ANGLE_BUFFER_SIZE + 1); // 270deg Rev 0
    add_angle_to_buffer(&buffer, M_PI_2 * 4, ANGLE_BUFFER_SIZE + 2); // 360deg Rev 1
    add_angle_to_buffer(&buffer, M_PI_2, ANGLE_BUFFER_SIZE + 3);     // 90deg Rev 1 -> absolute 450deg
    add_angle_to_buffer(&buffer, M_PI_2 * 2, ANGLE_BUFFER_SIZE + 4); // 180deg Rev 1 -> absolute 540deg
    add_angle_to_buffer(&buffer, M_PI_2 * 3, ANGLE_BUFFER_SIZE + 5); // 270deg Rev 1 -> absolute 630deg
    add_angle_to_buffer(&buffer, M_PI_2 * 4, ANGLE_BUFFER_SIZE + 6); // 360deg Rev 2 -> absolute 720deg
    add_angle_to_buffer(&buffer, M_PI_2, ANGLE_BUFFER_SIZE + 7);     // 90deg Rev 2 -> absolute 810deg
    add_angle_to_buffer(&buffer, M_PI_2 * 2, ANGLE_BUFFER_SIZE + 8); // 180deg Rev 2-> absolute 900deg
    add_angle_to_buffer(&buffer, M_PI_2 * 3, ANGLE_BUFFER_SIZE + 9); // 270deg Rev 2-> absolute 990deg

    // Assert
    auto result = get_current_buffer_value(&buffer);
    auto expected = to_radians(990.0);
    EXPECT_FLOAT_EQ(result, expected);

    auto offset = 3;

    auto estimated_angle = estimate_angle(&buffer, ANGLE_BUFFER_SIZE + 9 + offset);
    auto expected_angle = to_radians(990.0 + 3.0 * 90.0);

    EXPECT_FLOAT_EQ(estimated_angle, expected_angle);
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

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
