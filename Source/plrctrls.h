#pragma once
#ifndef __PLRCTRLS_H__
#define __PLRCTRLS_H__

void __fastcall checkTownersNearby(bool interact);
int checkNearbyObjs(int x, int y, int diff);
void __fastcall checkItemsNearby(bool interact);
void __fastcall keyboardExpension();
bool __fastcall checkMonstersNearby(bool attack, bool castspell);
extern bool newCurHidden;
void invMove(int key);
extern const InvXY InvRect[73]; // wasn't made public, so I'll add this here from inv.cpp

#define INV_TOP 240;
#define INV_LEFT 350;
#define INV_HEIGHT 590;
#define DISPLAY_HEIGHT 360;
#define DISPLAY_WIDTH 640;

#endif
