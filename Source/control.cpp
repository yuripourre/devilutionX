/**
 * @file control.cpp
 *
 * Implementation of the character and main control panels
 */
#include "all.h"

#include <cstddef>

DEVILUTION_BEGIN_NAMESPACE

namespace {

CelOutputBuffer pBtmBuff;
CelOutputBuffer pLifeBuff;
CelOutputBuffer pManaBuff;

} // namespace

BYTE sgbNextTalkSave;
BYTE sgbTalkSavePos;
BYTE *pDurIcons;
BYTE *pChrButtons;
BOOL drawhpflag;
BOOL dropGoldFlag;
BOOL panbtn[8];
BOOL chrbtn[4];
BYTE *pMultiBtns;
BYTE *pPanelButtons;
BYTE *pChrPanel;
BOOL lvlbtndown;
char sgszTalkSave[8][80];
int dropGoldValue;
BOOL drawmanaflag;
BOOL chrbtnactive;
char sgszTalkMsg[MAX_SEND_STR_LEN];
BYTE *pPanelText;
BYTE *pTalkBtns;
BOOL pstrjust[4];
int pnumlines;
BOOL pinfoflag;
BOOL talkbtndown[3];
spell_id pSpell;
char infoclr;
int sgbPlrTalkTbl;
BYTE *pGBoxBuff;
BYTE *pSBkBtnCel;
char tempstr[256];
BOOLEAN whisper[MAX_PLRS];
int sbooktab;
spell_type pSplType;
int initialDropGoldIndex;
BOOL talkflag;
BYTE *pSBkIconCels;
BOOL sbookflag;
BOOL chrflag;
BOOL drawbtnflag;
BYTE *pSpellBkCel;
char infostr[64];
int numpanbtns;
char panelstr[4][64];
BOOL panelflag;
BYTE SplTransTbl[256];
int initialDropGoldValue;
BYTE *pSpellCels;
BOOL panbtndown;
BOOL spselflag;

/** Map of hero class names */
const char *const ClassStrTbl[] = {
	"Warrior",
	"Rogue",
	"Sorcerer",
	"Monk",
	"Bard",
	"Barbarian",
};

/** Maps from font index to smaltext.cel frame number. */
const BYTE fontframe[128] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 54, 44, 57, 58, 56, 55, 47, 40, 41, 59, 39, 50, 37, 51, 52,
	36, 27, 28, 29, 30, 31, 32, 33, 34, 35, 48, 49, 60, 38, 61, 53,
	62, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 42, 63, 43, 64, 65,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 40, 66, 41, 67, 0
};

/**
 * Maps from smaltext.cel frame number to character width. Note, the
 * character width may be distinct from the frame width, which is 13 for every
 * smaltext.cel frame.
 */
const BYTE fontkern[68] = {
	8, 10, 7, 9, 8, 7, 6, 8, 8, 3,
	3, 8, 6, 11, 9, 10, 6, 9, 9, 6,
	9, 11, 10, 13, 10, 11, 7, 5, 7, 7,
	8, 7, 7, 7, 7, 7, 10, 4, 5, 6,
	3, 3, 4, 3, 6, 6, 3, 3, 3, 3,
	3, 2, 7, 6, 3, 10, 10, 6, 6, 7,
	4, 4, 9, 6, 6, 12, 3, 7
};
/**
 * Line start position for info box text when displaying 1, 2, 3, 4 and 5 lines respectivly
 */
const int lineOffsets[5][5] = {
	{ 82 },
	{ 70, 94 },
	{ 64, 82, 100 },
	{ 60, 75, 89, 104 },
	{ 58, 70, 82, 94, 105 },
};

/**
 * Maps ASCII character code to font index, as used by the
 * small, medium and large sized fonts; which corresponds to smaltext.cel,
 * medtexts.cel and bigtgold.cel respectively.
 */
const BYTE gbFontTransTbl[256] = {
	// clang-format off
	'\0', 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	' ',  '!',  '\"', '#',  '$',  '%',  '&',  '\'', '(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',
	'0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  ':',  ';',  '<',  '=',  '>',  '?',
	'@',  'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
	'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z',  '[',  '\\', ']',  '^',  '_',
	'`',  'a',  'b',  'c',  'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
	'p',  'q',  'r',  's',  't',  'u',  'v',  'w',  'x',  'y',  'z',  '{',  '|',  '}',  '~',  0x01,
	'C',  'u',  'e',  'a',  'a',  'a',  'a',  'c',  'e',  'e',  'e',  'i',  'i',  'i',  'A',  'A',
	'E',  'a',  'A',  'o',  'o',  'o',  'u',  'u',  'y',  'O',  'U',  'c',  'L',  'Y',  'P',  'f',
	'a',  'i',  'o',  'u',  'n',  'N',  'a',  'o',  '?',  0x01, 0x01, 0x01, 0x01, '!',  '<',  '>',
	'o',  '+',  '2',  '3',  '\'', 'u',  'P',  '.',  ',',  '1',  '0',  '>',  0x01, 0x01, 0x01, '?',
	'A',  'A',  'A',  'A',  'A',  'A',  'A',  'C',  'E',  'E',  'E',  'E',  'I',  'I',  'I',  'I',
	'D',  'N',  'O',  'O',  'O',  'O',  'O',  'X',  '0',  'U',  'U',  'U',  'U',  'Y',  'b',  'B',
	'a',  'a',  'a',  'a',  'a',  'a',  'a',  'c',  'e',  'e',  'e',  'e',  'i',  'i',  'i',  'i',
	'o',  'n',  'o',  'o',  'o',  'o',  'o',  '/',  '0',  'u',  'u',  'u',  'u',  'y',  'b',  'y',
	// clang-format on
};

/* data */

/** Maps from spell_id to spelicon.cel frame number. */
char SpellITbl[] = {
	27,
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	8,
	9,
	28,
	13,
	12,
	18,
	16,
	14,
	18,
	19,
	11,
	20,
	15,
	21,
	23,
	24,
	25,
	22,
	26,
	29,
	37,
	38,
	39,
	42,
	41,
	40,
	10,
	36,
	30,
	51,
	51,
	50,
	46,
	47,
	43,
	45,
	48,
	49,
	44,
	35,
	35,
	35,
	35,
	35,
};
/** Maps from panel_button_id to the position and dimensions of a panel button. */
int PanBtnPos[8][5] = {
	// clang-format off
	{   9,   9, 71, 19, TRUE  }, // char button
	{   9,  35, 71, 19, FALSE }, // quests button
	{   9,  75, 71, 19, TRUE  }, // map button
	{   9, 101, 71, 19, FALSE }, // menu button
	{ 560,   9, 71, 19, TRUE  }, // inv button
	{ 560,  35, 71, 19, FALSE }, // spells button
	{  87,  91, 33, 32, TRUE  }, // chat button
	{ 527,  91, 33, 32, TRUE  }, // friendly fire button
	// clang-format on
};
/** Maps from panel_button_id to hotkey name. */
const char *const PanBtnHotKey[8] = { "'c'", "'q'", "Tab", "Esc", "'i'", "'b'", "Enter", NULL };
/** Maps from panel_button_id to panel button description. */
const char *const PanBtnStr[8] = {
	"Character Information",
	"Quests log",
	"Automap",
	"Main Menu",
	"Inventory",
	"Spell book",
	"Send Message",
	"Player Attack"
};
/** Maps from attribute_id to the rectangle on screen used for attribute increment buttons. */
// TODO!
//RECT32 ChrBtnsRect[MAX_PLAYERS][4] = {
RECT32 ChrBtnsRect[4] = {
	{ 137, 138, 41, 22 },
	{ 137, 166, 41, 22 },
	{ 137, 195, 41, 22 },
	{ 137, 223, 41, 22 }
};

/** Maps from spellbook page number and position to spell_id. */
spell_id SpellPages[6][7] = {
	{ SPL_NULL, SPL_FIREBOLT, SPL_CBOLT, SPL_HBOLT, SPL_HEAL, SPL_HEALOTHER, SPL_FLAME },
	{ SPL_RESURRECT, SPL_FIREWALL, SPL_TELEKINESIS, SPL_LIGHTNING, SPL_TOWN, SPL_FLASH, SPL_STONE },
	{ SPL_RNDTELEPORT, SPL_MANASHIELD, SPL_ELEMENT, SPL_FIREBALL, SPL_WAVE, SPL_CHAIN, SPL_GUARDIAN },
	{ SPL_NOVA, SPL_GOLEM, SPL_TELEPORT, SPL_APOCA, SPL_BONESPIRIT, SPL_FLARE, SPL_ETHEREALIZE },
	{ SPL_LIGHTWALL, SPL_IMMOLAT, SPL_WARP, SPL_REFLECT, SPL_BERSERK, SPL_FIRERING, SPL_SEARCH },
	{ SPL_INVALID, SPL_INVALID, SPL_INVALID, SPL_INVALID, SPL_INVALID, SPL_INVALID, SPL_INVALID }
};

#define SPLICONLENGTH 56
#define SPLROWICONLS 10
#define SPLICONLAST (gbIsHellfire ? 52 : 43)

/**
 * Draw spell cell onto the given buffer.
 * @param out Output buffer
 * @param xp Buffer coordinate
 * @param yp Buffer coordinate
 * @param Trans Pointer to the cel buffer.
 * @param nCel Index of the cel frame to draw. 0 based.
 * @param w Width of the frame.
 */
static void DrawSpellCel(CelOutputBuffer out, int xp, int yp, BYTE *Trans, int nCel, int w)
{
	CelDrawLightTo(out, xp, yp, Trans, nCel, w, SplTransTbl);
}

