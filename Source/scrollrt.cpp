/**
 * @file plrmsg.cpp
 *
 * Implementation of functionality for rendering the dungeons, monsters and calling other render routines.
 */
#include "all.h"
#include <iostream>

DEVILUTION_BEGIN_NAMESPACE

/**
 * Specifies the current light entry.
 */
int light_table_index;
DWORD sgdwCursWdtOld;
DWORD sgdwCursX;
DWORD sgdwCursY;
/**
 * Lower bound of back buffer.
 */
DWORD sgdwCursHgt;

/**
 * Specifies the current MIN block of the level CEL file, as used during rendering of the level tiles.
 *
 * frameNum  := block & 0x0FFF
 * frameType := block & 0x7000 >> 12
 */
DWORD level_cel_block;
DWORD sgdwCursXOld;
DWORD sgdwCursYOld;
BOOLEAN AutoMapShowItems;
/**
 * Specifies the type of arches to render.
 */
char arch_draw_type;
/**
 * Specifies whether transparency is active for the current CEL file being decoded.
 */
int cel_transparency_active;
/**
 * Specifies whether foliage (tile has extra content that overlaps previous tile) being rendered.
 */
int cel_foliage_active = false;
/**
 * Specifies the current dungeon piece ID of the level, as used during rendering of the level tiles.
 */
int level_piece_id;
DWORD sgdwCursWdt;
void (*DrawPlrProc)(int, int, int, int, int, BYTE *, int, int, int, int);
BYTE sgSaveBack[8192];
DWORD sgdwCursHgtOld;

bool dRendered[MAXDUNX][MAXDUNY];

int frames;
BOOL frameflag;
int frameend;
int framerate;
int framestart;

/* data */

const char *const szMonModeAssert[] = {
	"standing",
	"walking (1)",
	"walking (2)",
	"walking (3)",
	"attacking",
	"getting hit",
	"dying",
	"attacking (special)",
	"fading in",
	"fading out",
	"attacking (ranged)",
	"standing (special)",
	"attacking (special ranged)",
	"delaying",
	"charging",
	"stoned",
	"healing",
	"talking"
};

const char *const szPlrModeAssert[] = {
	"standing",
	"walking (1)",
	"walking (2)",
	"walking (3)",
	"attacking (melee)",
	"attacking (ranged)",
	"blocking",
	"getting hit",
	"dying",
	"casting a spell",
	"changing levels",
	"quitting"
};

/**
 * @brief Clear cursor state
 */
void ClearCursor() // CODE_FIX: this was supposed to be in cursor.cpp
{
	sgdwCursWdt = 0;
	sgdwCursWdtOld = 0;
}

static void BlitCursor(BYTE *dst, int dst_pitch, BYTE *src, int src_pitch)
{
	const int h = std::min(sgdwCursY + 1, sgdwCursHgt);
	for (int i = 0; i < h; ++i, src += src_pitch, dst += dst_pitch) {
		memcpy(dst, src, sgdwCursWdt);
	}
}

/**
 * @brief Remove the cursor from the buffer
 */
static void scrollrt_draw_cursor_back_buffer(CelOutputBuffer out)
{
	if (sgdwCursWdt == 0) {
		return;
	}

	BlitCursor(out.at(sgdwCursX, sgdwCursY), out.pitch(), sgSaveBack, sgdwCursWdt);

	sgdwCursXOld = sgdwCursX;
	sgdwCursYOld = sgdwCursY;
	sgdwCursWdtOld = sgdwCursWdt;
	sgdwCursHgtOld = sgdwCursHgt;
	sgdwCursWdt = 0;
}

/**
 * @brief Draw the cursor on the given buffer
 */
static void scrollrt_draw_cursor_item(CelOutputBuffer out)
{
	int i, mx, my;
	BYTE col;

	assert(!sgdwCursWdt);

	if (pcurs <= CURSOR_NONE || cursW == 0 || cursH == 0) {
		return;
	}

	if (sgbControllerActive && !IsMovingMouseCursorWithController() && pcurs != CURSOR_TELEPORT && !invflag && (!chrflag || plr[myplr]._pStatPts <= 0)) {
		return;
	}

	mx = MouseX - 1;
	if (mx < 0 - cursW - 1) {
		return;
	} else if (mx > gnScreenWidth - 1) {
		return;
	}
	my = MouseY - 1;
	if (my < 0 - cursH - 1) {
		return;
	} else if (my > gnScreenHeight - 1) {
		return;
	}

	sgdwCursX = mx;
	sgdwCursWdt = sgdwCursX + cursW + 1;
	if (sgdwCursWdt > gnScreenWidth - 1) {
		sgdwCursWdt = gnScreenWidth - 1;
	}
	sgdwCursX &= ~3;
	sgdwCursWdt |= 3;
	sgdwCursWdt -= sgdwCursX;
	sgdwCursWdt++;

	sgdwCursY = my;
	sgdwCursHgt = sgdwCursY + cursH + 1;
	if (sgdwCursHgt > gnScreenHeight - 1) {
		sgdwCursHgt = gnScreenHeight - 1;
	}
	sgdwCursHgt -= sgdwCursY;
	sgdwCursHgt++;

	BlitCursor(sgSaveBack, sgdwCursWdt, out.at(sgdwCursX, sgdwCursY), out.pitch());

	mx++;
	my++;

	out = out.subregion(0, 0, out.w() - 2, out.h());
	if (pcurs >= CURSOR_FIRSTITEM) {
		col = PAL16_YELLOW + 5;
		if (plr[myplr].HoldItem._iMagical != 0) {
			col = PAL16_BLUE + 5;
		}
		if (!plr[myplr].HoldItem._iStatFlag) {
			col = PAL16_RED + 5;
		}
		if (pcurs <= 179) {
			CelBlitOutlineTo(out, col, mx, my + cursH - 1, pCursCels, pcurs, cursW);
			if (col != PAL16_RED + 5) {
				CelClippedDrawSafeTo(out, mx, my + cursH - 1, pCursCels, pcurs, cursW);
			} else {
				CelDrawLightRedSafeTo(out, mx, my + cursH - 1, pCursCels, pcurs, cursW, 1);
			}
		} else {
			CelBlitOutlineTo(out, col, mx, my + cursH - 1, pCursCels2, pcurs - 179, cursW);
			if (col != PAL16_RED + 5) {
				CelClippedDrawSafeTo(out, mx, my + cursH - 1, pCursCels2, pcurs - 179, cursW);
			} else {
				CelDrawLightRedSafeTo(out, mx, my + cursH - 1, pCursCels2, pcurs - 179, cursW, 0);
			}
		}
	} else {
		CelClippedDrawSafeTo(out, mx, my + cursH - 1, pCursCels, pcurs, cursW);
	}
}

