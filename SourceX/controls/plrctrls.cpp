#include "controls/plrctrls.h"

#include <cstdint>
#include <algorithm>
#include <list>

#include "controls/controller.h"
#include "controls/controller_motion.h"
#include "controls/game_controls.h"

#define SPLICONLENGTH 56

namespace dvl {

bool sgbControllerActive = false;
coords speedspellscoords[50];
int speedspellcount = 0;

/**
 * Native game menu, controlled by simulating a keyboard.
 */
bool InGameMenu(int pnum)
{
	return stextflag > 0
	    || helpflag
	    || talkflag
	    || qtextflag
	    || gmenu_is_active()
	    || PauseMode == 2
	    || plr[pnum]._pInvincible;
}

namespace {

int slot = SLOTXY_INV_FIRST;

/**
 * Number of angles to turn to face the coordinate
 * @param pnum Player index
 * @param x Tile coordinates
 * @param y Tile coordinates
 * @return -1 == down
 */
int GetRotaryDistance(int pnum, int x, int y)
{
	int d, d1, d2;

	if (plr[pnum]._pfutx == x && plr[pnum]._pfuty == y)
		return -1;

	d1 = plr[pnum]._pdir;
	d2 = GetDirection(plr[pnum]._pfutx, plr[pnum]._pfuty, x, y);

	d = abs(d1 - d2);
	if (d > 4)
		return 4 - (d % 4);

	return d;
}

/**
 * @brief Get the best case walking steps to coordinates
 * @param pnum Player index
 * @param dx Tile coordinates
 * @param dy Tile coordinates
 */
int GetMinDistance(int pnum, int dx, int dy)
{
	return std::max(abs(plr[pnum]._pfutx - dx), abs(plr[pnum]._pfuty - dy));
}

/**
 * @brief Get walking steps to coordinate
 * @param pnum Player index
 * @param dx Tile coordinates
 * @param dy Tile coordinates
 * @param maxDistance the max number of steps to search
 * @return number of steps, or 0 if not reachable
 */
int GetDistance(int pnum, int dx, int dy, int maxDistance)
{
	if (GetMinDistance(pnum, dx, dy) > maxDistance) {
		return 0;
	}

	Sint8 walkpath[MAX_PATH_LENGTH];
	int steps = FindPath(PosOkPlayer, pnum, plr[pnum]._pfutx, plr[pnum]._pfuty, dx, dy, walkpath);
	if (steps > maxDistance)
		return 0;

	return steps;
}

/**
 * @brief Get distance to coordinate
 * @param pnum Player index
 * @param dx Tile coordinates
 * @param dy Tile coordinates
 */
int GetDistanceRanged(int pnum, int dx, int dy)
{
	int a = plr[pnum]._pfutx - dx;
	int b = plr[pnum]._pfuty - dy;

	return sqrt(a * a + b * b);
}

void FindItemOrObject(int pnum)
{
	int mx = plr[pnum]._pfutx;
	int my = plr[pnum]._pfuty;
	int rotations = 5;

	// As the player can not stand on the edge fo the map this is safe from OOB
	for (int xx = -1; xx < 2; xx++) {
		for (int yy = -1; yy < 2; yy++) {
			if (dItem[mx + xx][my + yy] <= 0)
				continue;
			int i = dItem[mx + xx][my + yy] - 1;
			if (item[i].isEmpty()
			    || item[i]._iSelFlag == 0)
				continue;
			int newRotations = GetRotaryDistance(pnum, mx + xx, my + yy);
			if (rotations < newRotations)
				continue;
			if (xx != 0 && yy != 0 && GetDistance(pnum, mx + xx, my + yy, 1) == 0)
				continue;
			rotations = newRotations;
			pcursitem = i;
			cursmx = mx + xx;
			cursmy = my + yy;
		}
	}

	if (leveltype == DTYPE_TOWN || pcursitem != -1)
		return; // Don't look for objects in town

	for (int xx = -1; xx < 2; xx++) {
		for (int yy = -1; yy < 2; yy++) {
			if (dObject[mx + xx][my + yy] == 0)
				continue;
			int o = dObject[mx + xx][my + yy] > 0 ? dObject[mx + xx][my + yy] - 1 : -(dObject[mx + xx][my + yy] + 1);
			if (object[o]._oSelFlag == 0)
				continue;
			if (xx == 0 && yy == 0 && object[o]._oDoorFlag)
				continue; // Ignore doorway so we don't get stuck behind barrels
			int newRotations = GetRotaryDistance(pnum, mx + xx, my + yy);
			if (rotations < newRotations)
				continue;
			if (xx != 0 && yy != 0 && GetDistance(pnum, mx + xx, my + yy, 1) == 0)
				continue;
			rotations = newRotations;
			pcursobj = o;
			cursmx = mx + xx;
			cursmy = my + yy;
		}
	}
}

void CheckTownersNearby(int pnum)
{
	for (int i = 0; i < 16; i++) {
		int distance = GetDistance(pnum, towner[i]._tx, towner[i]._ty, 2);
		if (distance == 0)
			continue;
		pcursmonst = i;
	}
}

bool HasRangedSpell(int pnum)
{
	int spl = plr[pnum]._pRSpell;

	return spl != SPL_INVALID
	    && spl != SPL_TOWN
	    && spl != SPL_TELEPORT
	    && spelldata[spl].sTargeted
	    && !spelldata[spl].sTownSpell;
}

bool CanTargetMonster(int mi)
{
	const MonsterStruct &monst = monster[mi];

	if (monst._mFlags & (MFLAG_HIDDEN | MFLAG_GOLEM))
		return false;
	if (monst._mhitpoints >> 6 <= 0) // dead
		return false;

	const int mx = monst._mx;
	const int my = monst._my;
	if (!(dFlags[mx][my] & BFLAG_LIT)) // not visible
		return false;
	if (dMonster[mx][my] == 0)
		return false;

	return true;
}

void FindRangedTarget(int pnum)
{
	int rotations = 0;
	int distance = 0;
	bool canTalk = false;

	// The first MAX_PLRS monsters are reserved for players' golems.
	for (int mi = MAX_PLRS; mi < MAXMONSTERS; mi++) {
		const MonsterStruct &monst = monster[mi];
		const int mx = monst._mfutx;
		const int my = monst._mfuty;
		if (!CanTargetMonster(mi))
			continue;

		const bool newCanTalk = CanTalkToMonst(mi);
		if (pcursmonst != -1 && !canTalk && newCanTalk)
			continue;
		const int newDdistance = GetDistanceRanged(pnum, mx, my);
		const int newRotations = GetRotaryDistance(pnum, mx, my);
		if (pcursmonst != -1 && canTalk == newCanTalk) {
			if (distance < newDdistance)
				continue;
			if (distance == newDdistance && rotations < newRotations)
				continue;
		}
		distance = newDdistance;
		rotations = newRotations;
		canTalk = newCanTalk;
		pcursmonst = mi;
	}
}

void FindMeleeTarget(int pnum)
{
	bool visited[MAXDUNX][MAXDUNY] = { { 0 } };
	int maxSteps = 25; // Max steps for FindPath is 25
	int rotations = 0;
	bool canTalk = false;

	struct SearchNode {
		int x, y;
		int steps;
	};
	std::list<SearchNode> queue;

	{
		const int start_x = plr[pnum]._pfutx;
		const int start_y = plr[pnum]._pfuty;
		visited[start_x][start_y] = true;
		queue.push_back({ start_x, start_y, 0 });
	}

	while (!queue.empty()) {
		SearchNode node = queue.front();
		queue.pop_front();

		for (int i = 0; i < 8; i++) {
			const int dx = node.x + pathxdir[i];
			const int dy = node.y + pathydir[i];

			if (visited[dx][dy])
				continue; // already visisted

			if (node.steps > maxSteps) {
				visited[dx][dy] = true;
				continue;
			}

			if (!PosOkPlayer(pnum, dx, dy)) {
				visited[dx][dy] = true;

				if (dMonster[dx][dy] != 0) {
					const int mi = dMonster[dx][dy] > 0 ? dMonster[dx][dy] - 1 : -(dMonster[dx][dy] + 1);
					if (CanTargetMonster(mi)) {
						const bool newCanTalk = CanTalkToMonst(mi);
						if (pcursmonst != -1 && !canTalk && newCanTalk)
							continue;
						const int newRotations = GetRotaryDistance(pnum, dx, dy);
						if (pcursmonst != -1 && canTalk == newCanTalk && rotations < newRotations)
							continue;
						rotations = newRotations;
						canTalk = newCanTalk;
						pcursmonst = mi;
						if (!canTalk)
							maxSteps = node.steps; // Monsters found, cap search to current steps
					}
				}

				continue;
			}

			PATHNODE pPath;
			pPath.x = node.x;
			pPath.y = node.y;

			if (path_solid_pieces(&pPath, dx, dy)) {
				queue.push_back({ dx, dy, node.steps + 1 });
				visited[dx][dy] = true;
			}
		}
	}
}

void CheckMonstersNearby(int pnum)
{
	if (plr[pnum]._pwtype == WT_RANGED || HasRangedSpell(pnum)) {
		FindRangedTarget(pnum);
		return;
	}

	FindMeleeTarget(pnum);
}

void CheckPlayerNearby(int pnum)
{
	int newDdistance;
	int rotations = 0;
	int distance = 0;

	if (pcursmonst != -1)
		return;

	int spl = plr[pnum]._pRSpell;
	if (gbFriendlyMode && spl != SPL_RESURRECT && spl != SPL_HEALOTHER)
		return;

	for (int i = 0; i < MAX_PLRS; i++) {
		if (i == pnum)
			continue;
		const int mx = plr[i]._pfutx;
		const int my = plr[i]._pfuty;
		if (dPlayer[mx][my] == 0
		    || !(dFlags[mx][my] & BFLAG_LIT)
		    || (plr[i]._pHitPoints == 0 && spl != SPL_RESURRECT))
			continue;

		if (plr[pnum]._pwtype == WT_RANGED || HasRangedSpell(pnum) || spl == SPL_HEALOTHER) {
			newDdistance = GetDistanceRanged(pnum, mx, my);
		} else {
			newDdistance = GetDistance(pnum, mx, my, distance);
			if (newDdistance == 0)
				continue;
		}

		if (pcursplr != -1 && distance < newDdistance)
			continue;
		const int newRotations = GetRotaryDistance(pnum, mx, my);
		if (pcursplr != -1 && distance == newDdistance && rotations < newRotations)
			continue;

		distance = newDdistance;
		rotations = newRotations;
		pcursplr = i;
	}
}

void FindActor(int pnum)
{
	if (leveltype != DTYPE_TOWN)
		CheckMonstersNearby(pnum);
	else
		CheckTownersNearby(pnum);

	if (gbIsMultiplayer)
		CheckPlayerNearby(pnum);
}

int pcursmissile;
int pcurstrig;
int pcursquest;

void FindTrigger(int pnum)
{
	int rotations;
	int distance = 0;

	if (pcursitem != -1 || pcursobj != -1)
		return; // Prefer showing items/objects over triggers (use of cursm* conflicts)

	for (int i = 0; i < nummissiles; i++) {
		int mi = missileactive[i];
		if (missile[mi]._mitype == MIS_TOWN || missile[mi]._mitype == MIS_RPORTAL) {
			int mix = missile[mi]._mix;
			int miy = missile[mi]._miy;
			const int newDdistance = GetDistance(pnum, mix, miy, 2);
			if (newDdistance == 0)
				continue;
			if (pcursmissile != -1 && distance < newDdistance)
				continue;
			const int newRotations = GetRotaryDistance(pnum, mix, miy);
			if (pcursmissile != -1 && distance == newDdistance && rotations < newRotations)
				continue;
			cursmx = mix;
			cursmy = miy;
			pcursmissile = mi;
			distance = newDdistance;
			rotations = newRotations;
		}
	}

	if (pcursmissile == -1) {
		for (int i = 0; i < numtrigs; i++) {
			int tx = trigs[i]._tx;
			int ty = trigs[i]._ty;
			if (trigs[i]._tlvl == 13)
				ty -= 1;
			const int newDdistance = GetDistance(pnum, tx, ty, 2);
			if (newDdistance == 0)
				continue;
			cursmx = tx;
			cursmy = ty;
			pcurstrig = i;
		}

		if (pcurstrig == -1) {
			for (int i = 0; i < MAXQUESTS; i++) {
				if (i == Q_BETRAYER || currlevel != quests[i]._qlevel || quests[i]._qslvl == 0)
					continue;
				const int newDdistance = GetDistance(pnum, quests[i]._qtx, quests[i]._qty, 2);
				if (newDdistance == 0)
					continue;
				cursmx = quests[i]._qtx;
				cursmy = quests[i]._qty;
				pcursquest = i;
			}
		}
	}

	if (pcursmonst != -1 || pcursplr != -1 || cursmx == -1 || cursmy == -1)
		return; // Prefer monster/player info text

	CheckTrigForce();
	CheckTown();
	CheckRportal();
}

void Interact(int pnum)
{
	if (leveltype == DTYPE_TOWN && pcursmonst != -1) {
		NetSendCmdLocParam1(true, CMD_TALKXY, towner[pcursmonst]._tx, towner[pcursmonst]._ty, pcursmonst);
	} else if (pcursmonst != -1) {
		if (plr[pnum]._pwtype != WT_RANGED || CanTalkToMonst(pcursmonst)) {
			NetSendCmdParam1(true, CMD_ATTACKID, pcursmonst);
		} else {
			NetSendCmdParam1(true, CMD_RATTACKID, pcursmonst);
		}
	} else if (leveltype != DTYPE_TOWN && pcursplr != -1 && !gbFriendlyMode) {
		NetSendCmdParam1(true, plr[pnum]._pwtype == WT_RANGED ? CMD_RATTACKPID : CMD_ATTACKPID, pcursplr);
	}
}

void AttrIncBtnSnap(AxisDirection dir)
{
	static AxisDirectionRepeater repeater;
	dir = repeater.Get(dir);
	if (dir.y == AxisDirectionY_NONE)
		return;

	if (chrbtnactive && plr[myplr]._pStatPts <= 0)
		return;

	// first, find our cursor location
	int slot = 0;
	for (int i = 0; i < 4; i++) {
		if (MouseX >= ChrBtnsRect[i].x
		    && MouseX <= ChrBtnsRect[i].x + ChrBtnsRect[i].w
		    && MouseY >= ChrBtnsRect[i].y
		    && MouseY <= ChrBtnsRect[i].h + ChrBtnsRect[i].y) {
			slot = i;
			break;
		}
	}

	if (dir.y == AxisDirectionY_UP) {
		if (slot > 0)
			--slot;
	} else if (dir.y == AxisDirectionY_DOWN) {
		if (slot < 3)
			++slot;
	}

	// move cursor to our new location
	int x = ChrBtnsRect[slot].x + (ChrBtnsRect[slot].w / 2);
	int y = ChrBtnsRect[slot].y + (ChrBtnsRect[slot].h / 2);
	SetCursorPos(x, y);
}

/**
 * Move the cursor around in our inventory
 * If mouse coords are at SLOTXY_CHEST_LAST, consider this center of equipment
 * small inventory squares are 29x29 (roughly)
 */
void InvMove(AxisDirection dir)
{
	static AxisDirectionRepeater repeater(/*min_interval_ms=*/100);
	dir = repeater.Get(dir);
	if (dir.x == AxisDirectionX_NONE && dir.y == AxisDirectionY_NONE)
		return;

	int x = MouseX;
	int y = MouseY;

	// check which inventory rectangle the mouse is in, if any
	for (int r = 0; (DWORD)r < NUM_XY_SLOTS; r++) {
		int xo = RIGHT_PANEL;
		int yo = 0;
		if (r >= SLOTXY_BELT_FIRST) {
			xo = PANEL_LEFT;
			yo = PANEL_TOP;
		}

		if (x >= InvRect[r].X + xo && x < InvRect[r].X + xo + (INV_SLOT_SIZE_PX + 1) && y >= InvRect[r].Y + yo - (INV_SLOT_SIZE_PX + 1) && y < InvRect[r].Y + yo) {
			slot = r;
			break;
		}
	}

	if (slot < 0)
		slot = 0;
	if (slot > SLOTXY_BELT_LAST)
		slot = SLOTXY_BELT_LAST;

	// when item is on cursor, this is the real cursor XY
	if (dir.x == AxisDirectionX_LEFT) {
		if (slot >= SLOTXY_HAND_RIGHT_FIRST && slot <= SLOTXY_HAND_RIGHT_LAST) {
			x = InvRect[SLOTXY_CHEST_FIRST].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[SLOTXY_CHEST_FIRST].Y - (INV_SLOT_SIZE_PX / 2);
		} else if (slot >= SLOTXY_CHEST_FIRST && slot <= SLOTXY_CHEST_LAST) {
			x = InvRect[SLOTXY_HAND_LEFT_FIRST + 2].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[SLOTXY_HAND_LEFT_FIRST + 2].Y - (INV_SLOT_SIZE_PX / 2);
		} else if (slot == SLOTXY_AMULET) {
			x = InvRect[SLOTXY_HEAD_FIRST].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[SLOTXY_HEAD_FIRST].Y - (INV_SLOT_SIZE_PX / 2);
		} else if (slot == SLOTXY_RING_RIGHT) {
			x = InvRect[SLOTXY_RING_LEFT].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[SLOTXY_RING_LEFT].Y - (INV_SLOT_SIZE_PX / 2);
		} else if (slot == SLOTXY_BELT_FIRST) {
			// do nothing
		} else if (slot == SLOTXY_RING_LEFT) {                                        // left ring
			                                                                          // do nothing
		} else if (slot >= SLOTXY_HAND_LEFT_FIRST && slot <= SLOTXY_HAND_LEFT_LAST) { // left hand
			                                                                          // do nothing
		} else if (slot >= SLOTXY_HEAD_FIRST && slot <= SLOTXY_HEAD_LAST) {           // head
			                                                                          // do nothing
		} else if (slot > SLOTXY_INV_FIRST && slot <= SLOTXY_INV_LAST) {              // general inventory
			if (slot != SLOTXY_INV_FIRST && slot != 35 && slot != 45 && slot != 55) { // left bounds
				slot -= 1;
				x = InvRect[slot].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
				y = InvRect[slot].Y - (INV_SLOT_SIZE_PX / 2);
			}
		} else if (slot > SLOTXY_BELT_FIRST && slot <= SLOTXY_BELT_LAST) { // belt
			slot -= 1;
			x = InvRect[slot].X + PANEL_LEFT + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[slot].Y + PANEL_TOP - (INV_SLOT_SIZE_PX / 2);
		}
	} else if (dir.x == AxisDirectionX_RIGHT) {
		if (slot == SLOTXY_RING_LEFT) {
			x = InvRect[SLOTXY_RING_RIGHT].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[SLOTXY_RING_RIGHT].Y - (INV_SLOT_SIZE_PX / 2);
		} else if (slot >= SLOTXY_HAND_LEFT_FIRST && slot <= SLOTXY_HAND_LEFT_LAST) {
			x = InvRect[SLOTXY_CHEST_FIRST].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[SLOTXY_CHEST_FIRST].Y - (INV_SLOT_SIZE_PX / 2);
		} else if (slot >= SLOTXY_CHEST_FIRST && slot <= SLOTXY_CHEST_LAST) {
			x = InvRect[SLOTXY_HAND_RIGHT_FIRST + 2].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[SLOTXY_HAND_RIGHT_FIRST + 2].Y - (INV_SLOT_SIZE_PX / 2);
		} else if (slot >= SLOTXY_HEAD_FIRST && slot <= SLOTXY_HEAD_LAST) { // head to amulet
			x = InvRect[SLOTXY_AMULET].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[SLOTXY_AMULET].Y - (INV_SLOT_SIZE_PX / 2);
		} else if (slot >= SLOTXY_HAND_RIGHT_FIRST && slot <= SLOTXY_HAND_RIGHT_LAST) { // right hand
			                                                                            // do nothing
		} else if (slot == SLOTXY_AMULET) {
			// do nothing
		} else if (slot == SLOTXY_RING_RIGHT) {
			// do nothing
		} else if (slot >= SLOTXY_INV_FIRST && slot <= SLOTXY_INV_LAST) {            // general inventory
			if (slot != 34 && slot != 44 && slot != 54 && slot != SLOTXY_INV_LAST) { // right bounds
				slot += 1;
				x = InvRect[slot].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
				y = InvRect[slot].Y - (INV_SLOT_SIZE_PX / 2);
			}
		} else if (slot >= SLOTXY_BELT_FIRST && slot < SLOTXY_BELT_LAST) { // belt
			slot += 1;
			x = InvRect[slot].X + PANEL_LEFT + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[slot].Y + PANEL_TOP - (INV_SLOT_SIZE_PX / 2);
		}
	}
	if (dir.y == AxisDirectionY_UP) {
		if (slot > 24 && slot <= 27) { // first 3 general slots
			x = InvRect[SLOTXY_RING_LEFT].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[SLOTXY_RING_LEFT].Y - (INV_SLOT_SIZE_PX / 2);
		} else if (slot >= 28 && slot <= 32) { // middle 4 general slots
			x = InvRect[SLOTXY_CHEST_FIRST].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[SLOTXY_CHEST_FIRST].Y - (INV_SLOT_SIZE_PX / 2);
		} else if (slot >= 33 && slot < 35) { // last 3 general slots
			x = InvRect[SLOTXY_RING_RIGHT].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[SLOTXY_RING_RIGHT].Y - (INV_SLOT_SIZE_PX / 2);
		} else if (slot >= SLOTXY_CHEST_FIRST && slot <= SLOTXY_CHEST_LAST) { // chest to head
			x = InvRect[SLOTXY_HEAD_FIRST].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[SLOTXY_HEAD_FIRST].Y - (INV_SLOT_SIZE_PX / 2);
		} else if (slot == SLOTXY_RING_LEFT) { // left ring to left hand
			x = InvRect[SLOTXY_HAND_LEFT_FIRST + 2].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[SLOTXY_HAND_LEFT_FIRST + 2].Y - (INV_SLOT_SIZE_PX / 2);
		} else if (slot == SLOTXY_RING_RIGHT) { // right ring to right hand
			x = InvRect[SLOTXY_HAND_RIGHT_FIRST + 2].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[SLOTXY_HAND_RIGHT_FIRST + 2].Y - (INV_SLOT_SIZE_PX / 2);
		} else if (slot >= SLOTXY_HAND_RIGHT_FIRST && slot <= SLOTXY_HAND_RIGHT_LAST) { // right hand to amulet
			x = InvRect[SLOTXY_AMULET].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[SLOTXY_AMULET].Y - (INV_SLOT_SIZE_PX / 2);
		} else if (slot >= SLOTXY_HEAD_FIRST && slot <= SLOTXY_HEAD_LAST) {
			// do nothing
		} else if (slot >= SLOTXY_HAND_LEFT_FIRST && slot <= SLOTXY_HAND_LEFT_LAST) { // left hand to head
			x = InvRect[SLOTXY_HEAD_FIRST].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[SLOTXY_HEAD_FIRST].Y - (INV_SLOT_SIZE_PX / 2);
		} else if (slot == SLOTXY_AMULET) {
			// do nothing
		} else if (slot >= (SLOTXY_INV_FIRST + 10)) { // general inventory
			slot -= 10;
			x = InvRect[slot].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[slot].Y - (INV_SLOT_SIZE_PX / 2);
		}
	} else if (dir.y == AxisDirectionY_DOWN) {
		if (slot >= SLOTXY_HEAD_FIRST && slot <= SLOTXY_HEAD_LAST) {
			x = InvRect[SLOTXY_CHEST_FIRST].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[SLOTXY_CHEST_FIRST].Y - (INV_SLOT_SIZE_PX / 2);
		} else if (slot >= SLOTXY_CHEST_FIRST && slot <= SLOTXY_CHEST_LAST) {
			x = InvRect[30].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[30].Y - (INV_SLOT_SIZE_PX / 2);
		} else if (slot >= SLOTXY_HAND_LEFT_FIRST && slot <= SLOTXY_HAND_LEFT_LAST) {
			x = InvRect[SLOTXY_RING_LEFT].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[SLOTXY_RING_LEFT].Y - (INV_SLOT_SIZE_PX / 2);
		} else if (slot == SLOTXY_RING_LEFT) {
			x = InvRect[26].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[26].Y - (INV_SLOT_SIZE_PX / 2);
		} else if (slot == SLOTXY_RING_RIGHT) {
			x = InvRect[34].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[34].Y - (INV_SLOT_SIZE_PX / 2);
		} else if (slot == SLOTXY_AMULET) {
			x = InvRect[SLOTXY_HAND_RIGHT_FIRST + 2].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[SLOTXY_HAND_RIGHT_FIRST + 2].Y - (INV_SLOT_SIZE_PX / 2);
		} else if (slot >= SLOTXY_HAND_RIGHT_FIRST && slot <= SLOTXY_HAND_RIGHT_LAST) {
			x = InvRect[SLOTXY_RING_RIGHT].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[SLOTXY_RING_RIGHT].Y - (INV_SLOT_SIZE_PX / 2);
		} else if (slot <= (SLOTXY_INV_LAST - 10)) { // general inventory
			slot += 10;
			x = InvRect[slot].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[slot].Y - (INV_SLOT_SIZE_PX / 2);
		} else if (slot <= (SLOTXY_BELT_LAST - 10)) { // general inventory
			slot += 10;
			x = InvRect[slot].X + PANEL_LEFT + (INV_SLOT_SIZE_PX / 2);
			y = InvRect[slot].Y + PANEL_TOP - (INV_SLOT_SIZE_PX / 2);
		}
	}

	if (x == MouseX && y == MouseY) {
		return; // Avoid wobeling when scalled
	}

	if (pcurs > 1) {       // [3] Keep item in the same slot, don't jump it up
		if (x != MouseX) { // without this, the cursor keeps moving -10
			x -= 10;
			y -= 10;
		}
	}
	SetCursorPos(x, y);
}

/**
 * check if hot spell at X Y exists
 */
bool HSExists(int x, int y)
{
	for (int r = 0; r < speedspellcount; r++) {
		if (x >= speedspellscoords[r].x - SPLICONLENGTH / 2
		    && x < speedspellscoords[r].x + SPLICONLENGTH / 2
		    && y >= speedspellscoords[r].y - SPLICONLENGTH / 2
		    && y < speedspellscoords[r].y + SPLICONLENGTH / 2) {
			return true;
		}
	}
	return false;
}

void HotSpellMove(AxisDirection dir)
{
	static AxisDirectionRepeater repeater;
	dir = repeater.Get(dir);
	if (dir.x == AxisDirectionX_NONE && dir.y == AxisDirectionY_NONE)
		return;

	int spbslot = plr[myplr]._pRSpell;
	for (int r = 0; r < speedspellcount; r++) {
		if (MouseX >= speedspellscoords[r].x - SPLICONLENGTH / 2
		    && MouseX < speedspellscoords[r].x + SPLICONLENGTH / 2
		    && MouseY >= speedspellscoords[r].y - SPLICONLENGTH / 2
		    && MouseY < speedspellscoords[r].y + SPLICONLENGTH / 2) {
			spbslot = r;
			break;
		}
	}

	int x = speedspellscoords[spbslot].x;
	int y = speedspellscoords[spbslot].y;

	if (dir.x == AxisDirectionX_LEFT) {
		if (spbslot < speedspellcount - 1) {
			x = speedspellscoords[spbslot + 1].x;
			y = speedspellscoords[spbslot + 1].y;
		}
	} else if (dir.x == AxisDirectionX_RIGHT) {
		if (spbslot > 0) {
			x = speedspellscoords[spbslot - 1].x;
			y = speedspellscoords[spbslot - 1].y;
		}
	}

	if (dir.y == AxisDirectionY_UP) {
		if (HSExists(x, y - SPLICONLENGTH)) {
			y -= SPLICONLENGTH;
		}
	} else if (dir.y == AxisDirectionY_DOWN) {
		if (HSExists(x, y + SPLICONLENGTH)) {
			y += SPLICONLENGTH;
		}
	}

	if (x != MouseX || y != MouseY) {
		SetCursorPos(x, y);
	}
}

void SpellBookMove(AxisDirection dir)
{
	static AxisDirectionRepeater repeater;
	dir = repeater.Get(dir);

	if (dir.x == AxisDirectionX_LEFT) {
		if (sbooktab > 0)
			sbooktab--;
	} else if (dir.x == AxisDirectionX_RIGHT) {
		if ((gbIsHellfire && sbooktab < 4) || (!gbIsHellfire && sbooktab < 3))
			sbooktab++;
	}
}

static const direction kFaceDir[3][3] = {
	// NONE      UP      DOWN
	{ DIR_OMNI, DIR_N, DIR_S }, // NONE
	{ DIR_W, DIR_NW, DIR_SW },  // LEFT
	{ DIR_E, DIR_NE, DIR_SE },  // RIGHT
};
static const int kOffsets[8][2] = {
	{ 1, 1 },   // DIR_S
	{ 0, 1 },   // DIR_SW
	{ -1, 1 },  // DIR_W
	{ -1, 0 },  // DIR_NW
	{ -1, -1 }, // DIR_N
	{ 0, -1 },  // DIR_NE
	{ 1, -1 },  // DIR_E
	{ 1, 0 },   // DIR_SE
};

/**
 * @brief check if stepping in direction (dir) from x, y is blocked.
 *
 * If you step from A to B, at leat one of the Xs need to be clear:
 *
 *  AX
 *  XB
 *
 *  @return true if step is blocked
 */
bool IsPathBlocked(int pnum, int x, int y, int dir)
{
	int d1, d2, d1x, d1y, d2x, d2y;

	switch (dir) {
	case DIR_N:
		d1 = DIR_NW;
		d2 = DIR_NE;
		break;
	case DIR_E:
		d1 = DIR_NE;
		d2 = DIR_SE;
		break;
	case DIR_S:
		d1 = DIR_SE;
		d2 = DIR_SW;
		break;
	case DIR_W:
		d1 = DIR_SW;
		d2 = DIR_NW;
		break;
	default:
		return false;
	}

	d1x = x + kOffsets[d1][0];
	d1y = y + kOffsets[d1][1];
	d2x = x + kOffsets[d2][0];
	d2y = y + kOffsets[d2][1];

	if (!nSolidTable[dPiece[d1x][d1y]] && !nSolidTable[dPiece[d2x][d2y]])
		return false;

	return !PosOkPlayer(pnum, d1x, d1y) && !PosOkPlayer(pnum, d2x, d2y);
}

void WalkInDir(int pnum, AxisDirection dir)
{
	const int x = plr[pnum]._pfutx;
	const int y = plr[pnum]._pfuty;

	if (dir.x == AxisDirectionX_NONE && dir.y == AxisDirectionY_NONE) {
		if (sgbControllerActive && plr[pnum].walkpath[0] != WALK_NONE && plr[pnum].destAction == ACTION_NONE)
			NetSendCmdLoc(true, CMD_WALKXY, x, y); // Stop walking
		return;
	}

	const direction pdir = kFaceDir[static_cast<std::size_t>(dir.x)][static_cast<std::size_t>(dir.y)];
	const int dx = x + kOffsets[pdir][0];
	const int dy = y + kOffsets[pdir][1];
	plr[pnum]._pdir = pdir;

	if (PosOkPlayer(pnum, dx, dy) && IsPathBlocked(pnum, x, y, pdir))
		return; // Don't start backtrack around obstacles

	NetSendCmdLoc(true, CMD_WALKXY, dx, dy);
}

void QuestLogMove(AxisDirection move_dir)
{
	static AxisDirectionRepeater repeater;
	move_dir = repeater.Get(move_dir);
	if (move_dir.y == AxisDirectionY_UP)
		QuestlogUp();
	else if (move_dir.y == AxisDirectionY_DOWN)
		QuestlogDown();
}

void StoreMove(AxisDirection move_dir)
{
	static AxisDirectionRepeater repeater;
	move_dir = repeater.Get(move_dir);
	if (move_dir.y == AxisDirectionY_UP)
		STextUp();
	else if (move_dir.y == AxisDirectionY_DOWN)
		STextDown();
}

typedef void (*HandleLeftStickOrDPadFn)(dvl::AxisDirection);

HandleLeftStickOrDPadFn GetLeftStickOrDPadGameUIHandler()
{
	if (invflag) {
		return &InvMove;
	} else if (chrflag && plr[myplr]._pStatPts > 0) {
		return &AttrIncBtnSnap;
	} else if (spselflag) {
		return &HotSpellMove;
	} else if (sbookflag) {
		return &SpellBookMove;
	} else if (questlog) {
		return &QuestLogMove;
	} else if (stextflag != STORE_NONE) {
		return &StoreMove;
	}
	return NULL;
}

void ProcessLeftStickOrDPadGameUI() {
	HandleLeftStickOrDPadFn handler = GetLeftStickOrDPadGameUIHandler();
	if (handler != NULL)
		handler(Controller::GetLeftStickOrDpadDirection(true));
}

void Movement(int pnum)
{
	if (InGameMenu(pnum)
	    || Controller::IsControllerButtonPressed(ControllerButton_BUTTON_START)
	    || Controller::IsControllerButtonPressed(ControllerButton_BUTTON_BACK))
		return;

	AxisDirection move_dir = Controller::GetMoveDirection();
	if (move_dir.x != AxisDirectionX_NONE || move_dir.y != AxisDirectionY_NONE) {
		sgbControllerActive = true;
	}

	if (GetLeftStickOrDPadGameUIHandler() == NULL) {
		WalkInDir(pnum, move_dir);
	}
}

struct RightStickAccumulator {

