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
#define BOB_OFFSET_X (WARRIOR_FRAME_WIDTH / 2)
#define BOB_OFFSET_Y (WARRIOR_FRAME_HEIGHT)

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

static UBYTE areWarriorsColliding(
	const tWarrior *pWarrior1, const tWarrior *pWarrior2
) {
	return (
		pWarrior1->sPos.uwX < pWarrior2->sPos.uwX + LOOKUP_TILE_SIZE &&
		pWarrior1->sPos.uwX + LOOKUP_TILE_SIZE > pWarrior2->sPos.uwX &&
		pWarrior1->sPos.uwY < pWarrior2->sPos.uwY + LOOKUP_TILE_SIZE &&
		pWarrior1->sPos.uwY + LOOKUP_TILE_SIZE > pWarrior2->sPos.uwY
	);
}

static inline tWarrior *warriorGetAtPos(
	UWORD uwPosX, BYTE bLookupAddX, UWORD uwPosY, BYTE bLookupAddY
) {
	UWORD uwLookupX = uwPosX / LOOKUP_TILE_SIZE + bLookupAddX;
	UWORD uwLookupY = uwPosY / LOOKUP_TILE_SIZE + bLookupAddY;
	return s_pWarriorLookup[uwLookupX][uwLookupY];
}

static void warriorTryMoveBy(tWarrior *pWarrior, BYTE bDeltaX, BYTE bDeltaY) {
	// TODO: this assumes that deltas are -1/1, it might not always be the case
	UBYTE ubOldLookupX = pWarrior->sPos.uwX / LOOKUP_TILE_SIZE;
	UBYTE ubOldLookupY = pWarrior->sPos.uwY / LOOKUP_TILE_SIZE;
	UBYTE isMoved = 0;

	if (bDeltaX) {
		UBYTE isColliding = 0;
		tUwCoordYX sOldPos = {.ulYX = pWarrior->sPos.ulYX};
		pWarrior->sPos.uwX += bDeltaX;

		// collision with upper corner
		tWarrior *pUp = warriorGetAtPos(sOldPos.uwX, bDeltaX, sOldPos.uwY, 0);
		if(pUp && pUp != pWarrior) {
			isColliding = areWarriorsColliding(pWarrior, pUp);
		}

		// collision with lower corner
		if (!isColliding && (sOldPos.uwY & (LOOKUP_TILE_SIZE - 1))) {
			tWarrior *pDown = warriorGetAtPos(sOldPos.uwX, bDeltaX, sOldPos.uwY, +1);
			if(pDown && pDown != pWarrior) {
				isColliding = areWarriorsColliding(pWarrior, pDown);
			}
		}

		if(!isColliding) {
			isMoved = 1;
		}
		else {
			pWarrior->sPos.ulYX = sOldPos.ulYX;
		}
	}

	if (bDeltaY) {
		UBYTE isColliding = 0;
		tUwCoordYX sOldPos = {.ulYX = pWarrior->sPos.ulYX};
		pWarrior->sPos.uwY += bDeltaY;

		// collision with left corner
		tWarrior *pLeft = warriorGetAtPos(sOldPos.uwX, 0, sOldPos.uwY, bDeltaY);
		if(pLeft && pLeft != pWarrior) {
			isColliding = areWarriorsColliding(pWarrior, pLeft);
		}

		// collision with right corner
		if (!isColliding && (sOldPos.uwX & (LOOKUP_TILE_SIZE - 1))) {
			tWarrior *pRight = warriorGetAtPos(sOldPos.uwX, +1, sOldPos.uwY, bDeltaY);
			if(pRight && pRight != pWarrior) {
				isColliding = areWarriorsColliding(pWarrior, pRight);
			}
		}

		if(!isColliding) {
			isMoved = 1;
		}
		else {
			pWarrior->sPos.ulYX = sOldPos.ulYX;
		}
	}

	if(isMoved) {
		// Update bob position
		pWarrior->sBob.sPos.uwX = pWarrior->sPos.uwX - BOB_OFFSET_X;
		pWarrior->sBob.sPos.uwY = pWarrior->sPos.uwY - BOB_OFFSET_Y;

		// Update lookup
		if(
			s_pWarriorLookup[ubOldLookupX][ubOldLookupY] &&
			s_pWarriorLookup[ubOldLookupX][ubOldLookupY] != pWarrior
		) {
			logWrite(
				"ERR: Erasing other warrior %p\n",
				s_pWarriorLookup[ubOldLookupX][ubOldLookupY]
			);
		}
		s_pWarriorLookup[ubOldLookupX][ubOldLookupY] = 0;

		UBYTE ubNewLookupX = pWarrior->sPos.uwX / LOOKUP_TILE_SIZE;
		UBYTE ubNewLookupY = pWarrior->sPos.uwY / LOOKUP_TILE_SIZE;
		if(s_pWarriorLookup[ubNewLookupX][ubNewLookupY]) {
			logWrite(
				"ERR: Overwriting other warrior %p in lookup with %p\n",
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
	pWarrior->sPos = (tUwCoordYX){.uwX = uwSpawnX, .uwY = uwSpawnY};
	bobNewInit(
		&pWarrior->sBob, WARRIOR_FRAME_WIDTH, WARRIOR_FRAME_HEIGHT, 1,
		g_pWarriorFrames->Planes[0], g_pWarriorMasks->Planes[0],
		pWarrior->sPos.uwX - BOB_OFFSET_X, pWarrior->sPos.uwY - BOB_OFFSET_Y
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
