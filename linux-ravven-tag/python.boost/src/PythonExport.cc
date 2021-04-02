// Expose internals to Python
#define PY_ARRAY_UNIQUE_SYMBOL pbcvt_ARRAY_API

#include <utility>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <pyboostcvconverter/pyboostcvconverter.hpp>
#include "AprilTags/TagDetector.h"
#include "AprilTags/TagDetection.h"
#include "AprilTags/Tag36h11.h"
#include "AprilTags/util.h"
#include "AprilTags/XYWeight.h"
#include "AprilTags/MathUtil.h"

using namespace boost::python;
using namespace pbcvt;

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

class GlobalModule
{
    public:
        const AprilTag::TagCodes tagCodes36h11 = AprilTag::tagCodes36h11;
};

struct CameraParameters
{
    double tagSize;
    double fx;
    double fy;
    double px;
    double py;
};

struct DetectionData
{
    int id;
    int hammingDistance;
    std::vector<std::pair<float,float>> corners; // [4, 2] containing pairs of coordinates
    double distance;
    double x, y, z;
    double yaw, pitch, roll;
};

const double PI = 3.14159265358979323846;
const double TWOPI = 2.0*PI;

// Private static functions
inline double standardRad(double t);
void wRo_to_euler(const Eigen::Matrix3d& wRo, double& yaw, double& pitch, double& roll);

// Public static functions
void print_detection(struct CameraParameters cp, AprilTag::TagDetection& detection);
struct DetectionData break_out_detection(struct CameraParameters cp, AprilTag::TagDetection& detection);
void draw_detection(cv::Mat& originalImage, AprilTag::TagDetection& detection);
std::vector<AprilTag::Segment> ccaSegmentation(const cv::Mat& segPoints);
void reorientSegment(
    std::vector<AprilTag::Segment>& segments,
    std::pair<AprilTag::XYWeight, AprilTag::XYWeight>& pointPair,
    float length,
    float goldenTheta
);

// Tag extraction functions
std::vector<AprilTag::TagDetection> extractTags(AprilTag::TagDetector& detector, PyObject* im_in)
{
    PERFORM_TIMING("Pre-Pre Processing",
        cv::Mat image;
        image = pbcvt::fromNDArrayToMat(im_in);
    )
    return detector.detect(image);
}

std::vector<AprilTag::TagDetection> extractMagThetaTags(
    AprilTag::TagDetector& detector,
    PyObject* arrOrig, PyObject* arrMag, PyObject* arrTheta
)
{
    PERFORM_TIMING("Pre-Pre Processing",
        cv::Mat start, mag, theta;
        start = pbcvt::fromNDArrayToMat(arrOrig);
        mag = pbcvt::fromNDArrayToMat(arrMag);
        theta = pbcvt::fromNDArrayToMat(arrTheta);
        /* Pre-processing */
        // Normalize
        start.convertTo(start, CV_32FC1, (1. / 255.));
        mag.convertTo(mag, CV_32FC1, (1. / 255.));
        theta.convertTo(theta, CV_32FC1, ((2. * PI) / 255.), -PI);
    )
    return detector.detect(start, mag, theta);
}

std::vector<AprilTag::TagDetection> extractCCATags(
    AprilTag::TagDetector& detector,
    PyObject* arrOrig, PyObject* arrSegPts
)
{
    PERFORM_TIMING("Pre-Processiong",
        cv::Mat fimOrig, segPoints;
        fimOrig = pbcvt::fromNDArrayToMat(arrOrig);
        segPoints = pbcvt::fromNDArrayToMat(arrSegPts);
    )
    PERFORM_TIMING("CCA Segmentation",
        std::vector<AprilTag::Segment> segments = ccaSegmentation(segPoints);
    )
    return detector.detect<uchar>(fimOrig, segments);
}

std::vector<AprilTag::TagDetection> extractSegmentedTags(
    AprilTag::TagDetector& detector,
    PyObject* arrOrig, PyObject* arrSegments
)
{
    PERFORM_TIMING("Pre-Processing",
        cv::Mat fimOrig, segmentsMat;
        fimOrig = pbcvt::fromNDArrayToMat(arrOrig);
        segmentsMat = pbcvt::fromNDArrayToMat(arrSegments);
    )
    PERFORM_TIMING("Segment Correction",
        std::vector<AprilTag::Segment> segments;
        for (int i = 0; i < segmentsMat.rows; i++)
        {
            float x0 = segmentsMat.at<float>(i,0);
            float y0 = segmentsMat.at<float>(i,1);
            float x1 = segmentsMat.at<float>(i,2);
            float y1 = segmentsMat.at<float>(i,3);
            float theta = segmentsMat.at<float>(i,4);
            float length = AprilTag::MathUtil::distance2D(
                { x0, y0 },
                { x1, y1 }
            );
            std::pair<AprilTag::XYWeight, AprilTag::XYWeight> pointPair = {
                { x0, y0, 0 },
                { x1, y1, 0 }
            };
            reorientSegment(
                segments, 
                pointPair,
                length,
                theta
            );
        }
    )
    return detector.detect<uchar>(fimOrig, segments);
}

