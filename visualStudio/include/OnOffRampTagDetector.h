#pragma once

#include "TagDetector.h"
#include "OnOffRampDemoControls.h"

class OnOffRampTagDetector : public TagDetector {
    public:
        typedef struct
        {
            cv::Mat& baseImage;
            cv::Mat& image1;
            cv::Mat& image2;
        } ExtractionsInput;

        static std::vector<AprilTag::TagDetection> extractTags(ExtractionsInput input, AprilTag::TagFamily tagFamily, OnOffRampDemoControls* controls);
};
