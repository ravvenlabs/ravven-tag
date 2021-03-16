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
Python 3.6.5 for everything.

#### Installation
Boost can be found [here](https://www.boost.org/). Navigate to the
[Getting Started](https://www.boost.org/doc/libs/1_75_0/more/getting_started/unix-variants.html)
section for unix systems which provides easy step by step instructions
to install Boost itself.

For compiling Boost.Python, run the following commands:
```bash
cd path/to/boost_<boost_version>
./bootstrap.sh --with-libraries=python --with-python=3.6
./b2 install
```
This will install the Boost.Python library into the default lib folders
so it is accessible to both Python and C++.

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