std::vector<AprilTag::Segment> ccaSegmentation(const cv::Mat& segPoints)
{
    std::vector<AprilTag::Segment> goodSegments;

    int i;
    // Iterate over matrix points in strides of 4
    for (i = 0; i < segPoints.rows; i += 4)
    {
        // Retrieve 4 points
        AprilTag::XYWeight minXPt(
            segPoints.at<float>(i,0),   // minXx
            segPoints.at<float>(i,1),   // minXy
            segPoints.at<float>(i,2)    // theta
        );
        AprilTag::XYWeight maxXPt(
            segPoints.at<float>(i+1,0),   // maxXx
            segPoints.at<float>(i+1,1),   // maxXy
            segPoints.at<float>(i+1,2)    // theta
        );
        AprilTag::XYWeight minYPt(
            segPoints.at<float>(i+2,0),   // minYx
            segPoints.at<float>(i+2,1),   // minYy
            segPoints.at<float>(i+2,2)    // theta
        );
        AprilTag::XYWeight maxYPt(
            segPoints.at<float>(i+3,0),   // maxYx
            segPoints.at<float>(i+3,1),   // maxYy
            segPoints.at<float>(i+3,2)    // theta
        );
        // Save lines in an array
        std::vector<std::pair<AprilTag::XYWeight, AprilTag::XYWeight>> linePts =
        {
            { maxXPt, minXPt },
            { maxXPt, minYPt },
            { maxXPt, maxYPt },
            { maxYPt, minXPt },
            { maxYPt, minYPt },
            { minXPt, minYPt },
        };
        // Calculate line lengths
        float longestLength = 0;
        std::pair<AprilTag::XYWeight, AprilTag::XYWeight> longestPointPair = {minXPt, minYPt};

        for (const std::pair<AprilTag::XYWeight, AprilTag::XYWeight>& ptPair: linePts)
        {
            float length = AprilTag::MathUtil::distance2D(
                { ptPair.first.x, ptPair.first.y },
                { ptPair.second.x, ptPair.second.y }
            );
            if (length > longestLength)
            {
                longestLength = length;
                longestPointPair = ptPair;
            }
        }
        // Check if the length fits within the bounds
        if (longestLength > 10 && longestLength < 630)
        {
            reorientSegment(goodSegments, longestPointPair, longestLength, maxXPt.weight);
        }
        // if no, continue
    }

    return goodSegments;
}

void reorientSegment(
    std::vector<AprilTag::Segment>& segments,
    std::pair<AprilTag::XYWeight, AprilTag::XYWeight>& pointPair,
    float length,
    float goldenTheta
)
{
    AprilTag::Segment seg;
    // if yes, find its direction and save
    float dx = pointPair.second.x - pointPair.first.x;
    float dy = pointPair.second.y - pointPair.first.y;
    float theta = atan2(dy, dx);

    float err = AprilTag::MathUtil::mod2pi(goldenTheta - theta);
    if (err > 0)
    {
        theta += PI;
    }

    float dot = dx * cos(theta) + dy * sin(theta);
    if (dot > 0)
    {
        seg.setX0(pointPair.second.x);
        seg.setY0(pointPair.second.y);
        seg.setX1(pointPair.first.x);
        seg.setY1(pointPair.first.y);
    }
    else
    {
        seg.setX0(pointPair.first.x);
        seg.setY0(pointPair.first.y);
        seg.setX1(pointPair.second.x);
        seg.setY1(pointPair.second.y);
    }

    seg.setTheta(theta);
    seg.setLength(length);

    segments.push_back(seg);
}

