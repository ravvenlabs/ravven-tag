#pragma once

class DemoControls {
    public:
        bool timing; // Enables timing logic when executing a demo
        bool draw; // Outputs intermediate steps as images to the screen
        bool save; // Saves the intermediate stpes to RELATIVE_IMG_OUTPUT_DIR/Step#.png
};
