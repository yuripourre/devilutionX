#include "controls/controller.h"

#include "controls/devices/kbcontroller.h"
#include "controls/devices/joystick.h"
#include "controls/devices/game_controller.h"

namespace dvl {

std::vector<Controller> *const Controller::controllers_ = new std::vector<Controller>;

Controller *Controller::GetFromEvent(const SDL_Event &event) {
#ifndef USE_SDL1
	Controller *const controller = GameController::Get(event);
	if (controller != NULL) {
		return controller;
	}
#endif
	Controller *const joystick = Joystick::Get(event);
	if (joystick != NULL) {
    	return joystick;
    }
#if HAS_KBCTRL == 1
	Controller *const keyboardController = KeyboardController::Get(event);
	if (keyboardController != NULL) {
		return keyboardController;
	}
#endif
	return NULL;
}

AxisDirection Controller::GetLeftStickOrDpadDirection(bool allow_dpad)
{
	const float stickX = leftStickX;
	const float stickY = leftStickY;

	AxisDirection result { AxisDirectionX_NONE, AxisDirectionY_NONE };

	if (stickY >= 0.5 || (allow_dpad && Controller::IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_UP))) {
		result.y = AxisDirectionY_UP;
	} else if (stickY <= -0.5 || (allow_dpad && Controller::IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_DOWN))) {
		result.y = AxisDirectionY_DOWN;
	}

	if (stickX <= -0.5 || (allow_dpad && Controller::IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_LEFT))) {
		result.x = AxisDirectionX_LEFT;
	} else if (stickX >= 0.5 || (allow_dpad && Controller::IsControllerButtonPressed(ControllerButton_BUTTON_DPAD_RIGHT))) {
		result.x = AxisDirectionX_RIGHT;
	}

	return result;
}

void Controller::ScaleJoysticks()
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

void Controller::ScaleJoystickAxes(float *x, float *y, float deadzone)
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

ControllerButtonEvent Controller::ToControllerButtonEvent(const SDL_Event &event)
{
	ControllerButtonEvent result { ControllerButton_NONE, false };
	switch (event.type) {
#ifndef USE_SDL1
	case SDL_CONTROLLERBUTTONUP:
#endif
	case SDL_JOYBUTTONUP:
	case SDL_KEYUP:
		result.up = true;
		break;
	default:
		break;
	}

	Controller *const controller = Controller::GetFromEvent(event);
	if (controller != NULL) {
		result.button = controller->ToControllerButton(event);
		if (result.button != ControllerButton_NONE)
			return result;
	}
	return result;
}

bool Controller::IsPressed(ControllerButton button) const
{
	return false;
}

bool Controller::ProcessAxisMotion(const SDL_Event &event)
{
	return false;
}

bool Controller::IsControllerButtonPressed(ControllerButton button)
{
	for (std::size_t i = 0; i < controllers_->size(); ++i)
		if ((*controllers_)[i].IsPressed(button))
			return true;
	return false;
}

bool HandleControllerAddedOrRemovedEvent(const SDL_Event &event)
{
#ifndef USE_SDL1
	switch (event.type) {
	case SDL_CONTROLLERDEVICEADDED:
		GameController::Add(event.cdevice.which);
		break;
	case SDL_CONTROLLERDEVICEREMOVED:
		GameController::Remove(event.cdevice.which);
		break;
	case SDL_JOYDEVICEADDED:
		Joystick::Add(event.jdevice.which);
		break;
	case SDL_JOYDEVICEREMOVED:
		Joystick::Remove(event.jdevice.which);
		break;
	default:
		return false;
	}
	return true;
#else
	return false;
#endif
}

ControllerButton Controller::ToControllerButton(const SDL_Event &event) const
{
	return ControllerButton_IGNORE;
}

const std::vector<Controller> &Controller::All()
{
	return *controllers_;
}

} // namespace dvl
