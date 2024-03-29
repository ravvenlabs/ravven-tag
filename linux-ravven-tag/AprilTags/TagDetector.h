#ifndef TAGDETECTOR_H
#define TAGDETECTOR_H

#include <vector>

#include "opencv2/opencv.hpp"

#include "AprilTags/Segment.h"
#include "AprilTags/TagDetection.h"
#include "AprilTags/TagFamily.h"

namespace AprilTag {

    class TagDetector {
        public:
            const TagFamily thisTagFamily;

            //! Constructor
            // note: TagFamily is instantiated here from TagCodes
            TagDetector(const TagCodes& tagCodes) : thisTagFamily(tagCodes) {}
            
            std::vector<TagDetection> detect(const cv::Mat& image);
            std::vector<TagDetection> detect(const cv::Mat& fimOrig, const cv::Mat& fimMag, const cv::Mat& fimTheta);

            template<class T>
            std::vector<TagDetection> detect(const cv::Mat& fimOrig, std::vector<AprilTag::Segment> segments);
    };
} // namespace

#endif
