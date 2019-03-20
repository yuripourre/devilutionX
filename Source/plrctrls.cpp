#include "../types.h"

// JAKE: My functions for movement and interaction via keyboard/controller
bool newCurHidden = false;
int dh = DISPLAY_HEIGHT;
int dw = DISPLAY_WIDTH;
int inv_top = INV_TOP;
int inv_left = INV_LEFT;
int inv_height = INV_HEIGHT;

// 0 = not near, >0 = distance related player 1 coordinates
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
	int closest = 0;      // monster ID who is closest
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

// hide the cursor when we start walking via keyboard/controller
void HideCursor()
{
	SetCursorPos((dh/2), (dw/2));
	FreeCursor();
	newCurHidden = true;
}

// move the cursor around in our inventory
// if mouse coords are at SLOTXY_CHEST_LAST, consider this center of equipment
// small inventory squares are 29x29 (roughly)
void invMove(int key)
{
	if (!invflag)
		return;
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

void __fastcall keyboardExpension()
{
	if (stextflag || questlog || helpflag || talkflag || qtextflag)
		return;
	if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
		return;
	if (GetAsyncKeyState(VK_SPACE) & 0x8000) { // similar to X button on PS1 ccontroller. Talk to towners, click on inv items, attack.
		if (invflag) {
			LeftMouseCmd(false);
		} else {
			HideCursor();
			checkTownersNearby(true);
			checkMonstersNearby(true, false);
		}
	} else if (GetAsyncKeyState(VK_RETURN) & 0x8000) { // similar to [] button on PS1 controller. Open chests, doors, pickup items
		HideCursor();
		checkItemsNearby(true);
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