void SetSpellTrans(char t)
{
	int i;

	if (t == RSPLTYPE_SKILL) {
		for (i = 0; i < 128; i++)
			SplTransTbl[i] = i;
	}
	for (i = 128; i < 256; i++)
		SplTransTbl[i] = i;
	SplTransTbl[255] = 0;

	switch (t) {
	case RSPLTYPE_SPELL:
		SplTransTbl[PAL8_YELLOW] = PAL16_BLUE + 1;
		SplTransTbl[PAL8_YELLOW + 1] = PAL16_BLUE + 3;
		SplTransTbl[PAL8_YELLOW + 2] = PAL16_BLUE + 5;
		for (i = PAL16_BLUE; i < PAL16_BLUE + 16; i++) {
			SplTransTbl[PAL16_BEIGE - PAL16_BLUE + i] = i;
			SplTransTbl[PAL16_YELLOW - PAL16_BLUE + i] = i;
			SplTransTbl[PAL16_ORANGE - PAL16_BLUE + i] = i;
		}
		break;
	case RSPLTYPE_SCROLL:
		SplTransTbl[PAL8_YELLOW] = PAL16_BEIGE + 1;
		SplTransTbl[PAL8_YELLOW + 1] = PAL16_BEIGE + 3;
		SplTransTbl[PAL8_YELLOW + 2] = PAL16_BEIGE + 5;
		for (i = PAL16_BEIGE; i < PAL16_BEIGE + 16; i++) {
			SplTransTbl[PAL16_YELLOW - PAL16_BEIGE + i] = i;
			SplTransTbl[PAL16_ORANGE - PAL16_BEIGE + i] = i;
		}
		break;
	case RSPLTYPE_CHARGES:
		SplTransTbl[PAL8_YELLOW] = PAL16_ORANGE + 1;
		SplTransTbl[PAL8_YELLOW + 1] = PAL16_ORANGE + 3;
		SplTransTbl[PAL8_YELLOW + 2] = PAL16_ORANGE + 5;
		for (i = PAL16_ORANGE; i < PAL16_ORANGE + 16; i++) {
			SplTransTbl[PAL16_BEIGE - PAL16_ORANGE + i] = i;
			SplTransTbl[PAL16_YELLOW - PAL16_ORANGE + i] = i;
		}
		break;
	case RSPLTYPE_INVALID:
		SplTransTbl[PAL8_YELLOW] = PAL16_GRAY + 1;
		SplTransTbl[PAL8_YELLOW + 1] = PAL16_GRAY + 3;
		SplTransTbl[PAL8_YELLOW + 2] = PAL16_GRAY + 5;
		for (i = PAL16_GRAY; i < PAL16_GRAY + 15; i++) {
			SplTransTbl[PAL16_BEIGE - PAL16_GRAY + i] = i;
			SplTransTbl[PAL16_YELLOW - PAL16_GRAY + i] = i;
			SplTransTbl[PAL16_ORANGE - PAL16_GRAY + i] = i;
		}
		SplTransTbl[PAL16_BEIGE + 15] = 0;
		SplTransTbl[PAL16_YELLOW + 15] = 0;
		SplTransTbl[PAL16_ORANGE + 15] = 0;
		break;
	}
}

/**
 * Sets the spell frame to draw and its position then draws it.
 */
static void DrawSpell(CelOutputBuffer out)
{
	char st;
	int spl, tlvl;

	spl = plr[myplr]._pRSpell;
	st = plr[myplr]._pRSplType;

	// BUGFIX: Move the next line into the if statement to avoid OOB (SPL_INVALID is -1) (fixed)
	if (st == RSPLTYPE_SPELL && spl != SPL_INVALID) {
		tlvl = plr[myplr]._pISplLvlAdd + plr[myplr]._pSplLvl[spl];
		if (!CheckSpell(myplr, spl, RSPLTYPE_SPELL, TRUE))
			st = RSPLTYPE_INVALID;
		if (tlvl <= 0)
			st = RSPLTYPE_INVALID;
	}
	if (currlevel == 0 && st != RSPLTYPE_INVALID && !spelldata[spl].sTownSpell)
		st = RSPLTYPE_INVALID;
	if (plr[myplr]._pRSpell < 0)
		st = RSPLTYPE_INVALID;
	SetSpellTrans(st);
	if (spl != SPL_INVALID)
		DrawSpellCel(out, PANEL_X + 565, PANEL_Y + 119, pSpellCels, SpellITbl[spl], SPLICONLENGTH);
	else
		DrawSpellCel(out, PANEL_X + 565, PANEL_Y + 119, pSpellCels, 27, SPLICONLENGTH);
}

void DrawSpellList(CelOutputBuffer out)
{
	int x, y, c, s, t, v, lx, ly, trans;
	Uint64 mask, spl;

	pSpell = SPL_INVALID;
	infostr[0] = '\0';
	x = PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS;
	y = PANEL_Y - 17;
	ClearPanel();

	for (Sint32 i = RSPLTYPE_SKILL; i < RSPLTYPE_INVALID; i++) {
		switch ((spell_type)i) {
		case RSPLTYPE_SKILL:
			SetSpellTrans(RSPLTYPE_SKILL);
			mask = plr[myplr]._pAblSpells;
			c = SPLICONLAST + 3;
			break;
		case RSPLTYPE_SPELL:
			mask = plr[myplr]._pMemSpells;
			c = SPLICONLAST + 4;
			break;
		case RSPLTYPE_SCROLL:
			SetSpellTrans(RSPLTYPE_SCROLL);
			mask = plr[myplr]._pScrlSpells;
			c = SPLICONLAST + 1;
			break;
		case RSPLTYPE_CHARGES:
			SetSpellTrans(RSPLTYPE_CHARGES);
			mask = plr[myplr]._pISpells;
			c = SPLICONLAST + 2;
			break;
		case RSPLTYPE_INVALID:
			break;
		}
		Sint32 j = SPL_FIREBOLT;
		for (spl = 1; j < MAX_SPELLS; spl <<= 1, j++) {
			if (!(mask & spl))
				continue;
			if (i == RSPLTYPE_SPELL) {
				s = plr[myplr]._pISplLvlAdd + plr[myplr]._pSplLvl[j];
				if (s < 0)
					s = 0;
				if (s > 0)
					trans = RSPLTYPE_SPELL;
				else
					trans = RSPLTYPE_INVALID;
				SetSpellTrans(trans);
			}
			if (currlevel == 0 && !spelldata[j].sTownSpell)
				SetSpellTrans(RSPLTYPE_INVALID);
			DrawSpellCel(out, x, y, pSpellCels, SpellITbl[j], SPLICONLENGTH);
			lx = x;
			ly = y - SPLICONLENGTH;
			if (MouseX >= lx && MouseX < lx + SPLICONLENGTH && MouseY >= ly && MouseY < ly + SPLICONLENGTH) {
				pSpell = (spell_id)j;
				pSplType = (spell_type)i;
				if (plr[myplr]._pClass == PC_MONK && j == SPL_SEARCH)
					pSplType = RSPLTYPE_SKILL;
				DrawSpellCel(out, x, y, pSpellCels, c, SPLICONLENGTH);
				switch (pSplType) {
				case RSPLTYPE_SKILL:
					sprintf(infostr, "%s Skill", spelldata[pSpell].sSkillText);
					break;
				case RSPLTYPE_SPELL:
					sprintf(infostr, "%s Spell", spelldata[pSpell].sNameText);
					if (pSpell == SPL_HBOLT) {
						sprintf(tempstr, "Damages undead only");
						AddPanelString(tempstr, TRUE);
					}
					if (s == 0)
						sprintf(tempstr, "Spell Level 0 - Unusable");
					else
						sprintf(tempstr, "Spell Level %i", s);
					AddPanelString(tempstr, TRUE);
					break;
				case RSPLTYPE_SCROLL:
					sprintf(infostr, "Scroll of %s", spelldata[pSpell].sNameText);
					v = 0;
					for (t = 0; t < plr[myplr]._pNumInv; t++) {
						if (!plr[myplr].InvList[t].isEmpty()
						    && (plr[myplr].InvList[t]._iMiscId == IMISC_SCROLL || plr[myplr].InvList[t]._iMiscId == IMISC_SCROLLT)
						    && plr[myplr].InvList[t]._iSpell == pSpell) {
							v++;
						}
					}
					for (t = 0; t < MAXBELTITEMS; t++) {
						if (!plr[myplr].SpdList[t].isEmpty()
						    && (plr[myplr].SpdList[t]._iMiscId == IMISC_SCROLL || plr[myplr].SpdList[t]._iMiscId == IMISC_SCROLLT)
						    && plr[myplr].SpdList[t]._iSpell == pSpell) {
							v++;
						}
					}
					if (v == 1)
						strcpy(tempstr, "1 Scroll");
					else
						sprintf(tempstr, "%i Scrolls", v);
					AddPanelString(tempstr, TRUE);
					break;
				case RSPLTYPE_CHARGES:
					sprintf(infostr, "Staff of %s", spelldata[pSpell].sNameText);
					if (plr[myplr].InvBody[INVLOC_HAND_LEFT]._iCharges == 1)
						strcpy(tempstr, "1 Charge");
					else
						sprintf(tempstr, "%i Charges", plr[myplr].InvBody[INVLOC_HAND_LEFT]._iCharges);
					AddPanelString(tempstr, TRUE);
					break;
				case RSPLTYPE_INVALID:
					break;
				}
				for (t = 0; t < 4; t++) {
					if (plr[myplr]._pSplHotKey[t] == pSpell && plr[myplr]._pSplTHotKey[t] == pSplType) {
						DrawSpellCel(out, x, y, pSpellCels, t + SPLICONLAST + 5, SPLICONLENGTH);
						sprintf(tempstr, "Spell Hotkey #F%i", t + 5);
						AddPanelString(tempstr, TRUE);
					}
				}
			}
			x -= SPLICONLENGTH;
			if (x == PANEL_X + 12 - SPLICONLENGTH) {
				x = PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS;
				y -= SPLICONLENGTH;
			}
		}
		if (mask != 0 && x != PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS)
			x -= SPLICONLENGTH;
		if (x == PANEL_X + 12 - SPLICONLENGTH) {
			x = PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS;
			y -= SPLICONLENGTH;
		}
	}
}

void SetSpell(int pnum)
{
	spselflag = FALSE;
	if (pSpell != SPL_INVALID) {
		ClearPanel();
		plr[pnum]._pRSpell = pSpell;
		plr[pnum]._pRSplType = pSplType;
		force_redraw = 255;
	}
}

void SetSpeedSpell(int pnum, int slot)
{
	if (pSpell != SPL_INVALID) {
		for (int i = 0; i < 4; ++i) {
			if (plr[pnum]._pSplHotKey[i] == pSpell && plr[myplr]._pSplTHotKey[i] == pSplType)
				plr[pnum]._pSplHotKey[i] = SPL_INVALID;
		}
		plr[pnum]._pSplHotKey[slot] = pSpell;
		plr[pnum]._pSplTHotKey[slot] = pSplType;
	}
}

void ToggleSpell(int pnum, int slot)
{
	Uint64 spells;

	if (plr[pnum]._pSplHotKey[slot] == SPL_INVALID) {
		return;
	}

	switch (plr[pnum]._pSplTHotKey[slot]) {
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
	case RSPLTYPE_INVALID:
		return;
	}

	if (spells & GetSpellBitmask(plr[pnum]._pSplHotKey[slot])) {
		plr[myplr]._pRSpell = plr[pnum]._pSplHotKey[slot];
		plr[myplr]._pRSplType = plr[pnum]._pSplTHotKey[slot];
		force_redraw = 255;
	}
}

void PrintChar(CelOutputBuffer out, int sx, int sy, int nCel, char col)
{
	int i;
	BYTE pix;
	BYTE tbl[256];

	switch (col) {
	case COL_WHITE:
		CelDrawTo(out, sx, sy, pPanelText, nCel, 13);
		return;
	case COL_BLUE:
		for (i = 0; i < 256; i++) {
			pix = i;
			if (pix > PAL16_GRAY + 13)
				pix = PAL16_BLUE + 15;
			else if (pix >= PAL16_GRAY)
				pix -= PAL16_GRAY - (PAL16_BLUE + 2);
			tbl[i] = pix;
		}
		break;
	case COL_RED:
		for (i = 0; i < 256; i++) {
			pix = i;
			if (pix >= PAL16_GRAY)
				pix -= PAL16_GRAY - PAL16_RED;
			tbl[i] = pix;
		}
		break;
	default:
		for (i = 0; i < 256; i++) {
			pix = i;
			if (pix >= PAL16_GRAY) {
				if (pix >= PAL16_GRAY + 14)
					pix = PAL16_YELLOW + 15;
				else
					pix -= PAL16_GRAY - (PAL16_YELLOW + 2);
			}
			tbl[i] = pix;
		}
		break;
	}
	CelDrawLightTo(out, sx, sy, pPanelText, nCel, 13, tbl);
}

