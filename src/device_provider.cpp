#include "device_provider.h"

#include "driverlog.h"

#include "driver_settings.h"
#include "virtual_hmd_device_driver.h"

vr::EVRInitError VirtualDeviceProvider::Init( vr::IVRDriverContext *pDriverContext )
{
    VR_INIT_SERVER_DRIVER_CONTEXT( pDriverContext );

    bool hmd_enabled = vr::VRSettings()->GetBool(kDriverSettingsSection, "enable_hmd");
    if (hmd_enabled)
    {
        m_pMyHmdDevice = new VirtualHMDDeviceDriver();
        vr::VRServerDriverHost()->TrackedDeviceAdded( "VirtualHMD", vr::TrackedDeviceClass_HMD, m_pMyHmdDevice );    
    }
    
    DriverLog( "Virtual device initialized successfully." );

    return vr::VRInitError_None;
}

void VirtualDeviceProvider::Cleanup() 
{
    delete m_pMyHmdDevice;
    m_pMyHmdDevice = NULL;

    DriverLog( "Virtual device cleanup completed." );
}

const char * const *VirtualDeviceProvider::GetInterfaceVersions() { return vr::k_InterfaceVersions; }
void VirtualDeviceProvider::RunFrame() {}
bool VirtualDeviceProvider::ShouldBlockStandbyMode()  { return false; } 
void VirtualDeviceProvider::EnterStandby()  {}
void VirtualDeviceProvider::LeaveStandby()  {}