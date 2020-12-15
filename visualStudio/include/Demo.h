#pragma once

class Demo {
    public:
        const char* windowName;
        // default constructor
        Demo(const char* name) : windowName(name) {}

        // parse command line options to change default behavior
        virtual void setup() = 0;

        // Run the demo
        virtual void execute() = 0;
}; // Demo

