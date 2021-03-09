#include "OnOffRampDemo.h"
#include "OnOffRampTagDetector.h"
#include "util.h"

void OnOffRampDemo::setup()
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

void OnOffRampDemo::execute()
{
    std::string user_input;
    std::cout << "Assign on-off ramps to selected image(s) " << std::endl;

    std::cout << "Starting Step: ";
    std::getline(std::cin, user_input);
    controls->onRamp = stof(user_input);
    std::cout << "End Step: ";
    std::getline(std::cin, user_input);
    controls->offRamp = stof(user_input);

    loadImages();
}

void OnOffRampDemo::loadImages()
{
    std::string user_input;
    std::string additionalInputImgDir;
    std::list<std::string> additionalInputImageNames;
    int key;
    cv::Mat baseImage;
    cv::Mat image1;
    cv::Mat image2;

    std::list<std::string>::iterator it;

    additionalInputImageNames = imgNames;

    std::cout << "Set directory for additional input images..." << std::endl;
    std::cout << "Current directory: " << inputImgDir;

    std::getline(std::cin, user_input);
    additionalInputImgDir = inputImgDir + user_input;

    for (const auto& entry : fs::directory_iterator(additionalInputImgDir))
    {
        if (!entry.is_directory())
        {
            additionalInputImageNames.push_back(entry.path().string());
        }
    }
    it = additionalInputImageNames.begin();

    baseImage = cv::imread(*it);
    it++;
    if (additionalInputImageNames.size() > 3)
    {
        std::cout << "Only supports up to 3 additional images (for mag and theta)." << std::endl;
        std::cout << "Given: " << imgNames.size() << std::endl;
        return;
    }
    else if (additionalInputImageNames.size() == 3)
    {
        std::cout << "Is the magnitude/ix image first (0) or second (1)?" << std::endl;
        std::cout << "First: " << *it << std::endl;
        std::getline(std::cin, user_input);

        if (stoi(user_input) == 0)
        {
            image1 = read_csvf(*it);
            it++;
            image2 = read_csvf(*it);
        }
        else
        {
            image2 = read_csvf(*it);
            it++;
            image1 = read_csvf(*it);
        }
    }
    else
    {
        image1 = read_csvf(*it);
        image2 = read_csvf(*it);
    }

    processImage({ baseImage, image1, image2 });
    key = cv::waitKey(100);
    DO_IF_DRAW_BEGIN
        while ((key = cv::waitKey(100)) == -1) {}
    cv::destroyAllWindows();
    DO_IF_DRAW_END
}

void OnOffRampDemo::processImage(OnOffRampTagDetector::ExtractionsInput input)
{
    cv::cvtColor(input.baseImage, input.baseImage, cv::ColorConversionCodes::COLOR_BGR2GRAY);
    //cv::cvtColor(input.image1, input.image1, cv::ColorConversionCodes::COLOR_BGR2GRAY);
    //cv::cvtColor(input.image2, input.image2, cv::ColorConversionCodes::COLOR_BGR2GRAY);

    std::vector<AprilTag::TagDetection> detections;
    DO_TIMING_IF_ENABLED(1, "Tag Extraction",
        AprilTag::TagFamily tagFamily = AprilTag::TagFamily(tagCodes);
        detections = OnOffRampTagDetector::extractTags(input, tagFamily, controls);
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
            detections[i].draw(input.baseImage);
        }
    imshow(windowName, input.baseImage); // OpenCV call
    DO_IF_DRAW_END
}
