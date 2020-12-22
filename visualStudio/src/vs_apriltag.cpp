#include <Windows.h>
#include <iostream>

#include "Demo.h"
/* Include different versions here */
#include "baseline_demo.h"
#include <vs_apriltag.h>

typedef struct
{
    const char* demo_name;
    Demo* demo;
} DemoItem;

int main(int argc, char* argv[])
{
    Demo* demo;
    int user_input;
    /* Do not move outside. Doesn't work if you do. No idea why. */
    DemoControls* withVisualExports = new DemoControls();
    withVisualExports->draw = true;
    DemoControls* withTiming = new DemoControls();
    withTiming->timing = true;

    DemoItem demos[] =
    {
        {"Baseline Demo", new BaselineDemo("Baseline", RELATIVE_IMG_INPUT_DIR, new DemoControls())},
        {"Baseline With Visuals Demo", new BaselineDemo("Baseline", RELATIVE_IMG_INPUT_DIR, withVisualExports)},
        {"Baseline With Timing Demo", new BaselineDemo("Baseline", RELATIVE_IMG_INPUT_DIR, withTiming)},
    };

    std::cout << "Select one of the following demos:" << std::endl;

    for (int i = 1; i < (sizeof(demos) / sizeof(DemoItem)) + 1; i++)
    {
        std::cout << i << ") " << demos[i - 1].demo_name << std::endl;
    }

    user_input = getchar() - '1';

    std::cout << "Executing " << demos[user_input].demo_name << "..." << std::endl;

    demo = demos[user_input].demo;

    demo->setup();

    demo->execute();
    return 0;
}
