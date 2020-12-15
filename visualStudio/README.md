# Visual Studio April-Tag

The following Visual Studio solution was made with Visual Studio 2019 and
adapting the linux-c-code source files in the root of this project. Most of the
work was completed by fixing include paths and getting the correct libraries
but a lot of work was also done simply refactoring the code.

## Additional Dependencies

Aside from the normal dependencies required (such as OpenCV), the April Tag
algorithm also requires a template library called [Eigen](http://eigen.tuxfamily.org/index.php?title=Main_Page). Installation of that library is as easy as downloading and 
unzipping the project to the desired directory. By default, the vs_apriltag
solution will look for the <b>Eigen</b> library in the C:\Opencv-Cuda directory.
For this project, <b>Eigen</b> 3.3.7 was used and verified to be working. Since
it is a header library, no compiling is required and can be used out of the box.

## Testing

There is a main.cpp in the example folder found in the root directory.
Currently, there is a hard-coded file path to the april tag image(s) that can
be used to verify the solution is built correctly. Using the april tag image
found in the example_april_tags folder, the program should produce as seen in
the image below.

![](res\example_fig.PNG)
