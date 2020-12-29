#include "CordicAtan2.h"

int zsign(float var)
{
    if (var < 0)
        return -1;
    else
        return 1;
}

float bitsra(float var, unsigned int shift)
{
    if (var == 0.0f) return 0.0f;
    float32_t var_broke = { var };
    var_broke.exp -= shift;
    return var_broke.f;
}

std::pair<float, float> cordicAtan2(float y, float x, cordicLut_t lut)
{
    if (y == 0.0f && x == 0.0f)
    {
        return std::make_pair(0.0f, 0.0f);
    }
    else if (x == 0.0f)
    {
        return std::make_pair(abs(y), zsign(y) * 1.570796326795f);
    }
    // Return Variables
    float mag = 0;
    float angle = 0;
    // Intermediate Variables
    float xo = 0;
    float yo = 0;
    float zo = 0;
    float z = 0;
    // Calculate first and second run of the algorithm
    xo = y * zsign(y);
    yo = -1 * x * zsign(y);
    zo = -1 * zsign(y) * lut.lut[0];
    z = zo + lut.lut[1] * -1 * zsign(yo);
    zo = z;
    // Start iterations
    for (unsigned int iter = 1; iter < lut.precision; iter++)
    {
        // Compute an iteration
        x = xo - bitsra(yo, (iter - 1)) * -1 * zsign(yo);
        y = yo + bitsra(xo, (iter - 1)) * -1 * zsign(yo);
        z = zo + -1 * zsign(y) * lut.lut[1 + iter];
        // Save into intermediates
        zo = z;
        yo = y;
        xo = x;
    }
    mag = x * K;
    angle = -1 * z;
    return std::make_pair(mag, angle);
}
