#include <algorithm>
#include <cmath>
#include <climits>
#include <map>
#include <vector>
#include <iostream>

#include <Eigen/Dense>

#include "AprilTags/Edge.h"
#include "AprilTags/Gaussian.h"
#include "AprilTags/GrayModel.h"
#include "AprilTags/GLine2D.h"
#include "AprilTags/GLineSegment2D.h"
#include "AprilTags/Gridder.h"
#include "AprilTags/Homography33.h"
#include "AprilTags/MathUtil.h"
#include "AprilTags/Quad.h"
#include "AprilTags/Segment.h"
#include "AprilTags/TagFamily.h"
#include "AprilTags/UnionFindSimple.h"
#include "AprilTags/XYWeight.h"
#include "AprilTags/util.h"

#include "AprilTags/TagDetector.h"

//#define DEBUG_APRIL

#ifdef DEBUG_APRIL
#include <opencv/cv.h>
#include <opencv/highgui.h>
#endif

using namespace std;

namespace AprilTag
{
    std::vector<TagDetection> TagDetector::detect(const cv::Mat& image)
    {
        PERFORM_TIMING("Preprocessing",
            // convert to internal AprilTags image (todo: slow, change internally to OpenCV)
            int width = image.cols;
            int height = image.rows;
            cv::Mat fimOrig(image.rows, image.cols, CV_32FC1);
            // Uncomment for minor opencv optimization
            image.convertTo(fimOrig, CV_32FC1, (1. / 255.));
        )

        //================================================================
        // Step one: preprocess image (convert to grayscale) and low pass if necessary

        //! Gaussian smoothing kernel applied to image (0 == no filter).
        /*! Used when detecting the outline of the box. It is almost always
        * useful to have some filtering, since the loss of small details
        * won't hurt. Recommended value = 0.8. The case where sigma ==
        * segsigma has been optimized to avoid a redundant filter
        * operation.
        */
        float segSigma = 0.8f;

        PERFORM_TIMING("Gaussian Smoothing",
            cv::Mat fimSeg;
            if (segSigma > 0) {
                // blur anew
                int filtsz = ((int)AprilTag::max(3.0f, 3 * segSigma)) | 1;
                std::vector<float> filt = AprilTag::Gaussian::makeGaussianFilter(segSigma, filtsz);
                cv::Mat kernel_h = cv::Mat(filt.size(), 1, CV_32FC1);
                memcpy(kernel_h.data, filt.data(), filt.size() * sizeof(float));
                cv::Mat kernel_v = cv::Mat(1, filt.size(), CV_32FC1);
                memcpy(kernel_v.data, filt.data(), filt.size() * sizeof(float));

                cv::filter2D(fimOrig, fimSeg, -1, kernel_h);
                cv::filter2D(fimSeg, fimSeg, -1, kernel_v);
            } else {
                fimSeg = fimOrig.clone();
            }
        )

        //================================================================
        // Step two: Compute the local gradient. We store the direction and magnitude.
        // This step is quite sensitve to noise, since a few bad theta estimates will
        // break up segments, causing us to miss Quads. It is useful to do a Gaussian
        // low pass on this step even if we don't want it for encoding.


        PERFORM_TIMING("Gradient",
            cv::Mat fimTheta(fimSeg.rows, fimSeg.cols, CV_32FC1);
            cv::Mat fimMag(fimSeg.rows, fimSeg.cols, CV_32FC1);

            #pragma omp parallel for
            for (int y = 1; y < fimSeg.rows; y++) {
                for (int x = 1; x < fimSeg.cols; x++) {
                    float Ix = fimSeg.at<float>(y, x + 1) - fimSeg.at<float>(y, x - 1);
                    float Iy = fimSeg.at<float>(y + 1, x) - fimSeg.at<float>(y - 1, x);

                    float mag = Ix * Ix + Iy * Iy;
                    float theta = atan2(Iy, Ix);

                    fimTheta.at<float>(y, x) = theta;
                    fimMag.at<float>(y, x) = mag;
                }
            }
        )

        return detect(fimOrig, fimMag, fimTheta);
    }