	RightStickAccumulator()
	{
		lastTc = SDL_GetTicks();
		hiresDX = 0;
		hiresDY = 0;
	}

	void pool(int *x, int *y, int slowdown)
	{
		if (Controller::Empty())
        	return;
		Controller controller = Controller::GetFirst();

		const Uint32 tc = SDL_GetTicks();
		const int dtc = tc - lastTc;
		hiresDX += controller.rightStickX * dtc;
		hiresDY += controller.rightStickY * dtc;
		const int dx = hiresDX / slowdown;
		const int dy = hiresDY / slowdown;
		*x += dx;
		*y -= dy;
		lastTc = tc;
		// keep track of remainder for sub-pixel motion
		hiresDX -= dx * slowdown;
		hiresDY -= dy * slowdown;
	}

	void clear()
	{
		lastTc = SDL_GetTicks();
	}

	DWORD lastTc;
	float hiresDX;
	float hiresDY;
};

} // namespace

void StoreSpellCoords(int pnum)
{
	const int START_X = PANEL_LEFT + 12 + SPLICONLENGTH / 2;
	const int END_X = START_X + SPLICONLENGTH * 10;
	const int END_Y = PANEL_TOP - 17 - SPLICONLENGTH / 2;
	speedspellcount = 0;
	int xo = END_X;
	int yo = END_Y;
	for (int i = 0; i < 4; i++) {
		std::uint64_t spells;
		switch (i) {
		case RSPLTYPE_SKILL:
			spells = plr[pnum]._pAblSpells;
			break;
		case RSPLTYPE_SPELL:
			spells = plr[pnum]._pMemSpells;
			break;
		case RSPLTYPE_SCROLL:
			spells = plr[pnum]._pScrlSpells;
			break;
		case RSPLTYPE_CHARGES:
			spells = plr[pnum]._pISpells;
			break;
		default:
			continue;
		}
		std::uint64_t spell = 1;
		for (int j = 1; j < MAX_SPELLS; j++) {
			if ((spell & spells)) {
				speedspellscoords[speedspellcount] = { xo, yo };
				++speedspellcount;
				xo -= SPLICONLENGTH;
				if (xo < START_X) {
					xo = END_X;
					yo -= SPLICONLENGTH;
				}
			}
			spell <<= 1;
		}
		if (spells && xo != END_X)
			xo -= SPLICONLENGTH;
		if (xo < START_X) {
			xo = END_X;
			yo -= SPLICONLENGTH;
		}
	}
}

bool IsAutomapActive()
{
	return automapflag && leveltype != DTYPE_TOWN;
}

bool IsMovingMouseCursorWithController()
{
	if (Controller::Empty())
		return false;
	Controller controller = Controller::GetFirst();
	return controller.rightStickX != 0 || controller.rightStickY != 0;
}

void HandleRightStickMotion()
{
	if (Controller::Empty())
		return;
	Controller controller = Controller::GetFirst();
	static RightStickAccumulator acc;
	// deadzone is handled in ScaleJoystickAxes() already
	if (controller.rightStickX == 0 && controller.rightStickY == 0) {
		acc.clear();
		return;
	}

	if (IsAutomapActive()) { // move map
		int dx = 0, dy = 0;
		acc.pool(&dx, &dy, 32);
		AutoMapXOfs += dy + dx;
		AutoMapYOfs += dy - dx;
		return;
	}

	{ // move cursor
		sgbControllerActive = false;
		int x = MouseX;
		int y = MouseY;
		acc.pool(&x, &y, 2);
		x = std::min(std::max(x, 0), gnScreenWidth - 1);
		y = std::min(std::max(y, 0), gnScreenHeight - 1);

		// We avoid calling `SetCursorPos` within the same SDL tick because
		// that can cause all stick motion events to arrive before all
		// cursor position events.
		static int lastMouseSetTick = 0;
		const int now = SDL_GetTicks();
		if (now - lastMouseSetTick > 0)
		{
			SetCursorPos(x, y);
			lastMouseSetTick = now;
		}
	}
}

/**
 * @brief Moves the mouse to the first inventory slot.
 */
void FocusOnInventory()
{
	SetCursorPos(InvRect[25].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2), InvRect[25].Y - (INV_SLOT_SIZE_PX / 2));
}