void AddPanelString(const char *str, BOOL just)
{
	strcpy(panelstr[pnumlines], str);
	pstrjust[pnumlines] = just;

	if (pnumlines < 4)
		pnumlines++;
}

void ClearPanel()
{
	pnumlines = 0;
	pinfoflag = FALSE;
}

void DrawPanelBox(CelOutputBuffer out, int x, int y, int w, int h, int sx, int sy)
{
	const BYTE *src = pBtmBuff.at(x, y);
	BYTE *dst = out.at(sx, sy);

	for (int hgt = h; hgt; hgt--, src += pBtmBuff.pitch(), dst += out.pitch()) {
		memcpy(dst, src, w);
	}
}

/**
 * Draws a section of the empty flask cel on top of the panel to create the illusion
 * of the flask getting empty. This function takes a cel and draws a
 * horizontal stripe of height (max-min) onto the given buffer.
 * @param out Target buffer.
 * @param sx Buffer coordinate
 * @param sy Buffer coordinate
 * @param celBuf Buffer of the empty flask cel.
 * @param y0 Top of the flask cel section to draw.
 * @param y1 Bottom of the flask cel section to draw.
 */
static void DrawFlaskTop(CelOutputBuffer out, int sx, int sy, CelOutputBuffer celBuf, int y0, int y1)
{
	const BYTE *src = celBuf.at(0, y0);
	BYTE *dst = out.at(sx, sy);

	for (int h = y1 - y0; h != 0; --h, src += celBuf.pitch(), dst += out.pitch())
		memcpy(dst, src, celBuf.w());
}

/**
 * Draws the dome of the flask that protrudes above the panel top line.
 * It draws a rectangle of fixed width 59 and height 'h' from the source buffer
 * into the target buffer.
 * @param out The target buffer.
 * @param celBuf Buffer of the empty flask cel.
 * @param celX Source buffer start coordinate.
 * @param celY Source buffer start coordinate.
 * @param sx Target buffer coordinate.
 * @param sy Target buffer coordinate.
 * @param h How many lines of the source buffer that will be copied.
 */
static void DrawFlask(CelOutputBuffer out, CelOutputBuffer celBuf, int celX, int celY, int x, int y, int h)
{
	int wdt, hgt;
	const BYTE *src = celBuf.at(celX, celY);
	BYTE *dst = out.at(x, y);

	for (hgt = h; hgt; hgt--, src += celBuf.pitch() - 59, dst += out.pitch() - 59) {
		for (wdt = 59; wdt; wdt--) {
			if (*src)
				*dst = *src;
			src++;
			dst++;
		}
	}
}

void DrawLifeFlask(CelOutputBuffer out)
{
	double p;
	int filled;

	p = 0.0;
	if (plr[myplr]._pMaxHP > 0) {
		p = (double)plr[myplr]._pHitPoints / (double)plr[myplr]._pMaxHP * 80.0;
	}
	plr[myplr]._pHPPer = p;
	filled = plr[myplr]._pHPPer;

	if (filled > 80)
		filled = 80;

	filled = 80 - filled;
	if (filled > 11)
		filled = 11;
	filled += 2;

	DrawFlask(out, pLifeBuff, 13, 3, PANEL_LEFT + 109, PANEL_TOP - 13, filled);
	if (filled != 13)
		DrawFlask(out, pBtmBuff, 109, filled + 3, PANEL_LEFT + 109, PANEL_TOP - 13 + filled, 13 - filled);
}

void UpdateLifeFlask(CelOutputBuffer out)
{
	double p;
	int filled;

	p = 0.0;
	if (plr[myplr]._pMaxHP > 0) {
		p = (double)plr[myplr]._pHitPoints / (double)plr[myplr]._pMaxHP * 80.0;
	}
	filled = p;
	plr[myplr]._pHPPer = filled;

	if (filled > 69)
		filled = 69;
	else if (filled < 0)
		filled = 0;
	if (filled != 69)
		DrawFlaskTop(out, 96 + PANEL_X, PANEL_Y, pLifeBuff, 16, 85 - filled);
	if (filled != 0)
		DrawPanelBox(out, 96, 85 - filled, 88, filled, 96 + PANEL_X, PANEL_Y + 69 - filled);
}

void DrawManaFlask(CelOutputBuffer out)
{
	int filled = plr[myplr]._pManaPer;
	if (filled > 80)
		filled = 80;
	filled = 80 - filled;
	if (filled > 11)
		filled = 11;
	filled += 2;

	DrawFlask(out, pManaBuff, 13, 3, PANEL_LEFT + 475, PANEL_TOP - 13, filled);
	if (filled != 13)
		DrawFlask(out, pBtmBuff, 475, filled + 3, PANEL_LEFT + 475, PANEL_TOP - 13 + filled, 13 - filled);
}

void control_update_life_mana()
{
	int manaPer;
	int maxMana = plr[myplr]._pMaxMana;
	int mana = plr[myplr]._pMana;
	if (maxMana < 0)
		maxMana = 0;
	if (mana < 0)
		mana = 0;
	if (maxMana == 0)
		manaPer = 0;
	else
		manaPer = (double)mana / (double)maxMana * 80.0;
	plr[myplr]._pManaPer = manaPer;
	plr[myplr]._pHPPer = (double)plr[myplr]._pHitPoints / (double)plr[myplr]._pMaxHP * 80.0;
}

void UpdateManaFlask(CelOutputBuffer out)
{
	int filled;
	int maxMana = plr[myplr]._pMaxMana;
	int mana = plr[myplr]._pMana;
	if (maxMana < 0)
		maxMana = 0;
	if (mana < 0)
		mana = 0;

	if (maxMana == 0)
		filled = 0;
	else
		filled = (double)mana / (double)maxMana * 80.0;

	plr[myplr]._pManaPer = filled;

	if (filled > 69)
		filled = 69;
	if (filled != 69)
		DrawFlaskTop(out, PANEL_X + 464, PANEL_Y, pManaBuff, 16, 85 - filled);
	if (filled != 0)
		DrawPanelBox(out, 464, 85 - filled, 88, filled, PANEL_X + 464, PANEL_Y + 69 - filled);

	DrawSpell(out);
}

void InitControlPan()
{
	int i;
	pBtmBuff = CelOutputBuffer::Alloc(PANEL_WIDTH, (PANEL_HEIGHT + 16) * (gbIsMultiplayer ? 2 : 1));
	pManaBuff = CelOutputBuffer::Alloc(88, 88);
	pLifeBuff = CelOutputBuffer::Alloc(88, 88);

	pPanelText = LoadFileInMem("CtrlPan\\SmalText.CEL", NULL);
	pChrPanel = LoadFileInMem("Data\\Char.CEL", NULL);
	if (!gbIsHellfire)
		pSpellCels = LoadFileInMem("CtrlPan\\SpelIcon.CEL", NULL);
	else
		pSpellCels = LoadFileInMem("Data\\SpelIcon.CEL", NULL);
	SetSpellTrans(RSPLTYPE_SKILL);
	BYTE *pStatusPanel = LoadFileInMem("CtrlPan\\Panel8.CEL", NULL);
	CelDrawUnsafeTo(pBtmBuff, 0, (PANEL_HEIGHT + 16) - 1, pStatusPanel, 1, PANEL_WIDTH);
	MemFreeDbg(pStatusPanel);
	pStatusPanel = LoadFileInMem("CtrlPan\\P8Bulbs.CEL", NULL);
	CelDrawUnsafeTo(pLifeBuff, 0, 87, pStatusPanel, 1, 88);
	CelDrawUnsafeTo(pManaBuff, 0, 87, pStatusPanel, 2, 88);
	MemFreeDbg(pStatusPanel);
	talkflag = FALSE;
	if (gbIsMultiplayer) {
		BYTE *pTalkPanel = LoadFileInMem("CtrlPan\\TalkPanl.CEL", NULL);
		CelDrawUnsafeTo(pBtmBuff, 0, (PANEL_HEIGHT + 16) * 2 - 1, pTalkPanel, 1, PANEL_WIDTH);
		MemFreeDbg(pTalkPanel);
		pMultiBtns = LoadFileInMem("CtrlPan\\P8But2.CEL", NULL);
		pTalkBtns = LoadFileInMem("CtrlPan\\TalkButt.CEL", NULL);
		sgbPlrTalkTbl = 0;
		sgszTalkMsg[0] = '\0';
		for (i = 0; i < MAX_PLRS; i++)
			whisper[i] = TRUE;
		for (i = 0; i < sizeof(talkbtndown) / sizeof(talkbtndown[0]); i++)
			talkbtndown[i] = FALSE;
	}
	panelflag = FALSE;
	lvlbtndown = FALSE;
	pPanelButtons = LoadFileInMem("CtrlPan\\Panel8bu.CEL", NULL);
	for (i = 0; i < sizeof(panbtn) / sizeof(panbtn[0]); i++)
		panbtn[i] = FALSE;
	panbtndown = FALSE;
	if (!gbIsMultiplayer)
		numpanbtns = 6;
	else
		numpanbtns = 8;
	pChrButtons = LoadFileInMem("Data\\CharBut.CEL", NULL);
	for (i = 0; i < sizeof(chrbtn) / sizeof(chrbtn[0]); i++)
		chrbtn[i] = FALSE;
	chrbtnactive = FALSE;
	pDurIcons = LoadFileInMem("Items\\DurIcons.CEL", NULL);
	strcpy(infostr, "");
	ClearPanel();
	drawhpflag = TRUE;
	drawmanaflag = TRUE;
	chrflag = FALSE;
	spselflag = FALSE;
	pSpellBkCel = LoadFileInMem("Data\\SpellBk.CEL", NULL);
	pSBkBtnCel = LoadFileInMem("Data\\SpellBkB.CEL", NULL);
	pSBkIconCels = LoadFileInMem("Data\\SpellI2.CEL", NULL);
	sbooktab = 0;
	sbookflag = FALSE;
	if (plr[myplr]._pClass == PC_WARRIOR) {
		SpellPages[0][0] = SPL_REPAIR;
	} else if (plr[myplr]._pClass == PC_ROGUE) {
		SpellPages[0][0] = SPL_DISARM;
	} else if (plr[myplr]._pClass == PC_SORCERER) {
		SpellPages[0][0] = SPL_RECHARGE;
	} else if (plr[myplr]._pClass == PC_MONK) {
		SpellPages[0][0] = SPL_SEARCH;
	} else if (plr[myplr]._pClass == PC_BARD) {
		SpellPages[0][0] = SPL_IDENTIFY;
	} else if (plr[myplr]._pClass == PC_BARBARIAN) {
		SpellPages[0][0] = SPL_BLODBOIL;
	}
	pQLogCel = LoadFileInMem("Data\\Quest.CEL", NULL);
	pGBoxBuff = LoadFileInMem("CtrlPan\\Golddrop.cel", NULL);
	dropGoldFlag = FALSE;
	dropGoldValue = 0;
	initialDropGoldValue = 0;
	initialDropGoldIndex = 0;
}

