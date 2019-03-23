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
	return { -1, -1 };
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
	int closest = 0;                         // monster ID who is closest
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
	} else if (closest > 0) { // found monster, but we don't want to attack it
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
int slot = 25;
// move the cursor around in our inventory
// if mouse coords are at SLOTXY_CHEST_LAST, consider this center of equipment
// small inventory squares are 29x29 (roughly)
void invMove(int key)
{
	if (!invflag)
		return;
	if (ticks - invmove < 80) {
		return;
	}
	invmove = ticks;
	int x = MouseX;
	int y = MouseY;

	// check which inventory rectangle the mouse is in, if any
	for (int r = 0; (DWORD)r < NUM_XY_SLOTS; r++) {
		if (x >= InvRect[r].X && x < InvRect[r].X + (INV_SLOT_SIZE_PX + 1) && y >= InvRect[r].Y - (INV_SLOT_SIZE_PX + 1) && y < InvRect[r].Y) {
			slot = r;
			break;
		}
	}

	if (slot < 0)
		slot = 0;
	if (slot > SLOTXY_BELT_LAST)
		slot = SLOTXY_BELT_LAST;

	// when item is on cursor, this is the real cursor XY
	if (key == VK_LEFT) {
			if (slot >= SLOTXY_HAND_RIGHT_FIRST && slot <= SLOTXY_HAND_RIGHT_LAST) {
				x = InvRect[SLOTXY_CHEST_FIRST + 3].X + (INV_SLOT_SIZE_PX / 2);
				y = InvRect[SLOTXY_CHEST_FIRST + 3].Y - (INV_SLOT_SIZE_PX / 2);
			} else if (slot >= SLOTXY_CHEST_FIRST && slot <= SLOTXY_CHEST_LAST) {
				x = InvRect[SLOTXY_HAND_LEFT_FIRST + 2].X + (INV_SLOT_SIZE_PX / 2);
				y = InvRect[SLOTXY_HAND_LEFT_FIRST + 2].Y - (INV_SLOT_SIZE_PX / 2);
			} else if (slot == SLOTXY_AMULET) {
				x = InvRect[SLOTXY_HEAD_FIRST + 1].X + (INV_SLOT_SIZE_PX / 2);
				y = InvRect[SLOTXY_HEAD_FIRST + 1].Y - (INV_SLOT_SIZE_PX / 2);
			} else if (slot == SLOTXY_RING_RIGHT) {
				x = InvRect[SLOTXY_RING_LEFT].X + (INV_SLOT_SIZE_PX / 2);
				y = InvRect[SLOTXY_RING_LEFT].Y - (INV_SLOT_SIZE_PX / 2);
		    } else if (slot == SLOTXY_RING_LEFT) { // left ring
				// do nothing
		    } else if (slot >= SLOTXY_HAND_LEFT_FIRST && slot <= SLOTXY_HAND_LEFT_LAST) { // left hand
				// do nothing
		    } else if (slot >= SLOTXY_HEAD_FIRST && slot <= SLOTXY_HEAD_LAST) { // head
				// do nothing
		    } else if (slot > SLOTXY_INV_FIRST) { // general inventory
			    if (slot != SLOTXY_INV_FIRST && slot != 35 && slot != 45 && slot != 55) { // left bounds
					slot -= 1;
					x = InvRect[slot].X + (INV_SLOT_SIZE_PX / 2);
					y = InvRect[slot].Y - (INV_SLOT_SIZE_PX / 2);
				}
		    }
	} else if (key == VK_RIGHT) {
			if (slot == SLOTXY_RING_LEFT) {
				x = InvRect[SLOTXY_RING_RIGHT].X + (INV_SLOT_SIZE_PX / 2);
				y = InvRect[SLOTXY_RING_RIGHT].Y - (INV_SLOT_SIZE_PX / 2);
			} else if (slot >= SLOTXY_HAND_LEFT_FIRST && slot <= SLOTXY_HAND_LEFT_LAST) {
				x = InvRect[SLOTXY_CHEST_FIRST + 3].X + (INV_SLOT_SIZE_PX / 2);
				y = InvRect[SLOTXY_CHEST_FIRST + 3].Y - (INV_SLOT_SIZE_PX / 2);
			} else if (slot >= SLOTXY_CHEST_FIRST && slot <= SLOTXY_CHEST_LAST) {
				x = InvRect[SLOTXY_HAND_RIGHT_FIRST + 2].X + (INV_SLOT_SIZE_PX / 2);
				y = InvRect[SLOTXY_HAND_RIGHT_FIRST + 2].Y - (INV_SLOT_SIZE_PX / 2);
			} else if (slot >= SLOTXY_HEAD_FIRST && slot <= SLOTXY_HEAD_LAST) { // head to amulet
				x = InvRect[SLOTXY_AMULET].X + (INV_SLOT_SIZE_PX / 2);
				y = InvRect[SLOTXY_AMULET].Y - (INV_SLOT_SIZE_PX / 2);
		    } else if (slot >= SLOTXY_HAND_RIGHT_FIRST && slot <= SLOTXY_HAND_RIGHT_LAST) { // right hand
			    // do nothing
		    } else if (slot == SLOTXY_AMULET) {
				// do nothing
		    } else if (slot == SLOTXY_RING_RIGHT) {
				// do nothing
			} else if (slot < SLOTXY_BELT_LAST && slot >= SLOTXY_INV_FIRST) { // general inventory
			    if (slot != 34 && slot != 44 && slot != 54 && slot != SLOTXY_INV_LAST) { // right bounds
					slot += 1;
					x = InvRect[slot].X + (INV_SLOT_SIZE_PX / 2);
					y = InvRect[slot].Y - (INV_SLOT_SIZE_PX / 2);
				}
		}
	} else if (key == VK_UP) {
			if (slot >= 25 && slot <= 27) { // first 3 general slots
				x = InvRect[SLOTXY_RING_LEFT].X + (INV_SLOT_SIZE_PX / 2);
				y = InvRect[SLOTXY_RING_LEFT].Y - (INV_SLOT_SIZE_PX / 2);
			} else if (slot >= 28 && slot <= 32) { // middle 4 general slots
				x = InvRect[SLOTXY_CHEST_FIRST + 3].X + (INV_SLOT_SIZE_PX / 2);
				y = InvRect[SLOTXY_CHEST_FIRST + 3].Y - (INV_SLOT_SIZE_PX / 2);
			} else if (slot >= 33 && slot <= 36) { // last 3 general slots
				x = InvRect[SLOTXY_RING_RIGHT].X + (INV_SLOT_SIZE_PX / 2);
				y = InvRect[SLOTXY_RING_RIGHT].Y - (INV_SLOT_SIZE_PX / 2);
			} else if (slot >= SLOTXY_CHEST_FIRST && slot <= SLOTXY_CHEST_LAST) { // chest to head
				x = InvRect[SLOTXY_HEAD_FIRST + 1].X + (INV_SLOT_SIZE_PX / 2);
				y = InvRect[SLOTXY_HEAD_FIRST + 1].Y - (INV_SLOT_SIZE_PX / 2);
			} else if (slot == SLOTXY_RING_LEFT) { // left ring to left hand
				x = InvRect[SLOTXY_HAND_LEFT_FIRST + 2].X + (INV_SLOT_SIZE_PX / 2);
				y = InvRect[SLOTXY_HAND_LEFT_FIRST + 2].Y - (INV_SLOT_SIZE_PX / 2);
			} else if (slot == SLOTXY_RING_RIGHT) { // right ring to right hand
				x = InvRect[SLOTXY_HAND_RIGHT_FIRST + 2].X + (INV_SLOT_SIZE_PX / 2);
				y = InvRect[SLOTXY_HAND_RIGHT_FIRST + 2].Y - (INV_SLOT_SIZE_PX / 2);
			} else if (slot >= SLOTXY_HAND_RIGHT_FIRST && slot <= SLOTXY_HAND_RIGHT_LAST) { // right hand to amulet
				x = InvRect[SLOTXY_AMULET].X + (INV_SLOT_SIZE_PX / 2);
			    y = InvRect[SLOTXY_AMULET].Y - (INV_SLOT_SIZE_PX / 2);
		    } else if (slot >= SLOTXY_HEAD_FIRST && slot <= SLOTXY_HEAD_LAST) {
				// do nothing
		    } else if (slot >= SLOTXY_HAND_LEFT_FIRST && slot <= SLOTXY_HAND_LEFT_LAST) { // left hand to head
			    x = InvRect[SLOTXY_HEAD_FIRST + 1].X + (INV_SLOT_SIZE_PX / 2);
			    y = InvRect[SLOTXY_HEAD_FIRST + 1].Y - (INV_SLOT_SIZE_PX / 2);
		    } else if (slot == SLOTXY_AMULET) {
				// do nothing
		    } else if (slot > (SLOTXY_INV_FIRST + 10)) { // general inventory
			    slot -= 10;
			    x = InvRect[slot].X + (INV_SLOT_SIZE_PX / 2);
			    y = InvRect[slot].Y - (INV_SLOT_SIZE_PX / 2);
		    }
	} else if (key == VK_DOWN) {
			if (slot >= SLOTXY_HEAD_FIRST && slot <= SLOTXY_HEAD_LAST) {
				x = InvRect[SLOTXY_CHEST_FIRST + 3].X + (INV_SLOT_SIZE_PX / 2);
				y = InvRect[SLOTXY_CHEST_FIRST + 3].Y - (INV_SLOT_SIZE_PX / 2);
			} else if (slot >= SLOTXY_CHEST_FIRST && slot <= SLOTXY_CHEST_LAST) {
				x = InvRect[30].X + (INV_SLOT_SIZE_PX / 2);
				y = InvRect[30].Y - (INV_SLOT_SIZE_PX / 2);
			} else if (slot >= SLOTXY_HAND_LEFT_FIRST && slot <= SLOTXY_HAND_LEFT_LAST) {
				x = InvRect[SLOTXY_RING_LEFT].X + (INV_SLOT_SIZE_PX / 2);
				y = InvRect[SLOTXY_RING_LEFT].Y - (INV_SLOT_SIZE_PX / 2);
			} else if (slot == SLOTXY_RING_LEFT) {
				x = InvRect[26].X + (INV_SLOT_SIZE_PX / 2);
				y = InvRect[26].Y - (INV_SLOT_SIZE_PX / 2);
			} else if (slot == SLOTXY_RING_RIGHT) {
				x = InvRect[34].X + (INV_SLOT_SIZE_PX / 2);
				y = InvRect[34].Y - (INV_SLOT_SIZE_PX / 2);
			} else if (slot == SLOTXY_AMULET) {
				x = InvRect[SLOTXY_HAND_RIGHT_FIRST + 2].X + (INV_SLOT_SIZE_PX / 2);
				y = InvRect[SLOTXY_HAND_RIGHT_FIRST + 2].Y - (INV_SLOT_SIZE_PX / 2);
			} else if (slot >= SLOTXY_HAND_RIGHT_FIRST && slot <= SLOTXY_HAND_RIGHT_LAST) {
				x = InvRect[SLOTXY_RING_RIGHT].X + (INV_SLOT_SIZE_PX / 2);
				y = InvRect[SLOTXY_RING_RIGHT].Y - (INV_SLOT_SIZE_PX / 2);
			} else if (slot < (SLOTXY_BELT_LAST - 10)) { // general inventory
				slot += 10;
				x = InvRect[slot].X + (INV_SLOT_SIZE_PX / 2);
				y = InvRect[slot].Y - (INV_SLOT_SIZE_PX / 2);
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
	static DWORD opentimer;
	static DWORD clickinvtimer;
	ticks = GetTickCount();

	// TEST: inv - x:336 y:198

	if (stextflag || questlog || helpflag || talkflag || qtextflag)
		return;
	if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
		return;
	if (GetAsyncKeyState(VK_SPACE) & 0x8000) { // similar to X button on PS1 ccontroller. Talk to towners, click on inv items, attack.
		if (invflag) {                         // inventory is open
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
