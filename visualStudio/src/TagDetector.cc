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

TagDetector::Step_1 TagDetector::createOriginalImage(const cv::Mat& image)
{
    cv::Mat fimOrig(image.rows, image.cols, CV_32FC1);
    image.convertTo(fimOrig, CV_32FC1, (1. / 255.));
    std::pair<int, int> opticalCenter(image.cols / 2, image.rows / 2);

    return { fimOrig, opticalCenter };
}

TagDetector::Step_2 TagDetector::optionallyApplyLowPassFilter(Step_1 step1, float sigma)
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

TagDetector::Step_3 TagDetector::applyLowPassFilterAndGrayScale(Step_1 step1, Step_2 step2, float sigma, float segSigma)
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

TagDetector::Step_4 TagDetector::computeLocalGradients(Step_3 step3)
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

TagDetector::Step_5 TagDetector::extractEdges(Step_3 step3, Step_4 step4, int width, int height)
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

TagDetector::Step_6 TagDetector::createClusters(Step_3 step3, Step_4 step4, Step_5 step5)
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

TagDetector::Step_7 TagDetector::fitSegments(Step_4 step4, Step_6 step6)
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

TagDetector::Step_8 TagDetector::connectSegments(Step_7* step7, int width, int height)
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

TagDetector::Step_9 TagDetector::createQuads(Step_1 step1, Step_7* step7)
{
    std::vector<AprilTag::Quad> quads;
    std::vector<AprilTag::Segment*> tmp(5);
    for (unsigned int i = 0; i < step7->segments.size(); i++) {
        tmp[0] = &(step7->segments[i]);
        AprilTag::Quad::search(tmp, step7->segments[i], 0, quads, step1.opticalCenter);
    }

    return { quads };
}

TagDetector::Step_10 TagDetector::decodeQuads(Step_2 step2, Step_9 step9, int width, int height, AprilTag::TagFamily tagFamily)
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

TagDetector::Step_11 TagDetector::removeDuplicates(Step_10 step10)
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