void DrawCtrlPan(CelOutputBuffer out)
{
	DrawPanelBox(out, 0, sgbPlrTalkTbl + 16, PANEL_WIDTH, PANEL_HEIGHT, PANEL_X, PANEL_Y);
	DrawInfoBox(out);
}

void DrawCtrlBtns(CelOutputBuffer out)
{
	int i;

	for (i = 0; i < 6; i++) {
		if (!panbtn[i])
			DrawPanelBox(out, PanBtnPos[i][0], PanBtnPos[i][1] + 16, 71, 20, PanBtnPos[i][0] + PANEL_X, PanBtnPos[i][1] + PANEL_Y);
		else
			CelDrawTo(out, PanBtnPos[i][0] + PANEL_X, PanBtnPos[i][1] + PANEL_Y + 18, pPanelButtons, i + 1, 71);
	}
	if (numpanbtns == 8) {
		CelDrawTo(out, 87 + PANEL_X, 122 + PANEL_Y, pMultiBtns, panbtn[6] + 1, 33);
		if (gbFriendlyMode)
			CelDrawTo(out, 527 + PANEL_X, 122 + PANEL_Y, pMultiBtns, panbtn[7] + 3, 33);
		else
			CelDrawTo(out, 527 + PANEL_X, 122 + PANEL_Y, pMultiBtns, panbtn[7] + 5, 33);
	}
}

/**
 * Draws the "Speed Book": the rows of known spells for quick-setting a spell that
 * show up when you click the spell slot at the control panel.
 */
void DoSpeedBook(int pnum)
{
	int xo, yo, X, Y, i, j;

	spselflag = TRUE;
	xo = PANEL_X + 12 + SPLICONLENGTH * 10;
	yo = PANEL_Y - 17;
	X = xo + SPLICONLENGTH / 2;
	Y = yo - SPLICONLENGTH / 2;

	if (plr[pnum]._pRSpell != SPL_INVALID) {
		for (i = 0; i < 4; i++) {
			Uint64 spells;
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
			}
			Uint64 spell = 1;
			for (j = 1; j < MAX_SPELLS; j++) {
				if (spell & spells) {
					if (j == plr[pnum]._pRSpell && i == plr[pnum]._pRSplType) {
						X = xo + SPLICONLENGTH / 2;
						Y = yo - SPLICONLENGTH / 2;
					}
					xo -= SPLICONLENGTH;
					if (xo == PANEL_X + 12 - SPLICONLENGTH) {
						xo = PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS;
						yo -= SPLICONLENGTH;
					}
				}
				spell <<= 1ULL;
			}
			if (spells && xo != PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS)
				xo -= SPLICONLENGTH;
			if (xo == PANEL_X + 12 - SPLICONLENGTH) {
				xo = PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS;
				yo -= SPLICONLENGTH;
			}
		}
	}

	SetCursorPos(X, Y);
}

/**
 * Checks if the mouse cursor is within any of the panel buttons and flag it if so.
 */
void DoPanBtn()
{
	int i;

	for (i = 0; i < numpanbtns; i++) {
		int x = PanBtnPos[i][0] + PANEL_LEFT + PanBtnPos[i][2];
		int y = PanBtnPos[i][1] + PANEL_TOP + PanBtnPos[i][3];
		if (MouseX >= PanBtnPos[i][0] + PANEL_LEFT && MouseX <= x) {
			if (MouseY >= PanBtnPos[i][1] + PANEL_TOP && MouseY <= y) {
				panbtn[i] = TRUE;
				drawbtnflag = TRUE;
				panbtndown = TRUE;
			}
		}
	}
	if (!spselflag && MouseX >= 565 + PANEL_LEFT && MouseX < 621 + PANEL_LEFT && MouseY >= 64 + PANEL_TOP && MouseY < 120 + PANEL_TOP) {
		if (SDL_GetModState() & KMOD_SHIFT) {
			plr[myplr]._pRSpell = SPL_INVALID;
			plr[myplr]._pRSplType = RSPLTYPE_INVALID;
			force_redraw = 255;
			return;
		}
		DoSpeedBook(myplr);
		gamemenu_off();
	}
}

void control_set_button_down(int btn_id)
{
	panbtn[btn_id] = TRUE;
	drawbtnflag = TRUE;
	panbtndown = TRUE;
}

void control_check_btn_press()
{
	int x, y;

	x = PanBtnPos[3][0] + PANEL_LEFT + PanBtnPos[3][2];
	y = PanBtnPos[3][1] + PANEL_TOP + PanBtnPos[3][3];
	if (MouseX >= PanBtnPos[3][0] + PANEL_LEFT
	    && MouseX <= x
	    && MouseY >= PanBtnPos[3][1] + PANEL_TOP
	    && MouseY <= y) {
		control_set_button_down(3);
	}
	x = PanBtnPos[6][0] + PANEL_LEFT + PanBtnPos[6][2];
	y = PanBtnPos[6][1] + PANEL_TOP + PanBtnPos[6][3];
	if (MouseX >= PanBtnPos[6][0] + PANEL_LEFT
	    && MouseX <= x
	    && MouseY >= PanBtnPos[6][1] + PANEL_TOP
	    && MouseY <= y) {
		control_set_button_down(6);
	}
}

void DoAutoMap()
{
	if (currlevel != 0 || gbIsMultiplayer) {
		if (!automapflag)
			StartAutomap();
		else
			automapflag = FALSE;
	} else {
		InitDiabloMsg(EMSG_NO_AUTOMAP_IN_TOWN);
	}
}

/**
 * Checks the mouse cursor position within the control panel and sets information
 * strings if needed.
 */
void CheckPanelInfo()
{
	int i, c, s, xend, yend;

	panelflag = FALSE;
	ClearPanel();
	for (i = 0; i < numpanbtns; i++) {
		xend = PanBtnPos[i][0] + PANEL_LEFT + PanBtnPos[i][2];
		yend = PanBtnPos[i][1] + PANEL_TOP + PanBtnPos[i][3];
		if (MouseX >= PanBtnPos[i][0] + PANEL_LEFT && MouseX <= xend && MouseY >= PanBtnPos[i][1] + PANEL_TOP && MouseY <= yend) {
			if (i != 7) {
				strcpy(infostr, PanBtnStr[i]);
			} else {
				if (gbFriendlyMode)
					strcpy(infostr, "Player friendly");
				else
					strcpy(infostr, "Player attack");
			}
			if (PanBtnHotKey[i] != NULL) {
				sprintf(tempstr, "Hotkey: %s", PanBtnHotKey[i]);
				AddPanelString(tempstr, TRUE);
			}
			infoclr = COL_WHITE;
			panelflag = TRUE;
			pinfoflag = TRUE;
		}
	}
	if (!spselflag && MouseX >= 565 + PANEL_LEFT && MouseX < 621 + PANEL_LEFT && MouseY >= 64 + PANEL_TOP && MouseY < 120 + PANEL_TOP) {
		strcpy(infostr, "Select current spell button");
		infoclr = COL_WHITE;
		panelflag = TRUE;
		pinfoflag = TRUE;
		strcpy(tempstr, "Hotkey: 's'");
		AddPanelString(tempstr, TRUE);
		spell_id v = plr[myplr]._pRSpell;
		if (v != SPL_INVALID) {
			switch (plr[myplr]._pRSplType) {
			case RSPLTYPE_SKILL:
				sprintf(tempstr, "%s Skill", spelldata[v].sSkillText);
				AddPanelString(tempstr, TRUE);
				break;
			case RSPLTYPE_SPELL:
				sprintf(tempstr, "%s Spell", spelldata[v].sNameText);
				AddPanelString(tempstr, TRUE);
				c = plr[myplr]._pISplLvlAdd + plr[myplr]._pSplLvl[v];
				if (c < 0)
					c = 0;
				if (c == 0)
					sprintf(tempstr, "Spell Level 0 - Unusable");
				else
					sprintf(tempstr, "Spell Level %i", c);
				AddPanelString(tempstr, TRUE);
				break;
			case RSPLTYPE_SCROLL:
				sprintf(tempstr, "Scroll of %s", spelldata[v].sNameText);
				AddPanelString(tempstr, TRUE);
				s = 0;
				for (i = 0; i < plr[myplr]._pNumInv; i++) {
					if (!plr[myplr].InvList[i].isEmpty()
					    && (plr[myplr].InvList[i]._iMiscId == IMISC_SCROLL || plr[myplr].InvList[i]._iMiscId == IMISC_SCROLLT)
					    && plr[myplr].InvList[i]._iSpell == v) {
						s++;
					}
				}
				for (i = 0; i < MAXBELTITEMS; i++) {
					if (!plr[myplr].SpdList[i].isEmpty()
					    && (plr[myplr].SpdList[i]._iMiscId == IMISC_SCROLL || plr[myplr].SpdList[i]._iMiscId == IMISC_SCROLLT)
					    && plr[myplr].SpdList[i]._iSpell == v) {
						s++;
					}
				}
				if (s == 1)
					strcpy(tempstr, "1 Scroll");
				else
					sprintf(tempstr, "%i Scrolls", s);
				AddPanelString(tempstr, TRUE);
				break;
			case RSPLTYPE_CHARGES:
				sprintf(tempstr, "Staff of %s", spelldata[v].sNameText);
				AddPanelString(tempstr, TRUE);
				if (plr[myplr].InvBody[INVLOC_HAND_LEFT]._iCharges == 1)
					strcpy(tempstr, "1 Charge");
				else
					sprintf(tempstr, "%i Charges", plr[myplr].InvBody[INVLOC_HAND_LEFT]._iCharges);
				AddPanelString(tempstr, TRUE);
				break;
			case RSPLTYPE_INVALID:
				break;
			}
		}
	}
	if (MouseX > 190 + PANEL_LEFT && MouseX < 437 + PANEL_LEFT && MouseY > 4 + PANEL_TOP && MouseY < 33 + PANEL_TOP)
		pcursinvitem = CheckInvHLight();
}

/**
 * Check if the mouse is within a control panel button that's flagged.
 * Takes apropiate action if so.
 */