void plrctrls_after_check_curs_move()
{
	// check for monsters first, then items, then towners.
	if (sgbControllerActive) {
		// Clear focuse set by cursor
		pcursplr = -1;
		pcursmonst = -1;
		pcursitem = -1;
		pcursobj = -1;
		pcursmissile = -1;
		pcurstrig = -1;
		pcursquest = -1;
		cursmx = -1;
		cursmy = -1;
		if (!invflag) {
			*infostr = '\0';
			ClearPanel();
			FindActor(myplr);
			FindItemOrObject(myplr);
			FindTrigger(myplr);
		}
	}
}

void plrctrls_every_frame()
{
	ProcessLeftStickOrDPadGameUI();
	HandleRightStickMotion();
}

void plrctrls_after_game_logic()
{
	// TODO! For each local player
	for (int pnum = 0; pnum < MAX_PLRS; pnum++) {
		Movement(pnum);
	}
}

void UseBeltItem(int pnum, int type)
{
	for (int i = 0; i < MAXBELTITEMS; i++) {
		const int id = AllItemsList[plr[pnum].SpdList[i].IDidx].iMiscId;
		const int spellId = AllItemsList[plr[pnum].SpdList[i].IDidx].iSpell;
		if ((type == BLT_HEALING && (id == IMISC_HEAL || id == IMISC_FULLHEAL || (id == IMISC_SCROLL && spellId == SPL_HEAL)))
		    || (type == BLT_MANA && (id == IMISC_MANA || id == IMISC_FULLMANA))
		    || id == IMISC_REJUV || id == IMISC_FULLREJUV) {
			if (plr[pnum].SpdList[i]._itype > -1) {
				UseInvItem(pnum, INVITEM_BELT_FIRST + i);
				break;
			}
		}
	}
}

