#pragma once

#include "TagDetector.h"

class BaselineTagDetector : public TagDetector {
    public:
        static std::vector<AprilTag::TagDetection> extractTags(const cv::Mat& image, AprilTag::TagFamily tagFamily, DemoControls* controls);
};
