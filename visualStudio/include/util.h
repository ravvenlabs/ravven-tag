#pragma once

#include "timing.h"

#define DO_IF_DRAW_BEGIN if (controls->draw) {
#define DO_IF_DRAW_END }

#define DO_TIMING_IF_ENABLED(loops, message, ...) if (controls->timing) { \
    tic(); \
    for (unsigned long int timing_var_ ## __FILE__ ## _ ## __LINE__ = 0; \
        timing_var_ ## __FILE__ ## _ ## __LINE__ < loops; \
        timing_var_ ## __FILE__ ## _ ## __LINE__ ++) { \
        __VA_ARGS__ \
    } \
    toc(false, message); \
} \
else { \
    __VA_ARGS__ \
}
