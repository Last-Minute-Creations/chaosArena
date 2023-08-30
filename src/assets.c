/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "assets.h"
#include <ace/macros.h>
#include "display.h"

tBitMap *g_pWarriorFrames;
tBitMap *g_pWarriorMasks;

tBitMap *g_pChaos;
tBitMap *g_pTileset;
tBitMap *g_pTilesetMask;

tFont *g_pFontBig;
tFont *g_pFontSmall;
tTextBitMap *g_pTextBitmap;

tBitMap *g_pCountdownMask;
tBitMap *g_pCountdownFrames;
tBitMap *g_pFightBitmap;
tBitMap *g_pFightMask;
tBitMap *g_pTitleBitmap;
tBitMap *g_pTitleMask;
tBitMap *g_pFramesThunder[2];
tBitMap *g_pFramesCross;

tPtplayerSfx *g_pSfxNo;
tPtplayerSfx *g_pSfxSwipes[2];
tPtplayerSfx *g_pSfxSwipeHit;
tPtplayerSfx *g_pSfxCrumble;
tPtplayerSfx *g_pSfxCountdown[3];
tPtplayerSfx *g_pSfxCountdownFight;
tPtplayerSfx *g_pSfxThunder;

tPtplayerMod *g_pModCombat;
tPtplayerMod *g_pModMenu;
tPtplayerSamplePack *g_pModSamples = 0;

void assetsGlobalCreate(void) {
	g_pWarriorFrames = bitmapCreateFromFile("data/warrior.bm", 0);
	g_pWarriorMasks = bitmapCreateFromFile("data/warrior_mask.bm", 0);
	g_pCountdownFrames = bitmapCreateFromFile("data/countdown.bm", 0);
	g_pCountdownMask = bitmapCreateFromFile("data/countdown_mask.bm", 0);
	g_pFightBitmap = bitmapCreateFromFile("data/fight.bm", 0);
	g_pFightMask = bitmapCreateFromFile("data/fight_mask.bm", 0);
	g_pTitleBitmap = bitmapCreateFromFile("data/title.bm", 0);
	g_pTitleMask = bitmapCreateFromFile("data/title_mask.bm", 0);

	g_pChaos = bitmapCreateFromFile("data/chaos.bm", 0);
	g_pTileset = bitmapCreateFromFile("data/tiles.bm", 0);
	g_pTilesetMask = bitmapCreateFromFile("data/tiles_mask.bm", 0);
	g_pFramesThunder[0] = bitmapCreateFromFile("data/thunder_0.bm", 0);
	g_pFramesThunder[1] = bitmapCreateFromFile("data/thunder_1.bm", 0);
	g_pFramesCross = bitmapCreateFromFile("data/cross.bm", 0);

	g_pFontBig = fontCreate("data/menu.fnt");
	g_pFontSmall = fontCreate("data/uni54.fnt");
	g_pTextBitmap = fontCreateTextBitMap(320, g_pFontBig->uwHeight);

	g_pSfxCrumble = ptplayerSfxCreateFromFile("data/crumble.sfx", 0);
	g_pSfxNo = ptplayerSfxCreateFromFile("data/noo.sfx", 0);
	g_pSfxSwipes[0] = ptplayerSfxCreateFromFile("data/swipe1.sfx", 0);
	g_pSfxSwipes[1] = ptplayerSfxCreateFromFile("data/swipe2.sfx", 0);
	g_pSfxSwipeHit = ptplayerSfxCreateFromFile("data/swipeHit.sfx", 0);
	g_pSfxCountdown[2] = ptplayerSfxCreateFromFile("data/cd3.sfx", 0);
	g_pSfxCountdown[1] = ptplayerSfxCreateFromFile("data/cd2.sfx", 0);
	g_pSfxCountdown[0] = ptplayerSfxCreateFromFile("data/cd1.sfx", 0);
	g_pSfxCountdownFight = ptplayerSfxCreateFromFile("data/cdfight.sfx", 0);
	g_pSfxThunder = ptplayerSfxCreateFromFile("data/thunder.sfx", 0);

	g_pModCombat = ptplayerModCreate("data/charena_game.mod");
	g_pModMenu = ptplayerModCreate("data/charena_menu.mod");

	g_pModSamples = ptplayerSamplePackCreate("data/samples.samplepack");
}

void assetsGlobalDestroy(void) {
	bitmapDestroy(g_pWarriorFrames);
	bitmapDestroy(g_pWarriorMasks);
	bitmapDestroy(g_pCountdownMask);
	bitmapDestroy(g_pCountdownFrames);
	bitmapDestroy(g_pFightBitmap);
	bitmapDestroy(g_pFightMask);
	bitmapDestroy(g_pTitleBitmap);
	bitmapDestroy(g_pTitleMask);

	bitmapDestroy(g_pChaos);
	bitmapDestroy(g_pTileset);
	bitmapDestroy(g_pTilesetMask);
	bitmapDestroy(g_pFramesThunder[0]);
	bitmapDestroy(g_pFramesThunder[1]);
	bitmapDestroy(g_pFramesCross);

	fontDestroy(g_pFontBig);
	fontDestroy(g_pFontSmall);
	fontDestroyTextBitMap(g_pTextBitmap);

	ptplayerSfxDestroy(g_pSfxCrumble);
	ptplayerSfxDestroy(g_pSfxNo);
	ptplayerSfxDestroy(g_pSfxSwipes[0]);
	ptplayerSfxDestroy(g_pSfxSwipes[1]);
	ptplayerSfxDestroy(g_pSfxSwipeHit);
	ptplayerSfxDestroy(g_pSfxCountdown[2]);
	ptplayerSfxDestroy(g_pSfxCountdown[1]);
	ptplayerSfxDestroy(g_pSfxCountdown[0]);
	ptplayerSfxDestroy(g_pSfxCountdownFight);
	ptplayerSfxDestroy(g_pSfxThunder);

	ptplayerModDestroy(g_pModCombat);
	ptplayerModDestroy(g_pModMenu);
	ptplayerSamplePackDestroy(g_pModSamples);
}
