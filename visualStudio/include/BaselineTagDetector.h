#pragma once

#include "TagDetector.h"
#include "DemoControls.h"

std::vector<AprilTag::TagDetection> extractTags(const cv::Mat& image, AprilTag::TagFamily tagFamily, DemoControls* controls);
