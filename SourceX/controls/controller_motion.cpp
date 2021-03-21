#include "controls/controller_motion.h"

#include <cmath>

#include "controls/devices/game_controller.h"
#include "controls/devices/joystick.h"
#include "controls/devices/kbcontroller.h"
#include "controls/controller.h"
#include "controls/game_controls.h"

namespace dvl {

namespace {

void ScaleJoystickAxes(float *x, float *y, float deadzone)
{
	//radial and scaled dead_zone
	//http://www.third-helix.com/2013/04/12/doing-thumbstick-dead-zones-right.html
	//input values go from -32767.0...+32767.0, output values are from -1.0 to 1.0;

	if (deadzone == 0) {
		return;
	}
	if (deadzone >= 1.0) {
		*x = 0;
		*y = 0;
		return;
	}

	const float maximum = 32767.0f;
	float analog_x = *x;
	float analog_y = *y;
	float dead_zone = deadzone * maximum;

	float magnitude = std::sqrt(analog_x * analog_x + analog_y * analog_y);
	if (magnitude >= dead_zone) {
		// find scaled axis values with magnitudes between zero and maximum
		float scalingFactor = 1.0 / magnitude * (magnitude - dead_zone) / (maximum - dead_zone);
		analog_x = (analog_x * scalingFactor);
		analog_y = (analog_y * scalingFactor);

		// clamp to ensure results will never exceed the max_axis value
		float clamping_factor = 1.0f;
		float abs_analog_x = std::fabs(analog_x);
		float abs_analog_y = std::fabs(analog_y);
		if (abs_analog_x > 1.0 || abs_analog_y > 1.0) {
			if (abs_analog_x > abs_analog_y) {
				clamping_factor = 1 / abs_analog_x;
			} else {
				clamping_factor = 1 / abs_analog_y;
			}
		}
		*x = (clamping_factor * analog_x);
		*y = (clamping_factor * analog_y);
	} else {
		*x = 0;
		*y = 0;
	}
}

// SELECT + D-Pad to simulate right stick movement.
bool SimulateRightStickWithDpad(const SDL_Event &event, ControllerButtonEvent ctrl_event)
{
	if (dpad_hotkeys)
		return false;
	static bool simulating = false;
	if (ctrl_event.button == ControllerButton_BUTTON_BACK) {
		if (ctrl_event.up && simulating) {
			rightStickX = rightStickY = 0;
			simulating = false;
		}
		return false;
	}
	if (!IsControllerButtonPressed(ControllerButton_BUTTON_BACK))
		return false;
	switch (ctrl_event.button) {
	case ControllerButton_BUTTON_DPAD_LEFT:
		rightStickX = ctrl_event.up ? 0 : -1;
		break;
	case ControllerButton_BUTTON_DPAD_RIGHT:
		rightStickX = ctrl_event.up ? 0 : 1;
		break;
	case ControllerButton_BUTTON_DPAD_UP:
		rightStickY = ctrl_event.up ? 0 : 1;
		break;
	case ControllerButton_BUTTON_DPAD_DOWN:
		rightStickY = ctrl_event.up ? 0 : -1;
		break;
	default:
		return false;
	}
	simulating = !(rightStickX == 0 && rightStickY == 0);

	return true;
}

} // namespace

float leftStickX, leftStickY, rightStickX, rightStickY;
float leftStickXUnscaled, leftStickYUnscaled, rightStickXUnscaled, rightStickYUnscaled;
bool leftStickNeedsScaling, rightStickNeedsScaling;

namespace {

void ScaleJoysticks()
{
	const float rightDeadzone = 0.07;
	const float leftDeadzone = 0.07;

	if (leftStickNeedsScaling) {
		leftStickX = leftStickXUnscaled;
		leftStickY = leftStickYUnscaled;
		ScaleJoystickAxes(&leftStickX, &leftStickY, leftDeadzone);
		leftStickNeedsScaling = false;
	}

	if (rightStickNeedsScaling) {
		rightStickX = rightStickXUnscaled;
		rightStickY = rightStickYUnscaled;
		ScaleJoystickAxes(&rightStickX, &rightStickY, rightDeadzone);
		rightStickNeedsScaling = false;
	}
}

} // namespace

// Updates motion state for mouse and joystick sticks.
bool ProcessControllerMotion(const SDL_Event &event, ControllerButtonEvent ctrl_event)
{
#ifndef USE_SDL1
	GameController *const controller = GameController::Get(event);
	if (controller != NULL && controller->ProcessAxisMotion(event)) {
		ScaleJoysticks();
		return true;
	}
#endif
	Joystick *const joystick = Joystick::Get(event);
	if (joystick != NULL && joystick->ProcessAxisMotion(event)) {
		ScaleJoysticks();
		return true;
	}

#if HAS_KBCTRL == 1
	if (ProcessKbCtrlAxisMotion(event))
		return true;
#endif
	return SimulateRightStickWithDpad(event, ctrl_event);
}

AxisDirection GetLeftStickOrDpadDirection(bool allow_dpad)
{
	const float stickX = leftStickX;
	const float stickY = leftStickY;

	AxisDirection result { AxisDirectionX_NONE, AxisDirectionY_NONE };

	if (stickY >= 0.5 || (allow_dpad && IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_UP))) {
		result.y = AxisDirectionY_UP;
	} else if (stickY <= -0.5 || (allow_dpad && IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_DOWN))) {
		result.y = AxisDirectionY_DOWN;
	}

	if (stickX <= -0.5 || (allow_dpad && IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_LEFT))) {
		result.x = AxisDirectionX_LEFT;
	} else if (stickX >= 0.5 || (allow_dpad && IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_RIGHT))) {
		result.x = AxisDirectionX_RIGHT;
	}

	return result;
}

} // namespace dvl
