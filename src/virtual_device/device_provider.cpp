#include "device_provider.h"

#include <string_view>

#include "driverlog.h"

#include "driver_settings.h"
#include "virtual_hmd_device_driver.h"

vr::EVRInitError VirtualDeviceProvider::Init( vr::IVRDriverContext *pDriverContext )
{
    VR_INIT_SERVER_DRIVER_CONTEXT( pDriverContext );

    bool hmd_enabled = vr::VRSettings()->GetBool(kDriverSettingsSection, "enable_hmd");
    if (hmd_enabled)
    {
        m_pVirtualHmdDevice = new VirtualHMDDeviceDriver();
        vr::VRServerDriverHost()->TrackedDeviceAdded( "VirtualHMD", vr::TrackedDeviceClass_HMD, m_pVirtualHmdDevice );    
    }
    bool controller_enabled = vr::VRSettings()->GetBool(kDriverSettingsSection, "enable_controllers");
    if (controller_enabled)
    {
        m_pVirtualLeftControllerDevice = new VirtualControllerDeviceDriver(vr::TrackedControllerRole_LeftHand);
        vr::VRServerDriverHost()->TrackedDeviceAdded( "VirtualLeftController", vr::TrackedDeviceClass_Controller, m_pVirtualLeftControllerDevice );

        m_pVirtualRightControllerDevice = new VirtualControllerDeviceDriver(vr::TrackedControllerRole_RightHand);
        vr::VRServerDriverHost()->TrackedDeviceAdded( "VirtualRightController", vr::TrackedDeviceClass_Controller, m_pVirtualRightControllerDevice );
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
    if (m_pVirtualHmdDevice)
    {
        delete m_pVirtualHmdDevice;
        m_pVirtualHmdDevice = NULL;    
    }

    if (m_pVirtualLeftControllerDevice)
    {
        delete m_pVirtualLeftControllerDevice;
        m_pVirtualLeftControllerDevice = NULL;
    }

    if (m_pVirtualRightControllerDevice)
    {
        delete m_pVirtualRightControllerDevice;
        m_pVirtualRightControllerDevice = NULL;
    }

    m_pOSCReceiver->Stop();
    delete m_pOSCReceiver;
    m_pOSCReceiver = NULL;

    DriverLog( "Virtual device cleanup completed." );
}

const char * const *VirtualDeviceProvider::GetInterfaceVersions() { return vr::k_InterfaceVersions; }

void VirtualDeviceProvider::RunFrame() 
{
    if (m_pVirtualHmdDevice)
    {
        m_pVirtualHmdDevice->Update();
    }
    if (m_pVirtualLeftControllerDevice)
    {
        m_pVirtualLeftControllerDevice->Update();
    }
    if (m_pVirtualRightControllerDevice)
    {
        m_pVirtualRightControllerDevice->Update();
    }
}

bool VirtualDeviceProvider::ShouldBlockStandbyMode()  { return false; } 
void VirtualDeviceProvider::EnterStandby()  {}
void VirtualDeviceProvider::LeaveStandby()  {}

std::string_view ExtractAddressSegment(std::string_view str, size_t& offset) {
    if (offset == std::string::npos || offset >= str.size()) {
        return {};
    }

    size_t nextSlash = str.find('/', offset);
    std::string_view segment = (nextSlash == std::string::npos) 
        ? str.substr(offset)
        : str.substr(offset, nextSlash - offset);

    offset = (nextSlash == std::string::npos) ? std::string::npos : nextSlash + 1;
    return segment;
}

void VirtualDeviceProvider::OnOSCMessageReceived(const OSCParser::ParsedMessage& msg)
{
    size_t offset = 1;
    std::string_view segment = ExtractAddressSegment(msg.address, offset);

    if (segment == "tracking")
    {
        if (ExtractAddressSegment(msg.address, offset) != "trackers") return;
        
        VirtualTrackingDeviceDriver* device = GetTrackingDevice(ExtractAddressSegment(msg.address, offset));
        if (!device) return;

        segment = ExtractAddressSegment(msg.address, offset);
        if (msg.floats.size() != 3) return;

        if (segment == "position")
        {
            device->SetPosition(msg.floats[0], msg.floats[1], msg.floats[2]);
        }
        else if (segment == "rotation")
        {
            device->SetRotation(msg.floats[0], msg.floats[1], msg.floats[2]);
        }
        return;
    }

    if (segment == "input")
    {
        VirtualControllerDeviceDriver* device = GetControllerDevice(ExtractAddressSegment(msg.address, offset));
        if (!device) return;

        segment = ExtractAddressSegment(msg.address, offset);
        if (segment == "a") device->SetAButtonPressed(msg.booleanValue);
        else if (segment == "b") device->SetBButtonPressed(msg.booleanValue);
        else if (segment == "x") device->SetXButtonPressed(msg.booleanValue);
        else if (segment == "grab") device->SetGrabButtonPressed(msg.booleanValue);
        else if (segment == "trigger" && msg.floats.size() == 1) device->SetTriggerValue(msg.floats[0]);
        else if (segment == "joystick" && msg.floats.size() == 2)
        {
            device->SetJoystickX(msg.floats[0]);
            device->SetJoystickY(msg.floats[1]);
        }
        return;
    }
}

VirtualTrackingDeviceDriver* VirtualDeviceProvider::GetTrackingDevice(std::string_view segment)
{
    if (segment == "head") return m_pVirtualHmdDevice;
    if (segment == "l_hand") return m_pVirtualLeftControllerDevice;
    if (segment == "r_hand") return m_pVirtualRightControllerDevice;
    return nullptr;
}

VirtualControllerDeviceDriver* VirtualDeviceProvider::GetControllerDevice(std::string_view segment)
{
    if (segment == "l_hand") return m_pVirtualLeftControllerDevice;
    if (segment == "r_hand") return m_pVirtualRightControllerDevice;
    return nullptr;
}