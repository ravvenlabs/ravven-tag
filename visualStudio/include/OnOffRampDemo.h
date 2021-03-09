#pragma once

#include "Demo.h"
#include "OnOffRampDemoControls.h"
#include "OnOffRampTagDetector.h"

/**
 * The On-Off ramp demo will allow the user to programmically tell when the algorithm will start and stop. This includes input images.
 */
class OnOffRampDemo : public Demo {
    public:
        OnOffRampDemoControls* controls;

    public:
        OnOffRampDemo(const char* name, const char* inputDir, DemoControls* ctrl) : Demo(name, inputDir, ctrl)
        {
            if (imgNames.size() > 2)
            {
                std::cout << "Only 1 or 2 (for mag/theta) images are supported." << std::endl;
                exit(1);
            }

            controls = new OnOffRampDemoControls(ctrl);
        }

        void setup();

        void execute();

    private:
        void loadImages();

        void processImage(OnOffRampTagDetector::ExtractionsInput input);
};