/**
 * @brief Render a missile sprite
 * @param out Output buffer
 * @param m Pointer to MissileStruct struct
 * @param sx Output buffer coordinate
 * @param sy Output buffer coordinate
 * @param pre Is the sprite in the background
 */
void DrawMissilePrivate(CelOutputBuffer out, MissileStruct *m, int sx, int sy, BOOL pre)
{
	if (m->_miPreFlag != pre || !m->_miDrawFlag)
		return;

	BYTE *pCelBuff = m->_miAnimData;
	if (pCelBuff == NULL) {
		SDL_Log("Draw Missile 2 type %d: NULL Cel Buffer", m->_mitype);
		return;
	}
	int nCel = m->_miAnimFrame;
	int frames = SDL_SwapLE32(*(DWORD *)pCelBuff);
	if (nCel < 1 || frames > 50 || nCel > frames) {
		SDL_Log("Draw Missile 2: frame %d of %d, missile type==%d", nCel, frames, m->_mitype);
		return;
	}
	int mx = sx + m->_mixoff - m->_miAnimWidth2;
	int my = sy + m->_miyoff;
	if (m->_miUniqTrans)
		Cl2DrawLightTbl(out, mx, my, m->_miAnimData, m->_miAnimFrame, m->_miAnimWidth, m->_miUniqTrans + 3);
	else if (m->_miLightFlag)
		Cl2DrawLight(out, mx, my, m->_miAnimData, m->_miAnimFrame, m->_miAnimWidth);
	else
		Cl2Draw(out, mx, my, m->_miAnimData, m->_miAnimFrame, m->_miAnimWidth);
}

/**
 * @brief Render a missile sprites for a given tile
 * @param out Output buffer
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Output buffer coordinate
 * @param sy Output buffer coordinate
 * @param pre Is the sprite in the background
 */
void DrawMissile(CelOutputBuffer out, int x, int y, int sx, int sy, BOOL pre)
{
	int i;
	MissileStruct *m;

	if (!(dFlags[x][y] & BFLAG_MISSILE))
		return;

	if (dMissile[x][y] != -1) {
		m = &missile[dMissile[x][y] - 1];
		DrawMissilePrivate(out, m, sx, sy, pre);
		return;
	}

	for (i = 0; i < nummissiles; i++) {
		assert(missileactive[i] < MAXMISSILES);
		m = &missile[missileactive[i]];
		if (m->_mix != x || m->_miy != y)
			continue;
		DrawMissilePrivate(out, m, sx, sy, pre);
	}
}

/**
 * @brief Render a monster sprite
 * @param out Output buffer
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param mx Output buffer coordinate
 * @param my Output buffer coordinate
 * @param m Id of monster
 */
static void DrawMonster(CelOutputBuffer out, int x, int y, int mx, int my, int m)
{
	if (m < 0 || m >= MAXMONSTERS) {
		SDL_Log("Draw Monster: tried to draw illegal monster %d", m);
		return;
	}

	BYTE *pCelBuff = monster[m]._mAnimData;
	if (pCelBuff == NULL) {
		SDL_Log("Draw Monster \"%s\": NULL Cel Buffer", monster[m].mName);
		return;
	}

	int nCel = monster[m]._mAnimFrame;
	int frames = SDL_SwapLE32(*(DWORD *)pCelBuff);
	if (nCel < 1 || frames > 50 || nCel > frames) {
		const char *szMode = "unknown action";
		if (monster[m]._mmode <= 17)
			szMode = szMonModeAssert[monster[m]._mmode];
		SDL_Log(
		    "Draw Monster \"%s\" %s: facing %d, frame %d of %d",
		    monster[m].mName,
		    szMode,
		    monster[m]._mdir,
		    nCel,
		    frames);
		return;
	}

	if (!(dFlags[x][y] & BFLAG_LIT)) {
		Cl2DrawLightTbl(out, mx, my, monster[m]._mAnimData, monster[m]._mAnimFrame, monster[m].MType->width, 1);
		return;
	}

	char trans = 0;
	if (monster[m]._uniqtype)
		trans = monster[m]._uniqtrans + 4;
	if (monster[m]._mmode == MM_STONE)
		trans = 2;
	if (plr[myplr]._pInfraFlag && light_table_index > 8)
		trans = 1;
	if (trans)
		Cl2DrawLightTbl(out, mx, my, monster[m]._mAnimData, monster[m]._mAnimFrame, monster[m].MType->width, trans);
	else
		Cl2DrawLight(out, mx, my, monster[m]._mAnimData, monster[m]._mAnimFrame, monster[m].MType->width);
}

/**
 * @brief Helper for rendering player a Mana Shield
 * @param out Output buffer
 * @param pnum Player id
 * @param sx Output buffer coordinate
 * @param sy Output buffer coordinate
 * @param lighting Should lighting be applied
 */
static void DrawManaShield(CelOutputBuffer out, int pnum, int x, int y, bool lighting)
{
	if (!plr[pnum].pManaShield)
		return;

	x += plr[pnum]._pAnimWidth2 - misfiledata[MFILE_MANASHLD].mAnimWidth2[0];

	int width = misfiledata[MFILE_MANASHLD].mAnimWidth[0];
	BYTE *pCelBuff = misfiledata[MFILE_MANASHLD].mAnimData[0];

	if (pnum == myplr) {
		Cl2Draw(out, x, y, pCelBuff, 1, width);
		return;
	}

	if (lighting) {
		Cl2DrawLightTbl(out, x, y, pCelBuff, 1, width, 1);
		return;
	}

	Cl2DrawLight(out, x, y, pCelBuff, 1, width);
}

/**
 * @brief Render a player sprite
 * @param out Output buffer
 * @param pnum Player id
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param px Output buffer coordinate
 * @param py Output buffer coordinate
 * @param pCelBuff sprite buffer
 * @param nCel frame
 * @param nWidth width
 */