void CheckBtnUp()
{
	int i;
	BOOLEAN gamemenuOff;

	gamemenuOff = TRUE;
	drawbtnflag = TRUE;
	panbtndown = FALSE;

	for (i = 0; i < 8; i++) {
		if (!panbtn[i]) {
			continue;
		}

		panbtn[i] = FALSE;

		if (MouseX < PanBtnPos[i][0] + PANEL_LEFT
		    || MouseX > PanBtnPos[i][0] + PANEL_LEFT + PanBtnPos[i][2]
		    || MouseY < PanBtnPos[i][1] + PANEL_TOP
		    || MouseY > PanBtnPos[i][1] + PANEL_TOP + PanBtnPos[i][3]) {
			continue;
		}

		switch (i) {
		case PANBTN_CHARINFO:
			questlog = FALSE;
			chrflag = !chrflag;
			break;
		case PANBTN_QLOG:
			chrflag = FALSE;
			if (!questlog)
				StartQuestlog();
			else
				questlog = FALSE;
			break;
		case PANBTN_AUTOMAP:
			DoAutoMap();
			break;
		case PANBTN_MAINMENU:
			qtextflag = FALSE;
			gamemenu_handle_previous();
			gamemenuOff = FALSE;
			break;
		case PANBTN_INVENTORY:
			sbookflag = FALSE;
			invflag = !invflag;
			if (dropGoldFlag) {
				dropGoldFlag = FALSE;
				dropGoldValue = 0;
			}
			break;
		case PANBTN_SPELLBOOK:
			invflag = FALSE;
			if (dropGoldFlag) {
				dropGoldFlag = FALSE;
				dropGoldValue = 0;
			}
			sbookflag = !sbookflag;
			break;
		case PANBTN_SENDMSG:
			if (talkflag)
				control_reset_talk();
			else
				control_type_message();
			break;
		case PANBTN_FRIENDLY:
			gbFriendlyMode = !gbFriendlyMode;
			break;
		}
	}

	if (gamemenuOff)
		gamemenu_off();
}

void FreeControlPan()
{
	pBtmBuff.Free();
	pManaBuff.Free();
	pLifeBuff.Free();
	MemFreeDbg(pPanelText);
	MemFreeDbg(pChrPanel);
	MemFreeDbg(pSpellCels);
	MemFreeDbg(pPanelButtons);
	MemFreeDbg(pMultiBtns);
	MemFreeDbg(pTalkBtns);
	MemFreeDbg(pChrButtons);
	MemFreeDbg(pDurIcons);
	MemFreeDbg(pQLogCel);
	MemFreeDbg(pSpellBkCel);
	MemFreeDbg(pSBkBtnCel);
	MemFreeDbg(pSBkIconCels);
	MemFreeDbg(pGBoxBuff);
}

BOOL control_WriteStringToBuffer(BYTE *str)
{
	int k;
	BYTE ichar;

	k = 0;
	while (*str) {
		ichar = gbFontTransTbl[*str];
		str++;
		k += fontkern[fontframe[ichar]];
		if (k >= 125)
			return FALSE;
	}

	return TRUE;
}

static void CPrintString(CelOutputBuffer out, int y, const char *str, BOOL center, int lines)
{
	BYTE c;
	const char *tmp;
	int lineOffset, strWidth, sx, sy;

	lineOffset = 0;
	sx = 177 + PANEL_X;
	sy = lineOffsets[lines][y] + PANEL_Y;
	if (center == TRUE) {
		strWidth = 0;
		tmp = str;
		while (*tmp) {
			c = gbFontTransTbl[(BYTE)*tmp++];
			strWidth += fontkern[fontframe[c]] + 2;
		}
		if (strWidth < 288)
			lineOffset = (288 - strWidth) >> 1;
		sx += lineOffset;
	}
	while (*str) {
		c = gbFontTransTbl[(BYTE)*str++];
		c = fontframe[c];
		lineOffset += fontkern[c] + 2;
		if (c) {
			if (lineOffset < 288) {
				PrintChar(out, sx, sy, c, infoclr);
			}
		}
		sx += fontkern[c] + 2;
	}
}

static void PrintInfo(CelOutputBuffer out)
{
	int yo, lo, i;

	if (!talkflag) {
		yo = 0;
		lo = 1;
		if (infostr[0] != '\0') {
			CPrintString(out, 0, infostr, TRUE, pnumlines);
			yo = 1;
			lo = 0;
		}

		for (i = 0; i < pnumlines; i++) {
			CPrintString(out, i + yo, panelstr[i], pstrjust[i], pnumlines - lo);
		}
	}
}

void DrawInfoBox(CelOutputBuffer out)
{
	int nGold;

	DrawPanelBox(out, 177, 62, 288, 60, PANEL_X + 177, PANEL_Y + 46);
	if (!panelflag && !trigflag && pcursinvitem == -1 && !spselflag) {
		infostr[0] = '\0';
		infoclr = COL_WHITE;
		ClearPanel();
	}
	if (spselflag || trigflag) {
		infoclr = COL_WHITE;
	} else if (pcurs >= CURSOR_FIRSTITEM) {
		if (plr[myplr].HoldItem._itype == ITYPE_GOLD) {
			nGold = plr[myplr].HoldItem._ivalue;
			sprintf(infostr, "%i gold %s", nGold, get_pieces_str(nGold));
		} else if (!plr[myplr].HoldItem._iStatFlag) {
			ClearPanel();
			AddPanelString("Requirements not met", TRUE);
			pinfoflag = TRUE;
		} else {
			if (plr[myplr].HoldItem._iIdentified)
				strcpy(infostr, plr[myplr].HoldItem._iIName);
			else
				strcpy(infostr, plr[myplr].HoldItem._iName);
			if (plr[myplr].HoldItem._iMagical == ITEM_QUALITY_MAGIC)
				infoclr = COL_BLUE;
			if (plr[myplr].HoldItem._iMagical == ITEM_QUALITY_UNIQUE)
				infoclr = COL_GOLD;
		}
	} else {
		if (pcursitem != -1)
			GetItemStr(pcursitem);
		else if (pcursobj != -1)
			GetObjectStr(pcursobj);
		if (pcursmonst != -1) {
			if (leveltype != DTYPE_TOWN) {
				infoclr = COL_WHITE;
				strcpy(infostr, monster[pcursmonst].mName);
				ClearPanel();
				if (monster[pcursmonst]._uniqtype != 0) {
					infoclr = COL_GOLD;
					PrintUniqueHistory();
				} else {
					PrintMonstHistory(monster[pcursmonst].MType->mtype);
				}
			} else if (pcursitem == -1) {
				strcpy(infostr, towner[pcursmonst]._tName);
			}
		}
		if (pcursplr != -1) {
			infoclr = COL_GOLD;
			strcpy(infostr, plr[pcursplr]._pName);
			ClearPanel();
			sprintf(tempstr, "%s, Level: %i", ClassStrTbl[plr[pcursplr]._pClass], plr[pcursplr]._pLevel);
			AddPanelString(tempstr, TRUE);
			sprintf(tempstr, "Hit Points %i of %i", plr[pcursplr]._pHitPoints >> 6, plr[pcursplr]._pMaxHP >> 6);
			AddPanelString(tempstr, TRUE);
		}
	}
	if (infostr[0] != '\0' || pnumlines != 0)
		PrintInfo(out);
}

#define ADD_PlrStringXY(out, x, y, width, pszStr, col) MY_PlrStringXY(out, x, y, width, pszStr, col, 1)

void PrintGameStr(CelOutputBuffer out, int x, int y, const char *str, int color)
{
	while (*str) {
		BYTE c = gbFontTransTbl[(BYTE)*str++];
		c = fontframe[c];
		if (c)
			PrintChar(out, x, y, c, color);
		x += fontkern[c] + 1;
	}
}

/**
 * @brief Render text string to the given buffer
 * @param out Buffer to render to
 * @param x Screen coordinate
 * @param y Screen coordinate
 * @param endX End of line in screen coordinate
 * @param pszStr String to print, in Windows-1252 encoding
 * @param col text_color color value
 * @param base Letter spacing
 */
static void MY_PlrStringXY(CelOutputBuffer out, int x, int y, int endX, const char *pszStr, char col, int base)
{
	BYTE c;
	const char *tmp;
	int screen_x, line, widthOffset;

	widthOffset = endX - x + 1;
	line = 0;
	screen_x = 0;
	tmp = pszStr;
	while (*tmp) {
		c = gbFontTransTbl[(BYTE)*tmp++];
		screen_x += fontkern[fontframe[c]] + base;
	}
	if (screen_x < widthOffset)
		line = (widthOffset - screen_x) >> 1;
	x += line;
	while (*pszStr) {
		c = gbFontTransTbl[(BYTE)*pszStr++];
		c = fontframe[c];
		line += fontkern[c] + base;
		if (c) {
			if (line < widthOffset)
				PrintChar(out, x, y, c, col);
		}
		x += fontkern[c] + base;
	}
}

