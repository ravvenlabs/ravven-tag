#pragma once
#include <TagDetector.h>
#include <Tags.h>
#include <filesystem>

namespace fs = std::filesystem;

class Demo {
    protected:
        // Internal variables
        AprilTag::TagDetector* tagDetector;
        AprilTag::TagCodes tagCodes;
        int width; // image size in pixels
        int height;
        double tagSize; // April tag side length in meters of square black frame
        double fx; // camera focal length in pixels
        double fy;
        double px; // camera principal point
        double py;
        std::list<std::string> imgNames;

    public:
        // Constants set at initialization
        const char* windowName;
        const char* inputImgDir;
        // Control flags
        bool timing; // Compute timings for key points of the algorithm
        bool draw; // Draw intermediate steps to the screen

        // default constructor
        Demo(const char* name, const char* inputDir) :
            windowName(name),
            inputImgDir(inputDir),
            tagDetector(NULL),
            tagCodes(AprilTag::tagCodes36h11)
        {
            for (const auto& entry : fs::directory_iterator(inputDir))
            {
                imgNames.push_back(entry.path().string());
            }
        }

        // parse command line options to change default behavior
        virtual void setup() = 0;

        // Run the demo
        virtual void execute() = 0;

        void setTagCodes(std::string code);

        void printDetection(AprilTag::TagDetection& detection) const;
}; // Demo
