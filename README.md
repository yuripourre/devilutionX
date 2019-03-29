Adding keyboard movements and controller support.

![](https://newagesoldier.com/devilution/img/diablo_keyboard.jpg)

![](https://newagesoldier.com/devilution/img/diablo_controller.jpg)

## Roadmap

- ~~Part 1 : Impliment functions and map them to keyboard controls.~~
- ~~Part 2 : Integrate XInput for controller support, and impliment our functions.~~
- Part 3 (OPTIONAL) : Local co-op for controllers. Up to 4 players.

## New Keyboard Setup

- Spacebar - Attack nearby enemies, talk to towns people and merchants. Pickup & Drop items in inventory.
- Enter - Pickup gold, potions & equipment from ground, open chests and doors that are nearby.
- Arrow Keys and WASD Keys - Move character in that direction. Move cursor in inventory & speedbook.
- X Key - Cast spell or use skill. Back out of menus.
- Q Key - Use first health potion in belt.
- E Key - Use first mana potion in belt.
- Moved Speedbook key to H.

## New Functions

- Automap only moves when you hold down shift.
- Towns people, items and objects show information when nearby.
- Inventory snaps to grid system, use arrow/WASD keys to move around. Spacebar to pickup/drop items.
- Speedbook spells have snap grid system, use arrow/WASD keys to move around. Spacebar to select spell.
- Character info window level up attribute increase buttons now make cursor snap to them.
- Xbox One & 360 controller support added.

## Xbox Controller Button Layout

- **A Button:** Attack nearby enemies, talk to towns people and merchants, confirm menu clicks.
- **B Button:** Open inventory
- **X Button:** Pickup gold, potions & equipment from ground, open chests and doors that are nearby.
- **Y Button:** Cast spell or use skill. Back out of menus.
- **Back Button:** Open automap.
- **Start Button:** Open game menu.
- **Left Shoulder Button:** Open the speed spell book.
- **Right Shoulder Button:** Open character info window.
- **Direction Pad** & **Left Joystick:** Move character.
- **Right Joystick:** Move Cursor. Click for mouse left click.
- **Left Trigger:** Use first health potion in belt.
- **Right Trigger:** Use first mana potion in belt.

## How To Port To Your Mod

- 4 new files have been added that have the new code in them, plrctrls.cpp/.h and joystick.cpp/.h
- Any code that was altered in the other files were commented with `// JAKE`

## Support

If you find any bugs, please open an issue here on GitHub.