static void DrawPlayer(CelOutputBuffer out, int pnum, int x, int y, int px, int py, BYTE *pCelBuff, int nCel, int nWidth)
{
	if ((dFlags[x][y] & BFLAG_LIT) == 0 && !plr[myplr]._pInfraFlag && leveltype != DTYPE_TOWN) {
		return;
	}

	if (pCelBuff == NULL) {
		SDL_Log("Drawing player %d \"%s\": NULL Cel Buffer", pnum, plr[pnum]._pName);
		return;
	}

	int frames = SDL_SwapLE32(*(DWORD *)pCelBuff);
	if (nCel < 1 || frames > 50 || nCel > frames) {
		const char *szMode = "unknown action";
		if (plr[pnum]._pmode <= PM_QUIT)
			szMode = szPlrModeAssert[plr[pnum]._pmode];
		SDL_Log(
		    "Drawing player %d \"%s\" %s: facing %d, frame %d of %d",
		    pnum,
		    plr[pnum]._pName,
		    szMode,
		    plr[pnum]._pdir,
		    nCel,
		    frames);
		return;
	}

	if (pnum == pcursplr)
		Cl2DrawOutline(out, 165, px, py, pCelBuff, nCel, nWidth);

	if (pnum == myplr) {
		Cl2Draw(out, px, py, pCelBuff, nCel, nWidth);
		DrawManaShield(out, pnum, px, py, true);
		return;
	}

	if (!(dFlags[x][y] & BFLAG_LIT) || plr[myplr]._pInfraFlag && light_table_index > 8) {
		Cl2DrawLightTbl(out, px, py, pCelBuff, nCel, nWidth, 1);
		DrawManaShield(out, pnum, px, py, true);
		return;
	}

	int l = light_table_index;
	if (light_table_index < 5)
		light_table_index = 0;
	else
		light_table_index -= 5;

	Cl2DrawLight(out, px, py, pCelBuff, nCel, nWidth);
	DrawManaShield(out, pnum, px, py, false);

	light_table_index = l;
}

/**
 * @brief Render a player sprite
 * @param out Output buffer
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Output buffer coordinate
 * @param sy Output buffer coordinate
 */
void DrawDeadPlayer(CelOutputBuffer out, int x, int y, int sx, int sy)
{
	int i, px, py;
	PlayerStruct *p;

	dFlags[x][y] &= ~BFLAG_DEAD_PLAYER;

	for (i = 0; i < MAX_PLRS; i++) {
		p = &plr[i];
		if (p->plractive && p->_pHitPoints == 0 && p->plrlevel == (BYTE)currlevel && p->_px == x && p->_py == y) {
			dFlags[x][y] |= BFLAG_DEAD_PLAYER;
			px = sx + p->_pxoff - p->_pAnimWidth2;
			py = sy + p->_pyoff;
			DrawPlayer(out, i, x, y, px, py, p->_pAnimData, p->_pAnimFrame, p->_pAnimWidth);
		}
	}
}

/**
 * @brief Render an object sprite
 * @param out Output buffer
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param ox Output buffer coordinate
 * @param oy Output buffer coordinate
 * @param pre Is the sprite in the background
 */
static void DrawObject(CelOutputBuffer out, int x, int y, int ox, int oy, BOOL pre)
{
	if (dObject[x][y] == 0 || light_table_index >= lightmax)
		return;

	int sx, sy;
	char bv;
	if (dObject[x][y] > 0) {
		bv = dObject[x][y] - 1;
		if (object[bv]._oPreFlag != pre)
			return;
		sx = ox - object[bv]._oAnimWidth2;
		sy = oy;
	} else {
		bv = -(dObject[x][y] + 1);
		if (object[bv]._oPreFlag != pre)
			return;
		int xx = object[bv]._ox - x;
		int yy = object[bv]._oy - y;
		sx = (xx << 5) + ox - object[bv]._oAnimWidth2 - (yy << 5);
		sy = oy + (yy << 4) + (xx << 4);
	}

	assert(bv >= 0 && bv < MAXOBJECTS);

	BYTE *pCelBuff = object[bv]._oAnimData;
	if (pCelBuff == NULL) {
		SDL_Log("Draw Object type %d: NULL Cel Buffer", object[bv]._otype);
		return;
	}

	int nCel = object[bv]._oAnimFrame;
	int frames = SDL_SwapLE32(*(DWORD *)pCelBuff);
	if (nCel < 1 || frames > 50 || nCel > frames) {
		SDL_Log("Draw Object: frame %d of %d, object type==%d", nCel, frames, object[bv]._otype);
		return;
	}

	if (bv == pcursobj)
		CelBlitOutlineTo(out, 194, sx, sy, object[bv]._oAnimData, object[bv]._oAnimFrame, object[bv]._oAnimWidth);
	if (object[bv]._oLight) {
		CelClippedDrawLightTo(out, sx, sy, object[bv]._oAnimData, object[bv]._oAnimFrame, object[bv]._oAnimWidth);
	} else {
		CelClippedDrawTo(out, sx, sy, object[bv]._oAnimData, object[bv]._oAnimFrame, object[bv]._oAnimWidth);
	}
}

static void scrollrt_draw_dungeon(CelOutputBuffer, int, int, int, int);

/**
 * @brief Render a cell
 * @param out Target buffer
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Target buffer coordinate
 * @param sy Target buffer coordinate
 */
static void drawCell(CelOutputBuffer out, int x, int y, int sx, int sy)
{
	MICROS *pMap = &dpiece_defs_map_2[x][y];
	level_piece_id = dPiece[x][y];
	cel_transparency_active = (BYTE)(nTransTable[level_piece_id] & TransList[dTransVal[x][y]]);
	cel_foliage_active = !nSolidTable[level_piece_id];
	for (int i = 0; i < (MicroTileLen >> 1); i++) {
		level_cel_block = pMap->mt[2 * i];
		if (level_cel_block != 0) {
			arch_draw_type = i == 0 ? 1 : 0;
			RenderTile(out, sx, sy);
		}
		level_cel_block = pMap->mt[2 * i + 1];
		if (level_cel_block != 0) {
			arch_draw_type = i == 0 ? 2 : 0;
			RenderTile(out, sx + TILE_WIDTH / 2, sy);
		}
		sy -= TILE_HEIGHT;
	}
	cel_foliage_active = false;
}

/**
 * @brief Render a floor tiles
 * @param out Target buffer
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Target buffer coordinate
 * @param sy Target buffer coordinate
 */
static void drawFloor(CelOutputBuffer out, int x, int y, int sx, int sy)
{
	cel_transparency_active = 0;
	light_table_index = dLight[x][y];

	arch_draw_type = 1; // Left
	level_cel_block = dpiece_defs_map_2[x][y].mt[0];
	if (level_cel_block != 0) {
		RenderTile(out, sx, sy);
	}
	arch_draw_type = 2; // Right
	level_cel_block = dpiece_defs_map_2[x][y].mt[1];
	if (level_cel_block != 0) {
		RenderTile(out, sx + TILE_WIDTH / 2, sy);
	}
}

