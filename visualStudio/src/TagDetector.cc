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

//#define DEBUG_APRIL

#ifdef DEBUG_APRIL
#include <opencv/cv.h>
#include <opencv/highgui.h>
#endif


template<class T>
void cvToFloatImage(const cv::Mat& mat, AprilTag::FloatImage& toRet, double scale = 1.0)
{
    for (int y = 0; y < mat.rows; y++)
    {
        for (int x = 0; x < mat.cols; x++)
        {
            toRet.set(x, y, mat.at<T>(y, x) * scale);
        }
    }
}

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
Step_1 createOriginalImage(const cv::Mat& image)
{
    cv::Mat fimOrig(image.rows, image.cols, CV_32FC1);
    image.convertTo(fimOrig, CV_32FC1, (1. / 255.));
    std::pair<int, int> opticalCenter(image.cols / 2, image.rows / 2);

    return { fimOrig, opticalCenter };
}

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
Step_2 optionallyApplyLowPassFilter(Step_1 step1, float sigma)
{
    cv::Mat fim = step1.fimOrig.clone();
    cv::Mat kernel_h;
    cv::Mat kernel_v;

    if (sigma > 0) {
        int filtsz = ((int)AprilTag::max(3.0f, 3 * sigma)) | 1;
        std::vector<float> filt = AprilTag::Gaussian::makeGaussianFilter(sigma, filtsz);
        kernel_h = cv::Mat(filt.size(), 1, CV_32FC1);
        memcpy(kernel_h.data, filt.data(), filt.size() * sizeof(float));
        kernel_v = cv::Mat(1, filt.size(), CV_32FC1);
        memcpy(kernel_v.data, filt.data(), filt.size() * sizeof(float));

        cv::filter2D(step1.fimOrig, fim, -1, kernel_h);
        cv::filter2D(fim, fim, -1, kernel_v);
    }

    return { fim };
}

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
Step_3 applyLowPassFilterAndGrayScale(Step_1 step1, Step_2 step2, float sigma, float segSigma)
{
    cv::Mat fimSeg = step1.fimOrig.clone();
    if (segSigma > 0) {
        if (segSigma == sigma) {
            fimSeg = step2.fim.clone();
        }
        else {
            fimSeg = optionallyApplyLowPassFilter(step1, segSigma).fim;
        }
    }

    return { fimSeg };
}

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
Step_4 computeLocalGradients(Step_3 step3)
{
    cv::Mat fimTheta(step3.fimSeg.rows, step3.fimSeg.cols, CV_32FC1);
    cv::Mat fimMag(step3.fimSeg.rows, step3.fimSeg.cols, CV_32FC1);


#pragma omp parallel for
    for (int y = 1; y < step3.fimSeg.rows - 1; y++) {
        for (int x = 1; x < step3.fimSeg.cols - 1; x++) {
            float Ix = step3.fimSeg.at<float>(y, x + 1) - step3.fimSeg.at<float>(y, x - 1);
            float Iy = step3.fimSeg.at<float>(y + 1, x) - step3.fimSeg.at<float>(y - 1, x);

            float mag = Ix * Ix + Iy * Iy;
            float theta = atan2(Iy, Ix);

            fimTheta.at<float>(y, x) = theta;
            fimMag.at<float>(y, x) = mag;
        }
    }

    return { fimTheta, fimMag };
}

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
Step_5 extractEdges(Step_3 step3, Step_4 step4, int width, int height)
{
    AprilTag::UnionFindSimple uf(step3.fimSeg.cols * step3.fimSeg.rows);

    std::vector<AprilTag::Edge> edges(width * height * 4);
    size_t nEdges = 0;

    // Bounds on the thetas assigned to this group. Note that because
    // theta is periodic, these are defined such that the average
    // value is contained *within* the interval.
    { // limit scope of storage
        /* Previously all this was on the stack, but this is 1.2MB for 320x240 images
            * That's already a problem for OS X (default 512KB thread stack size),
            * could be a problem elsewhere for bigger images... so store on heap */
        std::vector<float> storage(width * height * 4);  // do all the memory in one big block, exception safe
        float* tmin = &storage[width * height * 0];
        float* tmax = &storage[width * height * 1];
        float* mmin = &storage[width * height * 2];
        float* mmax = &storage[width * height * 3];

        for (int y = 0; y + 1 < height; y++) {
            for (int x = 0; x + 1 < width; x++) {

                float mag0 = step4.fimMag.at<float>(y, x);
                if (mag0 < AprilTag::Edge::minMag)
                    continue;
                mmax[y * width + x] = mag0;
                mmin[y * width + x] = mag0;

                float theta0 = step4.fimTheta.at<float>(y, x);
                tmin[y * width + x] = theta0;
                tmax[y * width + x] = theta0;

                // Calculates then adds edges to 'vector<Edge> edges'
                AprilTag::Edge::calcEdges(theta0, x, y, step4.fimTheta, step4.fimMag, edges, nEdges);

                // XXX Would 8 connectivity help for rotated tags?
                // Probably not much, so long as input filtering hasn't been disabled.
            }
        }

        edges.resize(nEdges);
        std::stable_sort(edges.begin(), edges.end());
        AprilTag::Edge::mergeEdges(edges, uf, tmin, tmax, mmin, mmax);
    }

    return { uf };
}

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
Step_6 createClusters(Step_3 step3, Step_4 step4, Step_5 step5)
{
    std::map<int, std::vector<AprilTag::XYWeight> > clusters;
    for (int y = 0; y + 1 < step3.fimSeg.rows; y++) {
        for (int x = 0; x + 1 < step3.fimSeg.cols; x++) {
            if (step5.uf.getSetSize(y * step3.fimSeg.cols + x) < AprilTag::Segment::minimumSegmentSize)
                continue;

            int rep = (int)step5.uf.getRepresentative(y * step3.fimSeg.cols + x);

            std::map<int, std::vector<AprilTag::XYWeight> >::iterator it = clusters.find(rep);
            if (it == clusters.end()) {
                clusters[rep] = std::vector<AprilTag::XYWeight>();
                it = clusters.find(rep);
            }
            std::vector<AprilTag::XYWeight>& points = it->second;
            points.push_back(AprilTag::XYWeight(x, y, step4.fimMag.at<float>(y, x)));
        }
    }

    return { clusters };
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
Step_7 fitSegments(Step_4 step4, Step_6 step6)
{
    std::vector<AprilTag::Segment> segments; //used in Step six
    std::map<int, std::vector<AprilTag::XYWeight> >::const_iterator clustersItr;
    for (clustersItr = step6.clusters.begin(); clustersItr != step6.clusters.end(); clustersItr++) {
        std::vector<AprilTag::XYWeight> points = clustersItr->second;
        AprilTag::GLineSegment2D gseg = AprilTag::GLineSegment2D::lsqFitXYW(points);

        // filter short lines
        float length = AprilTag::MathUtil::distance2D(gseg.getP0(), gseg.getP1());
        if (length < AprilTag::Segment::minimumLineLength)
            continue;

        AprilTag::Segment seg;
        float dy = gseg.getP1().second - gseg.getP0().second;
        float dx = gseg.getP1().first - gseg.getP0().first;

        float tmpTheta = std::atan2(dy, dx);

        seg.setTheta(tmpTheta);
        seg.setLength(length);

        // We add an extra semantic to segments: the vector
        // p1->p2 will have dark on the left, white on the right.
        // To do this, we'll look at every gradient and each one
        // will vote for which way they think the gradient should
        // go. This is way more retentive than necessary: we
        // could probably sample just one point!

        float flip = 0, noflip = 0;
        for (unsigned int i = 0; i < points.size(); i++) {
            AprilTag::XYWeight xyw = points[i];

            float theta = step4.fimTheta.at<float>((int)xyw.y, (int)xyw.x);
            float mag = step4.fimMag.at<float>((int)xyw.y, (int)xyw.x);

            // err *should* be +M_PI/2 for the correct winding, but if we
            // got the wrong winding, it'll be around -M_PI/2.
            float err = AprilTag::MathUtil::mod2pi(theta - seg.getTheta());

            if (err < 0)
                noflip += mag;
            else
                flip += mag;
        }

        if (flip > noflip) {
            float temp = seg.getTheta() + (float)M_PI;
            seg.setTheta(temp);
        }

        float dot = dx * std::cos(seg.getTheta()) + dy * std::sin(seg.getTheta());
        if (dot > 0) {
            seg.setX0(gseg.getP1().first); seg.setY0(gseg.getP1().second);
            seg.setX1(gseg.getP0().first); seg.setY1(gseg.getP0().second);
        }
        else {
            seg.setX0(gseg.getP0().first); seg.setY0(gseg.getP0().second);
            seg.setX1(gseg.getP1().first); seg.setY1(gseg.getP1().second);
        }

        segments.push_back(seg);
    }

    return { segments };
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
Step_8 connectSegments(Step_7* step7, int width, int height)
{
    AprilTag::Gridder<AprilTag::Segment> gridder(0, 0, width, height, 10);

    // add every segment to the hash table according to the position of the segment's
    // first point. Remember that the first point has a specific meaning due to our
    // left-hand rule above.
    for (unsigned int i = 0; i < step7->segments.size(); i++) {
        gridder.add(step7->segments[i].getX0(), step7->segments[i].getY0(), &(step7->segments[i]));
    }

    // Now, find child segments that begin where each parent segment ends.
    for (unsigned i = 0; i < step7->segments.size(); i++) {
        AprilTag::Segment& parentseg = step7->segments[i];

        //compute length of the line segment
        AprilTag::GLine2D parentLine(std::pair<float, float>(parentseg.getX0(), parentseg.getY0()),
            std::pair<float, float>(parentseg.getX1(), parentseg.getY1()));

        AprilTag::Gridder<AprilTag::Segment>::iterator iter = gridder.find(parentseg.getX1(), parentseg.getY1(), 0.5f * parentseg.getLength());
        while (iter.hasNext()) {
            AprilTag::Segment& child = iter.next();
            if (AprilTag::MathUtil::mod2pi(child.getTheta() - parentseg.getTheta()) > 0) {
                continue;
            }

            // compute intersection of points
            AprilTag::GLine2D childLine(std::pair<float, float>(child.getX0(), child.getY0()),
                std::pair<float, float>(child.getX1(), child.getY1()));

            std::pair<float, float> p = parentLine.intersectionWith(childLine);
            if (p.first == -1) {
                continue;
            }

            float parentDist = AprilTag::MathUtil::distance2D(p, std::pair<float, float>(parentseg.getX1(), parentseg.getY1()));
            float childDist = AprilTag::MathUtil::distance2D(p, std::pair<float, float>(child.getX0(), child.getY0()));

            if (AprilTag::max(parentDist, childDist) > parentseg.getLength()) {
                continue;
            }

            // everything's OK, this child is a reasonable successor.
            parentseg.children.push_back(&child);
        }
    }

    return {};
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
Step_9 createQuads(Step_1 step1, Step_7* step7)
{
    std::vector<AprilTag::Quad> quads;
    std::vector<AprilTag::Segment*> tmp(5);
    for (unsigned int i = 0; i < step7->segments.size(); i++) {
        tmp[0] = &(step7->segments[i]);
        AprilTag::Quad::search(tmp, step7->segments[i], 0, quads, step1.opticalCenter);
    }

    return { quads };
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
Step_10 decodeQuads(Step_2 step2, Step_9 step9, int width, int height, AprilTag::TagFamily tagFamily)
{
    std::vector<AprilTag::TagDetection> detections;

    for (unsigned int qi = 0; qi < step9.quads.size(); qi++) {
        AprilTag::Quad& quad = step9.quads[qi];

        // Find a threshold
        AprilTag::GrayModel blackModel, whiteModel;
        const int dd = 2 * tagFamily.blackBorder + tagFamily.dimension;

        for (int iy = -1; iy <= dd; iy++) {
            float y = (iy + 0.5f) / dd;
            for (int ix = -1; ix <= dd; ix++) {
                float x = (ix + 0.5f) / dd;
                std::pair<float, float> pxy = quad.interpolate01(x, y);
                int irx = (int)(pxy.first + 0.5);
                int iry = (int)(pxy.second + 0.5);
                if (irx < 0 || irx >= width || iry < 0 || iry >= height)
                    continue;
                float v = step2.fim.at<float>(iry, irx);
                if (iy == -1 || iy == dd || ix == -1 || ix == dd)
                    whiteModel.addObservation(x, y, v);
                else if (iy == 0 || iy == (dd - 1) || ix == 0 || ix == (dd - 1))
                    blackModel.addObservation(x, y, v);
            }
        }

        bool bad = false;
        unsigned long long tagCode = 0;
        for (int iy = tagFamily.dimension - 1; iy >= 0; iy--) {
            float y = (tagFamily.blackBorder + iy + 0.5f) / dd;
            for (int ix = 0; ix < tagFamily.dimension; ix++) {
                float x = (tagFamily.blackBorder + ix + 0.5f) / dd;
                std::pair<float, float> pxy = quad.interpolate01(x, y);
                int irx = (int)(pxy.first + 0.5);
                int iry = (int)(pxy.second + 0.5);
                if (irx < 0 || irx >= width || iry < 0 || iry >= height) {
                    // cout << "*** bad:  irx=" << irx << "  iry=" << iry << endl;
                    bad = true;
                    continue;
                }
                float threshold = (blackModel.interpolate(x, y) + whiteModel.interpolate(x, y)) * 0.5f;
                float v = step2.fim.at<float>(iry, irx);
                tagCode = tagCode << 1;
                if (v > threshold)
                    tagCode |= 1;
            }
        }

        if (!bad) {
            AprilTag::TagDetection thisTagDetection;
            tagFamily.decode(thisTagDetection, tagCode);

            // compute the homography (and rotate it appropriately)
            thisTagDetection.homography = quad.homography.getH();
            thisTagDetection.hxy = quad.homography.getCXY();

            float c = std::cos(thisTagDetection.rotation * (float)M_PI / 2);
            float s = std::sin(thisTagDetection.rotation * (float)M_PI / 2);
            Eigen::Matrix3d R;
            R.setZero();
            R(0, 0) = R(1, 1) = c;
            R(0, 1) = -s;
            R(1, 0) = s;
            R(2, 2) = 1;
            Eigen::Matrix3d tmp;
            tmp = thisTagDetection.homography * R;
            thisTagDetection.homography = tmp;

            // Rotate points in detection according to decoded
            // orientation.  Thus the order of the points in the
            // detection object can be used to determine the
            // orientation of the target.
            std::pair<float, float> bottomLeft = thisTagDetection.interpolate(-1, -1);
            int bestRot = -1;
            float bestDist = FLT_MAX;
            for (int i = 0; i < 4; i++) {
                float const dist = AprilTag::MathUtil::distance2D(bottomLeft, quad.quadPoints[i]);
                if (dist < bestDist) {
                    bestDist = dist;
                    bestRot = i;
                }
            }

            for (int i = 0; i < 4; i++)
                thisTagDetection.p[i] = quad.quadPoints[(i + bestRot) % 4];

            if (thisTagDetection.good) {
                thisTagDetection.cxy = quad.interpolate01(0.5f, 0.5f);
                thisTagDetection.observedPerimeter = quad.observedPerimeter;
                detections.push_back(thisTagDetection);
            }
        }
    }

    return { detections };
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
Step_11 removeDuplicates(Step_10 step10)
{
    std::vector<AprilTag::TagDetection> goodDetections;

    // NOTE: allow multiple non-overlapping detections of the same target.

    for (std::vector<AprilTag::TagDetection>::const_iterator it = step10.detections.begin();
        it != step10.detections.end(); it++) {
        const AprilTag::TagDetection& thisTagDetection = *it;

        bool newFeature = true;

        for (unsigned int odidx = 0; odidx < goodDetections.size(); odidx++) {
            AprilTag::TagDetection& otherTagDetection = goodDetections[odidx];

            if (thisTagDetection.id != otherTagDetection.id ||
                !thisTagDetection.overlapsTooMuch(otherTagDetection))
                continue;

            // There's a conflict.  We must pick one to keep.
            newFeature = false;

            // This detection is worse than the previous one... just don't use it.
            if (thisTagDetection.hammingDistance > otherTagDetection.hammingDistance)
                continue;

            // Otherwise, keep the new one if it either has strictly *lower* error, or greater perimeter.
            if (thisTagDetection.hammingDistance < otherTagDetection.hammingDistance ||
                thisTagDetection.observedPerimeter > otherTagDetection.observedPerimeter)
                goodDetections[odidx] = thisTagDetection;
        }

        if (newFeature)
            goodDetections.push_back(thisTagDetection);

    }

    return { goodDetections };
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

std::vector<AprilTag::TagDetection> TagDetector::extractTags(const cv::Mat& image) {
    srand(time(0));
    int width = image.cols;
    int height = image.rows;

    imshow("Pre Steps", image);

    Step_1 step1 = createOriginalImage(image);
    imshow("Step 1", step1.fimOrig);

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
    imshow("Step 2", step2.fim);

    //! Gaussian smoothing kernel applied to image (0 == no filter).
    /*! Used when detecting the outline of the box. It is almost always
        * useful to have some filtering, since the loss of small details
        * won't hurt. Recommended value = 0.8. The case where sigma ==
        * segsigma has been optimized to avoid a redundant filter
        * operation.
        */
    float segSigma = 0.8f;

    Step_3 step3 = applyLowPassFilterAndGrayScale(step1, step2, sigma, segSigma);
    imshow("Step 3", step3.fimSeg);

    //================================================================
    // Step four: Compute the local gradient. We store the direction and magnitude.
    // This step is quite sensitve to noise, since a few bad theta estimates will
    // break up segments, causing us to miss Quads. It is useful to do a Gaussian
    // low pass on this step even if we don't want it for encoding.

    Step_4 step4 = computeLocalGradients(step3);
    imshow("Step 4a: Magnitutde", step4.fimMag);
    imshow("Step 4b: Theta", step4.fimTheta);

    //================================================================
    // Step five. Extract edges by grouping pixels with similar
    // thetas together. This is a greedy algorithm: we start with
    // the most similar pixels.  We use 4-connectivity.

    Step_5 step5 = extractEdges(step3, step4, width, height);
    showStep5(step5, step3.fimSeg.rows, step3.fimSeg.cols);

    //================================================================
    // Step six: Loop over the pixels again, collecting statistics for each cluster.
    // We will soon fit lines (segments) to these points.

    Step_6 step6 = createClusters(step3, step4, step5);
    showStep6(step6, step3.fimSeg.rows, step3.fimSeg.cols);

    //================================================================
    // Step seven: Loop over the clusters, fitting lines (which we call Segments).

    Step_7 step7 = fitSegments(step4, step6);
    showStep7(step7, step1);

    // Step eight: For each segment, find segments that begin where this segment ends.
    // (We will chain segments together next...) The gridder accelerates the search by
    // building (essentially) a 2D hash table.

    Step_8 step8 = connectSegments(&step7, width, height);
    showStep8(step7, step1);

    //================================================================
    // Step nine: Search all connected segments to see if any form a loop of length 4.
    // Add those to the quads list.

    Step_9 step9 = createQuads(step1, &step7);
    showStep9(step9, step1);

    //================================================================
    // Step ten. Decode the quads. For each quad, we first estimate a
    // threshold color to decide between 0 and 1. Then, we read off the
    // bits and see if they make sense.

    Step_10 step10 = decodeQuads(step2, step9, width, height, thisTagFamily);
    showStep10(step10, step1);

    //================================================================
    //Step eleven: Some quads may be detected more than once, due to
    //partial occlusion and our aggressive attempts to recover from
    //broken lines. When two quads (with the same id) overlap, we will
    //keep the one with the lowest error, and if the error is the same,
    //the one with the greatest observed perimeter.

    Step_11 step11 = removeDuplicates(step10);
    showStep11(step11, step1);

    return step11.goodDetections;
}