    std::vector<TagDetection> TagDetector::detect(const cv::Mat& fimOrig, const cv::Mat& fimMag, const cv::Mat& fimTheta)
    {
        PERFORM_TIMING("Establish Constants",
            int width = fimOrig.cols;
            int height = fimOrig.rows;
        )

        //================================================================
        // Step three. Extract edges by grouping pixels with similar
        // thetas together. This is a greedy algorithm: we start with
        // the most similar pixels.  We use 4-connectivity.
        PERFORM_TIMING("Edge Extraction",
            AprilTag::UnionFindSimple uf(width * height);

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

                        float mag0 = fimMag.at<float>(y, x);
                        if (mag0 < AprilTag::Edge::minMag)
                            continue;
                        mmax[y * width + x] = mag0;
                        mmin[y * width + x] = mag0;

                        float theta0 = fimTheta.at<float>(y, x);
                        tmin[y * width + x] = theta0;
                        tmax[y * width + x] = theta0;

                        // Calculates then adds edges to 'vector<Edge> edges'
                        AprilTag::Edge::calcEdges(theta0, x, y, fimTheta, fimMag, edges, nEdges);

                        // XXX Would 8 connectivity help for rotated tags?
                        // Probably not much, so long as input filtering hasn't been disabled.
                    }
                }

