// Expose internals to Python
#define PY_ARRAY_UNIQUE_SYMBOL pbcvt_ARRAY_API

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <pyboostcvconverter/pyboostcvconverter.hpp>
#include "AprilTags/TagDetector.h"
#include "AprilTags/TagDetection.h"
#include "AprilTags/Tag36h11.h"

using namespace boost::python;
using namespace pbcvt;

#define STAMP(x) std::cout << x << std::endl

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

// Tag extraction functions
std::vector<AprilTag::TagDetection> extractTags(AprilTag::TagDetector& detector, PyObject* im_in)
{
    cv::Mat image;
    image = pbcvt::fromNDArrayToMat(im_in);
    cv::cvtColor(image, image, CV_BGR2GRAY);
    return detector.extractTags(image);
}

std::vector<AprilTag::TagDetection> extractMagThetaTags(
    AprilTag::TagDetector& detector,
    PyObject* arrOrig, PyObject* arrMag, PyObject* arrTheta
)
{
    cv::Mat start, mag, theta;
    start = pbcvt::fromNDArrayToMat(arrOrig);
    mag = pbcvt::fromNDArrayToMat(arrMag);
    theta = pbcvt::fromNDArrayToMat(arrTheta);
    /* Pre-processing */
    // Convert to grayscale
    cv::cvtColor(start, start, CV_BGR2GRAY);
    cv::cvtColor(mag, mag, CV_BGR2GRAY);
    cv::cvtColor(theta, theta, CV_BGR2GRAY);
    // Normalize
    start.convertTo(start, CV_32FC1, (1. / 255.));
    mag.convertTo(mag, CV_32FC1, (1. / 255.));
    theta.convertTo(theta, CV_32FC1, ((2. * PI) / 255.), -PI);
    return detector.extractMagThetaTags(start, mag, theta);
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

    return {
        detection.id, detection.hammingDistance,
        {
            { detection.p[0].first, detection.p[0].second },
            { detection.p[1].first, detection.p[1].second },
            { detection.p[2].first, detection.p[2].second },
            { detection.p[3].first, detection.p[3].second },
        },
        translation.norm(), translation(0), translation(1), translation(2),
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
