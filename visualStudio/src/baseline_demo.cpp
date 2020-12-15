#include <Windows.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <list>
#include <time.h>

#include <cmath>

#include "common.h"

// OpenCV library for easy access to USB camera and drawing of images
// on screen
#include "opencv2/opencv.hpp"
#include "baseline_demo.h"
#include "vs_apriltag.h"

#include "Tag16h5.h"
#include "Tag25h7.h"
#include "Tag25h9.h"
#include "Tag36h9.h"
#include "Tag36h11.h"

// default constructor
BaselineDemo::BaselineDemo(const char* name) :
    Demo(name),
    // default settings, most can be modified through command line options (see below)
    m_tagDetector(NULL),
    m_tagCodes(AprilTag::tagCodes36h11),

    m_draw(true),
    m_arduino(false),
    m_timing(false),

    m_width(640),
    m_height(480),
    m_tagSize(0.166),
    m_fx(600),
    m_fy(600),
    m_px(m_width / 2),
    m_py(m_height / 2),

    m_exposure(-1),
    m_gain(-1),
    m_brightness(-1),

    m_deviceId(0)
{}

// changing the tag family
void BaselineDemo::setTagCodes(string s) {
    if (s == "16h5") {
        m_tagCodes = AprilTag::tagCodes16h5;
    }
    else if (s == "25h7") {
        m_tagCodes = AprilTag::tagCodes25h7;
    }
    else if (s == "25h9") {
        m_tagCodes = AprilTag::tagCodes25h9;
    }
    else if (s == "36h9") {
        m_tagCodes = AprilTag::tagCodes36h9;
    }
    else if (s == "36h11") {
        m_tagCodes = AprilTag::tagCodes36h11;
    }
    else {
        cout << "Invalid tag family specified" << endl;
        exit(1);
    }
}

// parse command line options to change default behavior
void BaselineDemo::setup() {
    m_tagDetector = new AprilTag::TagDetector(m_tagCodes);

    m_imgNames.push_back(RELATIVE_IMG_INPUT_DIR "start.png");

    // prepare window for drawing the camera images
    if (m_draw) {
        cv::namedWindow(windowName, 1);
    }
}

void BaselineDemo::setupVideo() {
    // find and open a USB camera (built in laptop camera, web cam etc)
    m_cap = cv::VideoCapture(m_deviceId);
    if (!m_cap.isOpened()) {
        cerr << "ERROR: Can't find video device " << m_deviceId << "\n";
        exit(1);
    }
    m_cap.set(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH, m_width);
    m_cap.set(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT, m_height);
    cout << "Camera successfully opened (ignore error messages above...)" << endl;
    cout << "Actual resolution: "
        << m_cap.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH) << "x"
        << m_cap.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT) << endl;

}

void BaselineDemo::print_detection(AprilTag::TagDetection& detection) const {
    cout << "  Id: " << detection.id
        << " (Hamming: " << detection.hammingDistance << ")";

    // recovering the relative pose of a tag:

    // NOTE: for this to be accurate, it is necessary to use the
    // actual camera parameters here as well as the actual tag size
    // (m_fx, m_fy, m_px, m_py, m_tagSize)

    Eigen::Vector3d translation;
    Eigen::Matrix3d rotation;
    detection.getRelativeTranslationRotation(m_tagSize, m_fx, m_fy, m_px, m_py,
        translation, rotation);

    Eigen::Matrix3d F;
    F <<
        1, 0, 0,
        0, -1, 0,
        0, 0, 1;
    Eigen::Matrix3d fixed_rot = F * rotation;
    double yaw, pitch, roll;
    wRo_to_euler(fixed_rot, yaw, pitch, roll);

    cout << "  distance=" << translation.norm()
        << "m, x=" << translation(0)
        << ", y=" << translation(1)
        << ", z=" << translation(2)
        << ", yaw=" << yaw
        << ", pitch=" << pitch
        << ", roll=" << roll
        << endl;

    // Also note that for SLAM/multi-view application it is better to
    // use reprojection error of corner points, because the noise in
    // this relative pose is very non-Gaussian; see iSAM source code
    // for suitable factors.
}

void BaselineDemo::processImage(cv::Mat& image, cv::Mat& image_gray) {
    // alternative way is to grab, then retrieve; allows for
    // multiple grab when processing below frame rate - v4l keeps a
    // number of frames buffered, which can lead to significant lag
    //      m_cap.grab();
    //      m_cap.retrieve(image);

    // detect April tags (requires a gray scale image)
    cv::cvtColor(image, image_gray, cv::ColorConversionCodes::COLOR_BGR2GRAY);
    if (m_timing) {
        tic();
    }
    vector<AprilTag::TagDetection> detections = m_tagDetector->extractTags(image_gray);
    if (m_timing) {
        LPSYSTEMTIME dt = toc(1);
        cout << "Extracting tags took " << dt->wSecond << " seconds." << endl;
    }

    // print out each detection
    cout << detections.size() << " tags detected:" << endl;
    for (int i = 0; i < detections.size(); i++) {
        print_detection(detections[i]);
    }

    // show the current image including any detections
    if (m_draw) {
        for (int i = 0; i < detections.size(); i++) {
            // also highlight in the image
            detections[i].draw(image);
        }
        imshow(windowName, image); // OpenCV call
    }
}

// Load and process a single image
void BaselineDemo::loadImages() {
    cv::Mat image;
    cv::Mat image_gray;

    for (list<string>::iterator it = m_imgNames.begin(); it != m_imgNames.end(); it++) {
        image = cv::imread(*it); // load image with opencv
        processImage(image, image_gray);
        while (cv::waitKey(100) == -1) {}
    }
}

// Video or image processing?
bool BaselineDemo::isVideo() {
    return m_imgNames.empty();
}

// The processing loop where images are retrieved, tags detected,
// and information about detections generated
void BaselineDemo::loop() {
    cv::Mat image;
    cv::Mat image_gray;

    int frame = 0;
    tic();
    while (true) {
        // capture frame
        m_cap >> image;

        processImage(image, image_gray);

        // print out the frame rate at which image frames are being processed
        frame++;
        if (frame % 10 == 0) {
            LPSYSTEMTIME t = toc(1);
            cout << "  " << 10. / t->wSecond << " fps" << endl;
        }

        // exit if any key is pressed
        if (cv::waitKey(1) >= 0) break;
    }
}

void BaselineDemo::execute() {
    if (isVideo()) {
        cout << "Processing video" << endl;

        // setup image source, window for drawing, serial port...
        setupVideo();

        // the actual processing loop where tags are detected and visualized
        loop();

    }
    else {
        cout << "Processing image" << endl;

        // process single image
        loadImages();

    }
}
