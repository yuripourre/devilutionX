#include "../types.h"
#include <Xinput.h>
#include <map>
#include <string>
#pragma comment(lib, "xinput.lib")

class CXBOXController {
private:
	XINPUT_STATE _controllerState;
	int _controllerNum;

public:
	CXBOXController(int playerNumber);
	XINPUT_STATE GetState();
	bool IsConnected();
	void Vibrate(int leftVal = 0, int rightVal = 0);
	std::map<WORD, int> Buttons;
};


CXBOXController::CXBOXController(int playerNumber)
{
	// Set the Controller Number
	_controllerNum = playerNumber - 1;
}

XINPUT_STATE CXBOXController::GetState()
{
	// Zeroise the state
	ZeroMemory(&_controllerState, sizeof(XINPUT_STATE));

	// Get the state
	XInputGetState(_controllerNum, &_controllerState);

	return _controllerState;
}

bool CXBOXController::IsConnected()
{
	// Zeroise the state
	ZeroMemory(&_controllerState, sizeof(XINPUT_STATE));

	// Get the state
	DWORD Result = XInputGetState(_controllerNum, &_controllerState);

	if (Result == ERROR_SUCCESS) {
		return true;
	} else {
		return false;
	}
}

void CXBOXController::Vibrate(int leftVal, int rightVal)
{
	XINPUT_VIBRATION Vibration;
	ZeroMemory(&Vibration, sizeof(XINPUT_VIBRATION));
	Vibration.wLeftMotorSpeed = leftVal;
	Vibration.wRightMotorSpeed = rightVal;
	XInputSetState(_controllerNum, &Vibration);
}

CXBOXController *Player1;
XINPUT_STATE previous;
void CheckForController()
{
	Player1 = new CXBOXController(1);
	Player1->Buttons.insert(std::pair<WORD, int>(XINPUT_GAMEPAD_A, VK_SPACE));
	//Player1->Buttons.insert(std::pair<WORD, int>(VK_SPACE, XINPUT_GAMEPAD_B)); // swap hot spells
	Player1->Buttons.insert(std::pair<WORD, int>(XINPUT_GAMEPAD_X, VK_RETURN));
	Player1->Buttons.insert(std::pair<WORD, int>(XINPUT_GAMEPAD_Y, 0x58)); // x key
	Player1->Buttons.insert(std::pair<WORD, int>(XINPUT_GAMEPAD_BACK, VK_TAB));
	Player1->Buttons.insert(std::pair<WORD, int>(XINPUT_GAMEPAD_START, VK_ESCAPE));
	Player1->Buttons.insert(std::pair<WORD, int>(XINPUT_GAMEPAD_LEFT_SHOULDER, 0x48));
	Player1->Buttons.insert(std::pair<WORD, int>(XINPUT_GAMEPAD_RIGHT_SHOULDER, VK_TAB));
	Player1->Buttons.insert(std::pair<WORD, int>(XINPUT_GAMEPAD_DPAD_UP, VK_UP));
	Player1->Buttons.insert(std::pair<WORD, int>(XINPUT_GAMEPAD_DPAD_DOWN, VK_DOWN));
	Player1->Buttons.insert(std::pair<WORD, int>(XINPUT_GAMEPAD_DPAD_LEFT, VK_LEFT));
	Player1->Buttons.insert(std::pair<WORD, int>(XINPUT_GAMEPAD_DPAD_RIGHT, VK_RIGHT));

	while (true) {
		if (Player1->IsConnected()) {
			for (auto button : Player1->Buttons) {
				if ((Player1->GetState().Gamepad.wButtons & button.first) != 0) {
					WORD mapping = (Player1->Buttons.find(button.first) != Player1->Buttons.end() ? Player1->Buttons.find(button.first)->second : button.first);
					keybd_event(mapping, 0, 0, 0);
				}

				if (previous.dwPacketNumber < Player1->GetState().dwPacketNumber) {
					if ((Player1->GetState().Gamepad.wButtons & button.first) == 0
					    && (previous.Gamepad.wButtons & button.first) != 0) {
						WORD mapping = (Player1->Buttons.find(button.first) != Player1->Buttons.end() ? Player1->Buttons.find(button.first)->second : button.first);
						keybd_event(mapping, 0, KEYEVENTF_KEYUP, 0);
					}
				}
			}
		} else {
			sprintf(tempstr, "PLAYER 1: CONTROLLER DISCONNECTED");
			NetSendCmdString(1 << myplr, tempstr);
			//break;
		}
	}
}
