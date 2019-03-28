#pragma once

#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE 7849
#define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 8689
#define XINPUT_GAMEPAD_TRIGGER_THRESHOLD 30

void CheckForController();
void ReleaseAllButtons();
extern bool conInv;

extern float leftStickX;
extern float leftStickY;
extern float rightStickX;
extern float rightStickY;
extern float leftTrigger;
extern float rightTrigger;
extern float deadzoneX;
extern float deadzoneY;
