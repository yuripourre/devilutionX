#pragma once
// Controller actions implementation

#include <stdint.h>

#include "all.h"

/*void WalkRight(int playerNumber);
void WalkLeft(int playerNumber);*/

namespace devilution {

void WalkDown(int playerNumber);
void WalkLeft(int playerNumber);
void WalkRight(int playerNumber);
void WalkUp(int playerNumber);

typedef enum belt_item_type : uint8_t {
	BLT_HEALING,
	BLT_MANA,
} belt_item_type;

// Runs every frame.
// Handles menu movement.
void plrctrls_every_frame();

// Run after every game logic iteration.
// Handles player movement.
void plrctrls_after_game_logic();

// Runs at the end of CheckCursMove()
// Handles item, object, and monster auto-aim.
void plrctrls_after_check_curs_move();

// Moves the map if active, the cursor otherwise.
void HandleRightStickMotion();

// Whether we're in a dialog menu that the game handles natively with keyboard controls.
bool InGameMenu();

// Whether the automap is being displayed.
bool IsAutomapActive();

// Whether the mouse cursor is being moved with the controller.
bool IsMovingMouseCursorWithController();

void UseBeltItem(int type);

// Talk to towners, click on inv items, attack, etc.
void PerformPrimaryAction();

// Open chests, doors, pickup items.
void PerformSecondaryAction();
bool TryDropItem();
void FocusOnInventory();
void PerformSpellAction();
void StoreSpellCoords();

typedef struct coords {
	int x;
	int y;
} coords;
extern int speedspellcount;

} // namespace devilution
