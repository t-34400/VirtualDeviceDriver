#pragma once

#include <atomic>
#include <openvr_driver.h>

class VRMathUtils {
public:
    static void GetPose(vr::DriverPose_t& pose, const std::atomic<double> position[3], const std::atomic<double> rotation[4]);
    static void ToQuaternion(float pitch, float yaw, float roll, double* quat);
};