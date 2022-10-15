/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "game.h"
#include <ace/managers/key.h>
#include <ace/managers/game.h>
#include "bob_new.h"
#include "display.h"
#include "warrior.h"
#include "assets.h"
#include "chaos_arena.h"

static tSimpleBufferManager *s_pVpManager;
static UBYTE s_isDebug;

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
	bobNewManagerCreate(s_pVpManager->pFront, s_pVpManager->pBack, 512);

	warriorsCreate();
	bobNewReallocateBgBuffers();

	s_isDebug = 0;
}

static void gameGsLoop(void) {
	if(keyUse(KEY_ESCAPE)) {
		// Game canceled - go back to menu
		stateChange(g_pStateMachineGame, &g_sStateMenu);
		return;
	}

	UBYTE ubAlivePlayers = warriorsGetAlivePlayerCount();
	if(
		(warriorsGetAliveCount() == 1 && ubAlivePlayers == 1) ||
		ubAlivePlayers == 0
	) {
		// TODO: Summary
		stateChange(g_pStateMachineGame, &g_sStateMenu);
		return;
	}

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
	// warriorsDrawLookup(s_pVpManager->pBack);
}

static void gameGsDestroy(void) {
	bobNewManagerDestroy();
	warriorsDestroy();
}

tState g_sStateGame = {
	.cbCreate = gameGsCreate, .cbLoop = gameGsLoop, .cbDestroy = gameGsDestroy
};