void PerformPrimaryAction(int pnum)
{
	if (invflag) { // inventory is open
		if (pcurs > CURSOR_HAND && pcurs < CURSOR_FIRSTITEM) {
			TryIconCurs();
			SetCursor_(CURSOR_HAND);
		} else {
			CheckInvItem();
		}
		return;
	}

	if (spselflag) {
		SetSpell(pnum);
		return;
	}

	if (chrflag && !chrbtnactive && plr[pnum]._pStatPts > 0) {
		CheckChrBtns(pnum);
		for (int i = 0; i < 4; i++) {
			if (MouseX >= ChrBtnsRect[i].x
			    && MouseX <= ChrBtnsRect[i].x + ChrBtnsRect[i].w
			    && MouseY >= ChrBtnsRect[i].y
			    && MouseY <= ChrBtnsRect[i].h + ChrBtnsRect[i].y) {
				chrbtn[i] = 1;
				chrbtnactive = true;
				ReleaseChrBtns(pnum, false);
			}
		}
		return;
	}

	Interact(pnum);
}

bool SpellHasActorTarget(int pnum)
{
	int spl = plr[pnum]._pRSpell;
	if (spl == SPL_TOWN || spl == SPL_TELEPORT)
		return false;

	if (spl == SPL_FIREWALL && pcursmonst != -1) {
		cursmx = monster[pcursmonst]._mx;
		cursmy = monster[pcursmonst]._my;
	}

	return pcursplr != -1 || pcursmonst != -1;
}

