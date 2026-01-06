#include <gtest/gtest.h>
#include <cmath>
#include "../common/bldc_lib.h"

TEST(BldcLibSuit, TrigAdd)
{
    auto alfa = 2.0 * M_PI;
    auto adder = M_PI / 2.0;
    auto result = add_angles(alfa, adder);
    EXPECT_FLOAT_EQ(result, M_PI_2);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
