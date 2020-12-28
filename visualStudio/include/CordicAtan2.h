#pragma once

#include <utility>

const float K = 0.607253f;

typedef struct CordicLUT
{
    unsigned int precision;
    float lut[];
} cordicLut_t;

typedef union BrokenFloat
{
    float f;
    unsigned int i;
    struct
    {
        unsigned int frac : 23;
        unsigned int exp : 8;
        unsigned int sign : 1;
    };
} float32_t;

const cordicLut_t lut12 =
{
    12,
    {
        1.57079632679490f, 0.785398163397448f, 0.463647609000806f,
        0.244978663126864f, 0.124354994546761f, 0.0624188099959574f,
        0.0312398334302683f, 0.0156237286204768f, 0.00781234106010111f,
        0.00390623013196697f, 0.00195312251647882f, 0.000976562189559320f
    },
};

/**Computes the atan2 function using a cordic implementation.
*
* This is to test the affect of the cordic algorithm on the baseline apriltag
* algorithm.
*
* @param y: The y param.
* @param x: The x param.
* @param lut: The cordic lut to use with the algorithm.
* @return: The magnitude and theta values respectively.
 */
std::pair<float, float> cordicAtan2(float y, float x, cordicLut_t lut);
