#pragma once

// Processes and stores mouse and joystick motion.

#include <SDL.h>

#include "./controller.h"

namespace dvl {

// Updates motion state for mouse and joystick sticks.
bool ProcessControllerMotion(const SDL_Event &event, ControllerButtonEvent ctrl_event);

} // namespace dvl
