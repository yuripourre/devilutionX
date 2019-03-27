#include "../types.h"

#include <algorithm>
#include <list>
#include <map>
#include <string>
#pragma comment(lib, "xinput.lib")
#include <Xinput.h>

std::list<WORD> heldBtns;

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

CXBOXController *Player1;

CXBOXController::CXBOXController(int playerNumber)
{
	_controllerNum = playerNumber - 1;
}

XINPUT_STATE CXBOXController::GetState()
{
	ZeroMemory(&_controllerState, sizeof(XINPUT_STATE));
	XInputGetState(_controllerNum, &_controllerState);

	return _controllerState;
}

bool CXBOXController::IsConnected()
{
	ZeroMemory(&_controllerState, sizeof(XINPUT_STATE));
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

void ReleaseAllButtons()
{
	for (std::map<WORD, int>::iterator it = Player1->Buttons.begin(); it != Player1->Buttons.end(); ++it) {
		keybd_event(it->second, 0, KEYEVENTF_KEYUP, 0);
		heldBtns.remove(it->first);
	}
}

XINPUT_STATE previous;
bool releaseController = false;
void CheckForController()
{
	Player1 = new CXBOXController(1);
	
	Player1->Buttons.insert(std::pair<WORD, int>(XINPUT_GAMEPAD_B, 0x49)); // I key
	Player1->Buttons.insert(std::pair<WORD, int>(XINPUT_GAMEPAD_X, VK_RETURN));
	if (inmainmenu) {
		Player1->Buttons.insert(std::pair<WORD, int>(XINPUT_GAMEPAD_A, VK_RETURN));
	} else {
		Player1->Buttons.insert(std::pair<WORD, int>(XINPUT_GAMEPAD_A, VK_SPACE));
	}
	Player1->Buttons.insert(std::pair<WORD, int>(XINPUT_GAMEPAD_Y, 0x58)); // X key
	Player1->Buttons.insert(std::pair<WORD, int>(XINPUT_GAMEPAD_BACK, VK_TAB));
	Player1->Buttons.insert(std::pair<WORD, int>(XINPUT_GAMEPAD_START, VK_ESCAPE));
	Player1->Buttons.insert(std::pair<WORD, int>(XINPUT_GAMEPAD_LEFT_SHOULDER, 0x48));  // H key
	Player1->Buttons.insert(std::pair<WORD, int>(XINPUT_GAMEPAD_RIGHT_SHOULDER, 0x43)); // C key
	Player1->Buttons.insert(std::pair<WORD, int>(XINPUT_GAMEPAD_DPAD_UP, VK_UP));
	Player1->Buttons.insert(std::pair<WORD, int>(XINPUT_GAMEPAD_DPAD_DOWN, VK_DOWN));
	Player1->Buttons.insert(std::pair<WORD, int>(XINPUT_GAMEPAD_DPAD_LEFT, VK_LEFT));
	Player1->Buttons.insert(std::pair<WORD, int>(XINPUT_GAMEPAD_DPAD_RIGHT, VK_RIGHT));

LABEL_14:
	while (Player1->IsConnected()) {
		releaseController = true;
		for (auto button : Player1->Buttons) {
			if ((Player1->GetState().Gamepad.wButtons & button.first) != 0) { // currently pressing
				WORD mapping = (Player1->Buttons.find(button.first) != Player1->Buttons.end() ? Player1->Buttons.find(button.first)->second : button.first);
				if (inmainmenu && mapping == VK_SPACE) // in main menu, swap space for return
					mapping = VK_RETURN;
				keybd_event(mapping, 0, 0, 0);
				heldBtns.push_back(button.first);
			}

			bool found = (std::find(heldBtns.begin(), heldBtns.end(), button.first) != heldBtns.end());
			if ((Player1->GetState().Gamepad.wButtons & button.first) == 0 && found) {
				WORD mapping = (Player1->Buttons.find(button.first) != Player1->Buttons.end() ? Player1->Buttons.find(button.first)->second : button.first);
				keybd_event(mapping, 0, KEYEVENTF_KEYUP, 0);
				heldBtns.remove(button.first);
			}
		}
		if (inmainmenu) {
			::Sleep(100);
			ReleaseAllButtons();
		}
	}

	if (releaseController) { // release all the keys
		ReleaseAllButtons();
		releaseController = false;
	}

	goto LABEL_14; // try again
}