/**
 * @brief Draw item for a given tile
 * @param out Output buffer
 * @param y dPiece coordinate
 * @param x dPiece coordinate
 * @param sx Output buffer coordinate
 * @param sy Output buffer coordinate
 * @param pre Is the sprite in the background
 */
static void DrawItem(CelOutputBuffer out, int x, int y, int sx, int sy, BOOL pre)
{
	char bItem = dItem[x][y];

	assert((unsigned char)bItem <= MAXITEMS);

	if (bItem > MAXITEMS || bItem <= 0)
		return;

	ItemStruct *pItem = &item[bItem - 1];
	if (pItem->_iPostDraw == pre)
		return;

	BYTE *pCelBuff = pItem->_iAnimData;
	if (pCelBuff == NULL) {
		SDL_Log("Draw Item \"%s\" 1: NULL Cel Buffer", pItem->_iIName);
		return;
	}

	int nCel = pItem->_iAnimFrame;
	int frames = SDL_SwapLE32(*(DWORD *)pCelBuff);
	if (nCel < 1 || frames > 50 || nCel > frames) {
		SDL_Log("Draw \"%s\" Item 1: frame %d of %d, item type==%d", pItem->_iIName, nCel, frames, pItem->_itype);
		return;
	}

	int px = sx - pItem->_iAnimWidth2;
	if (bItem - 1 == pcursitem || AutoMapShowItems) {
		CelBlitOutlineTo(out, 181, px, sy, pCelBuff, nCel, pItem->_iAnimWidth);
	}
	CelClippedDrawLightTo(out, px, sy, pCelBuff, nCel, pItem->_iAnimWidth);
}

/**
 * @brief Check if and how a monster should be rendered
 * @param out Output buffer
 * @param y dPiece coordinate
 * @param x dPiece coordinate
 * @param oy dPiece Y offset
 * @param sx Output buffer coordinate
 * @param sy Output buffer coordinate
 */
static void DrawMonsterHelper(CelOutputBuffer out, int x, int y, int oy, int sx, int sy)
{
	int mi, px, py;
	MonsterStruct *pMonster;

	mi = dMonster[x][y + oy];
	mi = mi > 0 ? mi - 1 : -(mi + 1);

	if (leveltype == DTYPE_TOWN) {
		px = sx - towner[mi]._tAnimWidth2;
		if (mi == pcursmonst) {
			CelBlitOutlineTo(out, 166, px, sy, towner[mi]._tAnimData, towner[mi]._tAnimFrame, towner[mi]._tAnimWidth);
		}
		assert(towner[mi]._tAnimData);
		CelClippedDrawTo(out, px, sy, towner[mi]._tAnimData, towner[mi]._tAnimFrame, towner[mi]._tAnimWidth);
		return;
	}

	if (!(dFlags[x][y] & BFLAG_LIT) && !plr[myplr]._pInfraFlag)
		return;

	if (mi < 0 || mi >= MAXMONSTERS) {
		SDL_Log("Draw Monster: tried to draw illegal monster %d", mi);
		return;
	}

	pMonster = &monster[mi];
	if (pMonster->_mFlags & MFLAG_HIDDEN) {
		return;
	}

	if (pMonster->MType == NULL) {
		SDL_Log("Draw Monster \"%s\": uninitialized monster", pMonster->mName);
		return;
	}

	px = sx + pMonster->_mxoff - pMonster->MType->width2;
	py = sy + pMonster->_myoff;
	if (mi == pcursmonst) {
		Cl2DrawOutline(out, 233, px, py, pMonster->_mAnimData, pMonster->_mAnimFrame, pMonster->MType->width);
	}
	DrawMonster(out, x, y, px, py, mi);
}

/**
 * @brief Check if and how a player should be rendered
 * @param out Output buffer
 * @param y dPiece coordinate
 * @param x dPiece coordinate
 * @param sx Output buffer coordinate
 * @param sy Output buffer coordinate
 */
static void DrawPlayerHelper(CelOutputBuffer out, int x, int y, int sx, int sy)
{
	int p = dPlayer[x][y];
	p = p > 0 ? p - 1 : -(p + 1);

	if (p < 0 || p >= MAX_PLRS) {
		SDL_Log("draw player: tried to draw illegal player %d", p);
		return;
	}

	PlayerStruct *pPlayer = &plr[p];
	int px = sx + pPlayer->_pxoff - pPlayer->_pAnimWidth2;
	int py = sy + pPlayer->_pyoff;

	DrawPlayer(out, p, x, y, px, py, pPlayer->_pAnimData, pPlayer->_pAnimFrame, pPlayer->_pAnimWidth);
}

/**
 * @brief Render object sprites
 * @param out Target buffer
 * @param sx dPiece coordinate
 * @param sy dPiece coordinate
 * @param dx Target buffer coordinate
 * @param dy Target buffer coordinate
 */
