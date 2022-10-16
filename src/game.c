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
#include "tile.h"
#include "chaos_arena.h"

static tSimpleBufferManager *s_pVpManager;
static UBYTE s_isDebug;
static tBobNew s_sBobCountdown;
static tBobNew s_sBobFight;
static UBYTE s_ubCountdownPhase;
static UBYTE s_ubCountdownCooldown;

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
	bobNewManagerCreate(s_pVpManager->pBack, s_pVpManager->pFront, 512);
	warriorsCreate();

	UBYTE ubCountdownWidth = bitmapGetByteWidth(g_pCountdownFrames) * 8;
	UBYTE ubFightWidth = bitmapGetByteWidth(g_pFightFrames) * 8;
	bobNewInit(
		&s_sBobCountdown, ubCountdownWidth, g_pCountdownFrames->Rows / 3, 1,
		g_pCountdownFrames->Planes[0], g_pCountdownMask->Planes[0],
		(DISPLAY_WIDTH - ubCountdownWidth) / 2, 100
	);
	bobNewInit(
		&s_sBobFight, ubFightWidth, g_pFightFrames->Rows, 1,
		g_pFightFrames->Planes[0], g_pFightMask->Planes[0],
		(DISPLAY_WIDTH - ubFightWidth) / 2, 100
	);

	s_ubCountdownPhase = 5;
	s_ubCountdownCooldown = 1;

	bobNewReallocateBgBuffers();
	tilesDrawOn(s_pVpManager->pBack);
	tilesDrawOn(s_pVpManager->pFront);
	warriorsEnableMove(0);
	s_isDebug = 0;
}

static void countdownProcess(void) {
	if(!s_ubCountdownPhase) {
		return;
	}

	if(--s_ubCountdownCooldown == 0) {
		s_ubCountdownCooldown = 50;
		--s_ubCountdownPhase;
		if(s_ubCountdownPhase > 1) {
			UWORD uwBytesPerFrame = g_pCountdownFrames->BytesPerRow * s_sBobCountdown.uwHeight;
			ULONG ulFrameOffset = uwBytesPerFrame * (4 - s_ubCountdownPhase);
			bobNewSetFrame(
				&s_sBobCountdown, &g_pCountdownFrames->Planes[0][ulFrameOffset],
				&g_pCountdownMask->Planes[0][ulFrameOffset]
			);
		}
		else if(!s_ubCountdownPhase) {
			warriorsEnableMove(1);
		}
	}

	switch(s_ubCountdownPhase) {
		case 4:
		case 3:
		case 2:
			bobNewPush(&s_sBobCountdown);
			break;
		case 1:
			bobNewPush(&s_sBobFight);
			break;
	}
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
	countdownProcess();

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
