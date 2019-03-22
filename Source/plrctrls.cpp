#include "../types.h"

// JAKE: My functions for movement and interaction via keyboard/controller
bool newCurHidden = false;
int dh = DISPLAY_HEIGHT;
int dw = DISPLAY_WIDTH;
int inv_top = INV_TOP;
int inv_left = INV_LEFT;
int inv_height = INV_HEIGHT;

struct closestMonster {
	int x;
	int y;
};

// 0 = not near, >0 = distance related player 1 coordinates
closestMonster checkNearbyObjs(int x, int y, int diff)
{
	int diff_x = abs(plr[myplr]._px - x);
	int diff_y = abs(plr[myplr]._py - y);

	if (diff_x <= diff && diff_y <= diff) {
		closestMonster cm = { diff_x, diff_y };
		return cm;
	}
	return {-1, -1};
}

void __fastcall checkItemsNearby(bool interact)
{
	for (int i = 0; i < MAXITEMS; i++) {
		if (checkNearbyObjs(item[i]._ix, item[i]._iy, 1).x != -1 && item[i]._iSelFlag > 0) {
			pcursitem = i;
			//if (newCurHidden) {
				if (dItem[item[i]._ix][item[i]._iy] <= 0)
					continue;
				//SetCursorPos(item[i]._ix, item[i]._iy);
			//}
			//sprintf(tempstr, "FOUND NEARBY ITEM AT X:%i Y:%i SEL:%i", item[i]._ix, item[i]._iy, item[i]._iSelFlag);
			//NetSendCmdString(1 << myplr, tempstr);
			if (interact) {
				LeftMouseCmd(false);
			}
			return; // item nearby, don't find objects
		}
	}
	if (newCurHidden)
		pcursitem = -1;
	//sprintf(tempstr, "SCANNING FOR OBJECTS");
	//NetSendCmdString(1 << myplr, tempstr);
	for (int i = 0; i < MAXOBJECTS; i++) {
		if (checkNearbyObjs(object[i]._ox, object[i]._oy, 1).x != -1 && object[i]._oSelFlag > 0) {
			pcursobj = i;
			if (interact) {
				LeftMouseCmd(false);
			}
			return;
		}
	}
	if (newCurHidden)
		pcursobj = -1;
}

void __fastcall checkTownersNearby(bool interact)
{
	for (int i = 0; i < 16; i++) {
		if (checkNearbyObjs(towner[i]._tx, towner[i]._ty, 2).x != -1) {
			pcursmonst = i;
			if (interact)
				TalkToTowner(myplr, i);
			break;
		}
	}
}

bool __fastcall checkMonstersNearby(bool attack, bool castspell)
{
	int closest = 0;      // monster ID who is closest
	closestMonster objDistLast = { 99, 99 }; // previous obj distance
	for (int i = 0; i < MAXMONSTERS; i++) {
		int d_monster = dMonster[monster[i]._mx][monster[i]._my + 1];
		if (monster[i]._mFlags & MFLAG_HIDDEN || monster[i]._mhitpoints <= 0) // is monster alive and not hiding
			continue;
		//if (d_monster && dFlags[monster[i]._mx][monster[i]._my + 1] & DFLAG_LIT) { // is monster visible
		//	if (monster[i].MData->mSelFlag & 1 || monster[i].MData->mSelFlag & 2 || monster[i].MData->mSelFlag & 3 || monster[i].MData->mSelFlag & 4) { // is monster selectable
				closestMonster objDist = checkNearbyObjs(monster[i]._mx, monster[i]._my, 6);
		if (objDist.x > -1 && objDist.x < objDistLast.x && objDist.y < objDistLast.y) {
			closest = i;
			objDistLast = objDist;
		}
		//	}
		//}
	}
	if (closest > 0 && (attack || castspell)) { // did we find a monster, and want to attack it?
		pcursmonst = closest;
		//sprintf(tempstr, "NEARBY MONSTER WITH HP:%i", monster[closest]._mhitpoints);
		//NetSendCmdString(1 << myplr, tempstr);
	} else if (closest > 0){ // found monster, but we don't want to attack it
		return true;
	} else {
		pcursmonst = -1;
		return false;
	}
	if (attack) {
		LeftMouseCmd(false);
		return true;
	} else if (castspell) {
		RightMouseDown();
		return true;
	} else {
		return true;
	}
	pcursmonst = -1;
	return false;
}

