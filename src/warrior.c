/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "warrior.h"
#include "assets.h"
#include "display.h"

#define WARRIOR_FRAME_WIDTH 16
#define WARRIOR_FRAME_HEIGHT 19
#define BYTES_PER_FRAME ((WARRIOR_FRAME_WIDTH / 8) * WARRIOR_FRAME_HEIGHT * GAME_BPP)
#define MAX_ANIM_FRAMES 4
#define FRAME_COOLDOWN 5
#define DIR_ID(dX, dY) ((dX + 1) | ((dY + 1) << 2))

typedef struct tFrameOffsets {
	UBYTE *pBitmap;
	UBYTE *pMask;
} tFrameOffsets;

static tFrameOffsets s_pFrameOffsets[ANIM_DIRECTION_COUNT][ANIM_COUNT][MAX_ANIM_FRAMES];

static const tAnimDirection s_pDirIdToAnimDir[] = {
	[DIR_ID(-1, -1)] = ANIM_DIRECTION_NW,
	[DIR_ID(-1,  0)] = ANIM_DIRECTION_W,
	[DIR_ID(-1,  1)] = ANIM_DIRECTION_SW,
	[DIR_ID( 0, -1)] = ANIM_DIRECTION_N,
	[DIR_ID( 0,  0)] = ANIM_DIRECTION_S, // whatever
	[DIR_ID( 0,  1)] = ANIM_DIRECTION_S,
	[DIR_ID( 1, -1)] = ANIM_DIRECTION_NE,
	[DIR_ID( 1,  0)] = ANIM_DIRECTION_E,
	[DIR_ID( 1,  1)] = ANIM_DIRECTION_SE,
};

static inline UBYTE getFrameCountForAnim(tAnim eAnim) {
	UBYTE ubCount = ((eAnim == ANIM_HURT || eAnim == ANIM_ATTACK) ? 4 : 2);
	return ubCount;
}

void warriorInit(void) {
	ULONG ulByteOffs = 0;
	for (tAnimDirection eDir = 0; eDir < ANIM_DIRECTION_COUNT; ++eDir) {
		for (tAnim eAnim = 0; eAnim < ANIM_COUNT; ++eAnim) {
			UBYTE ubFramesInAnim = getFrameCountForAnim(eAnim);
			for(UBYTE ubFrame = 0; ubFrame < ubFramesInAnim; ++ubFrame) {
				s_pFrameOffsets[eDir][eAnim][ubFrame] = (tFrameOffsets) {
					.pBitmap = &g_pWarriorFrames->Planes[0][ulByteOffs],
					.pMask = &g_pWarriorMasks->Planes[0][ulByteOffs]
				};
				ulByteOffs += BYTES_PER_FRAME;
			}
		}
	}
}

void warriorAdd(tWarrior *pWarrior, UWORD uwSpawnX, UWORD uwSpawnY, tSteer sSteer) {
	bobNewInit(
		&pWarrior->sBob, WARRIOR_FRAME_WIDTH, WARRIOR_FRAME_HEIGHT, 1,
		g_pWarriorFrames->Planes[0], g_pWarriorMasks->Planes[0],
		uwSpawnX, uwSpawnY
	);
	pWarrior->ubStunCooldown = 0;
	pWarrior->ubFrameCooldown = FRAME_COOLDOWN;
	pWarrior->eAnim = ANIM_IDLE;
	pWarrior->eDirection = ANIM_DIRECTION_S;
	pWarrior->sSteer = sSteer;
}

void warriorSetAnim(tWarrior *pWarrior, tAnim eAnim) {
	pWarrior->eAnim = eAnim;
	pWarrior->ubAnimFrame = 0;
	pWarrior->ubFrameCooldown = FRAME_COOLDOWN;
}

void warriorProcess(tWarrior *pWarrior) {
	steerProcess(&pWarrior->sSteer);
	BYTE bDeltaX = 0, bDeltaY = 0;
	if (steerDirCheck(&pWarrior->sSteer, DIRECTION_FIRE)) {
		pWarrior->eAnim = ANIM_ATTACK;
	}
	else {
		if (steerDirCheck(&pWarrior->sSteer, DIRECTION_UP)) {
			--bDeltaY;
		}
		if (steerDirCheck(&pWarrior->sSteer, DIRECTION_DOWN)) {
			++bDeltaY;
		}
		if (steerDirCheck(&pWarrior->sSteer, DIRECTION_LEFT)) {
			--bDeltaX;
		}
		if (steerDirCheck(&pWarrior->sSteer, DIRECTION_RIGHT)) {
			++bDeltaX;
		}
		UBYTE ubDirId = DIR_ID(bDeltaX, bDeltaY);
		if(ubDirId != DIR_ID(0, 0)) {
			pWarrior->eDirection = s_pDirIdToAnimDir[ubDirId];
			pWarrior->eAnim = ANIM_WALK;
			pWarrior->sBob.sPos.uwX += bDeltaX;
			pWarrior->sBob.sPos.uwY += bDeltaY;
			logWrite("delta: %hhd,%hhd, dir: %d\n", bDeltaX, bDeltaY, pWarrior->eDirection);
		}
		else {
			pWarrior->eAnim = ANIM_IDLE;
		}
	}

	if (!pWarrior->ubFrameCooldown) {
		if (++pWarrior->ubAnimFrame >= getFrameCountForAnim(pWarrior->eAnim)) {
			pWarrior->ubAnimFrame = 0;
		}
		pWarrior->ubFrameCooldown = FRAME_COOLDOWN;
		tFrameOffsets *pOffsets = &s_pFrameOffsets[pWarrior->eDirection][pWarrior->eAnim][pWarrior->ubAnimFrame];
		bobNewSetFrame(&pWarrior->sBob, pOffsets->pBitmap, pOffsets->pMask);
	}
	else {
		--pWarrior->ubFrameCooldown;
	}

	bobNewPush(&pWarrior->sBob);
}
