Adding keyboard movements and controller support.

## Current New Keyboard Functions
- Spacebar - Attack nearby enemies, talk to towns people and merchants.
- Enter - Pickup gold, potions & equipment from ground and open chests that are nearby.
- Arrow Keys and WASD Keys - Move character in that direction. Has diagonals programmed.
- Moved Debug Player key to O.
- Moved Speed Spell key to H.
- Automap only moves when you hold down shift.

## To-Do
- Make sure keys don't affect menus.
    - _Note:_ Did several menu checks already. More testing is needed.
- Add spell cast key. (maybe X key?)
    - _Note:_ Check ```void __fastcall CastSpell```
- When inventory is open, make cursor lock onto inventory and equipment grid.
    - Spacebar should emulate mouse clicks when inventory is open.
- When character levels up, and char info window is open, lock cursor onto [+] buttons.
    - Spacebar should emulate mouse clicks when this happens.
- Add XInput.
    - _Note:_ When you click the S key, it opens the spell window same as PS1 version.
