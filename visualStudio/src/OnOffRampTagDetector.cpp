#include "util.h"
#include "vs_apriltag.h"
#include "OnOffRampTagDetector.h"
#include "TagDisplay.h"
#include <filesystem>

#define ON_OFF_OUTPUT_DIR RELATIVE_IMG_OUTPUT_DIR "on_off_ramps\\"
#define END_IF_OFF_RAMP(step) if (controls->offRamp <= step) return {};

typedef struct
{
    cv::Mat iy;
    cv::Mat ix;
} Step_3_5;

TagDetector::Step_4 computeLocalGradientsWithHalf(TagDetector::Step_3 step3, Step_3_5 step3_5)
{
    cv::Mat fimTheta(step3.fimSeg.rows, step3.fimSeg.cols, CV_32FC1);
    cv::Mat fimMag(step3.fimSeg.rows, step3.fimSeg.cols, CV_32FC1);

#pragma omp parallel for
    for (int y = 3; y < step3.fimSeg.rows - 3; y++) {
        for (int x = 3; x < step3.fimSeg.cols - 3; x++) {
            float Ix = step3_5.ix.at<float>(y, x);
            float Iy = step3_5.iy.at<float>(y, x);

            float mag = Ix * Ix + Iy * Iy;
            float theta = atan2(Iy, Ix);

            fimTheta.at<float>(y, x) = theta;
            fimMag.at<float>(y, x) = mag;
        }
    }

    return { fimTheta, fimMag };
}

