#ifndef TAGDETECTOR_H
#define TAGDETECTOR_H

#include <vector>

#include "opencv2/opencv.hpp"

#include "TagDetection.h"
#include "TagFamily.h"
#include "FloatImage.h"

class TagDetector {
    public:

        const AprilTag::TagFamily thisTagFamily;

        //! Constructor
        // note: TagFamily is instantiated here from TagCodes
        TagDetector(const AprilTag::TagCodes& tagCodes) : thisTagFamily(tagCodes) {}

        virtual std::vector<AprilTag::TagDetection> extractTags(const cv::Mat& image) = 0;
};

#endif
