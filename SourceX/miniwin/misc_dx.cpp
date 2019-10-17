#include "devilution.h"
#include "miniwin/ddraw.h"
#include "stubs.h"
#include <SDL.h>

namespace dvl {

WINBOOL SetCursorPos(int X, int Y)
{
#ifndef SWITCH
	assert(window);
	LogicalToOutput(&X, &Y);
	SDL_WarpMouseInWindow(window, X, Y);
#endif
	return true;
}

WINBOOL TextOutA(HDC hdc, int x, int y, LPCSTR lpString, int c)
{
	DUMMY_ONCE();

#ifndef SWITCH
	assert(window);
#ifdef USE_SDL1
	SDL_WM_SetCaption(lpString, WINDOW_ICON_NAME);
#else
	SDL_SetWindowTitle(window, lpString);
#endif
#endif

	return true;
}

} // namespace dvl
