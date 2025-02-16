#pragma once

#include <memory>
#include <atomic>

#include "openvr_driver.h"

class VirtualHMDDisplayComponent : public vr::IVRDisplayComponent
{
public:
    explicit VirtualHMDDisplayComponent(uint32_t width = 1920, uint32_t height = 1080);
    
    virtual void GetWindowBounds( int32_t *pnX, int32_t *pnY, uint32_t *pnWidth, uint32_t *pnHeight );

    virtual bool IsDisplayOnDesktop();

    virtual bool IsDisplayRealDisplay();

    virtual void GetRecommendedRenderTargetSize( uint32_t *pnWidth, uint32_t *pnHeight );

    virtual void GetEyeOutputViewport( vr::EVREye eEye, uint32_t *pnX, uint32_t *pnY, uint32_t *pnWidth, uint32_t *pnHeight );

    virtual void GetProjectionRaw( vr::EVREye eEye, float *pfLeft, float *pfRight, float *pfTop, float *pfBottom );

    virtual vr::DistortionCoordinates_t ComputeDistortion( vr::EVREye eEye, float fU, float fV );

    virtual bool ComputeInverseDistortion( vr::HmdVector2_t *pResult, vr::EVREye eEye, uint32_t unChannel, float fU, float fV );

private:
    uint32_t window_width;
    uint32_t window_height;
};

class VirtualHMDDeviceDriver : public vr::ITrackedDeviceServerDriver
{
public:
    VirtualHMDDeviceDriver();
	virtual vr::EVRInitError Activate( uint32_t unObjectId ) override;

	virtual void Deactivate() override;

	virtual void EnterStandby() override;

	virtual void *GetComponent( const char *pchComponentNameAndVersion ) override;

	virtual void DebugRequest( const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize ) override;

	virtual vr::DriverPose_t GetPose() override;

    void UpdatePose();

    void SetPosition(float x, float y, float z);

    void SetRotation(float x, float y, float z);

private:
    std::unique_ptr<VirtualHMDDisplayComponent> display_component;
    std::atomic< uint32_t > device_index;

    std::atomic< bool > poseIsUpdate;

    std::atomic< double > position[3] = {0.0, 1.0, 0.0};
    std::atomic< double > rotation[4] = {0.0, 0.0, 0.0, 1.0};
};