#ifndef TAGDETECTOR_H
#define TAGDETECTOR_H

#include <vector>

#include "opencv2/opencv.hpp"

#include "TagDetection.h"
#include "TagFamily.h"
#include "FloatImage.h"

#ifndef M_PI
#define M_PI       3.14159265358979323846   // pi
#endif

#include <algorithm>
#include <cmath>
#include <climits>
#include <map>
#include <vector>
#include <iostream>

#include <Eigen/Dense>

#include "Edge.h"
#include "FloatImage.h"
#include "Gaussian.h"
#include "GrayModel.h"
#include "GLine2D.h"
#include "GLineSegment2D.h"
#include "Gridder.h"
#include "Homography33.h"
#include "MathUtil.h"
#include "Quad.h"
#include "Segment.h"
#include "TagFamily.h"
#include "UnionFindSimple.h"
#include "XYWeight.h"

#include "TagDetector.h"
#include "DemoControls.h"

//#define DEBUG_APRIL

#ifdef DEBUG_APRIL
#include <opencv/cv.h>
#include <opencv/highgui.h>
#endif

/**
    * Step 1 output.
    *
    * \var Step_1::fimOrig The original, unmodified image for this algorithm.
    * \var opticalCenter The center point of the image.
    */
typedef struct
{
    cv::Mat fimOrig;
    std::pair<int, int> opticalCenter;
} Step_1;

/**
    * Convert to internal AprilTag image.
    *
    * TODO: Change internally to OpenCV for speedup.
    *
    * \param image Greyscaled image
    * \return Step_1 object.
    */
Step_1 createOriginalImage(const cv::Mat& image);

/**
    * Step 2 output.
    *
    * \var fim Copy of the original image that may or may not have been filtered.
    */
typedef struct
{
    cv::Mat fim;
} Step_2;

/**
    * Applies a lowpass filter to the image from step 1.
    *
    * \param step1 Step 1 output data.
    * \param sigma The filter intensity.
    * \return Step 2 object.
    */
Step_2 optionallyApplyLowPassFilter(Step_1 step1, float sigma);

/**
    * Output for step 3.
    *
    * \var fimSeg The filtered and greyscaled version of original image.
    */
typedef struct
{
    cv::Mat fimSeg;
} Step_3;

/**
    * Applies a low pass filter to the image from step 1 and gray scales.
    *
    * \param step1 Output from step 1.
    * \param step2 Output from step 2.
    * \param sigma Filter intensity from step 2.
    * \param segSigma Filter intensity for this step.
    * \return Step 3 object.
    */
Step_3 applyLowPassFilterAndGrayScale(Step_1 step1, Step_2 step2, float sigma, float segSigma);

/**
    * Output from step 4.
    *
    * \var fimTheta The image's directional vector matrix.
    * \var fimMag The image's magnitude vector matrix.
    */
typedef struct
{
    cv::Mat fimTheta;
    cv::Mat fimMag;
} Step_4;

/**
    * Compute the local gradient. We store the direction and magnitude.
    * This step is quite sensitve to noise, since a few bad theta estimates will
    * break up segments, causing us to miss Quads. It is useful to do a Gaussian
    * low pass on this step even if we don't want it for encoding.
    *
    * \param step3 Step 3 object output.
    * \return Step 4 object.
    */
Step_4 computeLocalGradients(Step_3 step3);

/**
    * Output from step 5.
    *
    * \var uf A matrix of edge groups.
    */
typedef struct
{
    AprilTag::UnionFindSimple uf;
} Step_5;

/**
    * Finds the edges given by the thetas from step 4.
    *
    * \param step3 Step 3 object output. Used for only their width and height.
    * \param step4 Step 4 object output.
    * \param width The width of the original image.
    * \param height The height of the original image.
    * \return Step_5 object.
    */
Step_5 extractEdges(Step_3 step3, Step_4 step4, int width, int height);

/**
    * Output from step 6.
    *
    * \var clusters A map of clustered edges.
    */
typedef struct
{
    std::map<int, std::vector<AprilTag::XYWeight>> clusters;
} Step_6;

/**
    * Collect statistics and form clusters.
    *
    * \param step3 Step 3 object output. Used for only its width and height.
    * \param step4 Step 4 object output. Only the magnitude matrix is used.
    * \param step5 Step 5 object output.
    * \return Step_6 object.
    */
Step_6 createClusters(Step_3 step3, Step_4 step4, Step_5 step5);

/**
    * Output from step 7.
    *
    * \var segments A vector of line segments that were created from the clusters.
    */
typedef struct
{
    std::vector<AprilTag::Segment> segments;
} Step_7;

/**
    * Fits line segments from the given clusters. Uses step 4's magnitude and theta matricies
    * to determine the winding directions.
    *
    * \param step4 Output object from step 4.
    * \param step6 Output object from step 6.
    * \return Step_7 object.
    */
Step_7 fitSegments(Step_4 step4, Step_6 step6);

/**
    * Output from step 8.
    *
    * Updates the segments in-place. There is no output for this step.
    */
typedef struct
{
    // Nothing!
} Step_8;

/**
    * Connects the given segments together by their end/start points.
    *
    * \param step7 Output object from step 7.
    * \param width The width of the original image.
    * \param height The height of the original image.
    * \return Step_8 object.
    */
Step_8 connectSegments(Step_7* step7, int width, int height);

/**
    * Output from step 9.
    *
    * \var quads A vector of found quads.
    */
typedef struct
{
    std::vector<AprilTag::Quad> quads;
} Step_9;

/**
    * Joins line segments with loops of 4.
    *
    * \param step1 Output object from Step 1.
    * \param step7 Output object from Step 7.
    * \return Step_9 object.
    */
Step_9 createQuads(Step_1 step1, Step_7* step7);

/**
    * Output from step 10.
    *
    * \var detections Tags that have been recongized.
    */
typedef struct
{
    std::vector<AprilTag::TagDetection> detections;
} Step_10;

/**
    * Decodes the quads for the given family.
    *
    * \param step2 Output object from step 2.
    * \param step9 Output object from step 9.
    * \param width The width of the original image.
    * \param height The height of the original image.
    * \param tagFamily The tag family to decode for.
    * \return Step_10 object.
    */
Step_10 decodeQuads(Step_2 step2, Step_9 step9, int width, int height, AprilTag::TagFamily tagFamily);

/**
    * Output from step 11.
    *
    * \var goodDetections A vector of valid detections.
    */
typedef struct
{
    std::vector<AprilTag::TagDetection> goodDetections;
} Step_11;

/**
    * Removes any duplicate quads that are overlapping.
    *
    * \param step10 Output object from step 10.
    * \return Step_11 object.
    */
Step_11 removeDuplicates(Step_10 step10);

std::vector<AprilTag::TagDetection> extractTags(const cv::Mat& image, AprilTag::TagFamily tagFamily, DemoControls* controls);

#endif
