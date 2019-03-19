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
				//AutoGetItem(myplr, i);
				MakePlrPath(myplr, item[i]._ix, item[i]._iy, FALSE); // TEST
			}
			return;
		}
	}
	for (int i = 0; i < MAXOBJECTS; i++) {
		if (checkNearbyObjs(object[i]._ox, object[i]._oy, 1)) {
			pcursobj = i;
			if (interact) {
				//OperateChest(myplr, i, 0);
				MakePlrPath(myplr, object[i]._ox, object[i]._oy, FALSE); // TEST
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
				//int d = GetDirection(plr[myplr]._px, plr[myplr]._py, monster[i]._mx, monster[i]._my);
				//StartAttack(myplr, d);
				MakePlrPath(myplr, monster[i]._mx, monster[i]._my, FALSE); // TEST
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
	if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
		checkTownersNearby(true);
		checkMonstersNearby(true);
	} else if (GetAsyncKeyState(VK_RETURN) & 0x8000)
		checkItemsNearby(true);
	else if (GetAsyncKeyState(VK_RIGHT) && GetAsyncKeyState(VK_DOWN) || GetAsyncKeyState(0x44) && GetAsyncKeyState(0x53))
		plr[myplr].walkpath[0] = WALK_SE;
	else if (GetAsyncKeyState(VK_RIGHT) && GetAsyncKeyState(VK_UP) || GetAsyncKeyState(0x57) && GetAsyncKeyState(0x44))
		plr[myplr].walkpath[0] = WALK_NE;
	else if (GetAsyncKeyState(VK_LEFT) && GetAsyncKeyState(VK_DOWN) || GetAsyncKeyState(0x41) && GetAsyncKeyState(0x53))
		plr[myplr].walkpath[0] = WALK_SW;
	else if (GetAsyncKeyState(VK_LEFT) && GetAsyncKeyState(VK_UP) || GetAsyncKeyState(0x57) && GetAsyncKeyState(0x41))
		plr[myplr].walkpath[0] = WALK_NW;
	else if (GetAsyncKeyState(VK_UP) || GetAsyncKeyState(0x57))
		plr[myplr].walkpath[0] = WALK_N;
	else if (GetAsyncKeyState(VK_RIGHT) || GetAsyncKeyState(0x44))
		plr[myplr].walkpath[0] = WALK_E;
	else if (GetAsyncKeyState(VK_DOWN) || GetAsyncKeyState(0x53))
		plr[myplr].walkpath[0] = WALK_S;
	else if (GetAsyncKeyState(VK_LEFT) || GetAsyncKeyState(0x41))
		plr[myplr].walkpath[0] = WALK_W;
	ShowCursor(FALSE); // TEST
}
