#pragma once

#include <memory>
#include <atomic>

#include "openvr_driver.h"

class VirtualTrackingDeviceDriver : public vr::ITrackedDeviceServerDriver
{
public:
    VirtualTrackingDeviceDriver();
	virtual vr::EVRInitError Activate( uint32_t unObjectId ) override;

	virtual void Deactivate() override;

	virtual void EnterStandby() override;

	virtual void *GetComponent( const char *pchComponentNameAndVersion ) override;

	virtual void DebugRequest( const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize ) override;

	virtual vr::DriverPose_t GetPose() override;

    void UpdatePose();

    void SetPosition(float x, float y, float z);

    void SetRotation(float x, float y, float z);

protected:
    std::atomic< uint32_t > device_index;

private:
    std::atomic< bool > poseIsUpdate;

    std::atomic< double > position[3] = {0.0, 1.0, 0.0};
    std::atomic< double > rotation[4] = {0.0, 0.0, 0.0, 1.0};
};