#include "TagDetector.h"
#include "util.h"
#include "vs_apriltag.h"
#include "baseline_demo.h"

void showStep5(Step_5 step5, int rows, int cols)
{
    cv::Mat sizes(rows, cols, CV_16U);
    cv::Mat to_print(rows, cols, CV_32FC1);
    for (int y = 0; y < rows; y++)
    {
        for (int x = 0; x < cols; x++)
        {
            *(sizes.ptr<unsigned>(y, x)) = step5.uf.getSetSize(y * cols + x);
        }
    }
    cv::normalize(sizes, to_print, 0, 1, cv::NORM_MINMAX, CV_32FC1);
    imshow("Step 5", to_print);
}

void showStep6(Step_6 step6, int rows, int cols)
{
    cv::Mat to_print(rows, cols, CV_8UC3, cv::Vec3b(0, 0, 0));
    for (std::map<int, std::vector<AprilTag::XYWeight>>::iterator it = step6.clusters.begin(); it != step6.clusters.end(); it++)
    {
        cv::Vec3b color = cv::Vec3b((rand() % 190) + 50, (rand() % 190) + 50, (rand() % 190) + 50);
        for (std::vector<AprilTag::XYWeight>::iterator point = it->second.begin(); point != it->second.end(); point++)
        {
            *(to_print.ptr<cv::Vec3b>(point->y, point->x)) = color;
        }
    }
    imshow("Step 6", to_print);
}

void showStep7(Step_7 step7, Step_1 step1)
{
    cv::Mat baseImage(step1.fimOrig.rows, step1.fimOrig.cols, CV_8UC3);
    cv::cvtColor(step1.fimOrig, baseImage, cv::COLOR_GRAY2BGR, 3);
    for (std::vector<AprilTag::Segment>::iterator segment = step7.segments.begin(); segment != step7.segments.end(); segment++)
    {
        cv::Scalar color((rand() % 255) / 255., (rand() % 255) / 255., (rand() % 255) / 255.);
        cv::Point2d startPoint(segment->getX0(), segment->getY0());
        cv::Point2d endPoint(segment->getX1(), segment->getY1());
        cv::line(baseImage, startPoint, endPoint, color, 3);
    }
    imshow("Step 7", baseImage);
}

void showStep8(Step_7 step7, Step_1 step1)
{
    cv::Mat baseImage(step1.fimOrig.rows, step1.fimOrig.cols, CV_8UC3);
    cv::cvtColor(step1.fimOrig, baseImage, cv::COLOR_GRAY2BGR, 3);
    std::vector<int> segmentIdList(step7.segments.size());
    for (int i = 0; i < step7.segments.size(); i++)
        segmentIdList.push_back(i);
    int currSegId;
    int clusterCount = 0;
    while (segmentIdList.size() > 0)
    {
        cv::Scalar color((rand() % 255) / 255., (rand() % 255) / 255., (rand() % 255) / 255.);

        currSegId = segmentIdList.back();
        segmentIdList.pop_back();
        AprilTag::Segment segment = step7.segments[currSegId];
        for (std::vector<AprilTag::Segment*>::iterator segmentChild = segment.children.begin(); segmentChild != segment.children.end(); segmentChild++)
        {
            segmentIdList.erase(std::remove(segmentIdList.begin(), segmentIdList.end(), (*segmentChild)->getId()), segmentIdList.end());
            cv::Point2d startPoint((*segmentChild)->getX0(), (*segmentChild)->getY0());
            cv::Point2d endPoint((*segmentChild)->getX1(), (*segmentChild)->getY1());
            cv::line(baseImage, startPoint, endPoint, color, 3);
        }
        cv::Point2d startPoint(segment.getX0(), segment.getY0());
        cv::Point2d endPoint(segment.getX1(), segment.getY1());
        cv::line(baseImage, startPoint, endPoint, color, 3);
    }
    imshow("Step 8", baseImage);
}

void showStep9(Step_9 step9, Step_1 step1)
{
    cv::Mat baseImage(step1.fimOrig.rows, step1.fimOrig.cols, CV_8UC3);
    cv::cvtColor(step1.fimOrig, baseImage, cv::COLOR_GRAY2BGR, 3);
    for (std::vector<AprilTag::Quad>::iterator quad = step9.quads.begin(); quad != step9.quads.end(); quad++)
    {
        cv::Scalar color((rand() % 255) / 255., (rand() % 255) / 255., (rand() % 255) / 255.);
        cv::Rect2d rect(
            cv::Point2d(quad->quadPoints[0].first, quad->quadPoints[0].second),
            cv::Point2d(quad->quadPoints[2].first, quad->quadPoints[2].second)
        );
        cv::rectangle(baseImage, rect, color, 3);
    }
    imshow("Step 9", baseImage);
}

