#ifndef UTIL_H
#define UTIL_H

#include <sys/time.h>
#include <iostream>

#define PERFORM_TIMING(name, ...) \
    TIMING_VAR = tic(); \
    __VA_ARGS__ \
    std::cout << name " Time: " << tic() - TIMING_VAR << "s" << std::endl;

double TIMING_VAR;

// utility function to provide current system time (used below in
// determining frame rate at which images are being processed)
double tic() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return ((double)t.tv_sec + ((double)t.tv_usec)/1000000.);
}

#endif