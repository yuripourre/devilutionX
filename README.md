Adding keyboard movements and controller support.

## Roadmap

- Part 1 : Impliment functions and map them to keyboard controls.
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
- Automap only moves when you hold down shift.
- Towns people, items and objects show information when nearby.

## To-Do

- Make sure keys don't affect menus.
  - _Note:_ Did several menu checks already. More testing is needed.
- When inventory is open, make cursor lock onto inventory and equipment grid.
  - Spacebar should emulate mouse clicks when inventory is open.
- When character levels up, and char info window is open, lock cursor onto [+] buttons.
  - Spacebar should emulate mouse clicks when this happens.
- Add XInput.
  - _Note:_ When you click the S key, it opens the spell window same as PS1 version.
