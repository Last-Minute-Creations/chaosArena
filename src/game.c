/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "game.h"
#include <ace/managers/key.h>
#include "bob_new.h"
#include "display.h"
#include "warrior.h"
#include "assets.h"

#define TILE_WIDTH 20
#define TILE_HEIGHT 16

typedef enum tTile {
	TILE_VOID,
	TILE_WALL,
	TILE_FLOOR,
	TILE_COUNT
} tTile;

static tSimpleBufferManager *s_pVpManager;
static UBYTE s_isDebug;

static const char s_pMapPattern[TILE_HEIGHT][TILE_WIDTH + 1] = {
	"....................",
	"....................",
	".##################.",
	".##################.",
	".####...####...####.",
	".####...####...####.",
	".####...####...####.",
	".##################.",
	".##################.",
	".####...####...####.",
	".####...####...####.",
	".####...####...####.",
	".##################.",
	".##################.",
	"....................",
	"...................."
};

static tTile s_pTiles[TILE_WIDTH][TILE_HEIGHT];

static void debugColor(UWORD uwColor) {
	if (s_isDebug) {
		g_pCustom->color[0] = uwColor;
	}
}

static void debugReset(void) {
	g_pCustom->color[0] = s_pVpManager->sCommon.pVPort->pPalette[0];
}

static void gameGsCreate(void) {
	s_pVpManager = displayGetManager();
	bobNewManagerCreate(
		s_pVpManager->pFront, s_pVpManager->pBack,
		s_pVpManager->sCommon.pVPort->uwHeight
	);

	warriorsCreate();
	bobNewReallocateBgBuffers();

	// Tiles
	for(UBYTE ubY = 0; ubY < TILE_HEIGHT; ++ubY) {
		for(UBYTE ubX = 0; ubX < TILE_WIDTH; ++ubX) {
			s_pTiles[ubX][ubY] = s_pMapPattern[ubY][ubX] == '#' ? TILE_FLOOR : TILE_VOID;
			if (ubY > 0 && s_pTiles[ubX][ubY] == TILE_VOID && s_pTiles[ubX][ubY - 1] == TILE_FLOOR) {
				s_pTiles[ubX][ubY] = TILE_WALL;
			}
			blitCopyAligned(
				g_pTileset, 0, s_pTiles[ubX][ubY] * 16,
				s_pVpManager->pBack, ubX * 16, ubY * 16, 16, 16
			);
			blitCopyAligned(
				g_pTileset, 0, s_pTiles[ubX][ubY] * 16,
				s_pVpManager->pFront, ubX * 16, ubY * 16, 16, 16
			);
		}
	}

	s_isDebug = 0;
}

static void gameGsLoop(void) {
	if (keyUse(KEY_C)) {
		s_isDebug = !s_isDebug;
	}

	debugColor(0xf00);
	bobNewBegin(s_pVpManager->pBack);

	debugColor(0x0f0);
	warriorsProcess();

	debugColor(0x00f);
	bobNewPushingDone();

	bobNewEnd();
	debugReset();
	// warriorDrawLookup(s_pVpManager->pBack);
}

static void gameGsDestroy(void) {
	bobNewManagerDestroy();
	warriorsDestroy();
}

tState g_sStateGame = {
	.cbCreate = gameGsCreate, .cbLoop = gameGsLoop, .cbDestroy = gameGsDestroy
};
