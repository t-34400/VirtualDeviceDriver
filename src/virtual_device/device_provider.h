#pragma once

#include <string_view>

#include "openvr_driver.h"

#include "osc_receiver.h"
#include "virtual_hmd_device_driver.h"
#include "virtual_controller_device_driver.h"

#include "osc_parser.h"

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
    VirtualHMDDeviceDriver *m_pVirtualHmdDevice = nullptr;
	VirtualControllerDeviceDriver *m_pVirtualLeftControllerDevice = nullptr;
	VirtualControllerDeviceDriver *m_pVirtualRightControllerDevice = nullptr;

	OSCReceiver *m_pOSCReceiver = nullptr;

	void OnOSCMessageReceived(const OSCParser::ParsedMessage& msg);
	VirtualTrackingDeviceDriver* GetTrackingDevice(std::string_view segment);
	VirtualControllerDeviceDriver* GetControllerDevice(std::string_view segment);
};