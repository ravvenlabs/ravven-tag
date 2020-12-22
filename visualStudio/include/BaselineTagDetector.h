#pragma once

#include "TagDetector.h"

std::vector<AprilTag::TagDetection> extractTags(const cv::Mat& image, AprilTag::TagFamily tagFamily);
