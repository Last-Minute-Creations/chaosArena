/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "game.h"
#include "bob_new.h"
#include "display.h"
#include "assets.h"

#define BOB_COUNT 16
#define BOBS_PER_ROW 8

static tSimpleBufferManager *s_pVpManager;
static tBobNew s_pBobs[BOB_COUNT];

static void gameGsCreate(void) {
	s_pVpManager = displayGetManager();
	bobNewManagerCreate(
		s_pVpManager->pFront, s_pVpManager->pBack,
		s_pVpManager->sCommon.pVPort->uwHeight
	);

	for(UBYTE i = 0; i < BOB_COUNT; ++i) {
		bobNewInit(
			&s_pBobs[i], 16, 19, 1,
			g_pWarriorFrames->Planes[0], g_pWarriorMask->Planes[0],
			16 + 32 * (i % BOBS_PER_ROW), 16 + 32 * (i / BOBS_PER_ROW)
		);
	}

	bobNewReallocateBgBuffers();
}

static void gameGsLoop(void) {
	g_pCustom->color[0] = 0xf00;
	bobNewBegin(s_pVpManager->pBack);

	g_pCustom->color[0] = 0x0f0;
	for(UBYTE i = 0; i < BOB_COUNT; ++i) {
		bobNewPush(&s_pBobs[i]);
	}

	g_pCustom->color[0] = 0x00f;
	bobNewPushingDone();

	bobNewEnd();
	g_pCustom->color[0] = 0x000;
}

static void gameGsDestroy(void) {
	bobNewManagerDestroy();
}

tState g_sStateGame = {
	.cbCreate = gameGsCreate, .cbLoop = gameGsLoop, .cbDestroy = gameGsDestroy
};
