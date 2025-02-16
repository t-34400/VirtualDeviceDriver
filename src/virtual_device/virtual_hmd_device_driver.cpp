#include "virtual_hmd_device_driver.h"

#include <cstring>
#include <memory>

#include "driverlog.h"

#include "driver_settings.h"
#include "vr_math_utils.h"

VirtualHMDDisplayComponent::VirtualHMDDisplayComponent(uint32_t width, uint32_t height)
    : window_width(width), window_height(height) 
{
    DriverLog("VirtualHMDDisplayComponent created with width %d and height %d", width, height);
}

void VirtualHMDDisplayComponent::GetWindowBounds( int32_t *pnX, int32_t *pnY, uint32_t *pnWidth, uint32_t *pnHeight )
{
    *pnX = 0;
    *pnY = 0;
    *pnWidth = window_width;
    *pnHeight = window_height;
}

bool VirtualHMDDisplayComponent::IsDisplayOnDesktop()
{
    return false;
}

bool VirtualHMDDisplayComponent::IsDisplayRealDisplay()
{
    return false;
}

void VirtualHMDDisplayComponent::GetRecommendedRenderTargetSize( uint32_t *pnWidth, uint32_t *pnHeight )
{
    *pnWidth = window_width;
    *pnHeight = window_height;
}

void VirtualHMDDisplayComponent::GetEyeOutputViewport( vr::EVREye eEye, uint32_t *pnX, uint32_t *pnY, uint32_t *pnWidth, uint32_t *pnHeight )
{
    if (eEye == vr::Eye_Left)
    {
        *pnX = 0;
        *pnY = 0;
        *pnWidth = window_width;
        *pnHeight = window_height;
    }
    else if (eEye == vr::Eye_Right)
    {
        *pnX = window_width;
        *pnY = 0;
        *pnWidth = 0;
        *pnHeight = 0;
    }
}

void VirtualHMDDisplayComponent::GetProjectionRaw( vr::EVREye eEye, float *pfLeft, float *pfRight, float *pfTop, float *pfBottom )
{
    float aspectRatio = static_cast<float>(window_width) / static_cast<float>(window_height);

    *pfLeft = -1.0f;
    *pfRight = 1.0f;

    *pfTop = -1.0f / aspectRatio;
    *pfBottom = 1.0f / aspectRatio;
}

vr::DistortionCoordinates_t VirtualHMDDisplayComponent::ComputeDistortion( vr::EVREye eEye, float fU, float fV )
{
    vr::DistortionCoordinates_t coordinates{};
    coordinates.rfBlue[ 0 ] = fU;
    coordinates.rfBlue[ 1 ] = fV;
    coordinates.rfGreen[ 0 ] = fU;
    coordinates.rfGreen[ 1 ] = fV;
    coordinates.rfRed[ 0 ] = fU;
    coordinates.rfRed[ 1 ] = fV;
    return coordinates;        
}

bool VirtualHMDDisplayComponent::ComputeInverseDistortion( vr::HmdVector2_t *pResult, vr::EVREye eEye, uint32_t unChannel, float fU, float fV )
{
    *pResult = { fU, fV };
    return true;
}

VirtualHMDDeviceDriver::VirtualHMDDeviceDriver()
{
    uint32_t window_width = static_cast<uint32_t>(vr::VRSettings()->GetInt32(kDriverSettingsSection, "window_width"));
    uint32_t window_height = static_cast<uint32_t>(vr::VRSettings()->GetInt32(kDriverSettingsSection, "window_height"));

    if (window_width <= 0) {
        window_width = 1920;
        DriverLog("Invalid window width found in VR settings. Using default value: 1920.");
    }        
    if (window_height <= 0) {
        window_height = 1080;
        DriverLog("Invalid window height found in VR settings. Using default value: 1080.");
    }

    vr::VRSettings()->SetInt32(kDriverSettingsSection, "window_width", static_cast<int32_t>(window_width));
    vr::VRSettings()->SetInt32(kDriverSettingsSection, "window_height", static_cast<int32_t>(window_height));

    DriverLog("Virtual HMD display component created with dimensions: %d x %d.", window_width, window_height);

    display_component = std::make_unique<VirtualHMDDisplayComponent>(window_width, window_height);

    DriverLog("VirtualHMDDeviceDriver created.");
}

vr::EVRInitError VirtualHMDDeviceDriver::Activate( uint32_t unObjectId )
{
    device_index = unObjectId;

    vr::PropertyContainerHandle_t container = vr::VRProperties()->TrackedDeviceToPropertyContainer( device_index );

    vr::VRProperties()->SetFloatProperty( container, vr::Prop_DisplayFrequency_Float, 0.f );
    vr::VRProperties()->SetFloatProperty( container, vr::Prop_UserHeadToEyeDepthMeters_Float, 0.f );
    vr::VRProperties()->SetFloatProperty( container, vr::Prop_SecondsFromVsyncToPhotons_Float, 0.11f );
	vr::VRProperties()->SetBoolProperty( container, vr::Prop_IsOnDesktop_Bool, false );
	vr::VRProperties()->SetBoolProperty(container, vr::Prop_DisplayDebugMode_Bool, true);

    DriverLog("VirtualHMDDeviceDriver activated.");
    return vr::VRInitError_None;
}

void VirtualHMDDeviceDriver::Deactivate()
{
    device_index = vr::k_unTrackedDeviceIndexInvalid;

    DriverLog("VirtualHMDDeviceDriver deactivated.");
}

void VirtualHMDDeviceDriver::EnterStandby()
{
    DriverLog("VirtualHMDDeviceDriver entered standby.");
}

void *VirtualHMDDeviceDriver::GetComponent( const char *pchComponentNameAndVersion )
{
    if (!_stricmp(pchComponentNameAndVersion, vr::IVRDisplayComponent_Version))
    {
        return display_component.get();
    }

    return nullptr;
}

void VirtualHMDDeviceDriver::DebugRequest( const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize )
{
    if (unResponseBufferSize >= 1)
    {
        pchResponseBuffer[0] = 0;
    }
}

vr::DriverPose_t VirtualHMDDeviceDriver::GetPose()
{
    vr::DriverPose_t pose = { 0 };
    VRMathUtils::GetPose(pose, position, rotation);

	return pose;    
}

void VirtualHMDDeviceDriver::UpdatePose()
{
    bool updated = poseIsUpdate.exchange(false);

    if (!updated)
    {
        return;
    }

    vr::VRServerDriverHost()->TrackedDevicePoseUpdated(device_index, GetPose(), sizeof(vr::DriverPose_t));
}

void VirtualHMDDeviceDriver::SetPosition(float x, float y, float z)
{
    position[0] = x;
    position[1] = y;
    position[2] = -z;

    poseIsUpdate = true;
}

void VirtualHMDDeviceDriver::SetRotation(float x, float y, float z)
{
    double quat[4];
    VRMathUtils::ToQuaternion(x, y, z, quat);

    rotation[0] = quat[0];
    rotation[1] = quat[1];
    rotation[2] = quat[2];
    rotation[3] = -quat[3];

    poseIsUpdate = true;
}