#pragma once

#include <iostream>
#include <Windows.h>
#include <Eigen/Dense>
#include <cmath>

/* Returns the local start time. */
LPSYSTEMTIME tic();

/**
/* Returns the elapsed time from the last tic.
*
* This function also prints out the elapsed time.
*/
LPSYSTEMTIME toc(unsigned int silence = 0, const char* message = "");