static void scrollrt_draw_dungeon(CelOutputBuffer out, int sx, int sy, int dx, int dy)
{
	assert((DWORD)sx < MAXDUNX);
	assert((DWORD)sy < MAXDUNY);

	if (dRendered[sx][sy])
		return;
	dRendered[sx][sy] = true;

	light_table_index = dLight[sx][sy];

	drawCell(out, sx, sy, dx, dy);

	char bFlag = dFlags[sx][sy];
	char bDead = dDead[sx][sy];
	char bMap = dTransVal[sx][sy];

	int negMon = 0;
	if (sy > 0) // check for OOB
		negMon = dMonster[sx][sy - 1];

#ifdef _DEBUG
	if (visiondebug && bFlag & BFLAG_LIT) {
		CelClippedDrawTo(out, dx, dy, pSquareCel, 1, 64);
	}
#endif

	if (MissilePreFlag) {
		DrawMissile(out, sx, sy, dx, dy, TRUE);
	}

	if (light_table_index < lightmax && bDead != 0) {
		do {
			DeadStruct *pDeadGuy = &dead[(bDead & 0x1F) - 1];
			char dd = (bDead >> 5) & 7;
			int px = dx - pDeadGuy->_deadWidth2;
			BYTE *pCelBuff = pDeadGuy->_deadData[dd];
			assert(pCelBuff != NULL);
			if (pCelBuff == NULL)
				break;
			int frames = SDL_SwapLE32(*(DWORD *)pCelBuff);
			int nCel = pDeadGuy->_deadFrame;
			if (nCel < 1 || frames > 50 || nCel > frames) {
				SDL_Log("Unclipped dead: frame %d of %d, deadnum==%d", nCel, frames, (bDead & 0x1F) - 1);
				break;
			}
			if (pDeadGuy->_deadtrans != 0) {
				Cl2DrawLightTbl(out, px, dy, pCelBuff, nCel, pDeadGuy->_deadWidth, pDeadGuy->_deadtrans);
			} else {
				Cl2DrawLight(out, px, dy, pCelBuff, nCel, pDeadGuy->_deadWidth);
			}
		} while (0);
	}
	DrawObject(out, sx, sy, dx, dy, 1);
	DrawItem(out, sx, sy, dx, dy, 1);
	if (bFlag & BFLAG_PLAYERLR) {
		assert((DWORD)(sy - 1) < MAXDUNY);
		DrawPlayerHelper(out, sx, sy - 1, dx, dy);
	}
	if (bFlag & BFLAG_MONSTLR && negMon < 0) {
		DrawMonsterHelper(out, sx, sy, -1, dx, dy);
	}
	if (bFlag & BFLAG_DEAD_PLAYER) {
		DrawDeadPlayer(out, sx, sy, dx, dy);
	}
	if (dPlayer[sx][sy] > 0) {
		if (dPlayer[sx][sy] > 0) {
			//std::cout << "Drawing dPlayer[" << sx << "][" << sy << "] = " << (int)dPlayer[sx][sy] << std::endl;
		}
		DrawPlayerHelper(out, sx, sy, dx, dy);
	}
	if (dMonster[sx][sy] > 0) {
		DrawMonsterHelper(out, sx, sy, 0, dx, dy);
	}
	DrawMissile(out, sx, sy, dx, dy, FALSE);
	DrawObject(out, sx, sy, dx, dy, 0);
	DrawItem(out, sx, sy, dx, dy, 0);

	if (leveltype != DTYPE_TOWN) {
		char bArch = dSpecial[sx][sy];
		if (bArch != 0) {
			cel_transparency_active = TransList[bMap];
#ifdef _DEBUG
			if (GetAsyncKeyState(DVL_VK_MENU) & 0x8000) {
				cel_transparency_active = 0; // Turn transparency off here for debugging
			}
#endif
			CelClippedBlitLightTransTo(out, dx, dy, pSpecialCels, bArch, 64);
#ifdef _DEBUG
			if (GetAsyncKeyState(DVL_VK_MENU) & 0x8000) {
				cel_transparency_active = TransList[bMap]; // Turn transparency back to its normal state
			}
#endif
		}
	} else {
		// Tree leaves should always cover player when entering or leaving the tile,
		// So delay the rendering until after the next row is being drawn.
		// This could probably have been better solved by sprites in screen space.
		if (sx > 0 && sy > 0 && dy > TILE_HEIGHT) {
			char bArch = dSpecial[sx - 1][sy - 1];
			if (bArch != 0) {
				CelDrawTo(out, dx, dy - TILE_HEIGHT, pSpecialCels, bArch, 64);
			}
		}
	}
}

/**
 * @brief Render a row of tiles
 * @param out Buffer to render to
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Target buffer coordinate
 * @param sy Target buffer coordinate
 * @param rows Number of rows
 * @param columns Tile in a row
 */
static void scrollrt_drawFloor(CelOutputBuffer out, int x, int y, int sx, int sy, int rows, int columns)
{
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			if (x >= 0 && x < MAXDUNX && y >= 0 && y < MAXDUNY) {
				level_piece_id = dPiece[x][y];
				if (level_piece_id != 0) {
					if (!nSolidTable[level_piece_id])
						drawFloor(out, x, y, sx, sy);
				} else {
					world_draw_black_tile(out, sx, sy);
				}
			} else {
				world_draw_black_tile(out, sx, sy);
			}
			ShiftGrid(&x, &y, 1, 0);
			sx += TILE_WIDTH;
		}
		// Return to start of row
		ShiftGrid(&x, &y, -columns, 0);
		sx -= columns * TILE_WIDTH;

		// Jump to next row
		sy += TILE_HEIGHT / 2;
		if (i & 1) {
			x++;
			columns--;
			sx += TILE_WIDTH / 2;
		} else {
			y++;
			columns++;
			sx -= TILE_WIDTH / 2;
		}
	}
}

#define IsWall(x, y) (dPiece[x][y] == 0 || nSolidTable[dPiece[x][y]] || dSpecial[x][y] != 0)
#define IsWalkable(x, y) (dPiece[x][y] != 0 && !nSolidTable[dPiece[x][y]])

/**
 * @brief Render a row of tile
 * @param out Output buffer
 * @param x dPiece coordinate
 * @param y dPiece coordinate
 * @param sx Buffer coordinate
 * @param sy Buffer coordinate
 * @param rows Number of rows
 * @param columns Tile in a row
 */
static void scrollrt_draw(CelOutputBuffer out, int x, int y, int sx, int sy, int rows, int columns)
{
	// Keep evaluating until MicroTiles can't affect screen
	rows += MicroTileLen;
	memset(dRendered, 0, sizeof(dRendered));

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			if (x >= 0 && x < MAXDUNX && y >= 0 && y < MAXDUNY) {
				if (x + 1 < MAXDUNX && y - 1 >= 0 && sx + TILE_WIDTH <= gnScreenWidth) {
					// Render objects behind walls first to prevent sprites, that are moving
					// between tiles, from poking through the walls as they exceed the tile bounds.
					// A proper fix for this would probably be to layout the sceen and render by
					// sprite screen position rather than tile position.
					if (IsWall(x, y) && (IsWall(x + 1, y) || (x > 0 && IsWall(x - 1, y)))) { // Part of a wall aligned on the x-axis
						if (IsWalkable(x + 1, y - 1) && IsWalkable(x, y - 1)) {              // Has walkable area behind it
							scrollrt_draw_dungeon(out, x + 1, y - 1, sx + TILE_WIDTH, sy);
						}
					}
				}
				if (dPiece[x][y] != 0) {
					scrollrt_draw_dungeon(out, x, y, sx, sy);
				}
			}
			ShiftGrid(&x, &y, 1, 0);
			sx += TILE_WIDTH;
		}
		// Return to start of row
		ShiftGrid(&x, &y, -columns, 0);
		sx -= columns * TILE_WIDTH;

		// Jump to next row
		sy += TILE_HEIGHT / 2;
		if (i & 1) {
			x++;
			columns--;
			sx += TILE_WIDTH / 2;
		} else {
			y++;
			columns++;
			sx -= TILE_WIDTH / 2;
		}
	}
}

