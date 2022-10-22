/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INCLUDE_ASSETS_H
#define INCLUDE_ASSETS_H

#include <ace/utils/bitmap.h>
#include <ace/utils/font.h>
#include <ace/managers/ptplayer.h>

void assetsGlobalCreate(void);

void assetsGlobalDestroy(void);

extern tBitMap *g_pWarriorFrames;
extern tBitMap *g_pWarriorMasks;

extern tBitMap *g_pCountdownMask;
extern tBitMap *g_pCountdownFrames;
extern tBitMap *g_pFightFrames;
extern tBitMap *g_pFightMask;

extern tBitMap *g_pTileset;

extern tFont *g_pFontBig;
extern tFont *g_pFontSmall;
extern tTextBitMap *g_pTextBitmap;

extern tPtplayerSfx *g_pSfxNo;
extern tPtplayerSfx *g_pSfxSwipes[2];
extern tPtplayerSfx *g_pSfxSwipeHit;
extern tPtplayerSfx *g_pSfxCrumble;
extern tPtplayerSfx *g_pSfxCountdown[3]; // [0] is "one", [2] is "three"
extern tPtplayerSfx *g_pSfxCountdownFight;

extern tPtplayerMod *g_pModCombat;
extern tPtplayerMod *g_pModMenu;
extern UWORD *g_pModSamples;

#endif // INCLUDE_ASSETS_H
