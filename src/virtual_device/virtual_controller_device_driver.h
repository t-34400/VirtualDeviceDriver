#pragma once

#include <memory>
#include <atomic>

#include "openvr_driver.h"

#include "virtual_tracking_device_driver.h"

enum InputHandles {
	kInputHandle_A_click,
	kInputHandle_B_click,
	kInputHandle_X_click,
	kInputHandle_Y_click,
	kInputHandle_System_click,
	kInputHandle_grip_value,
	kInputHandle_trigger_value,
	kInputHandle_joystick_x,
	kInputHandle_joystick_y,
	kInputHandle_COUNT
};

class VirtualControllerDeviceDriver : public VirtualTrackingDeviceDriver
{
public:
    VirtualControllerDeviceDriver(vr::ETrackedControllerRole role);
    
	virtual vr::EVRInitError Activate( uint32_t unObjectId ) override;

	virtual void Deactivate() override;

	virtual void EnterStandby() override;

    virtual void Update() override;

    void SetAButtonPressed(bool pressed) { a_button_pressed_ = pressed; }
    void SetBButtonPressed(bool pressed) { b_button_pressed_ = pressed; }
    void SetXButtonPressed(bool pressed) { x_button_pressed_ = pressed; }
    void SetYButtonPressed(bool pressed) { y_button_pressed_ = pressed; }
    void SetSystemButtonPressed(bool pressed) { system_button_pressed_ = pressed; }

    void SetGripValue(float value) { grip_value_ = value; }
    void SetTriggerValue(float value) { trigger_value_ = value; }
    void SetJoystickX(float value) { joystick_x_ = value; }
    void SetJoystickY(float value) { joystick_y_ = value; }

private:
    vr::VRInputComponentHandle_t input_handles_[kInputHandle_COUNT];
    vr::ETrackedControllerRole role_;

    std::atomic_bool a_button_pressed_{false};
    std::atomic_bool b_button_pressed_{false};
    std::atomic_bool x_button_pressed_{false};
    std::atomic_bool y_button_pressed_{false};
    std::atomic_bool system_button_pressed_{false};

    std::atomic<float> grip_value_{0.0f};
    std::atomic<float> trigger_value_{0.0f};
    std::atomic<float> joystick_x_{0.0f};
    std::atomic<float> joystick_y_{0.0f};
};