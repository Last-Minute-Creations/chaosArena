/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "chaos_arena.h"
#include <ace/generic/main.h>
#include <ace/managers/key.h>
#include <ace/managers/joy.h>
#include <ace/managers/state.h>
#include <ace/managers/ptplayer.h>
#include "display.h"
#include "assets.h"
#include "game.h"
#include "tile.h"

tStateManager *g_pStateMachineGame;
tRandManager g_sRandManager;

void genericCreate(void) {
	g_pStateMachineGame = stateManagerCreate();
	keyCreate();
	joyOpen();
	ptplayerCreate(1);
	randInit(&g_sRandManager, 0x2184, 0x1911);

	assetsGlobalCreate();
	tilesInit();
	displayCreate();
	systemUnuse();

	displayOn();
	statePush(g_pStateMachineGame, &g_sStateGame);
}

void genericProcess(void) {
	ptplayerProcess();
	keyProcess();
	joyProcess();

	if (keyUse(KEY_ESCAPE)) {
		gameExit();
	}

	stateProcess(g_pStateMachineGame);
	displayProcess();
}

void genericDestroy(void) {
	displayOff();
	ptplayerStop();

	systemUse();
	displayDestroy();
	assetsGlobalDestroy();

	ptplayerDestroy();
	keyDestroy();
	joyClose();
	stateManagerDestroy(g_pStateMachineGame);
}
