#include "vr_math_utils.h"
#include <cmath>
#include <atomic>

void VRMathUtils::GetPose(vr::DriverPose_t& pose, const std::atomic<double> position[3], const std::atomic<double> rotation[4]) {
    pose = {};

    pose.qWorldFromDriverRotation.w = 1.f;
    pose.qDriverFromHeadRotation.w = 1.f;

    pose.qRotation.x = rotation[0];
    pose.qRotation.y = rotation[1];
    pose.qRotation.z = rotation[2];
    pose.qRotation.w = rotation[3];

    pose.vecPosition[0] = position[0];
    pose.vecPosition[1] = position[1];
    pose.vecPosition[2] = position[2];

    pose.poseIsValid = true;
    pose.deviceIsConnected = true;

    pose.result = vr::TrackingResult_Running_OK;

    pose.shouldApplyHeadModel = true;
}

void VRMathUtils::ToQuaternion(float pitch, float yaw, float roll, double* quat) {
    double cy = cos(yaw * 0.5);
    double sy = sin(yaw * 0.5);
    double cr = cos(roll * 0.5);
    double sr = sin(roll * 0.5);
    double cp = cos(pitch * 0.5);
    double sp = sin(pitch * 0.5);

    quat[0] = -cy * cr * sp - sy * sr * cp;
    quat[1] = -sy * cr * cp + cy * sr * sp;
    quat[2] =  cy * sr * cp - sy * cr * sp;
    quat[3] =  cy * cr * cp + sy * sr * sp;
}
