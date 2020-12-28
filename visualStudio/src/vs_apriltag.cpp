#include <Windows.h>
#include <iostream>

#include "Demo.h"
/* Include different versions here */
#include "baseline_demo.h"
#include "CordicDemo.h"
#include <vs_apriltag.h>

typedef struct
{
    const char* demo_name;
    Demo* demo;
} DemoItem;

int main(int argc, char* argv[])
{
    Demo* demo;
    int user_input = -1;
    int i;
    /* Do not move outside. Doesn't work if you do. No idea why. */
    DemoControls* nonActive = new DemoControls();
    DemoControls* withVisualExports = new DemoControls();
    withVisualExports->draw = true;
    DemoControls* withTiming = new DemoControls();
    withTiming->timing = true;
    DemoControls* withSavedData = new DemoControls();
    withSavedData->save = true;

    DemoItem demos[] =
    {
        {"Baseline Demo", new BaselineDemo("Baseline", RELATIVE_IMG_INPUT_DIR, nonActive)},
        {"Baseline With Visuals Demo", new BaselineDemo("Baseline", RELATIVE_IMG_INPUT_DIR, withVisualExports)},
        {"Baseline With Timing Demo", new BaselineDemo("Baseline", RELATIVE_IMG_INPUT_DIR, withTiming)},
        {"Baseline With Saved Data Demo", new BaselineDemo("Baseline", RELATIVE_IMG_INPUT_DIR, withSavedData)},
        {"Cordic Demo", new CordicDemo("Cordic", RELATIVE_IMG_INPUT_DIR, nonActive)},
        {"Cordic With Visuals Demo", new CordicDemo("Cordic", RELATIVE_IMG_INPUT_DIR, withVisualExports)},
        {"Cordic With Timing Demo", new CordicDemo("Cordic", RELATIVE_IMG_INPUT_DIR, withTiming)},
        {"Cordic With Saved Data Demo", new CordicDemo("Cordic", RELATIVE_IMG_INPUT_DIR, withSavedData)},
    };

    while (user_input != (sizeof(demos) / sizeof(DemoItem)) + 1)
    {
        std::cout << "Select one of the following demos:" << std::endl;

        for (i = 1; i < (sizeof(demos) / sizeof(DemoItem)) + 1; i++)
        {
            std::cout << (char) (i + '0') << ") " << demos[i - 1].demo_name << std::endl;
        }
        std::cout << i << ") Quit" << std::endl;

        user_input = getchar() - '1';

        if (user_input == i - 1)
        {
            std::cout << "Quitting..." << std::endl;
            break;
        }

        std::cout << "Executing " << demos[user_input].demo_name << "..." << std::endl;

        demo = demos[user_input].demo;

        demo->setup();

        demo->execute();

        std::cout << "Finished executing " << demos[user_input].demo_name << std::endl;
        std::cin.clear();
        std::cin.ignore(INT_MAX, '\n');
    }
    return 0;
}
