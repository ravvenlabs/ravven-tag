using namespace std;

#include <Windows.h>
#include <iostream>

#include "Demo.h"
/* Include different versions here */
#include "baseline_demo.h"

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
    DemoItem demos[] =
    {
        {"Baseline Demo", new BaselineDemo("Simple Test")},
    };

    cout << "Select one of the following demos:" << endl;

    for (int i = 1; i < (sizeof(demos) / sizeof(DemoItem)) + 1; i++)
    {
        cout << i << ") " << demos[i - 1].demo_name << endl;
    }

    user_input = getchar() - '1';

    cout << "Executing " << demos[user_input].demo_name << "..." << endl;

    demo = demos[user_input].demo;

    demo->setup();

    demo->execute();
    return 0;
}