void DrawChr(CelOutputBuffer out)
{
	char col;
	char chrstr[64];
	int mindam, maxdam;

	CelDrawTo(out, 0, 351, pChrPanel, 1, SPANEL_WIDTH);
	ADD_PlrStringXY(out, 20, 32, 151, plr[myplr]._pName, COL_WHITE);

	ADD_PlrStringXY(out, 168, 32, 299, ClassStrTbl[plr[myplr]._pClass], COL_WHITE);

	sprintf(chrstr, "%i", plr[myplr]._pLevel);
	ADD_PlrStringXY(out, 66, 69, 109, chrstr, COL_WHITE);

	sprintf(chrstr, "%i", plr[myplr]._pExperience);
	ADD_PlrStringXY(out, 216, 69, 300, chrstr, COL_WHITE);

	if (plr[myplr]._pLevel == MAXCHARLEVEL - 1) {
		strcpy(chrstr, "None");
		col = COL_GOLD;
	} else {
		sprintf(chrstr, "%i", plr[myplr]._pNextExper);
		col = COL_WHITE;
	}
	ADD_PlrStringXY(out, 216, 97, 300, chrstr, col);

	sprintf(chrstr, "%i", plr[myplr]._pGold);
	ADD_PlrStringXY(out, 216, 146, 300, chrstr, COL_WHITE);

	col = COL_WHITE;
	if (plr[myplr]._pIBonusAC > 0)
		col = COL_BLUE;
	if (plr[myplr]._pIBonusAC < 0)
		col = COL_RED;
	sprintf(chrstr, "%i", plr[myplr]._pIBonusAC + plr[myplr]._pIAC + plr[myplr]._pDexterity / 5);
	ADD_PlrStringXY(out, 258, 183, 301, chrstr, col);

	col = COL_WHITE;
	if (plr[myplr]._pIBonusToHit > 0)
		col = COL_BLUE;
	if (plr[myplr]._pIBonusToHit < 0)
		col = COL_RED;
	sprintf(chrstr, "%i%%", (plr[myplr]._pDexterity >> 1) + plr[myplr]._pIBonusToHit + 50);
	ADD_PlrStringXY(out, 258, 211, 301, chrstr, col);

	col = COL_WHITE;
	if (plr[myplr]._pIBonusDam > 0)
		col = COL_BLUE;
	if (plr[myplr]._pIBonusDam < 0)
		col = COL_RED;
	mindam = plr[myplr]._pIMinDam;
	mindam += plr[myplr]._pIBonusDam * mindam / 100;
	mindam += plr[myplr]._pIBonusDamMod;
	if (plr[myplr].InvBody[INVLOC_HAND_LEFT]._itype == ITYPE_BOW) {
		if (plr[myplr]._pClass == PC_ROGUE)
			mindam += plr[myplr]._pDamageMod;
		else
			mindam += plr[myplr]._pDamageMod >> 1;
	} else {
		mindam += plr[myplr]._pDamageMod;
	}
	maxdam = plr[myplr]._pIMaxDam;
	maxdam += plr[myplr]._pIBonusDam * maxdam / 100;
	maxdam += plr[myplr]._pIBonusDamMod;
	if (plr[myplr].InvBody[INVLOC_HAND_LEFT]._itype == ITYPE_BOW) {
		if (plr[myplr]._pClass == PC_ROGUE)
			maxdam += plr[myplr]._pDamageMod;
		else
			maxdam += plr[myplr]._pDamageMod >> 1;
	} else {
		maxdam += plr[myplr]._pDamageMod;
	}
	sprintf(chrstr, "%i-%i", mindam, maxdam);
	if (mindam >= 100 || maxdam >= 100)
		MY_PlrStringXY(out, 254, 239, 305, chrstr, col, -1);
	else
		MY_PlrStringXY(out, 258, 239, 301, chrstr, col, 0);

	if (plr[myplr]._pMagResist == 0)
		col = COL_WHITE;
	else
		col = COL_BLUE;
	if (plr[myplr]._pMagResist < MAXRESIST) {
		sprintf(chrstr, "%i%%", plr[myplr]._pMagResist);
	} else {
		col = COL_GOLD;
		sprintf(chrstr, "MAX");
	}
	ADD_PlrStringXY(out, 257, 276, 300, chrstr, col);

	if (plr[myplr]._pFireResist == 0)
		col = COL_WHITE;
	else
		col = COL_BLUE;
	if (plr[myplr]._pFireResist < MAXRESIST) {
		sprintf(chrstr, "%i%%", plr[myplr]._pFireResist);
	} else {
		col = COL_GOLD;
		sprintf(chrstr, "MAX");
	}
	ADD_PlrStringXY(out, 257, 304, 300, chrstr, col);

	if (plr[myplr]._pLghtResist == 0)
		col = COL_WHITE;
	else
		col = COL_BLUE;
	if (plr[myplr]._pLghtResist < MAXRESIST) {
		sprintf(chrstr, "%i%%", plr[myplr]._pLghtResist);
	} else {
		col = COL_GOLD;
		sprintf(chrstr, "MAX");
	}
	ADD_PlrStringXY(out, 257, 332, 300, chrstr, col);

	col = COL_WHITE;
	sprintf(chrstr, "%i", plr[myplr]._pBaseStr);
	if (MaxStats[plr[myplr]._pClass][ATTRIB_STR] == plr[myplr]._pBaseStr)
		col = COL_GOLD;
	ADD_PlrStringXY(out, 95, 155, 126, chrstr, col);

	col = COL_WHITE;
	sprintf(chrstr, "%i", plr[myplr]._pBaseMag);
	if (MaxStats[plr[myplr]._pClass][ATTRIB_MAG] == plr[myplr]._pBaseMag)
		col = COL_GOLD;
	ADD_PlrStringXY(out, 95, 183, 126, chrstr, col);

	col = COL_WHITE;
	sprintf(chrstr, "%i", plr[myplr]._pBaseDex);
	if (MaxStats[plr[myplr]._pClass][ATTRIB_DEX] == plr[myplr]._pBaseDex)
		col = COL_GOLD;
	ADD_PlrStringXY(out, 95, 211, 126, chrstr, col);

	col = COL_WHITE;
	sprintf(chrstr, "%i", plr[myplr]._pBaseVit);
	if (MaxStats[plr[myplr]._pClass][ATTRIB_VIT] == plr[myplr]._pBaseVit)
		col = COL_GOLD;
	ADD_PlrStringXY(out, 95, 239, 126, chrstr, col);

	col = COL_WHITE;
	if (plr[myplr]._pStrength > plr[myplr]._pBaseStr)
		col = COL_BLUE;
	if (plr[myplr]._pStrength < plr[myplr]._pBaseStr)
		col = COL_RED;
	sprintf(chrstr, "%i", plr[myplr]._pStrength);
	ADD_PlrStringXY(out, 143, 155, 173, chrstr, col);

	col = COL_WHITE;
	if (plr[myplr]._pMagic > plr[myplr]._pBaseMag)
		col = COL_BLUE;
	if (plr[myplr]._pMagic < plr[myplr]._pBaseMag)
		col = COL_RED;
	sprintf(chrstr, "%i", plr[myplr]._pMagic);
	ADD_PlrStringXY(out, 143, 183, 173, chrstr, col);

	col = COL_WHITE;
	if (plr[myplr]._pDexterity > plr[myplr]._pBaseDex)
		col = COL_BLUE;
	if (plr[myplr]._pDexterity < plr[myplr]._pBaseDex)
		col = COL_RED;
	sprintf(chrstr, "%i", plr[myplr]._pDexterity);
	ADD_PlrStringXY(out, 143, 211, 173, chrstr, col);

	col = COL_WHITE;
	if (plr[myplr]._pVitality > plr[myplr]._pBaseVit)
		col = COL_BLUE;
	if (plr[myplr]._pVitality < plr[myplr]._pBaseVit)
		col = COL_RED;
	sprintf(chrstr, "%i", plr[myplr]._pVitality);
	ADD_PlrStringXY(out, 143, 239, 173, chrstr, col);

	if (plr[myplr]._pStatPts > 0) {
		if (CalcStatDiff(myplr) < plr[myplr]._pStatPts) {
			plr[myplr]._pStatPts = CalcStatDiff(myplr);
		}
	}
	if (plr[myplr]._pStatPts > 0) {
		sprintf(chrstr, "%i", plr[myplr]._pStatPts);
		ADD_PlrStringXY(out, 95, 266, 126, chrstr, COL_RED);
		plr_class pc = plr[myplr]._pClass;
		if (plr[myplr]._pBaseStr < MaxStats[pc][ATTRIB_STR])
			CelDrawTo(out, 137, 159, pChrButtons, chrbtn[ATTRIB_STR] + 2, 41);
		if (plr[myplr]._pBaseMag < MaxStats[pc][ATTRIB_MAG])
			CelDrawTo(out, 137, 187, pChrButtons, chrbtn[ATTRIB_MAG] + 4, 41);
		if (plr[myplr]._pBaseDex < MaxStats[pc][ATTRIB_DEX])
			CelDrawTo(out, 137, 216, pChrButtons, chrbtn[ATTRIB_DEX] + 6, 41);
		if (plr[myplr]._pBaseVit < MaxStats[pc][ATTRIB_VIT])
			CelDrawTo(out, 137, 244, pChrButtons, chrbtn[ATTRIB_VIT] + 8, 41);
	}

	if (plr[myplr]._pMaxHP > plr[myplr]._pMaxHPBase)
		col = COL_BLUE;
	else
		col = COL_WHITE;
	sprintf(chrstr, "%i", plr[myplr]._pMaxHP >> 6);
	ADD_PlrStringXY(out, 95, 304, 126, chrstr, col);
	if (plr[myplr]._pHitPoints != plr[myplr]._pMaxHP)
		col = COL_RED;
	sprintf(chrstr, "%i", plr[myplr]._pHitPoints >> 6);
	ADD_PlrStringXY(out, 143, 304, 174, chrstr, col);

	if (plr[myplr]._pMaxMana > plr[myplr]._pMaxManaBase)
		col = COL_BLUE;
	else
		col = COL_WHITE;
	sprintf(chrstr, "%i", plr[myplr]._pMaxMana >> 6);
	ADD_PlrStringXY(out, 95, 332, 126, chrstr, col);
	if (plr[myplr]._pMana != plr[myplr]._pMaxMana)
		col = COL_RED;
	sprintf(chrstr, "%i", plr[myplr]._pMana >> 6);
	ADD_PlrStringXY(out, 143, 332, 174, chrstr, col);
}

void CheckLvlBtn()
{
	if (!lvlbtndown && MouseX >= 40 + PANEL_LEFT && MouseX <= 81 + PANEL_LEFT && MouseY >= -39 + PANEL_TOP && MouseY <= -17 + PANEL_TOP)
		lvlbtndown = TRUE;
}

void ReleaseLvlBtn()
{
	if (MouseX >= 40 + PANEL_LEFT && MouseX <= 81 + PANEL_LEFT && MouseY >= -39 + PANEL_TOP && MouseY <= -17 + PANEL_TOP)
		chrflag = TRUE;
	lvlbtndown = FALSE;
}

void DrawLevelUpIcon(CelOutputBuffer out)
{
	int nCel;

	if (stextflag == STORE_NONE) {
		nCel = lvlbtndown ? 3 : 2;
		ADD_PlrStringXY(out, PANEL_LEFT + 0, PANEL_TOP - 49, PANEL_LEFT + 120, "Level Up", COL_WHITE);
		CelDrawTo(out, 40 + PANEL_X, -17 + PANEL_Y, pChrButtons, nCel, 41);
	}
}

void CheckChrBtns(int myplr)
{
	int i, x, y;

	if (!chrbtnactive && plr[myplr]._pStatPts) {
		plr_class pc = plr[myplr]._pClass;
		for (i = 0; i < 4; i++) {
			switch (i) {
			case ATTRIB_STR:
				if (plr[myplr]._pBaseStr >= MaxStats[pc][ATTRIB_STR])
					continue;
				break;
			case ATTRIB_MAG:
				if (plr[myplr]._pBaseMag >= MaxStats[pc][ATTRIB_MAG])
					continue;
				break;
			case ATTRIB_DEX:
				if (plr[myplr]._pBaseDex >= MaxStats[pc][ATTRIB_DEX])
					continue;
				break;
			case ATTRIB_VIT:
				if (plr[myplr]._pBaseVit >= MaxStats[pc][ATTRIB_VIT])
					continue;
				break;
			default:
				continue;
			}
			x = ChrBtnsRect[i].x + ChrBtnsRect[i].w;
			y = ChrBtnsRect[i].y + ChrBtnsRect[i].h;
			if (MouseX >= ChrBtnsRect[i].x
			    && MouseX <= x
			    && MouseY >= ChrBtnsRect[i].y
			    && MouseY <= y) {
				chrbtn[i] = TRUE;
				chrbtnactive = TRUE;
			}
		}
	}
}

void ReleaseChrBtns(int myplr, bool addAllStatPoints)
{
	int i;

	chrbtnactive = FALSE;
	for (i = 0; i < 4; ++i) {
		if (chrbtn[i]) {
			chrbtn[i] = FALSE;
			if (MouseX >= ChrBtnsRect[i].x
			    && MouseX <= ChrBtnsRect[i].x + ChrBtnsRect[i].w
			    && MouseY >= ChrBtnsRect[i].y
			    && MouseY <= ChrBtnsRect[i].y + ChrBtnsRect[i].h) {
				int statPointsToAdd = addAllStatPoints ? plr[myplr]._pStatPts : 1;
				switch (i) {
				case 0:
					NetSendCmdParam1(TRUE, CMD_ADDSTR, statPointsToAdd);
					plr[myplr]._pStatPts -= statPointsToAdd;
					break;
				case 1:
					NetSendCmdParam1(TRUE, CMD_ADDMAG, statPointsToAdd);
					plr[myplr]._pStatPts -= statPointsToAdd;
					break;
				case 2:
					NetSendCmdParam1(TRUE, CMD_ADDDEX, statPointsToAdd);
					plr[myplr]._pStatPts -= statPointsToAdd;
					break;
				case 3:
					NetSendCmdParam1(TRUE, CMD_ADDVIT, statPointsToAdd);
					plr[myplr]._pStatPts -= statPointsToAdd;
					break;
				}
			}
		}
	}
}

