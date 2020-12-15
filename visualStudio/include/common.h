#pragma once

#include <Eigen/Dense>
#include "timing.h"

/**
 * Normalize angle to be within the interval [-pi,pi].
 */
inline double standardRad(double t);

/**
 * Convert rotation matrix to Euler angles
 */
void wRo_to_euler(const Eigen::Matrix3d& wRo, double& yaw, double& pitch, double& roll);
