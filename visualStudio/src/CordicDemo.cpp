#include "CordicDemo.h"
#include "CordicTagDetector.h"
#include "util.h"

void CordicDemo::setup()
{
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

void CordicDemo::execute()
{
    loadImages();
}

void CordicDemo::loadImages()
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

void CordicDemo::processImage(cv::Mat& image, cv::Mat& image_gray)
{
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
    detections = CordicTagDetector::extractTags(image_gray, tagFamily, controls);
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
