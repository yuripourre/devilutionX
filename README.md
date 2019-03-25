Adding keyboard movements and controller support.

## Roadmap

- ~~Part 1 : Impliment functions and map them to keyboard controls.~~
- Part 2 : Integrate XInput for controller support, and impliment our functions.
- Part 3 (OPTIONAL) : Local co-op for controllers. Up to 4 players.

## New Features

- Spacebar - Attack nearby enemies, talk to towns people and merchants.
- Enter - Pickup gold, potions & equipment from ground, open chests and doors that are nearby.
- Arrow Keys and WASD Keys - Move character in that direction. Has diagonals programmed.
- X Key - Cast spell at nearby enemies.
- Z Key - Hide/Back Out of menus.
- [ Key - Zoom in/out.
- Moved Debug Player key to O.
- Moved Speed Spell key to H.
- Moved debug_mode_key_w key to ;.
- Automap only moves when you hold down shift.
- Towns people, items and objects show information when nearby.
- Inventory snaps to grid system, use arrow/WASD keys to move around. Spacebar to pickup/drop items.
- Hotbook spells have snap grid system, use arrow/WASD keys to move around. Spacebar to select spell.
- Character info window level up attribute increase buttons now make cursor snap to them.

## How To Port

- 4 new files have been added that have the new code in them, plrctrls.cpp/.h and joystick.cpp/.h
- Any code that was altered in the other files were commented with `// JAKE`

## To-Do

- Make sure keys don't affect menus.
  - _Note:_ Did several menu checks already. More testing is needed.
- Make sure nearby enemies/objs/items work well together.
- Add XInput.
  - _Note:_ When you click the S key, it opens the spell window same as PS1 version.
