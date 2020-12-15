#pragma once

#include "Demo.h"

// April tags detector and various families that can be selected by command line option
#include "TagDetector.h"

class BaselineDemo : public Demo {
    AprilTag::TagDetector* m_tagDetector;
    AprilTag::TagCodes m_tagCodes;

    bool m_draw; // draw image and April tag detections?
    bool m_arduino; // send tag detections to serial port?
    bool m_timing; // print timing information for each tag extraction call

    int m_width; // image size in pixels
    int m_height;
    double m_tagSize; // April tag side length in meters of square black frame
    double m_fx; // camera focal length in pixels
    double m_fy;
    double m_px; // camera principal point
    double m_py;

    int m_deviceId; // camera id (in case of multiple cameras)

    list<string> m_imgNames;

    cv::VideoCapture m_cap;

    int m_exposure;
    int m_gain;
    int m_brightness;
public:

    // default constructor
    BaselineDemo(const char* name);

    // changing the tag family
    void setTagCodes(string s);

    // parse command line options to change default behavior
    void setup();

    void execute();

    void setupVideo();

    void print_detection(AprilTag::TagDetection& detection) const;

    void processImage(cv::Mat& image, cv::Mat& image_gray);

    // Load and process a single image
    void loadImages();

    // Video or image processing?
    bool isVideo();
    // The processing loop where images are retrieved, tags detected,
    // and information about detections generated
    void loop();

};