// hide the cursor when we start walking via keyboard/controller
void HideCursor()
{
	SetCursorPos(320, 180);
	//FreeCursor(); // glitches potion belt
	//DestroyCursor(LoadCursor(0, IDC_ARROW)); // doesnt work
	//pcurs = CURSOR_NONE; // makes potions unusable
	SetCursor_(CURSOR_NONE); // works?
	newCurHidden = true;
}

static DWORD invmove;
DWORD ticks;

// move the cursor around in our inventory
// if mouse coords are at SLOTXY_CHEST_LAST, consider this center of equipment
// small inventory squares are 29x29 (roughly)
void invMove(int key)
{
	if (!invflag)
		return;
	if (ticks - invmove < 150) {
		return;
	}
	invmove = ticks;
	int x = MouseX;
	int y = MouseY;
	if (key == VK_LEFT) {
		if (x == InvRect[SLOTXY_CHEST_LAST].X && y == (InvRect[SLOTXY_CHEST_LAST].Y - 5)) { // chest to left hand
			x = InvRect[SLOTXY_HAND_LEFT_LAST].X;
			y = InvRect[SLOTXY_HAND_LEFT_LAST].Y - 5;
		} else if (x == InvRect[SLOTXY_HAND_RIGHT_LAST].X && y == (InvRect[SLOTXY_HAND_RIGHT_LAST].Y - 5)) { // right hand to chest
			x = InvRect[SLOTXY_CHEST_LAST].X;
			y = InvRect[SLOTXY_CHEST_LAST].Y - 5;
		} else if (x == (InvRect[SLOTXY_RING_RIGHT].X + 5) && y == (InvRect[SLOTXY_RING_RIGHT].Y - 5)) { // right ring to left ring
			x = InvRect[SLOTXY_RING_LEFT].X + 5;
			y = InvRect[SLOTXY_RING_LEFT].Y - 5;
		} else if (x == (InvRect[SLOTXY_AMULET].X + 5) && y == (InvRect[SLOTXY_AMULET].Y - 5)) { // amulet to head
			x = InvRect[SLOTXY_HEAD_LAST].X;
			y = InvRect[SLOTXY_HEAD_LAST].Y - 5;
		} else if (x < dw && x > inv_left && y >= inv_top) {
			x = x - 29;
		}
	} else if (key == VK_RIGHT) {
		if (x == InvRect[SLOTXY_CHEST_LAST].X && y == (InvRect[SLOTXY_CHEST_LAST].Y - 5)) { // chest to right hand
			x = InvRect[SLOTXY_HAND_RIGHT_LAST].X;
			y = InvRect[SLOTXY_HAND_RIGHT_LAST].Y - 5;
		} else if (x == InvRect[SLOTXY_HAND_LEFT_LAST].X && y == (InvRect[SLOTXY_HAND_LEFT_LAST].Y - 5)) { // left hand to chest
			x = InvRect[SLOTXY_CHEST_LAST].X;
			y = InvRect[SLOTXY_CHEST_LAST].Y - 5;
		} else if (x == (InvRect[SLOTXY_RING_LEFT].X + 5) && y == (InvRect[SLOTXY_RING_LEFT].Y - 5)) { // left ring to right ring
			x = InvRect[SLOTXY_RING_RIGHT].X + 5;
			y = InvRect[SLOTXY_RING_RIGHT].Y - 5;
		} else if (x == InvRect[SLOTXY_HEAD_LAST].X && y == (InvRect[SLOTXY_HEAD_LAST].Y - 5)) { // head to amulet
			x = InvRect[SLOTXY_AMULET].X + 5;
			y = InvRect[SLOTXY_AMULET].Y - 5;
		} else if (x < dw && x >= inv_left && y >= inv_top) {
			x = x + 29;
		}
	} else if (key == VK_UP) {
		if (y > inv_top && x >= inv_left) {
			y = y - 29;
		} else { // go into equipment spaces
			if (x == InvRect[SLOTXY_CHEST_LAST].X && y == (InvRect[SLOTXY_CHEST_LAST].Y - 5)) { // if at chest, go to head
				x = InvRect[SLOTXY_HEAD_LAST].X;
				y = InvRect[SLOTXY_HEAD_LAST].Y - 5;
			} else if (x == (InvRect[SLOTXY_RING_RIGHT].X + 5) && y == (InvRect[SLOTXY_RING_RIGHT].Y - 5)) { // if at right ring, go to right hand
				x = InvRect[SLOTXY_HAND_RIGHT_LAST].X;
				y = InvRect[SLOTXY_HAND_RIGHT_LAST].Y - 5;
			} else if (x == (InvRect[SLOTXY_RING_LEFT].X + 5) && y == (InvRect[SLOTXY_RING_LEFT].Y - 5)) { // if at left ring, go to left hand
				x = InvRect[SLOTXY_HAND_LEFT_LAST].X;
				y = InvRect[SLOTXY_HAND_LEFT_LAST].Y - 5;
			} else if (x == InvRect[SLOTXY_HAND_LEFT_LAST].X && y == (InvRect[SLOTXY_HAND_LEFT_LAST].Y - 5)) { // if at left hand, go to head
				x = InvRect[SLOTXY_HEAD_LAST].X;
				y = InvRect[SLOTXY_HEAD_LAST].Y - 5;
			} else if (x == InvRect[SLOTXY_HAND_RIGHT_LAST].X && y == (InvRect[SLOTXY_HAND_RIGHT_LAST].Y - 5)) { // if at right hand, go to amulet
				x = InvRect[SLOTXY_AMULET].X + 5;
				y = InvRect[SLOTXY_AMULET].Y - 5;
			} else if (y == inv_top) { // general inventory, go to chest
				if (x >= inv_left && x < 422) { // left side goes up to left ring
					x = InvRect[SLOTXY_RING_LEFT].X + 5;
					y = InvRect[SLOTXY_RING_LEFT].Y - 5;
				} else if (x > 567 && x < dw) { // right side goes up to right ring
					x = InvRect[SLOTXY_RING_RIGHT].X + 5;
					y = InvRect[SLOTXY_RING_RIGHT].Y - 5;
				} else { // center goes to chest
					x = InvRect[SLOTXY_CHEST_LAST].X;
					y = InvRect[SLOTXY_CHEST_LAST].Y - 5;
				}
			}
		}
	} else if (key == VK_DOWN) {
		if ((x == InvRect[SLOTXY_CHEST_LAST].X && y == (InvRect[SLOTXY_CHEST_LAST].Y - 5)) ||
			(x == (InvRect[SLOTXY_RING_LEFT].X + 5) && y == (InvRect[SLOTXY_RING_LEFT].Y - 5)) ||
			(x == (InvRect[SLOTXY_RING_RIGHT].X + 5) && y == (InvRect[SLOTXY_RING_RIGHT].Y - 5))) { // go back to general inventory
			x = 350;
			y = 240;
		} else if ((x == InvRect[SLOTXY_AMULET].X + 5) && y == (InvRect[SLOTXY_AMULET].Y - 5)) { // if at amulet, go to right hand
			x = InvRect[SLOTXY_HAND_RIGHT_LAST].X;
			y = InvRect[SLOTXY_HAND_RIGHT_LAST].Y - 5;
		} else if (x == InvRect[SLOTXY_HEAD_LAST].X && y == (InvRect[SLOTXY_HEAD_LAST].Y - 5)) { // if at head, go to chest
			x = InvRect[SLOTXY_CHEST_LAST].X;
			y = InvRect[SLOTXY_CHEST_LAST].Y - 5;
		} else if (x == InvRect[SLOTXY_HAND_LEFT_LAST].X && y == (InvRect[SLOTXY_HAND_LEFT_LAST].Y - 5)) { // if at left hand, go to left ring
			x = InvRect[SLOTXY_RING_LEFT].X + 5;
			y = InvRect[SLOTXY_RING_LEFT].Y - 5;
		} else if (x == InvRect[SLOTXY_HAND_RIGHT_LAST].X && y == (InvRect[SLOTXY_HAND_RIGHT_LAST].Y - 5)) { // if at right hand, go to right ring
			x = InvRect[SLOTXY_RING_RIGHT].X + 5;
			y = InvRect[SLOTXY_RING_RIGHT].Y - 5;
		} else if (y >= inv_top && y < inv_height) { // general inventory area
			y = y + 29;
		}
	}
	SetCursorPos(x, y);
}

