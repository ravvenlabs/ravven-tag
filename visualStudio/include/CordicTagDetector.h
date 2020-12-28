#pragma once

#include "TagDetector.h"

class CordicTagDetector : public TagDetector {
    public:
        static Step_4 computeLocalGradients(Step_3 step3);

        static std::vector<AprilTag::TagDetection> extractTags(const cv::Mat& image, AprilTag::TagFamily tagFamily, DemoControls* controls);
};