/**
 * @brief Scale up the rendered part of the back buffer to take up the full view
 */
static void Zoom(CelOutputBuffer out)
{
	int wdt = gnScreenWidth / 2;

	int src_x = gnScreenWidth / 2 - 1;
	int dst_x = gnScreenWidth - 1;

	if (PANELS_COVER) {
		if (chrflag || questlog) {
			wdt >>= 1;
			src_x -= wdt;
		} else if (invflag || sbookflag) {
			wdt >>= 1;
			src_x -= wdt;
			dst_x -= SPANEL_WIDTH;
		}
	}

	BYTE *src = out.at(src_x, gnViewportHeight / 2 - 1);
	BYTE *dst = out.at(dst_x, gnViewportHeight - 1);

	for (int hgt = 0; hgt < gnViewportHeight / 2; hgt++) {
		for (int i = 0; i < wdt; i++) {
			*dst-- = *src;
			*dst-- = *src;
			src--;
		}
		memcpy(dst - out.pitch(), dst, wdt * 2 + 1);
		src -= out.pitch() - wdt;
		dst -= 2 * (out.pitch() - wdt);
	}
}

/**
 * @brief Shifting the view area along the logical grid
 *        Note: this won't allow you to shift between even and odd rows
 * @param horizontal Shift the screen left or right
 * @param vertical Shift the screen up or down
 */
void ShiftGrid(int *x, int *y, int horizontal, int vertical)
{
	*x += vertical + horizontal;
	*y += vertical - horizontal;
}

/**
 * @brief Gets the number of rows covered by the main panel
 */
int RowsCoveredByPanel()
{
	if (gnScreenWidth <= PANEL_WIDTH) {
		return 0;
	}

	int rows = PANEL_HEIGHT / TILE_HEIGHT;
	if (!zoomflag) {
		rows /= 2;
	}

	return rows;
}

/**
 * @brief Calculate the offset needed for centering tiles in view area
 * @param offsetX Offset in pixels
 * @param offsetY Offset in pixels
 */
void CalcTileOffset(int *offsetX, int *offsetY)
{
	int x, y;

	if (zoomflag) {
		x = gnScreenWidth % TILE_WIDTH;
		y = gnViewportHeight % TILE_HEIGHT;
	} else {
		x = (gnScreenWidth / 2) % TILE_WIDTH;
		y = (gnViewportHeight / 2) % TILE_HEIGHT;
	}

	if (x)
		x = (TILE_WIDTH - x) / 2;
	if (y)
		y = (TILE_HEIGHT - y) / 2;

	*offsetX = x;
	*offsetY = y;
}

/**
 * @brief Calculate the needed diamond tile to cover the view area
 * @param columns Tiles needed per row
 * @param rows Both even and odd rows
 */
void TilesInView(int *rcolumns, int *rrows)
{
	int columns = gnScreenWidth / TILE_WIDTH;
	if (gnScreenWidth % TILE_WIDTH) {
		columns++;
	}
	int rows = gnViewportHeight / TILE_HEIGHT;
	if (gnViewportHeight % TILE_HEIGHT) {
		rows++;
	}

	if (!zoomflag) {
		// Half the number of tiles, rounded up
		if (columns & 1) {
			columns++;
		}
		columns /= 2;
		if (rows & 1) {
			rows++;
		}
		rows /= 2;
	}

	*rcolumns = columns;
	*rrows = rows;
}

int tileOffsetX;
int tileOffsetY;
int tileShiftX;
int tileShiftY;
int tileColums;
int tileRows;

void CalcViewportGeometry()
{
	int xo, yo;
	tileShiftX = 0;
	tileShiftY = 0;

	// Adjust by player offset and tile grid alignment
	CalcTileOffset(&xo, &yo);
	tileOffsetX = 0 - xo;
	tileOffsetY = 0 - yo - 1 + TILE_HEIGHT / 2;

	TilesInView(&tileColums, &tileRows);
	int lrow = tileRows - RowsCoveredByPanel();

	// Center player tile on screen
	ShiftGrid(&tileShiftX, &tileShiftY, -tileColums / 2, -lrow / 2);

	tileRows *= 2;

	// Align grid
	if ((tileColums & 1) == 0) {
		tileShiftY--; // Shift player row to one that can be centered with out pixel offset
		if ((lrow & 1) == 0) {
			// Offset tile to vertically align the player when both rows and colums are even
			tileRows++;
			tileOffsetY -= TILE_HEIGHT / 2;
		}
	} else if (tileColums & 1 && lrow & 1) {
		// Offset tile to vertically align the player when both rows and colums are odd
		ShiftGrid(&tileShiftX, &tileShiftY, 0, -1);
		tileRows++;
		tileOffsetY -= TILE_HEIGHT / 2;
	}

	// Slightly lower the zoomed view
	if (!zoomflag) {
		tileOffsetY += TILE_HEIGHT / 4;
		if (yo < TILE_HEIGHT / 4)
			tileRows++;
	}

	tileRows++; // Cover lower edge saw tooth, right edge accounted for in scrollrt_draw()
}

/**
 * @brief Configure render and process screen rows
 * @param full_out Buffer to render to
 * @param x Center of view in dPiece coordinate
 * @param y Center of view in dPiece coordinate
 */
