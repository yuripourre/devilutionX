Adding keyboard movements and controller support.

## Current New Keyboard Functions
- Spacebar - Attack nearby enemies.
- Enter - Pickup gold, potions & equipment from ground and open chests that are nearby.
- Arrow Keys - Move character in that direction. Has diagonals programmed.

## To-Do
- Make sure keys don't affect menus.
- Move speedspell hotkey (S) to (V) key. Make W A S D movement keys as well.
- Add spell cast key. (maybe X key?)
    - _Note:_ Check ```void __fastcall CastSpell```
- If range weapon equiped, check further range for monsters
    - _Note:_ ```plr[myplr]._pwtype == WT_RANGED; plr[myplr]._pwtype = WT_MELEE;```
- Make talk to NPC on spacebar press.
    - _Note:_ Sometimes monsters might talk like towners ```CanTalkToMonst(pcursmonst)```
	- _Note2:_ Check if nearby towner via ```towner[i]._tx``` and ```towner[i]._ty```
	- _Note3:_ Maybe use function ```void __fastcall TalkToTowner``` ?
- When inventory is open, make cursor lock onto inventory and equipment grid.
    - Spacebar should emulate mouse clicks when inventory is open.
- When character levels up, and char info window is open, lock cursor onto [+] buttons.
    - Spacebar should emulate mouse clicks when this happens.
- Add XInput.
    - _Note:_ When you click the S key, it opens the spell window same as PS1 version.
