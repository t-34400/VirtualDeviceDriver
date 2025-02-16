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

    uint32_t port = static_cast<uint32_t>(vr::VRSettings()->GetInt32(kDriverSettingsSection, "osc_port"));

    m_pOSCReceiver = new OSCReceiver(port);
    m_pOSCReceiver->SetMessageCallback(std::bind(&VirtualDeviceProvider::OnOSCMessageReceived, this, std::placeholders::_1));
    m_pOSCReceiver->Start();

    return vr::VRInitError_None;
}

void VirtualDeviceProvider::Cleanup() 
{
    delete m_pMyHmdDevice;
    m_pMyHmdDevice = NULL;

    m_pOSCReceiver->Stop();
    delete m_pOSCReceiver;
    m_pOSCReceiver = NULL;

    DriverLog( "Virtual device cleanup completed." );
}

const char * const *VirtualDeviceProvider::GetInterfaceVersions() { return vr::k_InterfaceVersions; }

void VirtualDeviceProvider::RunFrame() 
{
    if (m_pMyHmdDevice)
    {
        m_pMyHmdDevice->UpdatePose();
    }
}

bool VirtualDeviceProvider::ShouldBlockStandbyMode()  { return false; } 
void VirtualDeviceProvider::EnterStandby()  {}
void VirtualDeviceProvider::LeaveStandby()  {}

void VirtualDeviceProvider::OnOSCMessageReceived(const OSCParser::ParsedMessage& msg)
{
    if (msg.address == "/tracking/trackers/head/position")
    {
        if (msg.floats.size() == 3)
        {
            m_pMyHmdDevice->SetPosition(msg.floats[0], msg.floats[1], msg.floats[2]);
        }
    }
    else if (msg.address == "/tracking/trackers/head/rotation")
    {
        if (msg.floats.size() == 3)
        {
            m_pMyHmdDevice->SetRotation(msg.floats[0], msg.floats[1], msg.floats[2]);
        }
    }
}