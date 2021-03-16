# Boost.Python project configuration
SET(ENV{BOOST_ROOT} "/usr/local/boost_1_75_0")

#substitute your version number
find_package(Boost 1.75 EXACT COMPONENTS python36 REQUIRED)

INCLUDE_DIRECTORIES("${Boost_INCLUDE_DIRS}" "/usr/include/python3.6/" "./include/")

file(GLOB PYTHON_BOOST_SRC "./src/*")
ADD_LIBRARY(pyboostapriltags SHARED ${PYTHON_BOOST_SRC})
set_property(TARGET pyboostapriltags PROPERTY POSITION_INDEPENDENT_CODE ON)
TARGET_LINK_LIBRARIES(pyboostapriltags ${Boost_LIBRARIES} apriltags)
pods_install_libraries(pyboostapriltags)
