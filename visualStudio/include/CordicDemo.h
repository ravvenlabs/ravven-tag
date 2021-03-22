#pragma once

#include "Demo.h"

/**
 * The cordic demo has no special functionality compared to the baseline.
 */
class CordicDemo : public Demo {
    public:
        CordicDemo(const char* name, const char* inputDir, DemoControls* ctrl) : Demo(name, inputDir, ctrl) {}

        void setup();

        void execute();
    private:
        void loadImages();

        void processImage(cv::Mat& image, cv::Mat& image_gray);
};
