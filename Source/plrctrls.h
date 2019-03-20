#pragma once
#ifndef __PLRCTRLS_H__
#define __PLRCTRLS_H__

void __fastcall checkTownersNearby(bool interact);
int checkNearbyObjs(int x, int y, int diff);
void __fastcall checkItemsNearby(bool interact);
void __fastcall keyboardExpension();
bool __fastcall checkMonstersNearby(bool attack, bool castspell);
extern bool newCurHidden;

#endif