#if (PY_VERSION_HEX >= 0x03000000)

    static void *init_ar() {
#else
        static void init_ar(){
#endif
        Py_Initialize();

        import_array();
        return NUMPY_IMPORT_ARRAY_RETVAL;
    }

BOOST_PYTHON_MODULE(libpyboostapriltags)
{
    init_ar();

    to_python_converter<cv::Mat,matToNDArrayBoostConverter>();
    matFromNDArrayBoostConverter();

    def("print_detection", &print_detection);
    def("break_out_detection", &break_out_detection);
    def("draw_detection", &draw_detection);
    def("extractTags", &extractTags);
    def("extractMagThetaTags", &extractMagThetaTags);
    def("extractCCATags", &extractCCATags);
    def("extractSegmentedTags", &extractSegmentedTags);

    class_<CameraParameters>("CameraParameters")
        .def_readwrite("tagSize", &CameraParameters::tagSize)
        .def_readwrite("fx", &CameraParameters::fx)
        .def_readwrite("fy", &CameraParameters::fy)
        .def_readwrite("px", &CameraParameters::px)
        .def_readwrite("py", &CameraParameters::py)
    ;

    class_<std::pair<float, float>>("FloatPair")
        .def_readwrite("x", &std::pair<float, float>::first)
        .def_readwrite("y", &std::pair<float, float>::second)
    ;

    class_<std::vector<std::pair<float, float>>>("FloatPairVector")
        .def(vector_indexing_suite<std::vector<std::pair<float, float>>>())
    ;

    class_<DetectionData>("DetectionData")
        .def_readwrite("id", &DetectionData::id)
        .def_readwrite("hammingDistance", &DetectionData::hammingDistance)
        .def_readwrite("distance", &DetectionData::distance)
        .def_readwrite("corners", &DetectionData::corners)
        .def_readwrite("x", &DetectionData::x)
        .def_readwrite("y", &DetectionData::y)
        .def_readwrite("z", &DetectionData::z)
        .def_readwrite("roll", &DetectionData::roll)
        .def_readwrite("pitch", &DetectionData::pitch)
        .def_readwrite("yaw", &DetectionData::yaw)
    ;

    class_<AprilTag::TagCodes>("TagCodes", init<int, int, unsigned long long*, int>());

    class_<AprilTag::TagDetection>("TagDetection");

    class_<std::vector<AprilTag::TagDetection>>("TagDetectionVector")
        .def(vector_indexing_suite<std::vector<AprilTag::TagDetection>>())
    ;

    class_<AprilTag::TagDetector>("TagDetector", init<AprilTag::TagCodes>());

    class_<GlobalModule>("Globals")
        .def_readonly("tagCodes36h11", &GlobalModule::tagCodes36h11)
    ;
}

void print_detection(struct CameraParameters cp, AprilTag::TagDetection& detection)
{
    struct DetectionData data = break_out_detection(cp, detection);
    cout    << "  Id: " << data.id
            << " (Hamming: " << data.hammingDistance << ")"
            << "  distance=" << data.distance
            << "m, x=" << data.x
            << ", y=" << data.y
            << ", z=" << data.z
            << ", yaw=" << data.yaw
            << ", pitch=" << data.pitch
            << ", roll=" << data.roll
            << endl;
}

struct DetectionData break_out_detection(struct CameraParameters cp, AprilTag::TagDetection& detection)
{
    // recovering the relative pose of a tag:

    // NOTE: for this to be accurate, it is necessary to use the
    // actual camera parameters here as well as the actual tag size
    // (m_fx, m_fy, m_px, m_py, m_tagSize)
    PERFORM_TIMING("Pose Estimation",
        Eigen::Vector3d translation;
        Eigen::Matrix3d rotation;
        detection.getRelativeTranslationRotation(cp.tagSize, cp.fx, cp.fy, cp.px, cp.py,
                                                translation, rotation);

        Eigen::Matrix3d F;
        F <<
        1, 0,  0,
        0,  -1,  0,
        0,  0,  1;
        Eigen::Matrix3d fixed_rot = F*rotation;
        double yaw, pitch, roll;
        wRo_to_euler(fixed_rot, yaw, pitch, roll);
        double x, y, z;
        x = translation(0);
        y = translation(1);
        z = translation(2);
        double distance = translation.norm();
    )

    return {
        detection.id, detection.hammingDistance,
        {
            { detection.p[0].first, detection.p[0].second },
            { detection.p[1].first, detection.p[1].second },
            { detection.p[2].first, detection.p[2].second },
            { detection.p[3].first, detection.p[3].second },
        },
        distance, x, y, z,
        yaw, pitch, roll
    };
}

void draw_detection(cv::Mat& originalImage, AprilTag::TagDetection& detection)
{
    detection.draw(originalImage);
}

inline double standardRad(double t)
{
  if (t >= 0.) {
    t = fmod(t+PI, TWOPI) - PI;
  } else {
    t = fmod(t-PI, -TWOPI) + PI;
  }
  return t;
}

void wRo_to_euler(const Eigen::Matrix3d& wRo, double& yaw, double& pitch, double& roll)
{
    yaw = standardRad(atan2(wRo(1,0), wRo(0,0)));
    double c = cos(yaw);
    double s = sin(yaw);
    pitch = standardRad(atan2(-wRo(2,0), wRo(0,0)*c + wRo(1,0)*s));
    roll  = standardRad(atan2(wRo(0,2)*s - wRo(1,2)*c, -wRo(0,1)*s + wRo(1,1)*c));
}