// walk in the direction specified
void walkInDir(int dir)
{
	if (invflag) // don't walk if inventory window is open
		return;
	HideCursor();
	plr[myplr].walkpath[0] = dir;
}

// clicks outside of game
/*void realClick(int x, int y)
{
	INPUT input;
	input.type = INPUT_MOUSE;
	input.mi.dx = x;
	input.mi.dy = y;
	input.mi.dwFlags = (MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP);
	input.mi.mouseData = 0;
	input.mi.dwExtraInfo = NULL;
	input.mi.time = 0;
	SendInput(1, &input, sizeof(INPUT));
}*/

void __fastcall keyboardExpension()
{
	static DWORD opentimer;
	static DWORD clickinvtimer;
	ticks = GetTickCount();

	if (stextflag || questlog || helpflag || talkflag || qtextflag)
		return;
	if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
		return;
	if (GetAsyncKeyState(VK_SPACE) & 0x8000) { // similar to X button on PS1 ccontroller. Talk to towners, click on inv items, attack.
		if (invflag) { // inventory is open
			//sprintf(tempstr, "INVENTORY IS OPEN, CLICKING X:%i Y:%i", MouseX, MouseY);
			//NetSendCmdString(1 << myplr, tempstr);
			if (ticks - clickinvtimer >= 300) {
				clickinvtimer = ticks;
				CheckInvItem();
			}
		} else {
			HideCursor();
			checkTownersNearby(true);
			checkMonstersNearby(true, false);
		}
	} else if (GetAsyncKeyState(VK_RETURN) & 0x8000) { // similar to [] button on PS1 controller. Open chests, doors, pickup items
		HideCursor();
		if (ticks - opentimer >= 300) {
			opentimer = ticks;
			checkItemsNearby(true);
		}
	} else if (GetAsyncKeyState(0x58) & 0x8000) { // x key, similar to /\ button on PS1 controller. Cast spell or use skill.
		HideCursor();
		checkMonstersNearby(false, true);
	} else if (GetAsyncKeyState(VK_RIGHT) & 0x8000 && GetAsyncKeyState(VK_DOWN) & 0x8000 || GetAsyncKeyState(0x44) & 0x8000 && GetAsyncKeyState(0x53) & 0x8000) {
		walkInDir(WALK_SE);
	} else if (GetAsyncKeyState(VK_RIGHT) & 0x8000 && GetAsyncKeyState(VK_UP) & 0x8000 || GetAsyncKeyState(0x57) & 0x8000 && GetAsyncKeyState(0x44) & 0x8000) {
		walkInDir(WALK_NE);
	} else if (GetAsyncKeyState(VK_LEFT) & 0x8000 && GetAsyncKeyState(VK_DOWN) & 0x8000 || GetAsyncKeyState(0x41) & 0x8000 && GetAsyncKeyState(0x53) & 0x8000) {
		walkInDir(WALK_SW);
	} else if (GetAsyncKeyState(VK_LEFT) & 0x8000 && GetAsyncKeyState(VK_UP) & 0x8000 || GetAsyncKeyState(0x57) & 0x8000 && GetAsyncKeyState(0x41) & 0x8000) {
		walkInDir(WALK_NW);
	} else if (GetAsyncKeyState(VK_UP) & 0x8000 || GetAsyncKeyState(0x57) & 0x8000) {
		invMove(VK_UP);
		walkInDir(WALK_N);
	} else if (GetAsyncKeyState(VK_RIGHT) & 0x8000 || GetAsyncKeyState(0x44) & 0x8000) {
		invMove(VK_RIGHT);
		walkInDir(WALK_E);
	} else if (GetAsyncKeyState(VK_DOWN) & 0x8000 || GetAsyncKeyState(0x53) & 0x8000) {
		invMove(VK_DOWN);
		walkInDir(WALK_S);
	} else if (GetAsyncKeyState(VK_LEFT) & 0x8000 || GetAsyncKeyState(0x41) & 0x8000) {
		invMove(VK_LEFT);
		walkInDir(WALK_W);
	}
}