void UpdateSpellTarget(int pnum)
{
	if (SpellHasActorTarget(pnum))
		return;

	pcursplr = -1;
	pcursmonst = -1;

	const PlayerStruct &player = plr[pnum];

	int range = 1;
	if (plr[pnum]._pRSpell == SPL_TELEPORT)
		range = 4;

	cursmx = player._pfutx + kOffsets[player._pdir][0] * range;
	cursmy = player._pfuty + kOffsets[player._pdir][1] * range;
}

/**
 * @brief Try dropping item in all 9 possible places
 */
bool TryDropItem(int pnum)
{
	cursmx = plr[pnum]._pfutx + 1;
	cursmy = plr[pnum]._pfuty;
	if (!DropItemBeforeTrig()) {
		// Try to drop on the other side
		cursmx = plr[pnum]._pfutx;
		cursmy = plr[pnum]._pfuty + 1;
		DropItemBeforeTrig();
	}

	return pcurs == CURSOR_HAND;
}

void PerformSpellAction(int pnum)
{
	if (InGameMenu(pnum) || questlog || sbookflag)
		return;

	if (invflag) {
		if (pcurs >= CURSOR_FIRSTITEM)
			TryDropItem(pnum);
		else if (pcurs > CURSOR_HAND) {
			TryIconCurs();
			SetCursor_(CURSOR_HAND);
		}
		return;
	}

	if (pcurs >= CURSOR_FIRSTITEM && !TryDropItem(pnum))
		return;
	if (pcurs > CURSOR_HAND)
		SetCursor_(CURSOR_HAND);

	if (spselflag) {
		SetSpell(pnum);
		return;
	}

	int spl = plr[pnum]._pRSpell;
	if ((pcursplr == -1 && (spl == SPL_RESURRECT || spl == SPL_HEALOTHER))
	    || (pcursobj == -1 && spl == SPL_DISARM)) {
		if (plr[pnum]._pClass == PC_WARRIOR) {
			PlaySFX(PS_WARR27);
		} else if (plr[pnum]._pClass == PC_ROGUE) {
			PlaySFX(PS_ROGUE27);
		} else if (plr[pnum]._pClass == PC_SORCERER) {
			PlaySFX(PS_MAGE27);
		}
		return;
	}

	UpdateSpellTarget(pnum);
	CheckPlrSpell(pnum);
}

