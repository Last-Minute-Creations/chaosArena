/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "display.h"
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/managers/system.h>
#include <ace/managers/sprite.h>
#include <ace/utils/palette.h>
#include "tile.h"
#include "debug.h"

#define GAME_COLORS (1 << DISPLAY_BPP)

static tView *s_pView;
static tVPort *s_pVp;
static tSimpleBufferManager *s_pVpManager;

void displayCreate(void) {
	UWORD uwDisplayCopperInstructions = simpleBufferGetRawCopperlistInstructionCount(DISPLAY_BPP);
	UWORD uwSpriteCopperInstructions = 8 * 2;
	s_pView = viewCreate(0,
		TAG_VIEW_COPLIST_MODE, VIEW_COPLIST_MODE_RAW,
		TAG_VIEW_COPLIST_RAW_COUNT, uwDisplayCopperInstructions + uwSpriteCopperInstructions + 2,
		TAG_VIEW_GLOBAL_PALETTE, 1,
	TAG_DONE);

	s_pVp = vPortCreate(0,
		TAG_VPORT_BPP, DISPLAY_BPP,
		TAG_VPORT_VIEW, s_pView,
	TAG_DONE);

	s_pVpManager = simpleBufferCreate(0,
		TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
		TAG_SIMPLEBUFFER_IS_DBLBUF, 1,
		TAG_SIMPLEBUFFER_USE_X_SCROLLING, 0,
		TAG_SIMPLEBUFFER_BOUND_WIDTH, DISPLAY_WIDTH,
		TAG_SIMPLEBUFFER_BOUND_HEIGHT, DISPLAY_HEIGHT,
		TAG_SIMPLEBUFFER_VPORT, s_pVp,
		TAG_SIMPLEBUFFER_COPLIST_OFFSET, uwSpriteCopperInstructions,
	TAG_DONE);

	cameraSetCoord(s_pVpManager->pCamera, DISPLAY_TILE_MARGIN * 16, DISPLAY_TILE_MARGIN * 16);

	paletteLoad("data/palette.plt", s_pVp->pPalette, GAME_COLORS);
	s_pVp->pPalette[16] = 0xF00;
	s_pVp->pPalette[17] = 0x0F0;
	s_pVp->pPalette[18] = 0xF0F;
	s_pVp->pPalette[19] = 0x0F0;
	tilesDrawAllOn(s_pVpManager->pBack);
	tilesDrawAllOn(s_pVpManager->pFront);
	debugInit(s_pVp->pPalette[0]);
	spriteManagerCreate(s_pView);
	copRawDisableSprites(
		s_pView->pCopList,
		SPRITE_0 | SPRITE_1 | SPRITE_2 | SPRITE_3 |
		SPRITE_4 | SPRITE_5 | SPRITE_6 | SPRITE_7, 0
	);
}

void displayDestroy(void) {
	spriteManagerDestroy();
	viewDestroy(s_pView);
}

void displayProcess(void) {
	viewProcessManagers(s_pView);
	copProcessBlocks();
	debugReset();
	systemIdleBegin();
	vPortWaitForEnd(s_pVp);
	systemIdleEnd();
}

void displayOn(void) {
	viewLoad(s_pView);
}

void displayOff(void) {
	viewLoad(0);
}

tSimpleBufferManager *displayGetManager(void) {
	return s_pVpManager;
}
