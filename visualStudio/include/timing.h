#pragma once

#include <iostream>
#include <Windows.h>
#include <Eigen/Dense>
#include <cmath>

/* Returns the local start time. */
const LPSYSTEMTIME tic();

/**
/* Returns the elapsed time from the last tic.
*
* This function also prints out the elapsed time.
*/
const LPSYSTEMTIME toc(bool silence = 0, const char* message = "");
