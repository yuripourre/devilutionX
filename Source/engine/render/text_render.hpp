/**
 * @file text_render.hpp
 *
 * Text rendering.
 */
#pragma once

#include <cstdint>

#include <SDL.h>

#include "DiabloUI/ui_item.h"
#include "engine.h"
#include "utils/stdcompat/optional.hpp"

namespace devilution {

enum GameFontTables : uint8_t {
	GameFontSmall,
	GameFontMed,
	GameFontBig,
};

extern std::optional<CelSprite> pSPentSpn2Cels;

void InitText();
void FreeText();

/**
 * @brief Calculate pixel width of first line of text, respecting kerning
 * @param text Text to check, will read until first eol or terminator
 * @param size Font size to use
 * @param spacing Extra spacing to add per character
 * @param charactersInLine Receives characters read until newline or terminator
 * @return Line width in pixels
 */
int GetLineWidth(const char *text, GameFontTables size = GameFontSmall, int spacing = 1, int *charactersInLine = nullptr);
void WordWrapGameString(char *text, size_t width, GameFontTables size = GameFontSmall, int spacing = 1);
int DrawString(const CelOutputBuffer &out, const char *text, const SDL_Rect &rect, uint16_t flags = 0, int spacing = 1, int lineHeight = -1, bool drawTextCursor = false);
int PentSpn2Spin();

} // namespace devilution