std::vector<AprilTag::TagDetection> OnOffRampTagDetector::extractTags(ExtractionsInput input, AprilTag::TagFamily tagFamily, OnOffRampDemoControls * controls)
{
    DO_IF_DRAW_BEGIN
    srand(time(0));
    DO_IF_DRAW_END
    DO_IF_SAVE_BEGIN
    std::filesystem::create_directories(ON_OFF_OUTPUT_DIR);
    DO_IF_SAVE_END
    std::ofstream myfile;

    int width = input.baseImage.cols;
    int height = input.baseImage.rows;

    DO_IF_DRAW_BEGIN
    imshow("Pre Steps", input.baseImage);
    DO_IF_DRAW_END

    TagDetector::Step_1 step1;
    if (controls->onRamp != 1)
    {
        DO_TIMING_IF_ENABLED(100, "Step 1 (x100)",
        step1 = TagDetector::createOriginalImage(input.baseImage);)
        DO_IF_DRAW_BEGIN
        imshow("Step 1", step1.fimOrig);
        DO_IF_DRAW_END
        DO_IF_SAVE_BEGIN
        myfile.open(ON_OFF_OUTPUT_DIR "step1.txt");
        // Needs to be csv file as its the only format that works with 'load' in matlab.
        myfile << cv::format(step1.fimOrig, cv::Formatter::FMT_CSV);
        myfile.close();
        DO_IF_SAVE_END
    }
    else
    {
        input.image1.convertTo(step1.fimOrig, CV_32FC1, (1. / 255.));
        step1.opticalCenter = { input.image1.cols / 2, input.image1.rows / 2 };
    }

    //! Gaussian smoothing kernel applied to image (0 == no filter).
    /*! Used when sampling bits. Filtering is a good idea in cases
        * where A) a cheap camera is introducing artifical sharpening, B)
        * the bayer pattern is creating artifcats, C) the sensor is very
        * noisy and/or has hot/cold pixels. However, filtering makes it
        * harder to decode very small tags. Reasonable values are 0, or
        * [0.8, 1.5].
        */
    float sigma = 0;

    TagDetector::Step_2 step2;
    if (controls->onRamp != 2)
    {
        DO_TIMING_IF_ENABLED(1000, "Step 2 (x1k)",
        step2 = TagDetector::optionallyApplyLowPassFilter(step1, sigma);)
        DO_IF_DRAW_BEGIN
        imshow("Step 2", step2.fim);
        DO_IF_DRAW_END
        DO_IF_SAVE_BEGIN
        myfile.open(ON_OFF_OUTPUT_DIR "step2.txt");
        myfile << cv::format(step2.fim, cv::Formatter::FMT_CSV);
        myfile.close();
        DO_IF_SAVE_END
    }
    else
    {
        input.image1.convertTo(step2.fim, CV_32FC1, (1. / 255.));
    }

    END_IF_OFF_RAMP(2);

    //! Gaussian smoothing kernel applied to image (0 == no filter).
    /*! Used when detecting the outline of the box. It is almost always
        * useful to have some filtering, since the loss of small details
        * won't hurt. Recommended value = 0.8. The case where sigma ==
        * segsigma has been optimized to avoid a redundant filter
        * operation.
        */
    float segSigma = 0.8f;

    TagDetector::Step_3 step3;

    if (controls->onRamp != 3)
    {
        DO_TIMING_IF_ENABLED(1000, "Step 3 (x1k)",
            step3 = TagDetector::applyLowPassFilterAndGrayScale(step1, step2, sigma, segSigma);)
        DO_IF_DRAW_BEGIN
        imshow("Step 3", step3.fimSeg);
        DO_IF_DRAW_END
        DO_IF_SAVE_BEGIN
        myfile.open(ON_OFF_OUTPUT_DIR "step3.txt");
        myfile << cv::format(step3.fimSeg, cv::Formatter::FMT_CSV);
        myfile.close();
        DO_IF_SAVE_END
    }
    else
    {
        input.image1.convertTo(step3.fimSeg, CV_32FC1, (1. / 255.));
    }

    END_IF_OFF_RAMP(3);

    //================================================================
    // Step four: Compute the local gradient. We store the direction and magnitude.
    // This step is quite sensitve to noise, since a few bad theta estimates will
    // break up segments, causing us to miss Quads. It is useful to do a Gaussian
    // low pass on this step even if we don't want it for encoding.

    TagDetector::Step_4 step4;
    if (controls->onRamp == 4.5)
    {
        Step_3_5 step3_5{ input.image2, input.image1 };
        //input.image1.convertTo(step3_5.ix, CV_32FC1, (1. / 255.));
        //input.image2.convertTo(step3_5.iy, CV_32FC1, (1. / 255.));
        DO_TIMING_IF_ENABLED(100, "Step 4 (x100)",
            step4 = computeLocalGradientsWithHalf(step3, step3_5);)
        DO_IF_DRAW_BEGIN
        imshow("Step 4.5a: Ix", step3_5.ix);
        imshow("Step 4.5b: Iy", step3_5.iy);
        imshow("Step 4a: Magnitutde", step4.fimMag);
        imshow("Step 4b: Theta", step4.fimTheta);
        DO_IF_DRAW_END
        DO_IF_SAVE_BEGIN
        myfile.open(ON_OFF_OUTPUT_DIR "step4-mag.txt");
        myfile << cv::format(step4.fimMag, cv::Formatter::FMT_CSV);
        myfile.close();
        myfile.open(ON_OFF_OUTPUT_DIR "step4-theta.txt");
        myfile << cv::format(step4.fimTheta, cv::Formatter::FMT_CSV);
        myfile.close();
        DO_IF_SAVE_END
    }
    else if (controls->onRamp != 4)
    {
        DO_TIMING_IF_ENABLED(100, "Step 4 (x100)",
            step4 = TagDetector::computeLocalGradients(step3);)
        DO_IF_DRAW_BEGIN
        imshow("Step 4a: Magnitutde", step4.fimMag);
        imshow("Step 4b: Theta", step4.fimTheta);
        DO_IF_DRAW_END
        DO_IF_SAVE_BEGIN
        myfile.open(ON_OFF_OUTPUT_DIR "step4-mag.txt");
        myfile << cv::format(step4.fimMag, cv::Formatter::FMT_CSV);
        myfile.close();
        myfile.open(ON_OFF_OUTPUT_DIR "step4-theta.txt");
        myfile << cv::format(step4.fimTheta, cv::Formatter::FMT_CSV);
        myfile.close();
        DO_IF_SAVE_END
    }
    else
    {
        cv::Mat temp(step3.fimSeg.rows, step3.fimSeg.cols, CV_32FC1);
        input.image1.convertTo(step4.fimMag, CV_32FC1, (1. / 255.));
        input.image2.convertTo(temp, CV_32FC1, ((2. * 3.14159265358979323846) / 255.));
        step4.fimTheta = temp - 3.14159265358979323846;
        imshow("Step 4a: Magnitutde", step4.fimMag);
        imshow("Step 4b: Theta", step4.fimTheta);
    }

    END_IF_OFF_RAMP(4);

    //================================================================
    // Step five. Extract edges by grouping pixels with similar
    // thetas together. This is a greedy algorithm: we start with
    // the most similar pixels.  We use 4-connectivity.

    TagDetector::Step_5 step5;
    DO_TIMING_IF_ENABLED(100, "Step 5 (x100)",
        step5 = TagDetector::extractEdges(step3, step4, width, height);)
    DO_IF_DRAW_BEGIN
    showStep5(step5, step3.fimSeg.rows, step3.fimSeg.cols);
    DO_IF_DRAW_END

    //================================================================
    // Step six: Loop over the pixels again, collecting statistics for each cluster.
    // We will soon fit lines (segments) to these points.

    TagDetector::Step_6 step6;
    DO_TIMING_IF_ENABLED(1000, "Step 6 (x1k)",
        step6 = TagDetector::createClusters(step3, step4, step5);)
    DO_IF_DRAW_BEGIN
    showStep6(step6, step3.fimSeg.rows, step3.fimSeg.cols);
    DO_IF_DRAW_END

    //================================================================
    // Step seven: Loop over the clusters, fitting lines (which we call Segments).

    TagDetector::Step_7 step7;
    DO_TIMING_IF_ENABLED(1000, "Step 7 (x1k)",
        step7 = TagDetector::fitSegments(step4, step6);)
    DO_IF_DRAW_BEGIN
    showStep7(step7, step1);
    DO_IF_DRAW_END

    // Step eight: For each segment, find segments that begin where this segment ends.
    // (We will chain segments together next...) The gridder accelerates the search by
    // building (essentially) a 2D hash table.

    TagDetector::Step_7 step7_new;
    TagDetector::Step_8 step8;
    DO_TIMING_IF_ENABLED(1000, "Step 8 (x1k)",
        step7_new = step7;
        step8 = TagDetector::connectSegments(&step7_new, width, height);)
    DO_IF_DRAW_BEGIN
    showStep8(step7_new, step1);
    DO_IF_DRAW_END

    //================================================================
    // Step nine: Search all connected segments to see if any form a loop of length 4.
    // Add those to the quads list.

    TagDetector::Step_9 step9;
    DO_TIMING_IF_ENABLED(1000, "Step 9 (x1k)",
        step9 = TagDetector::createQuads(step1, &step7_new);)
    DO_IF_DRAW_BEGIN
    showStep9(step9, step1);
    DO_IF_DRAW_END

    //================================================================
    // Step ten. Decode the quads. For each quad, we first estimate a
    // threshold color to decide between 0 and 1. Then, we read off the
    // bits and see if they make sense.

    TagDetector::Step_10 step10;
    DO_TIMING_IF_ENABLED(1000, "Step 10 (x1k)",
        step10 = TagDetector::decodeQuads(step2, step9, width, height, tagFamily);)
    DO_IF_DRAW_BEGIN
    showStep10(step10, step1);
    DO_IF_DRAW_END

    //================================================================
    //Step eleven: Some quads may be detected more than once, due to
    //partial occlusion and our aggressive attempts to recover from
    //broken lines. When two quads (with the same id) overlap, we will
    //keep the one with the lowest error, and if the error is the same,
    //the one with the greatest observed perimeter.

    TagDetector::Step_11 step11;
    DO_TIMING_IF_ENABLED(1000, "Step 11 (x1k)",
        step11 = TagDetector::removeDuplicates(step10);)
    DO_IF_DRAW_BEGIN
    showStep11(step11, step1);
    DO_IF_DRAW_END

    return step11.goodDetections;
}
