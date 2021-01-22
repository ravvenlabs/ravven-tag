#pragma once

#include "TagDetector.h"

class CordicTagDetector : public TagDetector {
    private:
        /**
        * Output from step 4.
        *
        * This was expanded to output the iX and iY differences if so desired.
        *
        * \var fimTheta The image's directional vector matrix.
        * \var fimMag The image's magnitude vector matrix.
        * \var fimIX The image's iX differences.
        * \var fimIY the image's iY differences.
        */
        typedef struct
        {
            cv::Mat fimTheta;
            cv::Mat fimMag;
            cv::Mat fimIX;
            cv::Mat fimIY;
        } Cordic_Step_4;

    public:
        static Cordic_Step_4 computeLocalGradients(Step_3 step3);

        static std::vector<AprilTag::TagDetection> extractTags(const cv::Mat& image, AprilTag::TagFamily tagFamily, DemoControls* controls);
};
