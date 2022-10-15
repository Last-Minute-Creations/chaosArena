/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "assets.h"
#include <ace/macros.h>
#include "display.h"

tBitMap *g_pWarriorFrames;
tBitMap *g_pWarriorMasks;
tBitMap *g_pTileset;
tFont *g_pFontMenu;
tTextBitMap *g_pTextBitmap;
tBitMap *g_pCountdownMask;
tBitMap *g_pCountdownFrames;
tBitMap *g_pFightFrames;
tBitMap *g_pFightMask;

void assetsGlobalCreate(void) {
	g_pWarriorFrames = bitmapCreateFromFile("data/warrior.bm", 0);
	g_pWarriorMasks = bitmapCreateFromFile("data/warrior_mask.bm", 0);
	g_pCountdownMask = bitmapCreateFromFile("data/countdown.bm", 0);
	g_pCountdownFrames = bitmapCreateFromFile("data/countdown_mask.bm", 0);
	g_pFightFrames = bitmapCreateFromFile("data/fight.bm", 0);
	g_pFightMask = bitmapCreateFromFile("data/fight_mask.bm", 0);
	g_pTileset = bitmapCreateFromFile("data/tiles.bm", 0);
	g_pFontMenu = fontCreate("data/menu.fnt");
	g_pTextBitmap = fontCreateTextBitMap(320, g_pFontMenu->uwHeight);
}

void assetsGlobalDestroy(void) {
	bitmapDestroy(g_pWarriorFrames);
	bitmapDestroy(g_pWarriorMasks);
	bitmapDestroy(g_pCountdownMask);
	bitmapDestroy(g_pCountdownFrames);
	bitmapDestroy(g_pFightFrames);
	bitmapDestroy(g_pFightMask);
	bitmapDestroy(g_pTileset);
	fontDestroy(g_pFontMenu);
	fontDestroyTextBitMap(g_pTextBitmap);
}
