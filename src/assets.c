/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "assets.h"
#include <ace/macros.h>
#include "display.h"

tBitMap *g_pWarriorFrames;
tBitMap *g_pWarriorMasks;

tBitMap *g_pTileset;

tFont *g_pFontBig;
tFont *g_pFontSmall;
tTextBitMap *g_pTextBitmap;

tBitMap *g_pCountdownMask;
tBitMap *g_pCountdownFrames;
tBitMap *g_pFightFrames;
tBitMap *g_pFightMask;

tPtplayerSfx *g_pSfxNo;
tPtplayerSfx *g_pSfxSwipes[2];
tPtplayerSfx *g_pSfxSwipeHit;
tPtplayerSfx *g_pSfxCrumble;

tPtplayerMod *g_pModCombat;
UWORD *g_pModSamples = 0;
// static ULONG s_ulSampleSize;

void assetsGlobalCreate(void) {
	g_pWarriorFrames = bitmapCreateFromFile("data/warrior.bm", 0);
	g_pWarriorMasks = bitmapCreateFromFile("data/warrior_mask.bm", 0);
	g_pCountdownMask = bitmapCreateFromFile("data/countdown.bm", 0);
	g_pCountdownFrames = bitmapCreateFromFile("data/countdown_mask.bm", 0);
	g_pFightFrames = bitmapCreateFromFile("data/fight.bm", 0);
	g_pFightMask = bitmapCreateFromFile("data/fight_mask.bm", 0);

	g_pTileset = bitmapCreateFromFile("data/tiles.bm", 0);

	g_pFontBig = fontCreate("data/menu.fnt");
	g_pFontSmall = fontCreate("data/uni54.fnt");
	g_pTextBitmap = fontCreateTextBitMap(320, g_pFontBig->uwHeight);

	g_pSfxCrumble = ptplayerSfxCreateFromFile("data/crumble.sfx");
	g_pSfxNo = ptplayerSfxCreateFromFile("data/noo.sfx");
	g_pSfxSwipes[0] = ptplayerSfxCreateFromFile("data/swipe1.sfx");
	g_pSfxSwipes[1] = ptplayerSfxCreateFromFile("data/swipe2.sfx");
	g_pSfxSwipeHit = ptplayerSfxCreateFromFile("data/swipeHit.sfx");

	g_pModCombat = ptplayerModCreate("data/charena2.mod");

	// s_ulSampleSize = fileGetSize("data/samples.samplepack");
	// g_pModSamples = memAllocChip(s_ulSampleSize);
	// tFile *pFileSamples = fileOpen("data/samples.samplepack", "rb");
	// fileRead(pFileSamples, g_pModSamples, s_ulSampleSize);
	// fileClose(pFileSamples);
}

void assetsGlobalDestroy(void) {
	bitmapDestroy(g_pWarriorFrames);
	bitmapDestroy(g_pWarriorMasks);
	bitmapDestroy(g_pCountdownMask);
	bitmapDestroy(g_pCountdownFrames);
	bitmapDestroy(g_pFightFrames);
	bitmapDestroy(g_pFightMask);

	bitmapDestroy(g_pTileset);

	fontDestroy(g_pFontBig);
	fontDestroy(g_pFontSmall);
	fontDestroyTextBitMap(g_pTextBitmap);

	ptplayerSfxDestroy(g_pSfxCrumble);
	ptplayerSfxDestroy(g_pSfxNo);
	ptplayerSfxDestroy(g_pSfxSwipes[0]);
	ptplayerSfxDestroy(g_pSfxSwipes[1]);
	ptplayerSfxDestroy(g_pSfxSwipeHit);

	ptplayerModDestroy(g_pModCombat);
	// memFree(g_pModSamples, s_ulSampleSize);
}
