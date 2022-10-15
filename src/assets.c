/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "assets.h"

tBitMap *g_pWarriorFrames;
tBitMap *g_pWarriorMasks;
tBitMap *g_pTileset;
tFont *g_pFontMenu;
tTextBitMap *g_pTextBitmap;

void assetsGlobalCreate(void) {
	g_pWarriorFrames = bitmapCreateFromFile("data/warrior.bm", 0);
	g_pWarriorMasks = bitmapCreateFromFile("data/warrior_mask.bm", 0);
	g_pTileset = bitmapCreateFromFile("data/tiles.bm", 0);
	g_pFontMenu = fontCreate("data/menu.fnt");
	g_pTextBitmap = fontCreateTextBitMap(320, g_pFontMenu->uwHeight);
}

void assetsGlobalDestroy(void) {
	bitmapDestroy(g_pWarriorFrames);
	bitmapDestroy(g_pWarriorMasks);
	bitmapDestroy(g_pTileset);
	fontDestroy(g_pFontMenu);
	fontDestroyTextBitMap(g_pTextBitmap);
}
