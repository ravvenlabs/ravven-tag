#pragma once

#include "DemoControls.h"

class OnOffRampDemoControls : public DemoControls {
    public:
        float onRamp; // When to start running the algorithm
        float offRamp; // When to stop running the algorithm

        OnOffRampDemoControls(const DemoControls* ctrl)
        {
            timing = ctrl->timing;
            draw = ctrl->draw;
            save = ctrl->save;
            onRamp = 0;
            offRamp = 0;
        }
};
