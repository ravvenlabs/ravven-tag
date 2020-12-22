#include "BaselineTagDetector.h"

std::vector<AprilTag::TagDetection> extractTags(const cv::Mat& image, AprilTag::TagFamily tagFamily) {
    //srand(time(0));
    int width = image.cols;
    int height = image.rows;

    //imshow("Pre Steps", image);

    Step_1 step1 = createOriginalImage(image);
    //imshow("Step 1", step1.fimOrig);

    //! Gaussian smoothing kernel applied to image (0 == no filter).
    /*! Used when sampling bits. Filtering is a good idea in cases
        * where A) a cheap camera is introducing artifical sharpening, B)
        * the bayer pattern is creating artifcats, C) the sensor is very
        * noisy and/or has hot/cold pixels. However, filtering makes it
        * harder to decode very small tags. Reasonable values are 0, or
        * [0.8, 1.5].
        */
    float sigma = 0;

    Step_2 step2 = optionallyApplyLowPassFilter(step1, sigma);
    //imshow("Step 2", step2.fim);

    //! Gaussian smoothing kernel applied to image (0 == no filter).
    /*! Used when detecting the outline of the box. It is almost always
        * useful to have some filtering, since the loss of small details
        * won't hurt. Recommended value = 0.8. The case where sigma ==
        * segsigma has been optimized to avoid a redundant filter
        * operation.
        */
    float segSigma = 0.8f;

    Step_3 step3 = applyLowPassFilterAndGrayScale(step1, step2, sigma, segSigma);
    //imshow("Step 3", step3.fimSeg);

    //================================================================
    // Step four: Compute the local gradient. We store the direction and magnitude.
    // This step is quite sensitve to noise, since a few bad theta estimates will
    // break up segments, causing us to miss Quads. It is useful to do a Gaussian
    // low pass on this step even if we don't want it for encoding.

    Step_4 step4 = computeLocalGradients(step3);
    //imshow("Step 4a: Magnitutde", step4.fimMag);
    //imshow("Step 4b: Theta", step4.fimTheta);

    //================================================================
    // Step five. Extract edges by grouping pixels with similar
    // thetas together. This is a greedy algorithm: we start with
    // the most similar pixels.  We use 4-connectivity.

    Step_5 step5 = extractEdges(step3, step4, width, height);
    //showStep5(step5, step3.fimSeg.rows, step3.fimSeg.cols);

    //================================================================
    // Step six: Loop over the pixels again, collecting statistics for each cluster.
    // We will soon fit lines (segments) to these points.

    Step_6 step6 = createClusters(step3, step4, step5);
    //showStep6(step6, step3.fimSeg.rows, step3.fimSeg.cols);

    //================================================================
    // Step seven: Loop over the clusters, fitting lines (which we call Segments).

    Step_7 step7 = fitSegments(step4, step6);
    //showStep7(step7, step1);

    // Step eight: For each segment, find segments that begin where this segment ends.
    // (We will chain segments together next...) The gridder accelerates the search by
    // building (essentially) a 2D hash table.

    Step_8 step8 = connectSegments(&step7, width, height);
    //showStep8(step7, step1);

    //================================================================
    // Step nine: Search all connected segments to see if any form a loop of length 4.
    // Add those to the quads list.

    Step_9 step9 = createQuads(step1, &step7);
    //showStep9(step9, step1);

    //================================================================
    // Step ten. Decode the quads. For each quad, we first estimate a
    // threshold color to decide between 0 and 1. Then, we read off the
    // bits and see if they make sense.

    Step_10 step10 = decodeQuads(step2, step9, width, height, tagFamily);
    //showStep10(step10, step1);

    //================================================================
    //Step eleven: Some quads may be detected more than once, due to
    //partial occlusion and our aggressive attempts to recover from
    //broken lines. When two quads (with the same id) overlap, we will
    //keep the one with the lowest error, and if the error is the same,
    //the one with the greatest observed perimeter.

    Step_11 step11 = removeDuplicates(step10);
    //showStep11(step11, step1);

    return step11.goodDetections;
}

