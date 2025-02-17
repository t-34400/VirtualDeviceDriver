#include "virtual_controller_device_driver.h"

#include <memory>

#include "driverlog.h"

#include "driver_settings.h"

VirtualControllerDeviceDriver::VirtualControllerDeviceDriver(vr::ETrackedControllerRole role) : role_(role)
{
    DriverLog("VirtualControllerDeviceDriver created.");
}

vr::EVRInitError VirtualControllerDeviceDriver::Activate( uint32_t unObjectId )
{
    device_index = unObjectId;

    vr::PropertyContainerHandle_t container = vr::VRProperties()->TrackedDeviceToPropertyContainer( device_index );

    vr::VRProperties()->SetInt32Property(container, vr::Prop_ControllerRoleHint_Int32, role_);

	vr::VRProperties()->SetStringProperty(container, vr::Prop_InputProfilePath_String,
		"{virtual_device_driver}/resources/input/virtual_controller_profile.json");

    vr::VRDriverInput()->CreateBooleanComponent(container, "/input/a/click", &input_handles_[kInputHandle_A_click]);
    vr::VRDriverInput()->CreateBooleanComponent(container, "/input/b/click", &input_handles_[kInputHandle_B_click]);
    vr::VRDriverInput()->CreateBooleanComponent(container, "/input/x/click", &input_handles_[kInputHandle_X_click]);
    vr::VRDriverInput()->CreateBooleanComponent(container, "/input/grab/click", &input_handles_[kInputHandle_grab_click]);

	vr::VRDriverInput()->CreateScalarComponent(container, "/input/trigger/value", &input_handles_[kInputHandle_trigger_value],
		vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);

	vr::VRDriverInput()->CreateScalarComponent(container, "/input/joystick/x", &input_handles_[kInputHandle_joystick_x],
		vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
	vr::VRDriverInput()->CreateScalarComponent(container, "/input/joystick/y", &input_handles_[kInputHandle_joystick_y],
		vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);

    DriverLog("VirtualControllerDeviceDriver activated.");
    return vr::VRInitError_None;
}

void VirtualControllerDeviceDriver::Deactivate()
{
    device_index = vr::k_unTrackedDeviceIndexInvalid;

    DriverLog("VirtualControllerDeviceDriver deactivated.");
}

void VirtualControllerDeviceDriver::EnterStandby()
{
    DriverLog("VirtualControllerDeviceDriver entered standby.");
}

void VirtualControllerDeviceDriver::Update()
{
    VirtualTrackingDeviceDriver::Update();

    if (device_index != vr::k_unTrackedDeviceIndexInvalid)
    {
        vr::VRDriverInput()->UpdateBooleanComponent(input_handles_[kInputHandle_A_click], a_button_pressed_, 0.0);
        vr::VRDriverInput()->UpdateBooleanComponent(input_handles_[kInputHandle_B_click], b_button_pressed_, 0.0);
        vr::VRDriverInput()->UpdateBooleanComponent(input_handles_[kInputHandle_X_click], x_button_pressed_, 0.0);
        vr::VRDriverInput()->UpdateBooleanComponent(input_handles_[kInputHandle_grab_click], grab_button_pressed_, 0.0);

        vr::VRDriverInput()->UpdateScalarComponent(input_handles_[kInputHandle_trigger_value], trigger_value_, 0.0);
        vr::VRDriverInput()->UpdateScalarComponent(input_handles_[kInputHandle_joystick_x], joystick_x_, 0.0);
        vr::VRDriverInput()->UpdateScalarComponent(input_handles_[kInputHandle_joystick_y], joystick_y_, 0.0);
    }
}