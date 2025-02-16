#pragma once

#include "openvr_driver.h"
#include "virtual_hmd_device_driver.h"

class VirtualDeviceProvider : public vr::IServerTrackedDeviceProvider
{
public:
	vr::EVRInitError Init( vr::IVRDriverContext *pDriverContext );
	const char *const *GetInterfaceVersions();

	void RunFrame();

	bool ShouldBlockStandbyMode();
	void EnterStandby();
	void LeaveStandby();

	void Cleanup();

private:
    VirtualHMDDeviceDriver *m_pMyHmdDevice = nullptr;
};