void showStep10(Step_10 step10, Step_1 step1)
{
    cv::Mat baseImage(step1.fimOrig.rows, step1.fimOrig.cols, CV_8UC3);
    cv::cvtColor(step1.fimOrig, baseImage, cv::COLOR_GRAY2BGR, 3);
    for (std::vector<AprilTag::TagDetection>::iterator detect = step10.detections.begin(); detect != step10.detections.end(); detect++)
    {
        detect->draw(baseImage);
    }
    imshow("Step 10", baseImage);
}

void showStep11(Step_11 step11, Step_1 step1)
{
    cv::Mat baseImage(step1.fimOrig.rows, step1.fimOrig.cols, CV_8UC3);
    cv::cvtColor(step1.fimOrig, baseImage, cv::COLOR_GRAY2BGR, 3);
    for (std::vector<AprilTag::TagDetection>::iterator detect = step11.goodDetections.begin(); detect != step11.goodDetections.end(); detect++)
    {
        detect->draw(baseImage);
    }
    imshow("Step 11", baseImage);
}

std::vector<AprilTag::TagDetection> extractTags(const cv::Mat& image, AprilTag::TagFamily tagFamily, DemoControls* controls) {
    DO_IF_DRAW_BEGIN
    srand(time(0));
    DO_IF_DRAW_END
    std::ofstream myfile;

    int width = image.cols;
    int height = image.rows;

    DO_IF_DRAW_BEGIN
    imshow("Pre Steps", image);
    DO_IF_DRAW_END

    Step_1 step1;
    DO_TIMING_IF_ENABLED(100, "Step 1 (x100)",
        step1 = createOriginalImage(image);)
    DO_IF_DRAW_BEGIN
    imshow("Step 1", step1.fimOrig);
    DO_IF_DRAW_END
    DO_IF_SAVE_BEGIN
    myfile.open(RELATIVE_IMG_OUTPUT_DIR "step1.txt");
    // Needs to be csv file as its the only format that works with 'load' in matlab.
    myfile << cv::format(step1.fimOrig, cv::Formatter::FMT_CSV);
    myfile.close();
    DO_IF_SAVE_END

    //! Gaussian smoothing kernel applied to image (0 == no filter).
    /*! Used when sampling bits. Filtering is a good idea in cases
        * where A) a cheap camera is introducing artifical sharpening, B)
        * the bayer pattern is creating artifcats, C) the sensor is very
        * noisy and/or has hot/cold pixels. However, filtering makes it
        * harder to decode very small tags. Reasonable values are 0, or
        * [0.8, 1.5].
        */
    float sigma = 0;

    Step_2 step2;
    DO_TIMING_IF_ENABLED(1000, "Step 2 (x1k)",
        step2 = optionallyApplyLowPassFilter(step1, sigma);)
    DO_IF_DRAW_BEGIN
    imshow("Step 2", step2.fim);
    DO_IF_DRAW_END
    DO_IF_SAVE_BEGIN
    myfile.open(RELATIVE_IMG_OUTPUT_DIR "step2.txt");
    myfile << cv::format(step2.fim, cv::Formatter::FMT_CSV);
    myfile.close();
    DO_IF_SAVE_END

    //! Gaussian smoothing kernel applied to image (0 == no filter).
    /*! Used when detecting the outline of the box. It is almost always
        * useful to have some filtering, since the loss of small details
        * won't hurt. Recommended value = 0.8. The case where sigma ==
        * segsigma has been optimized to avoid a redundant filter
        * operation.
        */
    float segSigma = 0.8f;

    Step_3 step3;
    DO_TIMING_IF_ENABLED(1000, "Step 3 (x1k)",
        step3 = applyLowPassFilterAndGrayScale(step1, step2, sigma, segSigma);)
    DO_IF_DRAW_BEGIN
    imshow("Step 3", step3.fimSeg);
    DO_IF_DRAW_END
    DO_IF_SAVE_BEGIN
    myfile.open(RELATIVE_IMG_OUTPUT_DIR "step3.txt");
    myfile << cv::format(step3.fimSeg, cv::Formatter::FMT_CSV);
    myfile.close();
    DO_IF_SAVE_END

    //================================================================
    // Step four: Compute the local gradient. We store the direction and magnitude.
    // This step is quite sensitve to noise, since a few bad theta estimates will
    // break up segments, causing us to miss Quads. It is useful to do a Gaussian
    // low pass on this step even if we don't want it for encoding.

    Step_4 step4;
    DO_TIMING_IF_ENABLED(100, "Step 4 (x100)",
        step4 = computeLocalGradients(step3);)
    DO_IF_DRAW_BEGIN
    imshow("Step 4a: Magnitutde", step4.fimMag);
    imshow("Step 4b: Theta", step4.fimTheta);
    DO_IF_DRAW_END
    DO_IF_SAVE_BEGIN
    myfile.open(RELATIVE_IMG_OUTPUT_DIR "step4-mag.txt");
    myfile << cv::format(step4.fimMag, cv::Formatter::FMT_CSV);
    myfile.close();
    myfile.open(RELATIVE_IMG_OUTPUT_DIR "step4-theta.txt");
    myfile << cv::format(step4.fimTheta, cv::Formatter::FMT_CSV);
    myfile.close();
    DO_IF_SAVE_END

    //================================================================
    // Step five. Extract edges by grouping pixels with similar
    // thetas together. This is a greedy algorithm: we start with
    // the most similar pixels.  We use 4-connectivity.

    Step_5 step5;
    DO_TIMING_IF_ENABLED(100, "Step 5 (x100)",
        step5 = extractEdges(step3, step4, width, height);)
    DO_IF_DRAW_BEGIN
    showStep5(step5, step3.fimSeg.rows, step3.fimSeg.cols);
    DO_IF_DRAW_END

    //================================================================
    // Step six: Loop over the pixels again, collecting statistics for each cluster.
    // We will soon fit lines (segments) to these points.

    Step_6 step6;
    DO_TIMING_IF_ENABLED(1000, "Step 6 (x1k)",
        step6 = createClusters(step3, step4, step5);)
    DO_IF_DRAW_BEGIN
    showStep6(step6, step3.fimSeg.rows, step3.fimSeg.cols);
    DO_IF_DRAW_END

    //================================================================
    // Step seven: Loop over the clusters, fitting lines (which we call Segments).

    Step_7 step7;
    DO_TIMING_IF_ENABLED(1000, "Step 7 (x1k)",
        step7 = fitSegments(step4, step6);)
    DO_IF_DRAW_BEGIN
    showStep7(step7, step1);
    DO_IF_DRAW_END

    // Step eight: For each segment, find segments that begin where this segment ends.
    // (We will chain segments together next...) The gridder accelerates the search by
    // building (essentially) a 2D hash table.

    Step_7 step7_new;
    Step_8 step8;
    DO_TIMING_IF_ENABLED(1000, "Step 8 (x1k)",
        step7_new = step7;
        step8 = connectSegments(&step7_new, width, height);)
    DO_IF_DRAW_BEGIN
    showStep8(step7_new, step1);
    DO_IF_DRAW_END

    //================================================================
    // Step nine: Search all connected segments to see if any form a loop of length 4.
    // Add those to the quads list.

    Step_9 step9;
    DO_TIMING_IF_ENABLED(1000, "Step 9 (x1k)",
        step9 = createQuads(step1, &step7_new);)
    DO_IF_DRAW_BEGIN
    showStep9(step9, step1);
    DO_IF_DRAW_END

    //================================================================
    // Step ten. Decode the quads. For each quad, we first estimate a
    // threshold color to decide between 0 and 1. Then, we read off the
    // bits and see if they make sense.

    Step_10 step10;
    DO_TIMING_IF_ENABLED(1000, "Step 10 (x1k)",
        step10 = decodeQuads(step2, step9, width, height, tagFamily);)
    DO_IF_DRAW_BEGIN
    showStep10(step10, step1);
    DO_IF_DRAW_END

    //================================================================
    //Step eleven: Some quads may be detected more than once, due to
    //partial occlusion and our aggressive attempts to recover from
    //broken lines. When two quads (with the same id) overlap, we will
    //keep the one with the lowest error, and if the error is the same,
    //the one with the greatest observed perimeter.

    Step_11 step11;
    DO_TIMING_IF_ENABLED(1000, "Step 11 (x1k)",
        step11 = removeDuplicates(step10);)
    DO_IF_DRAW_BEGIN
    showStep11(step11, step1);
    DO_IF_DRAW_END

    return step11.goodDetections;
}
