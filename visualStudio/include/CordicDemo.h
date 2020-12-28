#pragma once

#include "baseline_demo.h"

/**
 * The cordic demo has no special functionality compared to the baseline.
 */
class CordicDemo : public BaselineDemo {
    public:
        CordicDemo(const char* name, const char* inputDir, DemoControls* ctrl) : BaselineDemo(name, inputDir, ctrl) {}
};
