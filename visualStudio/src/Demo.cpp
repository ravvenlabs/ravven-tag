#include "..\include\Demo.h"
#include <common.h>

void Demo::setTagCodes(std::string code)
{
    if (code == "16h5") {
        tagCodes = AprilTag::tagCodes16h5;
    }
    else if (code == "25h7") {
        tagCodes = AprilTag::tagCodes25h7;
    }
    else if (code == "25h9") {
        tagCodes = AprilTag::tagCodes25h9;
    }
    else if (code == "36h9") {
        tagCodes = AprilTag::tagCodes36h9;
    }
    else if (code == "36h11") {
        tagCodes = AprilTag::tagCodes36h11;
    }
    else {
        std::cout << "Invalid tag family specified" << std::endl;
        exit(1);
    }
}

void Demo::printDetection(AprilTag::TagDetection& detection) const
{
    std::cout << "  Id: " << detection.id
        << " (Hamming: " << detection.hammingDistance << ")";

    // recovering the relative pose of a tag:

    // NOTE: for this to be accurate, it is necessary to use the
    // actual camera parameters here as well as the actual tag size
    // (m_fx, m_fy, m_px, m_py, m_tagSize)

    Eigen::Vector3d translation;
    Eigen::Matrix3d rotation;
    detection.getRelativeTranslationRotation(tagSize, fx, fy, px, py,
        translation, rotation);

    Eigen::Matrix3d F;
    F <<
        1, 0, 0,
        0, -1, 0,
        0, 0, 1;
    Eigen::Matrix3d fixed_rot = F * rotation;
    double yaw, pitch, roll;
    wRo_to_euler(fixed_rot, yaw, pitch, roll);

    std::cout << "  distance=" << translation.norm()
        << "m, x=" << translation(0)
        << ", y=" << translation(1)
        << ", z=" << translation(2)
        << ", yaw=" << yaw
        << ", pitch=" << pitch
        << ", roll=" << roll
        << std::endl;

    // Also note that for SLAM/multi-view application it is better to
    // use reprojection error of corner points, because the noise in
    // this relative pose is very non-Gaussian; see iSAM source code
    // for suitable factors.
}
