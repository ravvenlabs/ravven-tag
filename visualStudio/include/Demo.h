#pragma once
#include <TagDetector.h>
#include <Tags.h>
#include <filesystem>
#include "DemoControls.h"

namespace fs = std::filesystem;

class Demo {
    protected:
        // Internal variables
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
        DemoControls* controls;

        // default constructor
        Demo(const char* name, const char* inputDir, DemoControls* ctrl) :
            windowName(name),
            inputImgDir(inputDir),
            tagCodes(AprilTag::tagCodes36h11),
            controls(ctrl)
        {
            for (const auto& entry : fs::directory_iterator(inputDir))
            {
                if (!entry.is_directory())
                {
                    imgNames.push_back(entry.path().string());
                }
            }
        }

        // parse command line options to change default behavior
        virtual void setup() = 0;

        // Run the demo
        virtual void execute() = 0;

        void setTagCodes(std::string code);

        void printDetection(AprilTag::TagDetection& detection) const;
}; // Demo
