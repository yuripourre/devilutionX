#pragma once

#include <SDL.h>

#include "all.h"
#include "./axis_direction.h"
#include "./controller_buttons.h"

namespace dvl {

struct ControllerButtonEvent {
	ControllerButton button;
	bool up;
};

enum controller_type {
	CONTROLLER_UNKNOWN,
	CONTROLLER_GAME_CONTROLLER,
	CONTROLLER_JOYSTICK,
	CONTROLLER_KEYBOARD,
};

class Controller {

public:
	Controller() { type_ = CONTROLLER_UNKNOWN; };
	// Raw axis values.
	float leftStickX, leftStickY, rightStickX, rightStickY;
	// Axis values scaled to [-1, 1] range and clamped to a deadzone.
	float leftStickXUnscaled, leftStickYUnscaled, rightStickXUnscaled, rightStickYUnscaled;
	// Whether stick positions have been updated and need rescaling.
	bool leftStickNeedsScaling, rightStickNeedsScaling;

	// Returns direction of the left thumb stick or DPad (if allow_dpad = true).
	AxisDirection GetLeftStickOrDpadDirection(bool allow_dpad = true);

	// Normalize joystick values
	void ScaleJoysticks();

	// NOTE: Not idempotent because of how it handles axis triggers.
	// Must be called exactly once for each SDL input event.
	ControllerButton ToControllerButton(const SDL_Event &event) const;

	bool IsPressed(ControllerButton button) const;

	static Controller *GetFromEvent(const SDL_Event &event);

	SDL_JoystickID instance_id() const
	{
		return instance_id_;
	}

	controller_type type() const
	{
		return type_;
	}

	bool ProcessAxisMotion(const SDL_Event &event);

	static const std::vector<Controller> &All();

	static bool IsControllerButtonPressed(ControllerButton button);

	// NOTE: Not idempotent because of how it handles axis triggers.
    // Must be called exactly once per SDL input event.
    static ControllerButtonEvent ToControllerButtonEvent(const SDL_Event &event);

protected:
	static std::vector<Controller> *const controllers_;
	controller_type type_ = CONTROLLER_UNKNOWN;
	SDL_JoystickID instance_id_ = -1;

private:
	void ScaleJoystickAxes(float *x, float *y, float deadzone);
};

bool HandleControllerAddedOrRemovedEvent(const SDL_Event &event);

} // namespace dvl
