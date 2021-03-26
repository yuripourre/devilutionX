#include "controls/controller_motion.h"

#include <cmath>

#include "controls/devices/game_controller.h"
#include "controls/devices/joystick.h"
#include "controls/devices/kbcontroller.h"
#include "controls/controller.h"
#include "controls/game_controls.h"

namespace dvl {

// SELECT + D-Pad to simulate right stick movement.
bool SimulateRightStickWithDpad(const SDL_Event &event, ControllerButtonEvent ctrl_event)
{
	if (dpad_hotkeys)
		return false;
	static bool simulating = false;
	if (ctrl_event.button == ControllerButton_BUTTON_BACK) {
		if (ctrl_event.up && simulating) {
			Controller::rightStickX = Controller::rightStickY = 0;
			simulating = false;
		}
		return false;
	}
	if (!Controller::IsControllerButtonPressed(ControllerButton_BUTTON_BACK))
		return false;
	switch (ctrl_event.button) {
	case ControllerButton_BUTTON_DPAD_LEFT:
		Controller::rightStickX = ctrl_event.up ? 0 : -1;
		break;
	case ControllerButton_BUTTON_DPAD_RIGHT:
		Controller::rightStickX = ctrl_event.up ? 0 : 1;
		break;
	case ControllerButton_BUTTON_DPAD_UP:
		Controller::rightStickY = ctrl_event.up ? 0 : 1;
		break;
	case ControllerButton_BUTTON_DPAD_DOWN:
		Controller::rightStickY = ctrl_event.up ? 0 : -1;
		break;
	default:
		return false;
	}
	simulating = !(Controller::rightStickX == 0 && Controller::rightStickY == 0);

	return true;
}

// Updates motion state for mouse and joystick sticks.
bool ProcessControllerMotion(const SDL_Event &event, ControllerButtonEvent ctrl_event)
{
	SDL_Log("ProcessControllerMotion");
	if (Controller::Empty())
		return false;

	SDL_Log("Trying to call Controller::GetFromEvent");
	Controller *const controller = Controller::GetFromEvent(event);
	if (controller != NULL) {
		SDL_Log("ProcessControllerMotion::Found controller: %d",controller);
	}
	SDL_Log("ProcessControllerMotion::NULL");
	if (controller != NULL && controller->ProcessAxisMotion(event)) {
		SDL_Log("ProcessControllerMotion::ScaleJoystics?");
		controller->ScaleJoysticks();
		return true;
	}

	SDL_Log("Simulating SimulateRightStickWithDpad");
	// If there is no controller associated to the event, we simulate the events with DPAD
	return SimulateRightStickWithDpad(event, ctrl_event);
}

} // namespace dvl
