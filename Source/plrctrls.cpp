#include "../types.h"

// JAKE: My functions for movement and interaction via keyboard/controller
bool newCurHidden = false;

// 0 = not near, >0 = distance near plr
int checkNearbyObjs(int x, int y, int diff)
{
	int diff_x = abs(plr[myplr]._px - x);
	int diff_y = abs(plr[myplr]._py - y);

	if (diff_x <= diff && diff_y <= diff) 
		return diff_x;
	return 0;
}

void __fastcall checkItemsNearby(bool interact)
{
	for (int i = 0; i < MAXITEMS; i++) {
		if (checkNearbyObjs(item[i]._ix, item[i]._iy, 1) != 0) {
			pcursitem = i;
			if (interact) {
				LeftMouseCmd(false);
			}
			break;
		}
	}
	for (int i = 0; i < MAXOBJECTS; i++) {
		if (checkNearbyObjs(object[i]._ox, object[i]._oy, 1) != 0) {
			if (object[i]._oSelFlag == 0) // already opened chests, torches, etc.
				continue;
			pcursobj = i;
			if (interact) {
				LeftMouseCmd(false);
			}
			break;
		}
	}
}

void __fastcall checkTownersNearby(bool interact)
{
	for (int i = 0; i < 16; i++) {
		if (checkNearbyObjs(towner[i]._tx, towner[i]._ty, 1) != 0) {
			pcursmonst = i;
			if (interact)
				TalkToTowner(myplr, i);
			break;
		}
	}
}

bool __fastcall checkMonstersNearby(bool attack, bool castspell)
{
	int closest = 0; // monster ID who is closest
	int objDistLast = 99; // previous obj distance
	for (int i = 0; i < MAXMONSTERS; i++) {
		if (monster[i]._mFlags & MFLAG_HIDDEN || monster[i]._mhitpoints <= 0)
			continue;
		int objDist = checkNearbyObjs(monster[i]._mx, monster[i]._my, 6);
		if (objDist > 0 && objDist < objDistLast) {
			closest = i;
			objDistLast = objDist;
		}
	}
	if (closest > 0) // did we find a monster? 
		pcursmonst = closest;
	else
		return false;
	if (attack) {
		LeftMouseCmd(false);
	} else if (castspell) {
		RightMouseDown();
	}
	return false;
}

void HideCursor()
{
	SetCursorPos(320, 180);
	FreeCursor();
	newCurHidden = true;
}

void __fastcall keyboardExpension()
{
	if (stextflag || questlog || helpflag || invflag || talkflag || qtextflag)
		return;
	if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
		return;
	if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
		HideCursor();
		checkTownersNearby(true);
		checkMonstersNearby(true, false);
	} else if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
		HideCursor();
		checkItemsNearby(true);
	} else if (GetAsyncKeyState(0x58) & 0x8000) { // x key
		HideCursor();
		checkMonstersNearby(false, true);
	} else if (GetAsyncKeyState(VK_RIGHT) & 0x8000 && GetAsyncKeyState(VK_DOWN) & 0x8000 || GetAsyncKeyState(0x44) & 0x8000 && GetAsyncKeyState(0x53) & 0x8000) {
		HideCursor();
		plr[myplr].walkpath[0] = WALK_SE;
	} else if (GetAsyncKeyState(VK_RIGHT) & 0x8000 && GetAsyncKeyState(VK_UP) & 0x8000 || GetAsyncKeyState(0x57) & 0x8000 && GetAsyncKeyState(0x44) & 0x8000) {
		HideCursor();
		plr[myplr].walkpath[0] = WALK_NE;
	} else if (GetAsyncKeyState(VK_LEFT) & 0x8000 && GetAsyncKeyState(VK_DOWN) & 0x8000 || GetAsyncKeyState(0x41) & 0x8000 && GetAsyncKeyState(0x53) & 0x8000) {
		HideCursor();
		plr[myplr].walkpath[0] = WALK_SW;
	} else if (GetAsyncKeyState(VK_LEFT) & 0x8000 && GetAsyncKeyState(VK_UP) & 0x8000 || GetAsyncKeyState(0x57) & 0x8000 && GetAsyncKeyState(0x41) & 0x8000) {
		HideCursor();
		plr[myplr].walkpath[0] = WALK_NW;
	} else if (GetAsyncKeyState(VK_UP) & 0x8000 || GetAsyncKeyState(0x57) & 0x8000) {
		HideCursor();
		plr[myplr].walkpath[0] = WALK_N;
	} else if (GetAsyncKeyState(VK_RIGHT) & 0x8000 || GetAsyncKeyState(0x44) & 0x8000) {
		HideCursor();
		plr[myplr].walkpath[0] = WALK_E;
	} else if (GetAsyncKeyState(VK_DOWN) & 0x8000 || GetAsyncKeyState(0x53) & 0x8000) {
		HideCursor();
		plr[myplr].walkpath[0] = WALK_S;
	} else if (GetAsyncKeyState(VK_LEFT) & 0x8000 || GetAsyncKeyState(0x41) & 0x8000) {
		HideCursor();
		plr[myplr].walkpath[0] = WALK_W;
	}
	//ShowCursor(FALSE); // doesnt really hide the cursor
}
