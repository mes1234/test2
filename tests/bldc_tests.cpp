#include <gtest/gtest.h>
#include <cmath>
#include "../lib/bldc_lib.h"

TEST(BldcLibSuit, TrigAdd)
{
    auto alfa = 2.0 * M_PI;
    auto adder = M_PI / 2.0;
    auto result = add_angles(alfa, adder);
    EXPECT_FLOAT_EQ(result, M_PI_2);
}

TEST(BldcLibSuit, AngularSpeed)
{
    int i = 0;
    for (i = 0; i < 10; i++)
    {
        add_angle_to_buffer(i * M_PI / 2, i);
    }

    auto result = estimate_angle(i-1);

    EXPECT_FLOAT_EQ((i-1) * M_PI / 2, result);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
