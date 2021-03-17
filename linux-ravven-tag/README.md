# Zynq 7020 SoC C/C++ Code Base
This code base contains the ravventag algroithm that is based on the HDL acceleration via
https://www.mathworks.com/matlabcentral/fileexchange/88066-mag-and-theta-with-hdl-coder?s_tid=srchtitle

## Requirements
### OpenCV 3.X
OpenCV for both Python and C++ is used extensively to provide fundamental
support for image and video processing.

### Boost.Python
Boost is a header library that provides additional functionality to C++.
The code base uses a compiled version of that library called Boost.Python
to provide a Python <=> C++ interface that is easy to use and extendable.
Boost.Python works for a wide range of python versions but we are using
Python 2.7 for everything.

#### Dependencies
Below are the following dependencies that should be installed prior to trying
to install Boost and Boost.Python.
```bash
sudo apt-get install python-dev cmake libopencv-dev libeigen3-dev libv4l-dev python-numpy python-opencv
```

#### Installation
Boost can be found [here](https://www.boost.org/). Navigate to the
[Getting Started](https://www.boost.org/doc/libs/1_75_0/more/getting_started/unix-variants.html)
section for unix systems which provides easy step by step instructions
to install Boost itself or use wget below to download the tar directly.

For compiling Boost.Python, run the following commands:
```bash
wget https://dl.bintray.com/boostorg/release/1.75.0/source/boost_1_75_0.tar.gz
tar -zxf boost_1_75_0.tar
cd path/to/boost_1_75_0
./bootstrap.sh --with-libraries=python --with-python=2.7
./b2 install
```
This will install the Boost.Python library into the default lib folders
so it is accessible to both Python and C++.

If there are any issues, make sure that all the dependencies are installed and
that you're running as a super user. The `./b2 install` will copy the necessary
headers onto the default path and put the compiled .so binary files to default
library path.

### Eigen Library
Coming soon! (Also a header library)

## Compilation
Compiling the code itself is very simple. It utilizes cmake to auto-generate
the Makefiles which are used to compile and build the various binaries and
libraries. Simply run `make` in the root directory of the project.

## Execution
Under the root directory is a `scripts` folder containing two Python scripts.
- AprilTagDetector.py: Main library to import when utilizing the underlying
C++ code. Abstracts away a lot of the complexity for an easy to use interface.
- python_exporter.py: Main testing script that exercises the expected
functionality of the Python/C++ library.

To run the script:
```bash
python3 python_exporter.py
```
**Note**, in the code are a lot of relative paths. This was hard-coded for
simplicity and will be cleaned up at a later date.

# Old README

AprilTags library

Detect April tags (2D bar codes) in images; reports unique ID of each
detection, and optionally its position and orientation relative to a
calibrated camera.

See examples/apriltags_demo.cpp for a simple example that detects
April tags (see tags/pdf/tag36h11.pdf) in laptop or webcam images and
marks any tags in the live image.

Ubuntu dependencies:
sudo apt-get install subversion cmake libopencv-dev libeigen3-dev libv4l-dev

Mac dependencies:
sudo port install pkgconfig opencv eigen3

Uses the pods build system in connection with cmake, see:
http://sourceforge.net/p/pods/

Michael Kaess
October 2012

----------------------------

AprilTags were developed by Professor Edwin Olson of the University of
Michigan.  His Java implementation is available on this web site:
  http://april.eecs.umich.edu.

Olson's Java code was ported to C++ and integrated into the Tekkotsu
framework by Jeffrey Boyland and David Touretzky.

See this Tekkotsu wiki article for additional links and references:
  http://wiki.tekkotsu.org/index.php/AprilTags

----------------------------

This C++ code was further modified by
Michael Kaess (kaess@mit.edu) and Hordur Johannson (hordurj@mit.edu)
and the code has been released under the LGPL 2.1 license.

- converted to standalone library
- added stable homography recovery using OpenCV
- robust tag code table that does not require a terminating 0
  (omission results in false positives by illegal codes being accepted)
- changed example tags to agree with Ed Olson's Java version and added
  all his other tag families
- added principal point as parameter as in original code - essential
  for homography
- added some debugging code (visualization using OpenCV to show
  intermediate detection steps)
- added fast approximation of arctan2 from Ed's original Java code
- using interpolation instead of homography in Quad: requires less
  homography computations and provides a small improvement in correct
  detections

todo:
- significant speedup could be achieved by performing image operations
  using OpenCV (Gaussian filter, but also operations in
  TagDetector.cc)
- replacing arctan2 by precomputed lookup table
- converting matrix operations to Eigen (mostly for simplifying code,
  maybe some speedup)

