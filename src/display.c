/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "display.h"
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/utils/palette.h>

#define GAME_BPP 4
#define GAME_COLORS (1 << GAME_BPP)

static tView *s_pView;
static tVPort *s_pVp;
static tSimpleBufferManager *s_pVpManager;

void displayCreate(void) {
	s_pView = viewCreate(0,
		TAG_VIEW_COPLIST_MODE, VIEW_COPLIST_MODE_BLOCK,
		TAG_VIEW_GLOBAL_CLUT, 1,
	TAG_DONE);

	s_pVp = vPortCreate(0,
		TAG_VPORT_BPP, GAME_BPP,
		TAG_VPORT_VIEW, s_pView,
	TAG_DONE);

	s_pVpManager = simpleBufferCreate(0,
		TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
		TAG_SIMPLEBUFFER_IS_DBLBUF, 1,
		TAG_SIMPLEBUFFER_USE_X_SCROLLING, 0,
		TAG_SIMPLEBUFFER_VPORT, s_pVp,
	TAG_DONE);

	paletteLoad("data/palette.plt", s_pVp->pPalette, GAME_COLORS);
}

void displayDestroy(void) {
	viewDestroy(s_pView);
}

void displayProcess(void) {
	viewProcessManagers(s_pView);
	copProcessBlocks();
	vPortWaitForEnd(s_pVp);
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