                edges.resize(nEdges);
                std::stable_sort(edges.begin(), edges.end());
                AprilTag::Edge::mergeEdges(edges, uf, tmin, tmax, mmin, mmax);
            }
        )

        //================================================================
        // Step four: Loop over the pixels again, collecting statistics for each cluster.
        // We will soon fit lines (segments) to these points.
        PERFORM_TIMING("Cluster",
            std::map<int, std::vector<AprilTag::XYWeight> > clusters;
            for (int y = 0; y + 1 < fimOrig.rows; y++) {
                for (int x = 0; x + 1 < fimOrig.cols; x++) {
                    if (uf.getSetSize(y * fimOrig.cols + x) < AprilTag::Segment::minimumSegmentSize)
                        continue;

                    int rep = (int)uf.getRepresentative(y * fimOrig.cols + x);

                    std::map<int, std::vector<AprilTag::XYWeight> >::iterator it = clusters.find(rep);
                    if (it == clusters.end()) {
                        clusters[rep] = std::vector<AprilTag::XYWeight>();
                        it = clusters.find(rep);
                    }
                    std::vector<AprilTag::XYWeight>& points = it->second;
                    points.push_back(AprilTag::XYWeight(x, y, fimMag.at<float>(y, x)));
                }
            }
        )
        //================================================================
        // Step five: Loop over the clusters, fitting lines (which we call Segments).
        PERFORM_TIMING("Segmentation",
            std::vector<AprilTag::Segment> segments; //used in Step six
            std::map<int, std::vector<AprilTag::XYWeight> >::const_iterator clustersItr;
            for (clustersItr = clusters.begin(); clustersItr != clusters.end(); clustersItr++) {
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

                    float theta = fimTheta.at<float>((int)xyw.y, (int)xyw.x);
                    float mag = fimMag.at<float>((int)xyw.y, (int)xyw.x);

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
        )
        
        return detect<float>(fimOrig, segments);
    }

    template<class T>
    std::vector<TagDetection> TagDetector::detect(const cv::Mat& fimOrig, std::vector<AprilTag::Segment> segments)
    {
        PERFORM_TIMING("Establish Constants",
            int width = fimOrig.cols;
            int height = fimOrig.rows;

            std::pair<int, int> opticalCenter(width / 2, height / 2);
        )
        // Step six: For each segment, find segments that begin where this segment ends.
        // (We will chain segments together next...) The gridder accelerates the search by
        // building (essentially) a 2D hash table.
        PERFORM_TIMING("Segment Connection",
            AprilTag::Gridder<AprilTag::Segment> gridder(0, 0, width, height, 10);

            // add every segment to the hash table according to the position of the segment's
            // first point. Remember that the first point has a specific meaning due to our
            // left-hand rule above.
            for (unsigned int i = 0; i < segments.size(); i++) {
                gridder.add(segments[i].getX0(), segments[i].getY0(), &(segments[i]));
            }

            // Now, find child segments that begin where each parent segment ends.
            for (unsigned i = 0; i < segments.size(); i++) {
                AprilTag::Segment& parentseg = segments[i];

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
        )
        //================================================================
        // Step seven: Search all connected segments to see if any form a loop of length 4.
        // Add those to the quads list.
        PERFORM_TIMING("Create Quads",
            std::vector<AprilTag::Quad> quads;
            std::vector<AprilTag::Segment*> tmp(5);
            for (unsigned int i = 0; i < segments.size(); i++) {
                tmp[0] = &(segments[i]);
                AprilTag::Quad::search(tmp, segments[i], 0, quads, opticalCenter);
            }
        )
        //================================================================
        // Step eight. Decode the quads. For each quad, we first estimate a
        // threshold color to decide between 0 and 1. Then, we read off the
        // bits and see if they make sense.

        PERFORM_TIMING("Decode Quads",
            std::vector<AprilTag::TagDetection> detections;

            for (unsigned int qi = 0; qi < quads.size(); qi++) {
                AprilTag::Quad& quad = quads[qi];

                // Find a threshold
                AprilTag::GrayModel blackModel, whiteModel;
                const int dd = 2 * thisTagFamily.blackBorder + thisTagFamily.dimension;

                for (int iy = -1; iy <= dd; iy++) {
                    float y = (iy + 0.5f) / dd;
                    for (int ix = -1; ix <= dd; ix++) {
                        float x = (ix + 0.5f) / dd;
                        std::pair<float, float> pxy = quad.interpolate01(x, y);
                        int irx = (int)(pxy.first + 0.5);
                        int iry = (int)(pxy.second + 0.5);
                        if (irx < 0 || irx >= width || iry < 0 || iry >= height)
                            continue;
                        float v = fimOrig.at<T>(iry, irx);
                        if (iy == -1 || iy == dd || ix == -1 || ix == dd)
                            whiteModel.addObservation(x, y, v);
                        else if (iy == 0 || iy == (dd - 1) || ix == 0 || ix == (dd - 1))
                            blackModel.addObservation(x, y, v);
                    }
                }

                bool bad = false;
                unsigned long long tagCode = 0;
                for (int iy = thisTagFamily.dimension - 1; iy >= 0; iy--) {
                    float y = (thisTagFamily.blackBorder + iy + 0.5f) / dd;
                    for (int ix = 0; ix < thisTagFamily.dimension; ix++) {
                        float x = (thisTagFamily.blackBorder + ix + 0.5f) / dd;
                        std::pair<float, float> pxy = quad.interpolate01(x, y);
                        int irx = (int)(pxy.first + 0.5);
                        int iry = (int)(pxy.second + 0.5);
                        if (irx < 0 || irx >= width || iry < 0 || iry >= height) {
                            // cout << "*** bad:  irx=" << irx << "  iry=" << iry << endl;
                            bad = true;
                            continue;
                        }
                        float threshold = (blackModel.interpolate(x, y) + whiteModel.interpolate(x, y)) * 0.5f;
                        float v = fimOrig.at<T>(iry, irx);
                        tagCode = tagCode << 1;
                        if (v > threshold)
                            tagCode |= 1;
                    }
                }

                if (!bad) {
                    AprilTag::TagDetection thisTagDetection;
                    thisTagFamily.decode(thisTagDetection, tagCode);

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
        )
        //================================================================
        //Step nine: Some quads may be detected more than once, due to
        //partial occlusion and our aggressive attempts to recover from
        //broken lines. When two quads (with the same id) overlap, we will
        //keep the one with the lowest error, and if the error is the same,
        //the one with the greatest observed perimeter.

        PERFORM_TIMING("Duplication Removal",
            std::vector<AprilTag::TagDetection> goodDetections;

            // NOTE: allow multiple non-overlapping detections of the same target.

            for (std::vector<AprilTag::TagDetection>::const_iterator it = detections.begin();
                it != detections.end(); it++) {
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
        )

        return goodDetections;
    }

    /** When calling this detection portion with the segments pre-calculated,
     * the grayscaled original image doesn't have to be normalized, thus saving
     * ~8ms
     */
    template std::vector<TagDetection> TagDetector::detect<uchar>(const cv::Mat& fimOrig, std::vector<AprilTag::Segment> segments);

} // namespace