static void DrawGame(CelOutputBuffer full_out, int x, int y)
{
	int sx, sy, columns, rows;

	// Limit rendering to the view area
	CelOutputBuffer out = zoomflag
	    ? full_out.subregionY(0, gnViewportHeight)
	    : full_out.subregionY(0, gnViewportHeight / 2);

	// Adjust by player offset and tile grid alignment
	sx = ScrollInfo._sxoff + tileOffsetX;
	sy = ScrollInfo._syoff + tileOffsetY;

	columns = tileColums;
	rows = tileRows;

	x += tileShiftX;
	y += tileShiftY;

	// Skip rendering parts covered by the panels
	if (PANELS_COVER) {
		if (zoomflag) {
			if (chrflag || questlog) {
				ShiftGrid(&x, &y, 2, 0);
				columns -= 4;
				sx += SPANEL_WIDTH - TILE_WIDTH / 2;
			}
			if (invflag || sbookflag) {
				ShiftGrid(&x, &y, 2, 0);
				columns -= 4;
				sx += -TILE_WIDTH / 2;
			}
		} else {
			if (chrflag || questlog) {
				ShiftGrid(&x, &y, 1, 0);
				columns -= 2;
				sx += -TILE_WIDTH / 2 / 2; // SPANEL_WIDTH accounted for in Zoom()
			}
			if (invflag || sbookflag) {
				ShiftGrid(&x, &y, 1, 0);
				columns -= 2;
				sx += -TILE_WIDTH / 2 / 2;
			}
		}
	}

	// Draw areas moving in and out of the screen
	switch (ScrollInfo._sdir) {
	case SDIR_N:
		sy -= TILE_HEIGHT;
		ShiftGrid(&x, &y, 0, -1);
		rows += 2;
		break;
	case SDIR_NE:
		sy -= TILE_HEIGHT;
		ShiftGrid(&x, &y, 0, -1);
		columns++;
		rows += 2;
		break;
	case SDIR_E:
		columns++;
		break;
	case SDIR_SE:
		columns++;
		rows++;
		break;
	case SDIR_S:
		rows += 2;
		break;
	case SDIR_SW:
		sx -= TILE_WIDTH;
		ShiftGrid(&x, &y, -1, 0);
		columns++;
		rows++;
		break;
	case SDIR_W:
		sx -= TILE_WIDTH;
		ShiftGrid(&x, &y, -1, 0);
		columns++;
		break;
	case SDIR_NW:
		sx -= TILE_WIDTH / 2;
		sy -= TILE_HEIGHT / 2;
		x--;
		columns++;
		rows++;
		break;
	}

	scrollrt_drawFloor(out, x, y, sx, sy, rows, columns);
	scrollrt_draw(out, x, y, sx, sy, rows, columns);

	if (!zoomflag) {
		Zoom(full_out.subregionY(0, gnScreenHeight));
	}
}

// DevilutionX extension.
extern void DrawControllerModifierHints(CelOutputBuffer out);

void DrawView(CelOutputBuffer out, int StartX, int StartY)
{
	DrawGame(out, StartX, StartY);
	if (automapflag) {
		DrawAutomap(out.subregionY(0, gnViewportHeight));
	}
	DrawMonsterHealthBar(out);

	if (stextflag && !qtextflag)
		DrawSText(out);
	if (invflag) {
		DrawInv(out);
	} else if (sbookflag) {
		DrawSpellBook(out);
	}

	DrawDurIcon(out);

	if (chrflag) {
		DrawChr(out);
	} else if (questlog) {
		DrawQuestLog(out);
	}
	if (!chrflag && plr[myplr]._pStatPts != 0 && !spselflag
	    && (!questlog || gnScreenHeight >= SPANEL_HEIGHT + PANEL_HEIGHT + 74 || gnScreenWidth >= 4 * SPANEL_WIDTH)) {
		DrawLevelUpIcon(out);
	}
	if (uitemflag) {
		DrawUniqueInfo(out);
	}
	if (qtextflag) {
		DrawQText(out);
	}
	if (spselflag) {
		DrawSpellList(out);
	}
	if (dropGoldFlag) {
		DrawGoldSplit(out, dropGoldValue);
	}
	if (helpflag) {
		DrawHelp(out);
	}
	if (msgflag) {
		DrawDiabloMsg(out);
	}
	if (deathflag) {
		RedBack(out);
	} else if (PauseMode != 0) {
		gmenu_draw_pause(out);
	}

	DrawControllerModifierHints(out);
	DrawPlrMsg(out);
	gmenu_draw(out);
	doom_draw(out);
	DrawInfoBox(out);
	DrawLifeFlask(out);
	DrawManaFlask(out);
}

extern SDL_Surface *pal_surface;

/**
 * @brief Render the whole screen black
 */
void ClearScreenBuffer()
{
	lock_buf(3);

	assert(pal_surface != NULL);

	SDL_Rect SrcRect = {
		BUFFER_BORDER_LEFT,
		BUFFER_BORDER_TOP,
		gnScreenWidth,
		gnScreenHeight,
	};
	SDL_FillRect(pal_surface, &SrcRect, 0);

	unlock_buf(3);
}

#ifdef _DEBUG
/**
 * @brief Scroll the screen when mouse is close to the edge
 */
void ScrollView()
{
	BOOL scroll;

	if (pcurs >= CURSOR_FIRSTITEM)
		return;

	scroll = FALSE;

	if (MouseX < 20) {
		if (dmaxy - 1 <= ViewY || dminx >= ViewX) {
			if (dmaxy - 1 > ViewY) {
				ViewY++;
				scroll = TRUE;
			}
			if (dminx < ViewX) {
				ViewX--;
				scroll = TRUE;
			}
		} else {
			ViewY++;
			ViewX--;
			scroll = TRUE;
		}
	}
	if (MouseX > gnScreenWidth - 20) {
		if (dmaxx - 1 <= ViewX || dminy >= ViewY) {
			if (dmaxx - 1 > ViewX) {
				ViewX++;
				scroll = TRUE;
			}
			if (dminy < ViewY) {
				ViewY--;
				scroll = TRUE;
			}
		} else {
			ViewY--;
			ViewX++;
			scroll = TRUE;
		}
	}
	if (MouseY < 20) {
		if (dminy >= ViewY || dminx >= ViewX) {
			if (dminy < ViewY) {
				ViewY--;
				scroll = TRUE;
			}
			if (dminx < ViewX) {
				ViewX--;
				scroll = TRUE;
			}
		} else {
			ViewX--;
			ViewY--;
			scroll = TRUE;
		}
	}
	if (MouseY > gnScreenHeight - 20) {
		if (dmaxy - 1 <= ViewY || dmaxx - 1 <= ViewX) {
			if (dmaxy - 1 > ViewY) {
				ViewY++;
				scroll = TRUE;
			}
			if (dmaxx - 1 > ViewX) {
				ViewX++;
				scroll = TRUE;
			}
		} else {
			ViewX++;
			ViewY++;
			scroll = TRUE;
		}
	}

	if (scroll)
		ScrollInfo._sdir = SDIR_NONE;
}
#endif

/**
 * @brief Initialize the FPS meter
 */
void EnableFrameCount()
{
	frameflag = frameflag == 0;
	framestart = SDL_GetTicks();
}

/**
 * @brief Display the current average FPS over 1 sec
 */
