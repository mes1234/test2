#include <cmath>
#include "../common/bldc_lib.h"

#define TWO_PI (2.0 * M_PI)

double add_angles(double alfa, double beta)
{
    float wrapped = fmodf(alfa + beta, TWO_PI);
    return wrapped < 0 ? wrapped + TWO_PI : wrapped;
}