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

// Must be power of 2!
#define LOOKUP_TILE_SIZE 8

#define LOOKUP_TILE_WIDTH (320 / LOOKUP_TILE_SIZE)
#define LOOKUP_TILE_HEIGHT (256 / LOOKUP_TILE_SIZE)

typedef struct tFrameOffsets {
	UBYTE *pBitmap;
	UBYTE *pMask;
} tFrameOffsets;

static tWarrior *s_pWarriorLookup[LOOKUP_TILE_WIDTH][LOOKUP_TILE_HEIGHT];

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

static void initFrameOffsets(void) {
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

static void resetWarriorLookup(void) {
	tWarrior **pBegin = &s_pWarriorLookup[0][0];
	tWarrior **pEnd = &s_pWarriorLookup[LOOKUP_TILE_WIDTH - 1][LOOKUP_TILE_HEIGHT - 1 + 1];
	for(tWarrior **pEntry = pBegin; pEntry != pEnd; ++pEntry) {
		*pEntry = 0;
	}
}

static void warriorTryMoveBy(tWarrior *pWarrior, BYTE bDeltaX, BYTE bDeltaY) {
	// TODO: this assumes that deltas are -1/1, it might not always be the case
	UBYTE ubOldLookupX = pWarrior->sBob.sPos.uwX / LOOKUP_TILE_SIZE;
	UBYTE ubOldLookupY = pWarrior->sBob.sPos.uwY / LOOKUP_TILE_SIZE;
	UBYTE isUpdateLookup = 0;

	if (bDeltaX) {
		UBYTE isColliding = 0;
		UWORD uwNewX = pWarrior->sBob.sPos.uwX + bDeltaX;

		// collision with upper corner
		tWarrior *pUp = s_pWarriorLookup[ubOldLookupX + bDeltaX][ubOldLookupY];
		if(pUp && pUp != pWarrior) {
			isColliding = (
				bDeltaX < 0 ?
				(pUp->sBob.sPos.uwX + LOOKUP_TILE_SIZE >= uwNewX) :
				(uwNewX + LOOKUP_TILE_SIZE >= pUp->sBob.sPos.uwX)
			);
		}

		// collision with lower corner
		if (!isColliding && (pWarrior->sBob.sPos.uwY & (LOOKUP_TILE_SIZE - 1))) {
			tWarrior *pUp = s_pWarriorLookup[ubOldLookupX + bDeltaX][ubOldLookupY + 1];
			if(pUp && pUp != pWarrior) {
				isColliding = (
					bDeltaX < 0 ?
					(pUp->sBob.sPos.uwX + LOOKUP_TILE_SIZE >= uwNewX) :
					(uwNewX + LOOKUP_TILE_SIZE >= pUp->sBob.sPos.uwX)
				);
			}
		}

		if(!isColliding) {
			pWarrior->sBob.sPos.uwX = uwNewX;
			isUpdateLookup = 1;
		}
	}

	if (bDeltaY) {
		UBYTE isColliding = 0;
		UWORD uwNewY = pWarrior->sBob.sPos.uwY + bDeltaY;

		// collision with left corner
		tWarrior *pLeft = s_pWarriorLookup[ubOldLookupX][ubOldLookupY + bDeltaY];
		if(pLeft && pLeft != pWarrior) {
			isColliding = (
				bDeltaY < 0 ?
				(pLeft->sBob.sPos.uwY + LOOKUP_TILE_SIZE >= uwNewY) :
				(uwNewY + LOOKUP_TILE_SIZE >= pLeft->sBob.sPos.uwY)
			);
		}

		// collision with right corner
		if (!isColliding && (pWarrior->sBob.sPos.uwX & (LOOKUP_TILE_SIZE - 1))) {
			tWarrior *pRight = s_pWarriorLookup[ubOldLookupX + 1][ubOldLookupY + bDeltaY];
			if(pRight && pRight != pWarrior) {
				isColliding = (
					bDeltaY < 0 ?
					(pRight->sBob.sPos.uwY + LOOKUP_TILE_SIZE >= uwNewY) :
					(uwNewY + LOOKUP_TILE_SIZE >= pRight->sBob.sPos.uwY)
				);
			}
		}

		if(!isColliding) {
			pWarrior->sBob.sPos.uwY = uwNewY;
			isUpdateLookup = 1;
		}
	}

	if(isUpdateLookup) {
		UBYTE ubNewLookupX = pWarrior->sBob.sPos.uwX / LOOKUP_TILE_SIZE;
		UBYTE ubNewLookupY = pWarrior->sBob.sPos.uwY / LOOKUP_TILE_SIZE;

		if(s_pWarriorLookup[ubOldLookupX][ubOldLookupY] != pWarrior) {

		}
		s_pWarriorLookup[ubOldLookupX][ubOldLookupY] = 0;

		if(s_pWarriorLookup[ubNewLookupX][ubNewLookupY]) {
			logWrite(
				"ERR: Overwriting other warrior %p in lookup with %p",
				s_pWarriorLookup[ubNewLookupX][ubNewLookupY], pWarrior
			);
		}
		s_pWarriorLookup[ubNewLookupX][ubNewLookupY] = pWarrior;
	}
}

void warriorDrawLookup(tBitMap *pBuffer) {
	for(UBYTE ubY = 0; ubY < LOOKUP_TILE_HEIGHT; ++ubY) {
		for(UBYTE ubX = 0; ubX < LOOKUP_TILE_WIDTH; ++ubX) {
			if (s_pWarriorLookup[ubX][ubY]) {
				blitRect(
					pBuffer, ubX * LOOKUP_TILE_SIZE, ubY * LOOKUP_TILE_SIZE,
					LOOKUP_TILE_SIZE, LOOKUP_TILE_SIZE, 6
				);
			}
		}
	}
}

void warriorInit(void) {
	initFrameOffsets();
	resetWarriorLookup();
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
	s_pWarriorLookup[uwSpawnX / LOOKUP_TILE_SIZE][uwSpawnY / LOOKUP_TILE_SIZE] = pWarrior;
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
			warriorTryMoveBy(pWarrior, bDeltaX, bDeltaY);
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