static void DrawFPS(CelOutputBuffer out)
{
	DWORD tc, frames;
	char String[12];

	if (frameflag && gbActive && pPanelText) {
		frameend++;
		tc = SDL_GetTicks();
		frames = tc - framestart;
		if (tc - framestart >= 1000) {
			framestart = tc;
			framerate = 1000 * frameend / frames;
			frameend = 0;
		}
		snprintf(String, 12, "%d FPS", framerate);
		PrintGameStr(out, 8, 65, String, COL_RED);
	}
}

/**
 * @brief Update part of the screen from the back buffer
 * @param dwX Back buffer coordinate
 * @param dwY Back buffer coordinate
 * @param dwWdt Back buffer coordinate
 * @param dwHgt Back buffer coordinate
 */
static void DoBlitScreen(Sint16 dwX, Sint16 dwY, Uint16 dwWdt, Uint16 dwHgt)
{
	// In SDL1 SDL_Rect x and y are Sint16. Cast explicitly to avoid a compiler warning.
	using CoordType = decltype(SDL_Rect {}.x);
	SDL_Rect src_rect {
		static_cast<CoordType>(BUFFER_BORDER_LEFT + dwX),
		static_cast<CoordType>(BUFFER_BORDER_TOP + dwY),
		dwWdt, dwHgt
	};
	SDL_Rect dst_rect { dwX, dwY, dwWdt, dwHgt };

	BltFast(&src_rect, &dst_rect);
}

/**
 * @brief Check render pipeline and blit individual screen parts
 * @param dwHgt Section of screen to update from top to bottom
 * @param draw_desc Render info box
 * @param draw_hp Render health bar
 * @param draw_mana Render mana bar
 * @param draw_sbar Render belt
 * @param draw_btn Render panel buttons
 */
static void DrawMain(int dwHgt, BOOL draw_desc, BOOL draw_hp, BOOL draw_mana, BOOL draw_sbar, BOOL draw_btn)
{
	if (!gbActive) {
		return;
	}

	assert(dwHgt >= 0 && dwHgt <= gnScreenHeight);

	if (dwHgt > 0) {
		DoBlitScreen(0, 0, gnScreenWidth, dwHgt);
	}
	if (dwHgt < gnScreenHeight) {
		if (draw_sbar) {
			DoBlitScreen(PANEL_LEFT + 204, PANEL_TOP + 5, 232, 28);
		}
		if (draw_desc) {
			DoBlitScreen(PANEL_LEFT + 176, PANEL_TOP + 46, 288, 60);
		}
		if (draw_mana) {
			DoBlitScreen(PANEL_LEFT + 460, PANEL_TOP, 88, 72);
			DoBlitScreen(PANEL_LEFT + 564, PANEL_TOP + 64, 56, 56);
		}
		if (draw_hp) {
			DoBlitScreen(PANEL_LEFT + 96, PANEL_TOP, 88, 72);
		}
		if (draw_btn) {
			DoBlitScreen(PANEL_LEFT + 8, PANEL_TOP + 5, 72, 119);
			DoBlitScreen(PANEL_LEFT + 556, PANEL_TOP + 5, 72, 48);
			if (gbIsMultiplayer) {
				DoBlitScreen(PANEL_LEFT + 84, PANEL_TOP + 91, 36, 32);
				DoBlitScreen(PANEL_LEFT + 524, PANEL_TOP + 91, 36, 32);
			}
		}
		if (sgdwCursWdtOld != 0) {
			DoBlitScreen(sgdwCursXOld, sgdwCursYOld, sgdwCursWdtOld, sgdwCursHgtOld);
		}
		if (sgdwCursWdt != 0) {
			DoBlitScreen(sgdwCursX, sgdwCursY, sgdwCursWdt, sgdwCursHgt);
		}
	}
}

/**
 * @brief Redraw screen
 * @param draw_cursor
 */
void scrollrt_draw_game_screen(BOOL draw_cursor)
{
	int hgt = 0;

	if (force_redraw == 255) {
		force_redraw = 0;
		hgt = gnScreenHeight;
	}

	if (draw_cursor) {
		lock_buf(0);
		scrollrt_draw_cursor_item(GlobalBackBuffer());
		unlock_buf(0);
	}

	DrawMain(hgt, FALSE, FALSE, FALSE, FALSE, FALSE);

	if (draw_cursor) {
		lock_buf(0);
		scrollrt_draw_cursor_back_buffer(GlobalBackBuffer());
		unlock_buf(0);
	}
	RenderPresent();
}

/**
 * @brief Render the game
 */
void DrawAndBlit()
{
	if (!gbRunGame) {
		return;
	}

	int hgt = 0;
	bool ddsdesc = false;
	bool ctrlPan = false;

	if (gnScreenWidth > PANEL_WIDTH || force_redraw == 255) {
		drawhpflag = TRUE;
		drawmanaflag = TRUE;
		drawbtnflag = TRUE;
		drawsbarflag = TRUE;
		ddsdesc = false;
		ctrlPan = true;
		hgt = gnScreenHeight;
	} else if (force_redraw == 1) {
		ddsdesc = true;
		ctrlPan = false;
		hgt = gnViewportHeight;
	}

	force_redraw = 0;

	lock_buf(0);
	CelOutputBuffer out = GlobalBackBuffer();

	//if (!gbIsCouchCoop) {
		DrawView(out, ViewX, ViewY);
		if (ctrlPan) {
			DrawCtrlPan(out);
		}
		if (drawhpflag) {
			UpdateLifeFlask(out);
		}
		if (drawmanaflag) {
			UpdateManaFlask(out);
		}
		if (drawbtnflag) {
			DrawCtrlBtns(out);
		}
		if (drawsbarflag) {
			DrawInvBelt(out);
		}
		if (talkflag) {
			DrawTalkPan(out);
			hgt = gnScreenHeight;
		}
		DrawXPBar(out);
		scrollrt_draw_cursor_item(out);
	//} else {
    //	DrawGame(out, ViewX, ViewY);
    //}

	DrawFPS(out);

	unlock_buf(0);

	DrawMain(hgt, ddsdesc, drawhpflag, drawmanaflag, drawsbarflag, drawbtnflag);

	lock_buf(0);
	scrollrt_draw_cursor_back_buffer(GlobalBackBuffer());
	unlock_buf(0);
	RenderPresent();

	drawhpflag = FALSE;
	drawmanaflag = FALSE;
	drawbtnflag = FALSE;
	drawsbarflag = FALSE;
}

DEVILUTION_END_NAMESPACE
