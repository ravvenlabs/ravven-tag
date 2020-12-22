#pragma once

#include "Demo.h"

// April tags detector and various families that can be selected by command line option
//#include "TagDetector.h"

class BaselineDemo : public Demo {
    bool m_arduino; // send tag detections to serial port?

    int m_deviceId; // camera id (in case of multiple cameras)

    cv::VideoCapture m_cap;

    int m_exposure;
    int m_gain;
    int m_brightness;
public:
    BaselineDemo(const char* name, const char* inputDir);

    // parse command line options to change default behavior
    void setup();

    void execute();

    void setupVideo();

    // Video or image processing?
    bool isVideo();
    // The processing loop where images are retrieved, tags detected,
    // and information about detections generated
    void loop();

    void loadImages();

    void processImage(cv::Mat& image, cv::Mat& image_gray);
};
