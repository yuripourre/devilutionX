/**
 * @file defs.h
 *
 * Global definitions and Macros.
 */

#define DMAXX					40
#define DMAXY					40

#define LIGHTSIZE				6912 // 27 * 256
#define NO_LIGHT				-1

#define GMENU_SLIDER			0x40000000
#define GMENU_ENABLED			0x80000000

// must be unsigned to generate unsigned comparisons with pnum
#define MAX_PLRS				4

#define MAX_CHARACTERS			99
#define MAX_LVLS				24
#define MAX_LVLMTYPES			24
#define MAX_SPELLS				52
#define MAX_SPELL_LEVEL			15

#define MAX_CHUNKS				(MAX_LVLS + 5)

// #define MAX_PATH				260
#define MAX_SEND_STR_LEN		80

#define MAXDEAD					31
#define MAXDUNX					112
#define MAXDUNY					112
#define MAXITEMS				127
#define MAXBELTITEMS			8
#define MAXLIGHTS				32
#define MAXMISSILES				125
#define MAXMONSTERS				200
#define MAXOBJECTS				127
#define MAXPORTAL				4

#define MAXQUESTS				24
#define MAXMULTIQUESTS			10

#define MAXTHEMES				50
#define MAXTILES				2048

#define MAXTRIGGERS				7

#define MAXVISION				32
#define MDMAXX					40
#define MDMAXY					40
#define MAXCHARLEVEL			51
#define ITEMTYPES				43

// number of inventory grid cells
#define NUM_INV_GRID_ELEM		40
#define INV_SLOT_SIZE_PX		28

// Item indestructible durability
#define DUR_INDESTRUCTIBLE		255

#define VOLUME_MIN				-1600
#define VOLUME_MAX				0

#define NUM_TOWNERS				16

// todo: enums
#define NUMLEVELS				25
#define WITCH_ITEMS				50
#define SMITH_ITEMS				25
#define SMITH_PREMIUM_ITEMS		50
#define STORE_LINES				104

// from diablo 2 beta
#define MAXEXP					2000000000
#define MAXRESIST				75

#define GOLD_SMALL_LIMIT		1000
#define GOLD_MEDIUM_LIMIT		2500
#define GOLD_MAX_LIMIT			5000

#define PLR_NAME_LEN			32

#define MAXPATHNODES			300

#define MAX_PATH_LENGTH			25

// 256 kilobytes + 3 bytes (demo leftover) for file magic (262147)
// final game uses 4-byte magic instead of 3
#define FILEBUFF				((256 * 1024) + 3)

#define PMSG_COUNT				8

#define GAME_ID					(gbIsHellfire ? (gbIsSpawn ? LOAD_BE32("HSHR") : LOAD_BE32("HRTL")) : (gbIsSpawn ? LOAD_BE32("DSHR") : LOAD_BE32("DRTL")))

// Diablo uses a 256 color palette
// Entry 0-127 (0x00-0x7F) are level specific
// Entry 128-255 (0x80-0xFF) are global

// standard palette for all levels
// 8 or 16 shades per color
// example (dark blue): PAL16_BLUE+14, PAL8_BLUE+7
// example (light red): PAL16_RED+2, PAL8_RED
// example (orange): PAL16_ORANGE+8, PAL8_ORANGE+4
#define PAL8_BLUE		128
#define PAL8_RED		136
#define PAL8_YELLOW		144
#define PAL8_ORANGE		152
#define PAL16_BEIGE		160
#define PAL16_BLUE		176
#define PAL16_YELLOW	192
#define PAL16_ORANGE	208
#define PAL16_RED		224
#define PAL16_GRAY		240

// If defined, use 32-bit colors instead of 8-bit [Default -> Undefined]
//#define RGBMODE

#ifndef RGBMODE
#define SCREEN_BPP		8
#else
#define SCREEN_BPP		32
#endif

#define BUFFER_BORDER_LEFT		64
#define BUFFER_BORDER_TOP		160
#define BUFFER_BORDER_RIGHT	dvl::borderRight
#define BUFFER_BORDER_BOTTOM	16

#define UI_OFFSET_Y		((Sint16)((gnScreenHeight - 480) / 2))

#define TILE_WIDTH		64
#define TILE_HEIGHT		32

#define PANEL_WIDTH     640
#define PANEL_HEIGHT    128
#define PANEL_TOP		(gnScreenHeight - PANEL_HEIGHT)
#define PANEL_LEFT		(gnScreenWidth - PANEL_WIDTH) / 2
#define PANEL_X			PANEL_LEFT
#define PANEL_Y			PANEL_TOP

#define SPANEL_WIDTH	 320
#define SPANEL_HEIGHT	 352
#define PANELS_COVER (gnScreenWidth <= PANEL_WIDTH && gnScreenHeight <= SPANEL_HEIGHT + PANEL_HEIGHT)

#define RIGHT_PANEL		(gnScreenWidth - SPANEL_WIDTH)
#define RIGHT_PANEL_X	RIGHT_PANEL

#define DIALOG_TOP		((gnScreenHeight - PANEL_HEIGHT) / 2 - 18)
#define DIALOG_Y		DIALOG_TOP

#define NIGHTMARE_TO_HIT_BONUS  85
#define HELL_TO_HIT_BONUS      120

#define NIGHTMARE_AC_BONUS 50
#define HELL_AC_BONUS      80

#define MemFreeDbg(p)       \
	{                       \
		void *p__p;         \
		p__p = p;           \
		p = NULL;           \
		mem_free_dbg(p__p); \
	}

#undef assert

#ifndef _DEBUG
#define assert(exp)
#define assurance(exp, value) if (!(exp)) return
#define commitment(exp, value) if (!(exp)) return false
#else
#define assert(exp) (void)((exp) || (assert_fail(__LINE__, __FILE__, #exp), 0))
#define assurance(exp, value) (void)((exp) || (app_fatal("%s: %s was %i", __func__, #exp, value), 0))
#define commitment(exp, value) (void)((exp) || (app_fatal("%s: %s was %i", __func__, #exp, value), 0))
#endif

// To apply to certain functions which have local variables aligned by 1 for unknown yet reason
#if (_MSC_VER == 1200)
#define ALIGN_BY_1 __declspec(align(1))
#else
#define ALIGN_BY_1
#endif

#define SwapLE32 SDL_SwapLE32
#define SwapLE16 SDL_SwapLE16

#define ErrSdl() ErrDlg("SDL Error", SDL_GetError(), __FILE__, __LINE__)

#ifdef _MSC_VER
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

#ifdef __has_attribute
#define DVL_HAVE_ATTRIBUTE(x) __has_attribute(x)
#else
#define DVL_HAVE_ATTRIBUTE(x) 0
#endif

#if DVL_HAVE_ATTRIBUTE(always_inline) ||  (defined(__GNUC__) && !defined(__clang__))
#define DVL_ATTRIBUTE_ALWAYS_INLINE __attribute__((always_inline))
#else
#define DVL_ATTRIBUTE_ALWAYS_INLINE
#endif
