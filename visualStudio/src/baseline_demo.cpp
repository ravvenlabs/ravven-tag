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
#include "BaselineTagDetector.h"
#include "vs_apriltag.h"
#include "util.h"

#include "Tags.h"

// default constructor
BaselineDemo::BaselineDemo(const char* name, const char* inputDir, DemoControls* ctrl) :
    Demo(name, inputDir, ctrl),
    // default settings, most can be modified through command line options (see below)
    m_arduino(false),

    m_exposure(-1),
    m_gain(-1),
    m_brightness(-1),

    m_deviceId(0)
{}

// parse command line options to change default behavior
void BaselineDemo::setup() {
    // Params specific to this demo
    width = 640;
    height = 480;
    tagSize = 0.166;
    fx = 600;
    fy = 600;
    px = width / 2;
    py = height / 2;

    // prepare window for drawing the camera images
    if (controls->draw) {
        cv::namedWindow(windowName, 1);
    }
}

void BaselineDemo::setupVideo() {
    // find and open a USB camera (built in laptop camera, web cam etc)
    m_cap = cv::VideoCapture(m_deviceId);
    if (!m_cap.isOpened()) {
        std::cerr << "ERROR: Can't find video device " << m_deviceId << "\n";
        exit(1);
    }
    m_cap.set(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH, width);
    m_cap.set(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT, height);
    std::cout << "Camera successfully opened (ignore error messages above...)" << std::endl;
    std::cout << "Actual resolution: "
        << m_cap.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH) << "x"
        << m_cap.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT) << std::endl;

}

// Video or image processing?
bool BaselineDemo::isVideo() {
    return imgNames.empty();
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
            std::cout << "  " << 10. / t->wSecond << " fps" << std::endl;
        }

        // exit if any key is pressed
        if (cv::waitKey(1) >= 0) break;
    }
}

void BaselineDemo::loadImages()
{
    int key;
    cv::Mat image;
    cv::Mat image_gray;

    for (std::list<std::string>::iterator it = imgNames.begin(); it != imgNames.end(); it++) {
        image = cv::imread(*it); // load image with opencv
        processImage(image, image_gray);
        key = cv::waitKey(100);
        DO_IF_DRAW_BEGIN
        while ((key = cv::waitKey(100)) == -1) {}
        cv::destroyAllWindows();
        DO_IF_DRAW_END
    }
}

void BaselineDemo::processImage(cv::Mat& image, cv::Mat& image_gray) {
    // alternative way is to grab, then retrieve; allows for
    // multiple grab when processing below frame rate - v4l keeps a
    // number of frames buffered, which can lead to significant lag
    //      m_cap.grab();
    //      m_cap.retrieve(image);

    // detect April tags (requires a gray scale image)
    cv::cvtColor(image, image_gray, cv::ColorConversionCodes::COLOR_BGR2GRAY);

    std::vector<AprilTag::TagDetection> detections;
    DO_TIMING_IF_ENABLED(1, "Tag Extraction",
        AprilTag::TagFamily tagFamily = AprilTag::TagFamily(tagCodes);
        detections = BaselineTagDetector::extractTags(image_gray, tagFamily, controls);
    )

    // print out each detection
    std::cout << detections.size() << " tags detected:" << std::endl;
    for (int i = 0; i < detections.size(); i++) {
        printDetection(detections[i]);
    }

    // show the current image including any detections
    DO_IF_DRAW_BEGIN
        for (int i = 0; i < detections.size(); i++) {
            // also highlight in the image
            detections[i].draw(image);
        }
        imshow(windowName, image); // OpenCV call
    DO_IF_DRAW_END
}

void BaselineDemo::execute() {
    if (isVideo()) {
        std::cout << "Processing video" << std::endl;

        // setup image source, window for drawing, serial port...
        setupVideo();

        // the actual processing loop where tags are detected and visualized
        loop();

    }
    else {
        std::cout << "Processing images" << std::endl;

        // process single image
        loadImages();

        std::cout << "Finished processing images" << std::endl;
    }
}