static int DrawDurIcon4Item(CelOutputBuffer out, ItemStruct *pItem, int x, int c)
{
	if (pItem->isEmpty())
		return x;
	if (pItem->_iDurability > 5)
		return x;
	if (c == 0) {
		switch (pItem->_itype) {
		case ITYPE_SWORD:
			c = 2;
			break;
		case ITYPE_AXE:
			c = 6;
			break;
		case ITYPE_BOW:
			c = 7;
			break;
		case ITYPE_MACE:
			c = 5;
			break;
		case ITYPE_STAFF:
			c = 8;
			break;
		default:
			c = 1;
			break;
		}
	}
	if (pItem->_iDurability > 2)
		c += 8;
	CelDrawTo(out, x, -17 + PANEL_Y, pDurIcons, c, 32);
	return x - 32 - 8;
}

void DrawDurIcon(CelOutputBuffer out)
{
	PlayerStruct *p;
	int x;

	bool hasRoomBetweenPanels = gnScreenWidth >= PANEL_WIDTH + 16 + (32 + 8 + 32 + 8 + 32 + 8 + 32) + 16;
	bool hasRoomUnderPanels = gnScreenHeight >= SPANEL_HEIGHT + PANEL_HEIGHT + 16 + 32 + 16;

	if (!hasRoomBetweenPanels && !hasRoomUnderPanels) {
		if ((chrflag || questlog) && (invflag || sbookflag))
			return;
	}

	x = PANEL_X + PANEL_WIDTH - 32 - 16;
	if (!hasRoomUnderPanels) {
		if (invflag || sbookflag)
			x -= SPANEL_WIDTH - (gnScreenWidth - PANEL_WIDTH) / 2;
	}

	p = &plr[myplr];
	x = DrawDurIcon4Item(out, &p->InvBody[INVLOC_HEAD], x, 4);
	x = DrawDurIcon4Item(out, &p->InvBody[INVLOC_CHEST], x, 3);
	x = DrawDurIcon4Item(out, &p->InvBody[INVLOC_HAND_LEFT], x, 0);
	DrawDurIcon4Item(out, &p->InvBody[INVLOC_HAND_RIGHT], x, 0);
}

void RedBack(CelOutputBuffer out)
{
	int idx;

	idx = light4flag ? 1536 : 4608;
	int w, h;
	BYTE *dst, *tbl;

	if (leveltype != DTYPE_HELL) {
		dst = out.begin();
		tbl = &pLightTbl[idx];
		for (h = gnViewportHeight; h; h--, dst += out.pitch() - gnScreenWidth) {
			for (w = gnScreenWidth; w; w--) {
				*dst = tbl[*dst];
				dst++;
			}
		}
	} else {
		dst = out.begin();
		tbl = &pLightTbl[idx];
		for (h = gnViewportHeight; h; h--, dst += out.pitch() - gnScreenWidth) {
			for (w = gnScreenWidth; w; w--) {
				if (*dst >= 32)
					*dst = tbl[*dst];
				dst++;
			}
		}
	}
}

static void PrintSBookStr(CelOutputBuffer out, int x, int y, BOOL cjustflag, const char *pszStr, char col)
{
	BYTE c;
	const char *tmp;
	int screen_x, line, sx;

	sx = x + RIGHT_PANEL_X + SPLICONLENGTH;
	line = 0;
	if (cjustflag) {
		screen_x = 0;
		tmp = pszStr;
		while (*tmp) {
			c = gbFontTransTbl[(BYTE)*tmp++];
			screen_x += fontkern[fontframe[c]] + 1;
		}
		if (screen_x < 222)
			line = (222 - screen_x) >> 1;
		sx += line;
	}
	while (*pszStr) {
		c = gbFontTransTbl[(BYTE)*pszStr++];
		c = fontframe[c];
		line += fontkern[c] + 1;
		if (c) {
			if (line <= 222)
				PrintChar(out, sx, y, c, col);
		}
		sx += fontkern[c] + 1;
	}
}

char GetSBookTrans(int ii, BOOL townok)
{
	char st;

	if ((plr[myplr]._pClass == PC_MONK) && (ii == SPL_SEARCH))
		return RSPLTYPE_SKILL;
	st = RSPLTYPE_SPELL;
	if (plr[myplr]._pISpells & GetSpellBitmask(ii)) {
		st = RSPLTYPE_CHARGES;
	}
	if (plr[myplr]._pAblSpells & GetSpellBitmask(ii)) {
		st = RSPLTYPE_SKILL;
	}
	if (st == RSPLTYPE_SPELL) {
		if (!CheckSpell(myplr, ii, RSPLTYPE_SPELL, TRUE)) {
			st = RSPLTYPE_INVALID;
		}
		if ((char)(plr[myplr]._pSplLvl[ii] + plr[myplr]._pISplLvlAdd) <= 0) {
			st = RSPLTYPE_INVALID;
		}
	}
	if (townok && currlevel == 0 && st != RSPLTYPE_INVALID && !spelldata[ii].sTownSpell) {
		st = RSPLTYPE_INVALID;
	}

	return st;
}

void DrawSpellBook(CelOutputBuffer out)
{
	int i, sn, mana, lvl, yp, min, max;
	char st;

	CelDrawTo(out, RIGHT_PANEL_X, 351, pSpellBkCel, 1, SPANEL_WIDTH);
	if (gbIsHellfire && sbooktab < 5) {
		CelDrawTo(out, RIGHT_PANEL_X + 61 * sbooktab + 7, 348, pSBkBtnCel, sbooktab + 1, 61);
	} else {
		// BUGFIX: rendering of page 3 and page 4 buttons are both off-by-one pixel (fixed).
		int sx = RIGHT_PANEL_X + 76 * sbooktab + 7;
		if (sbooktab == 2 || sbooktab == 3) {
			sx++;
		}
		CelDrawTo(out, sx, 348, pSBkBtnCel, sbooktab + 1, 76);
	}
	Uint64 spl = plr[myplr]._pMemSpells | plr[myplr]._pISpells | plr[myplr]._pAblSpells;

	yp = 55;
	for (i = 1; i < 8; i++) {
		sn = SpellPages[sbooktab][i - 1];
		if (sn != -1 && spl & GetSpellBitmask(sn)) {
			st = GetSBookTrans(sn, TRUE);
			SetSpellTrans(st);
			DrawSpellCel(out, RIGHT_PANEL_X + 11, yp, pSBkIconCels, SpellITbl[sn], 37);
			if (sn == plr[myplr]._pRSpell && st == plr[myplr]._pRSplType) {
				SetSpellTrans(RSPLTYPE_SKILL);
				DrawSpellCel(out, RIGHT_PANEL_X + 11, yp, pSBkIconCels, SPLICONLAST, 37);
			}
			PrintSBookStr(out, 10, yp - 23, FALSE, spelldata[sn].sNameText, COL_WHITE);
			switch (GetSBookTrans(sn, FALSE)) {
			case RSPLTYPE_SKILL:
				strcpy(tempstr, "Skill");
				break;
			case RSPLTYPE_CHARGES:
				sprintf(tempstr, "Staff (%i charges)", plr[myplr].InvBody[INVLOC_HAND_LEFT]._iCharges);
				break;
			default:
				mana = GetManaAmount(myplr, sn) >> 6;
				GetDamageAmt(sn, &min, &max);
				if (min != -1) {
					sprintf(tempstr, "Mana: %i  Dam: %i - %i", mana, min, max);
				} else {
					sprintf(tempstr, "Mana: %i   Dam: n/a", mana);
				}
				if (sn == SPL_BONESPIRIT) {
					sprintf(tempstr, "Mana: %i  Dam: 1/3 tgt hp", mana);
				}
				PrintSBookStr(out, 10, yp - 1, FALSE, tempstr, COL_WHITE);
				lvl = plr[myplr]._pSplLvl[sn] + plr[myplr]._pISplLvlAdd;
				if (lvl < 0) {
					lvl = 0;
				}
				if (lvl == 0) {
					sprintf(tempstr, "Spell Level 0 - Unusable");
				} else {
					sprintf(tempstr, "Spell Level %i", lvl);
				}
				break;
			}
			PrintSBookStr(out, 10, yp - 12, FALSE, tempstr, COL_WHITE);
		}
		yp += 43;
	}
}

void CheckSBook()
{
	if (MouseX >= RIGHT_PANEL + 11 && MouseX < RIGHT_PANEL + 48 && MouseY >= 18 && MouseY < 314) {
		spell_id sn = SpellPages[sbooktab][(MouseY - 18) / 43];
		Uint64 spl = plr[myplr]._pMemSpells | plr[myplr]._pISpells | plr[myplr]._pAblSpells;
		if (sn != SPL_INVALID && spl & GetSpellBitmask(sn)) {
			spell_type st = RSPLTYPE_SPELL;
			if (plr[myplr]._pISpells & GetSpellBitmask(sn)) {
				st = RSPLTYPE_CHARGES;
			}
			if (plr[myplr]._pAblSpells & GetSpellBitmask(sn)) {
				st = RSPLTYPE_SKILL;
			}
			plr[myplr]._pRSpell = sn;
			plr[myplr]._pRSplType = st;
			force_redraw = 255;
		}
	}
	if (MouseX >= RIGHT_PANEL + 7 && MouseX < RIGHT_PANEL + 311 && MouseY >= SPANEL_WIDTH && MouseY < 349) {
		sbooktab = (MouseX - (RIGHT_PANEL + 7)) / (gbIsHellfire ? 61 : 76);
	}
}

const char *get_pieces_str(int nGold)
{
	const char *result;

	result = "piece";
	if (nGold != 1)
		result = "pieces";
	return result;
}

void DrawGoldSplit(CelOutputBuffer out, int amount)
{
	int screen_x, i;

	screen_x = 0;
	CelDrawTo(out, 351, 178, pGBoxBuff, 1, 261);
	sprintf(tempstr, "You have %u gold", initialDropGoldValue);
	ADD_PlrStringXY(out, 366, 87, 600, tempstr, COL_GOLD);
	sprintf(tempstr, "%s.  How many do", get_pieces_str(initialDropGoldValue));
	ADD_PlrStringXY(out, 366, 103, 600, tempstr, COL_GOLD);
	ADD_PlrStringXY(out, 366, 121, 600, "you want to remove?", COL_GOLD);
	if (amount > 0) {
		sprintf(tempstr, "%u", amount);
		PrintGameStr(out, 388, 140, tempstr, COL_WHITE);
	}
	if (amount > 0) {
		for (i = 0; i < tempstr[i]; i++) {
			BYTE c = fontframe[gbFontTransTbl[(BYTE)tempstr[i]]];
			screen_x += fontkern[c] + 1;
		}
		screen_x += 452;
	} else {
		screen_x = 450;
	}
	CelDrawTo(out, screen_x, 140, pSPentSpn2Cels, PentSpn2Spin(), 12);
}

