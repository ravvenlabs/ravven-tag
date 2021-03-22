# Visual Studio April-Tag

The following Visual Studio solution was made with Visual Studio 2019 and
adapting the linux-c-code source files in the root of this project. Most of the
work was completed by fixing include paths and getting the correct libraries
but a lot of work was also done simply refactoring the code.

## Directory Breakdown

The folders below are described by their intended purposes and where one may
find specific files.

- dll: Contains external libraries/dlls that are linked together with the final
binary.
- images: Contains input and output images for the project to execute with.
- images/input: Contains the input images for all executions of the project. The
project will read ALL images in this directory to perform the apriltag
algorithm.
- images/output: Contains (or will contain) the output images for a specific
execution if the option is enabled. They will be broken down further into the
demo name and the files will be labeled with the step of execution. Output is
currently set as text files so they can loaded easily into matlab for further
testing.
- include: Contains all the header files for this project.
- lib: Contains any additional libraries that are to be compiled into the final
binary.
- res: A resource folder for any documentation.
- solutions: Contains the various projects for this visual studio solution.
- src: Contains all the source files for this project.

## Testing

There is a main.cpp in the example folder found in the root directory.
Currently, there is a hard-coded file path to the april tag image(s) that can
be used to verify the solution is built correctly. Using the april tag image
found in the example_april_tags folder, the program should produce as seen in
the image below.

![](res\example_fig.PNG)
