#include "virtual_tracking_device_driver.h"

#include <cstring>
#include <memory>

#include "driverlog.h"

#include "driver_settings.h"
#include "vr_math_utils.h"

VirtualTrackingDeviceDriver::VirtualTrackingDeviceDriver()
{
    DriverLog("VirtualTrackingDeviceDriver created.");
}

vr::EVRInitError VirtualTrackingDeviceDriver::Activate( uint32_t unObjectId )
{
    device_index = unObjectId;

    return vr::VRInitError_None;
}

void VirtualTrackingDeviceDriver::Deactivate()
{
    device_index = vr::k_unTrackedDeviceIndexInvalid;

    DriverLog("VirtualTrackingDeviceDriver deactivated.");
}

void VirtualTrackingDeviceDriver::EnterStandby()
{
    DriverLog("VirtualTrackingDeviceDriver entered standby.");
}

void *VirtualTrackingDeviceDriver::GetComponent( const char *pchComponentNameAndVersion )
{
    return nullptr;
}

void VirtualTrackingDeviceDriver::DebugRequest( const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize )
{
    if (unResponseBufferSize >= 1)
    {
        pchResponseBuffer[0] = 0;
    }
}

vr::DriverPose_t VirtualTrackingDeviceDriver::GetPose()
{
    vr::DriverPose_t pose = { 0 };
    VRMathUtils::GetPose(pose, position, rotation);

	return pose;    
}

void VirtualTrackingDeviceDriver::Update()
{
    bool updated = poseIsUpdate.exchange(false);

    if (!updated)
    {
        return;
    }

    vr::VRServerDriverHost()->TrackedDevicePoseUpdated(device_index, GetPose(), sizeof(vr::DriverPose_t));
}

void VirtualTrackingDeviceDriver::SetPosition(float x, float y, float z)
{
    position[0] = x;
    position[1] = y;
    position[2] = -z;

    poseIsUpdate = true;
}

void VirtualTrackingDeviceDriver::SetRotation(float x, float y, float z)
{
    double quat[4];
    VRMathUtils::ToQuaternion(x, y, z, quat);

    rotation[0] = quat[0];
    rotation[1] = quat[1];
    rotation[2] = quat[2];
    rotation[3] = -quat[3];

    poseIsUpdate = true;
}