void control_drop_gold(char vkey)
{
	char input[6];

	if (plr[myplr]._pHitPoints >> 6 <= 0) {
		dropGoldFlag = FALSE;
		dropGoldValue = 0;
		return;
	}

	memset(input, 0, sizeof(input));
	snprintf(input, sizeof(input), "%d", dropGoldValue);
	if (vkey == DVL_VK_RETURN) {
		if (dropGoldValue > 0)
			control_remove_gold(myplr, initialDropGoldIndex);
		dropGoldFlag = FALSE;
	} else if (vkey == DVL_VK_ESCAPE) {
		dropGoldFlag = FALSE;
		dropGoldValue = 0;
	} else if (vkey == DVL_VK_BACK) {
		input[strlen(input) - 1] = '\0';
		dropGoldValue = atoi(input);
	} else if (vkey - '0' >= 0 && vkey - '0' <= 9) {
		if (dropGoldValue != 0 || atoi(input) <= initialDropGoldValue) {
			input[strlen(input)] = vkey;
			if (atoi(input) > initialDropGoldValue)
				return;
			if (strlen(input) > strlen(input))
				return;
		} else {
			input[0] = vkey;
		}
		dropGoldValue = atoi(input);
	}
}

void control_remove_gold(int pnum, int gold_index)
{
	int gi;

	if (gold_index <= INVITEM_INV_LAST) {
		gi = gold_index - INVITEM_INV_FIRST;
		plr[pnum].InvList[gi]._ivalue -= dropGoldValue;
		if (plr[pnum].InvList[gi]._ivalue > 0)
			SetGoldCurs(pnum, gi);
		else
			RemoveInvItem(pnum, gi);
	} else {
		gi = gold_index - INVITEM_BELT_FIRST;
		plr[pnum].SpdList[gi]._ivalue -= dropGoldValue;
		if (plr[pnum].SpdList[gi]._ivalue > 0)
			SetSpdbarGoldCurs(pnum, gi);
		else
			RemoveSpdBarItem(pnum, gi);
	}
	SetPlrHandItem(&plr[pnum].HoldItem, IDI_GOLD);
	GetGoldSeed(pnum, &plr[pnum].HoldItem);
	plr[pnum].HoldItem._ivalue = dropGoldValue;
	plr[pnum].HoldItem._iStatFlag = TRUE;
	control_set_gold_curs(pnum);
	plr[pnum]._pGold = CalculateGold(pnum);
	dropGoldValue = 0;
}

void control_set_gold_curs(int pnum)
{
	SetPlrHandGoldCurs(&plr[pnum].HoldItem);
	NewCursor(plr[pnum].HoldItem._iCurs + CURSOR_FIRSTITEM);
}

static char *control_print_talk_msg(CelOutputBuffer out, char *msg, int *x, int y, int color)
{
	BYTE c;
	int width;

	*x += 200;
	y += 22 + PANEL_Y;
	width = *x;
	while (*msg) {

		c = gbFontTransTbl[(BYTE)*msg];
		c = fontframe[c];
		width += fontkern[c] + 1;
		if (width > 450 + PANEL_X)
			return msg;
		msg++;
		if (c != 0) {
			PrintChar(out, *x, y, c, color);
		}
		*x += fontkern[c] + 1;
	}
	return NULL;
}

void DrawTalkPan(CelOutputBuffer out)
{
	int i, off, talk_btn, color, nCel, x;
	char *msg;

	if (!talkflag)
		return;

	DrawPanelBox(out, 175, sgbPlrTalkTbl + 20, 294, 5, PANEL_X + 175, PANEL_Y + 4);
	off = 0;
	for (i = 293; i > 283; off++, i--) {
		DrawPanelBox(out, (off >> 1) + 175, sgbPlrTalkTbl + off + 25, i, 1, (off >> 1) + PANEL_X + 175, off + PANEL_Y + 9);
	}
	DrawPanelBox(out, 185, sgbPlrTalkTbl + 35, 274, 30, PANEL_X + 185, PANEL_Y + 19);
	DrawPanelBox(out, 180, sgbPlrTalkTbl + 65, 284, 5, PANEL_X + 180, PANEL_Y + 49);
	for (i = 0; i < 10; i++) {
		DrawPanelBox(out, 180, sgbPlrTalkTbl + i + 70, i + 284, 1, PANEL_X + 180, i + PANEL_Y + 54);
	}
	DrawPanelBox(out, 170, sgbPlrTalkTbl + 80, 310, 55, PANEL_X + 170, PANEL_Y + 64);
	msg = sgszTalkMsg;
	for (i = 0; i < 39; i += 13) {
		x = 0 + PANEL_LEFT;
		msg = control_print_talk_msg(out, msg, &x, i, 0);
		if (!msg)
			break;
	}
	if (msg)
		*msg = '\0';
	CelDrawTo(out, x, i + 22 + PANEL_Y, pSPentSpn2Cels, PentSpn2Spin(), 12);
	talk_btn = 0;
	for (i = 0; i < 4; i++) {
		if (i == myplr)
			continue;
		if (whisper[i]) {
			color = COL_GOLD;
			if (talkbtndown[talk_btn]) {
				if (talk_btn != 0)
					nCel = 4;
				else
					nCel = 3;
				CelDrawTo(out, 172 + PANEL_X, 84 + 18 * talk_btn + PANEL_Y, pTalkBtns, nCel, 61);
			}
		} else {
			color = COL_RED;
			if (talk_btn != 0)
				nCel = 2;
			else
				nCel = 1;
			if (talkbtndown[talk_btn])
				nCel += 4;
			CelDrawTo(out, 172 + PANEL_X, 84 + 18 * talk_btn + PANEL_Y, pTalkBtns, nCel, 61);
		}
		if (plr[i].plractive) {
			x = 46 + PANEL_LEFT;
			control_print_talk_msg(out, plr[i]._pName, &x, 60 + talk_btn * 18, color);
		}

		talk_btn++;
	}
}

BOOL control_check_talk_btn()
{
	int i;

	if (!talkflag)
		return FALSE;

	if (MouseX < 172 + PANEL_LEFT)
		return FALSE;
	if (MouseY < 69 + PANEL_TOP)
		return FALSE;
	if (MouseX > 233 + PANEL_LEFT)
		return FALSE;
	if (MouseY > 123 + PANEL_TOP)
		return FALSE;

	for (i = 0; i < sizeof(talkbtndown) / sizeof(talkbtndown[0]); i++) {
		talkbtndown[i] = FALSE;
	}

	talkbtndown[(MouseY - (69 + PANEL_TOP)) / 18] = TRUE;

	return TRUE;
}

void control_release_talk_btn()
{
	int i, p, off;

	if (talkflag) {
		for (i = 0; i < sizeof(talkbtndown) / sizeof(talkbtndown[0]); i++)
			talkbtndown[i] = FALSE;
		if (MouseX >= 172 + PANEL_LEFT && MouseY >= 69 + PANEL_TOP && MouseX <= 233 + PANEL_LEFT && MouseY <= 123 + PANEL_TOP) {
			off = (MouseY - (69 + PANEL_TOP)) / 18;

			for (p = 0; p < MAX_PLRS && off != -1; p++) {
				if (p != myplr)
					off--;
			}
			if (p <= MAX_PLRS)
				whisper[p - 1] = !whisper[p - 1];
		}
	}
}

void control_reset_talk_msg(char *msg)
{
	int i, pmask;
	pmask = 0;

	for (i = 0; i < MAX_PLRS; i++) {
		if (whisper[i])
			pmask |= 1 << i;
	}
	NetSendCmdString(pmask, sgszTalkMsg);
}

void control_type_message()
{
	int i;

	if (!gbIsMultiplayer) {
		return;
	}

	talkflag = TRUE;
	sgszTalkMsg[0] = '\0';
	for (i = 0; i < 3; i++) {
		talkbtndown[i] = FALSE;
	}
	sgbPlrTalkTbl = PANEL_HEIGHT + 16;
	force_redraw = 255;
	sgbTalkSavePos = sgbNextTalkSave;
}

void control_reset_talk()
{
	talkflag = FALSE;
	sgbPlrTalkTbl = 0;
	force_redraw = 255;
}

static void control_press_enter()
{
	int i;
	BYTE talk_save;

	if (sgszTalkMsg[0] != 0) {
		control_reset_talk_msg(sgszTalkMsg);
		for (i = 0; i < 8; i++) {
			if (!strcmp(sgszTalkSave[i], sgszTalkMsg))
				break;
		}
		if (i >= 8) {
			strcpy(sgszTalkSave[sgbNextTalkSave], sgszTalkMsg);
			sgbNextTalkSave++;
			sgbNextTalkSave &= 7;
		} else {
			talk_save = sgbNextTalkSave - 1;
			talk_save &= 7;
			if (i != talk_save) {
				strcpy(sgszTalkSave[i], sgszTalkSave[talk_save]);
				strcpy(sgszTalkSave[talk_save], sgszTalkMsg);
			}
		}
		sgszTalkMsg[0] = '\0';
		sgbTalkSavePos = sgbNextTalkSave;
	}
	control_reset_talk();
}

BOOL control_talk_last_key(int vkey)
{
	int result;

	if (!gbIsMultiplayer)
		return FALSE;

	if (!talkflag)
		return FALSE;

	if ((DWORD)vkey < DVL_VK_SPACE)
		return FALSE;

	result = strlen(sgszTalkMsg);
	if (result < 78) {
		sgszTalkMsg[result] = vkey;
		sgszTalkMsg[result + 1] = '\0';
	}
	return TRUE;
}

static void control_up_down(int v)
{
	int i;

	for (i = 0; i < 8; i++) {
		sgbTalkSavePos = (v + sgbTalkSavePos) & 7;
		if (sgszTalkSave[sgbTalkSavePos][0]) {
			strcpy(sgszTalkMsg, sgszTalkSave[sgbTalkSavePos]);
			return;
		}
	}
}

BOOL control_presskeys(int vkey)
{
	int len;
	BOOL ret;

	if (gbIsMultiplayer) {
		if (!talkflag) {
			ret = FALSE;
		} else {
			if (vkey == DVL_VK_SPACE) {
			} else if (vkey == DVL_VK_ESCAPE) {
				control_reset_talk();
			} else if (vkey == DVL_VK_RETURN) {
				control_press_enter();
			} else if (vkey == DVL_VK_BACK) {
				len = strlen(sgszTalkMsg);
				if (len > 0)
					sgszTalkMsg[len - 1] = '\0';
			} else if (vkey == DVL_VK_DOWN) {
				control_up_down(1);
			} else if (vkey == DVL_VK_UP) {
				control_up_down(-1);
			} else {
				return FALSE;
			}
			ret = TRUE;
		}
	} else {
		ret = FALSE;
	}
	return ret;
}

DEVILUTION_END_NAMESPACE
