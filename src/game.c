/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "game.h"
#include <ace/managers/key.h>
#include "bob_new.h"
#include "display.h"
#include "warrior.h"

#define WARRIOR_COUNT 3
#define WARRIORS_PER_ROW 8

static tSimpleBufferManager *s_pVpManager;
static tWarrior s_pWarriors[WARRIOR_COUNT];
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
	warriorInit();
	s_pVpManager = displayGetManager();
	bobNewManagerCreate(
		s_pVpManager->pFront, s_pVpManager->pBack,
		s_pVpManager->sCommon.pVPort->uwHeight
	);

	for(UBYTE i = 0; i < WARRIOR_COUNT; ++i) {
		warriorAdd(
			&s_pWarriors[i],
			16 + 32 * (i % WARRIORS_PER_ROW),
			16 + 32 * (i / WARRIORS_PER_ROW),
			i == 0 ? steerInitKey(KEYMAP_WSAD) : steerInitIdle()
		);
	}

	bobNewReallocateBgBuffers();

	s_isDebug = 0;
}

static void gameGsLoop(void) {
	if (keyUse(KEY_C)) {
		s_isDebug = !s_isDebug;
	}

	debugColor(0xf00);
	bobNewBegin(s_pVpManager->pBack);

	debugColor(0x0f0);
	for(UBYTE i = 0; i < WARRIOR_COUNT; ++i) {
		warriorProcess(&s_pWarriors[i]);
	}

	debugColor(0x00f);
	bobNewPushingDone();

	bobNewEnd();
	debugReset();
}

static void gameGsDestroy(void) {
	bobNewManagerDestroy();
}

tState g_sStateGame = {
	.cbCreate = gameGsCreate, .cbLoop = gameGsLoop, .cbDestroy = gameGsDestroy
};
