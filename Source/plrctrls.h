#pragma once
#ifndef __PLRCTRLS_H__
#define __PLRCTRLS_H__

void __fastcall checkTownersNearby(bool interact);
bool checkNearbyObjs(int x, int y, int diff);
void __fastcall checkItemsNearby(bool interact);
void __fastcall keyboardExpension();
void __fastcall checkMonstersNearby(bool attack);

#endif
