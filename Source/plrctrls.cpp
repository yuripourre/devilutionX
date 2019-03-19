#include "../types.h"

// JAKE: My functions for movement and interaction via keyboard/controller

bool checkNearbyObjs(int x, int y, int diff)
{
	int diff_x = abs(plr[myplr]._px - x);
	int diff_y = abs(plr[myplr]._py - y);

	if (diff_x <= diff && diff_y <= diff)
		return true;
	return false;
}

void __fastcall checkItemsNearby(bool interact)
{
	for (int i = 0; i < MAXITEMS; i++) {
		if (checkNearbyObjs(item[i]._ix, item[i]._iy, 1)) {
			pcursitem = i;
			if (interact) {
				LeftMouseCmd(false);
			}
			return;
		}
	}
	for (int i = 0; i < MAXOBJECTS; i++) {
		if (checkNearbyObjs(object[i]._ox, object[i]._oy, 1)) {
			int ot = object[i]._otype;
			if (ot == OBJ_CANDLE1 || ot == OBJ_CANDLE2 || ot == OBJ_CANDLEO || ot == OBJ_L1LIGHT || ot == OBJ_TORCHL || ot == OBJ_TORCHR)
				continue;
			pcursobj = i;
			if (interact) {
				LeftMouseCmd(false);
			}
			return;
		}
	}
}

void __fastcall checkTownersNearby(bool interact)
{
	for (int i = 0; i < 16; i++) {
		if (checkNearbyObjs(towner[i]._tx, towner[i]._ty, 1)) {
			pcursmonst = i;
			if (interact)
				TalkToTowner(myplr, i);
			return;
		}
	}
}

void __fastcall checkMonstersNearby(bool attack)
{
	for (int i = 0; i < MAXMONSTERS; i++) {
		bool cN = false;
		if (plr[myplr]._pwtype == WT_MELEE)
			cN = checkNearbyObjs(monster[i]._mx, monster[i]._my, 1);
		else if (plr[myplr]._pwtype == WT_RANGED)
			cN = checkNearbyObjs(monster[i]._mx, monster[i]._my, 6);
		if (cN && monster[i]._mhitpoints > 0) {
			if (attack) {
				LeftMouseCmd(false);
			}
			pcursmonst = i;
			//sprintf(tempstr, "ATTACKING NEARBY MONSTER! PX:%i PY:%i MX:%i MY:%i", plr[myplr]._px, plr[myplr]._py, monster[i]._mx, monster[i]._my);
			//NetSendCmdString(1 << myplr, tempstr);
			return; // just attack 1 monster
		}
	}
}

void __fastcall keyboardExpension()
{
	if (stextflag || questlog || helpflag || invflag || talkflag || qtextflag)
		return;
	if (GetAsyncKeyState(VK_SHIFT))
		return;
	if (GetAsyncKeyState(VK_SPACE)) {
		SetCursorPos(-1, -1);
		checkTownersNearby(true);
		checkMonstersNearby(true);
	} else if (GetAsyncKeyState(VK_RETURN)) {
		SetCursorPos(-1, -1);
		checkItemsNearby(true);
	}
	else if (GetAsyncKeyState(VK_RIGHT) && GetAsyncKeyState(VK_DOWN) || GetAsyncKeyState(0x44) && GetAsyncKeyState(0x53)) {
		plr[myplr].walkpath[0] = WALK_SE;
		SetCursorPos(-1, -1);
	} else if (GetAsyncKeyState(VK_RIGHT) && GetAsyncKeyState(VK_UP) || GetAsyncKeyState(0x57) && GetAsyncKeyState(0x44)) {
		plr[myplr].walkpath[0] = WALK_NE;
		SetCursorPos(-1, -1);
	} else if (GetAsyncKeyState(VK_LEFT) && GetAsyncKeyState(VK_DOWN) || GetAsyncKeyState(0x41) && GetAsyncKeyState(0x53)) {
		plr[myplr].walkpath[0] = WALK_SW;
		SetCursorPos(-1, -1);
	} else if (GetAsyncKeyState(VK_LEFT) && GetAsyncKeyState(VK_UP) || GetAsyncKeyState(0x57) && GetAsyncKeyState(0x41)) {
		plr[myplr].walkpath[0] = WALK_NW;
		SetCursorPos(-1, -1);
	} else if (GetAsyncKeyState(VK_UP) || GetAsyncKeyState(0x57)) {
		plr[myplr].walkpath[0] = WALK_N;
		SetCursorPos(-1, -1);
	} else if (GetAsyncKeyState(VK_RIGHT) || GetAsyncKeyState(0x44)) {
		plr[myplr].walkpath[0] = WALK_E;
		SetCursorPos(-1, -1);
	} else if (GetAsyncKeyState(VK_DOWN) || GetAsyncKeyState(0x53)) {
		plr[myplr].walkpath[0] = WALK_S;
		SetCursorPos(-1, -1);
	} else if (GetAsyncKeyState(VK_LEFT) || GetAsyncKeyState(0x41)) {
		plr[myplr].walkpath[0] = WALK_W;
		SetCursorPos(-1, -1);
	}
	//ShowCursor(FALSE); // doesnt really hide the cursor
}
