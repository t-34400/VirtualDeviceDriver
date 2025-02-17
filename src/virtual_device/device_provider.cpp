#include "device_provider.h"

#include <algorithm>
#include <string_view>

#include "driverlog.h"

#include "driver_settings.h"
#include "virtual_hmd_device_driver.h"

vr::EVRInitError VirtualDeviceProvider::Init( vr::IVRDriverContext *pDriverContext )
{
    VR_INIT_SERVER_DRIVER_CONTEXT( pDriverContext );

    char buffer[4096];

    bool hmd_enabled = vr::VRSettings()->GetBool( kDriverSettingsSection, "enable_hmd" );
    if (hmd_enabled)
    {
        vr::VRSettings()->GetString( kDriverSettingsSection, "hmd_serial", buffer, sizeof(buffer) );
        std::string hmd_serial = buffer;

        m_pVirtualHmdDevice = std::make_unique<VirtualHMDDeviceDriver>();
        vr::VRServerDriverHost()->TrackedDeviceAdded( hmd_serial.c_str(), vr::TrackedDeviceClass_HMD, m_pVirtualHmdDevice.get() );
    }

    bool controller_enabled = vr::VRSettings()->GetBool( kDriverSettingsSection, "enable_controllers" );
    if (controller_enabled)
    {
        vr::VRSettings()->GetString( kDriverSettingsSection, "left_controller_serial", buffer, sizeof(buffer) );
        std::string left_controller_serial = buffer;

        m_pVirtualLeftControllerDevice = std::make_unique<VirtualControllerDeviceDriver>(vr::TrackedControllerRole_LeftHand);
        vr::VRServerDriverHost()->TrackedDeviceAdded( left_controller_serial.c_str(), vr::TrackedDeviceClass_Controller, m_pVirtualLeftControllerDevice.get() );

        vr::VRSettings()->GetString(kDriverSettingsSection, "right_controller_serial", buffer, sizeof(buffer));
        std::string right_controller_serial = buffer;

        m_pVirtualRightControllerDevice = std::make_unique<VirtualControllerDeviceDriver>(vr::TrackedControllerRole_RightHand);
        vr::VRServerDriverHost()->TrackedDeviceAdded( right_controller_serial.c_str(), vr::TrackedDeviceClass_Controller, m_pVirtualRightControllerDevice.get() );
    }

    int32_t tracker_count = vr::VRSettings()->GetInt32( kDriverSettingsSection, "tracker_count" );
    for (int idx = 0; idx < tracker_count; ++idx)
    {
        vr::VRSettings()->GetString( kDriverSettingsSection, ("tracker_" + std::to_string(idx) + "_serial").c_str(), buffer, sizeof(buffer) );
        std::string tracker_serial = buffer;

        m_pVirtualTrackingDevices.push_back(std::make_unique<VirtualTrackingDeviceDriver>());
        vr::VRServerDriverHost()->TrackedDeviceAdded( tracker_serial.c_str(), vr::TrackedDeviceClass_GenericTracker, m_pVirtualTrackingDevices.back().get() );
    }
    
    DriverLog( "Virtual device initialized successfully." );

    uint32_t port = static_cast<uint32_t>(vr::VRSettings()->GetInt32(kDriverSettingsSection, "osc_port"));

    m_pOSCReceiver = std::make_unique<OSCReceiver>(port);
    m_pOSCReceiver->SetMessageCallback(std::bind(&VirtualDeviceProvider::OnOSCMessageReceived, this, std::placeholders::_1));
    m_pOSCReceiver->Start();

    return vr::VRInitError_None;
}

void VirtualDeviceProvider::Cleanup() 
{
    m_pVirtualHmdDevice.reset();
    m_pVirtualLeftControllerDevice.reset();
    m_pVirtualRightControllerDevice.reset();

    m_pOSCReceiver->Stop();
    m_pOSCReceiver.reset();

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

    for (auto& device : m_pVirtualTrackingDevices)
    {
        device->Update();
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
        else if (segment == "grip") device->SetGripButtonPressed(msg.booleanValue);
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
    if (segment == "head") return m_pVirtualHmdDevice.get();
    if (segment == "l_hand") return m_pVirtualLeftControllerDevice.get();
    if (segment == "r_hand") return m_pVirtualRightControllerDevice.get();

    // Get tracker by index
    if (!segment.empty() && std::all_of(segment.begin(), segment.end(), ::isdigit)) {
        size_t index = std::stoi(std::string(segment)) - 1;

        if (index >= 0 && index < m_pVirtualTrackingDevices.size()) {
            return m_pVirtualTrackingDevices[index].get();
        }
    }

    return nullptr;
}

VirtualControllerDeviceDriver* VirtualDeviceProvider::GetControllerDevice(std::string_view segment)
{
    if (segment == "l_hand") return m_pVirtualLeftControllerDevice.get();
    if (segment == "r_hand") return m_pVirtualRightControllerDevice.get();
    return nullptr;
}