void CtrlUseInvItem(int pnum)
{
	ItemStruct *Item;

	if (pcursinvitem == -1)
		return;

	if (pcursinvitem <= INVITEM_INV_LAST)
		Item = &plr[pnum].InvList[pcursinvitem - INVITEM_INV_FIRST];
	else
		Item = &plr[pnum].SpdList[pcursinvitem - INVITEM_BELT_FIRST];

	if ((Item->_iMiscId == IMISC_SCROLLT || Item->_iMiscId == IMISC_SCROLL) && spelldata[Item->_iSpell].sTargeted) {
		return;
	}

	UseInvItem(pnum, pcursinvitem);
}

void PerformSecondaryAction(int pnum)
{
	if (invflag) {
		CtrlUseInvItem(pnum);
		return;
	}

	if (pcurs >= CURSOR_FIRSTITEM && !TryDropItem(pnum))
		return;
	if (pcurs > CURSOR_HAND)
		SetCursor_(CURSOR_HAND);

	if (pcursitem != -1) {
		NetSendCmdLocParam1(true, CMD_GOTOAGETITEM, cursmx, cursmy, pcursitem);
	} else if (pcursobj != -1) {
		NetSendCmdLocParam1(true, CMD_OPOBJXY, cursmx, cursmy, pcursobj);
	} else if (pcursmissile != -1) {
		MakePlrPath(pnum, missile[pcursmissile]._mix, missile[pcursmissile]._miy, true);
		plr[pnum].destAction = ACTION_WALK;
	} else if (pcurstrig != -1) {
		MakePlrPath(pnum, trigs[pcurstrig]._tx, trigs[pcurstrig]._ty, true);
		plr[pnum].destAction = ACTION_WALK;
	} else if (pcursquest != -1) {
		MakePlrPath(pnum, quests[pcursquest]._qtx, quests[pcursquest]._qty, true);
		plr[pnum].destAction = ACTION_WALK;
	}
}

} // namespace dvl
