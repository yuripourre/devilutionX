#pragma once
// Joystick mappings for SDL1 and additional buttons on SDL2.

#include <vector>

#include <SDL.h>

#ifdef USE_SDL1
#include "sdl2_to_1_2_backports.h"
#endif

#include "controls/controller.h"
#include "controls/controller_buttons.h"

namespace dvl {

class Joystick : public Controller {

public:
	Joystick() { type_ = CONTROLLER_JOYSTICK; };
	static void Add(int device_index);
	static void Remove(SDL_JoystickID instance_id);
	static Controller *Get(SDL_JoystickID instance_id);
    static Controller *Get(const SDL_Event &event);

	// NOTE: Not idempotent.
	// Must be called exactly once for each SDL input event.
	ControllerButton ToControllerButton(const SDL_Event &event) const;
	bool IsPressed(ControllerButton button) const;
	bool ProcessAxisMotion(const SDL_Event &event);

private:
	int ToSdlJoyButton(ControllerButton button) const;
	bool IsHatButtonPressed(ControllerButton button) const;

	SDL_Joystick *sdl_joystick_ = NULL;
};

} // namespace dvl
