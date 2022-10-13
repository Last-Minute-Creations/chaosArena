/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "warrior.h"
#include "assets.h"
#include "display.h"

//---------------------------------------------------------------------- DEFINES

#define WARRIOR_COUNT 3
#define WARRIORS_PER_ROW 8
#define WARRIOR_FRAME_WIDTH 16
#define WARRIOR_FRAME_HEIGHT 19
#define BYTES_PER_FRAME ((WARRIOR_FRAME_WIDTH / 8) * WARRIOR_FRAME_HEIGHT * GAME_BPP)
#define MAX_ANIM_FRAMES 4
#define FRAME_COOLDOWN 5
#define DIR_ID(dX, dY) ((dX + 1) | ((dY + 1) << 2))
#define BOB_OFFSET_X (WARRIOR_FRAME_WIDTH / 2)
#define BOB_OFFSET_Y (WARRIOR_FRAME_HEIGHT)
#define WARRIOR_PUSH_DELTA 2

// Must be power of 2!
#define LOOKUP_TILE_SIZE 8

#define LOOKUP_TILE_WIDTH (320 / LOOKUP_TILE_SIZE)
#define LOOKUP_TILE_HEIGHT (256 / LOOKUP_TILE_SIZE)

//------------------------------------------------------------------------ TYPES

typedef struct tFrameOffsets {
	UBYTE *pBitmap;
	UBYTE *pMask;
} tFrameOffsets;

//-------------------------------------------------------------- PRIVATE GLOBALS

static tWarrior *s_pWarriors[WARRIOR_COUNT];

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

static const tBCoordYX s_pAnimDirToAttackDelta[ANIM_DIRECTION_COUNT] = {
	[ANIM_DIRECTION_S] =  {.bX =                 0, .bY =  LOOKUP_TILE_SIZE},
	[ANIM_DIRECTION_SE] = {.bX =  LOOKUP_TILE_SIZE, .bY =  LOOKUP_TILE_SIZE},
	[ANIM_DIRECTION_E] =  {.bX =  LOOKUP_TILE_SIZE, .bY =                 0},
	[ANIM_DIRECTION_NE] = {.bX =  LOOKUP_TILE_SIZE, .bY = -LOOKUP_TILE_SIZE},
	[ANIM_DIRECTION_N] =  {.bX =                 0, .bY = -LOOKUP_TILE_SIZE},
	[ANIM_DIRECTION_NW] = {.bX = -LOOKUP_TILE_SIZE, .bY = -LOOKUP_TILE_SIZE},
	[ANIM_DIRECTION_W] =  {.bX = -LOOKUP_TILE_SIZE, .bY =                 0},
	[ANIM_DIRECTION_SW] = {.bX = -LOOKUP_TILE_SIZE, .bY = LOOKUP_TILE_SIZE},
};

static const tBCoordYX s_pAnimDirToPushDelta[ANIM_DIRECTION_COUNT] = {
	[ANIM_DIRECTION_S] =  {.bX =                   0, .bY =  WARRIOR_PUSH_DELTA},
	[ANIM_DIRECTION_SE] = {.bX =  WARRIOR_PUSH_DELTA, .bY =  WARRIOR_PUSH_DELTA},
	[ANIM_DIRECTION_E] =  {.bX =  WARRIOR_PUSH_DELTA, .bY =                   0},
	[ANIM_DIRECTION_NE] = {.bX =  WARRIOR_PUSH_DELTA, .bY = -WARRIOR_PUSH_DELTA},
	[ANIM_DIRECTION_N] =  {.bX =                   0, .bY = -WARRIOR_PUSH_DELTA},
	[ANIM_DIRECTION_NW] = {.bX = -WARRIOR_PUSH_DELTA, .bY = -WARRIOR_PUSH_DELTA},
	[ANIM_DIRECTION_W] =  {.bX = -WARRIOR_PUSH_DELTA, .bY =                  0},
	[ANIM_DIRECTION_SW] = {.bX = -WARRIOR_PUSH_DELTA, .bY = WARRIOR_PUSH_DELTA},
};

//------------------------------------------------------------------ PRIVATE FNS

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

static UBYTE isPositionCollidingWithWarrior(
	tUwCoordYX sPos, const tWarrior *pWarrior
) {
	return (
		sPos.uwX < pWarrior->sPos.uwX + LOOKUP_TILE_SIZE &&
		sPos.uwX + LOOKUP_TILE_SIZE > pWarrior->sPos.uwX &&
		sPos.uwY < pWarrior->sPos.uwY + LOOKUP_TILE_SIZE &&
		sPos.uwY + LOOKUP_TILE_SIZE > pWarrior->sPos.uwY
	);
}

static UBYTE areWarriorsColliding(
	const tWarrior *pWarrior1, const tWarrior *pWarrior2
) {
	return isPositionCollidingWithWarrior(pWarrior1->sPos, pWarrior2);
}

static inline tWarrior *warriorGetAtPos(
	UWORD uwPosX, BYTE bLookupAddX, UWORD uwPosY, BYTE bLookupAddY
) {
	UWORD uwLookupX = uwPosX / LOOKUP_TILE_SIZE + bLookupAddX;
	UWORD uwLookupY = uwPosY / LOOKUP_TILE_SIZE + bLookupAddY;
	return s_pWarriorLookup[uwLookupX][uwLookupY];
}

static void warriorTryMoveBy(tWarrior *pWarrior, BYTE bDeltaX, BYTE bDeltaY) {
	UBYTE ubOldLookupX = pWarrior->sPos.uwX / LOOKUP_TILE_SIZE;
	UBYTE ubOldLookupY = pWarrior->sPos.uwY / LOOKUP_TILE_SIZE;
	UBYTE isMoved = 0;

	if (bDeltaX) {
		UBYTE isColliding = 0;
		tUwCoordYX sOldPos = {.ulYX = pWarrior->sPos.ulYX};
		pWarrior->sPos.uwX += bDeltaX;

		// collision with upper corner
		tWarrior *pUp = warriorGetAtPos(sOldPos.uwX, SGN(bDeltaX), sOldPos.uwY, 0);
		if(pUp && pUp != pWarrior) {
			isColliding = areWarriorsColliding(pWarrior, pUp);
		}

		// collision with lower corner
		if (!isColliding && (sOldPos.uwY & (LOOKUP_TILE_SIZE - 1))) {
			tWarrior *pDown = warriorGetAtPos(sOldPos.uwX, SGN(bDeltaX), sOldPos.uwY, +1);
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
		tWarrior *pLeft = warriorGetAtPos(sOldPos.uwX, 0, sOldPos.uwY, SGN(bDeltaY));
		if(pLeft && pLeft != pWarrior) {
			isColliding = areWarriorsColliding(pWarrior, pLeft);
		}

		// collision with right corner
		if (!isColliding && (sOldPos.uwX & (LOOKUP_TILE_SIZE - 1))) {
			tWarrior *pRight = warriorGetAtPos(sOldPos.uwX, +1, sOldPos.uwY, SGN(bDeltaY));
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

static void warriorAdd(
	tWarrior *pWarrior, UWORD uwSpawnX, UWORD uwSpawnY, tSteer sSteer
) {
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

static void warriorSetAnim(tWarrior *pWarrior, tAnim eAnim) {
	pWarrior->eAnim = eAnim;
	pWarrior->ubAnimFrame = 0;
	pWarrior->ubFrameCooldown = FRAME_COOLDOWN;
}

static void warriorSetAnimOnce(tWarrior *pWarrior, tAnim eAnim) {
	if(pWarrior->eAnim != eAnim) {
		warriorSetAnim(pWarrior, eAnim);
	}
}

static void warriorStrike(const tWarrior *pWarrior) {
	const tBCoordYX *pDelta = &s_pAnimDirToAttackDelta[pWarrior->eDirection];
	const tBCoordYX *pPushDelta = &s_pAnimDirToPushDelta[pWarrior->eDirection];
	tUwCoordYX sAttackPos = (tUwCoordYX) {
		.uwX = pWarrior->sPos.uwX + pDelta->bX,
		.uwY = pWarrior->sPos.uwY + pDelta->bY
	};

	tWarrior *pOther;

	pOther = warriorGetAtPos(sAttackPos.uwX, 0, sAttackPos.uwY, 0);
	if(pOther && pOther != pWarrior && isPositionCollidingWithWarrior(sAttackPos, pOther)) {
		warriorSetAnim(pOther, ANIM_HURT);
		pOther->sPushDelta = *pPushDelta;
		return;
	}

	pOther = warriorGetAtPos(sAttackPos.uwX, 1, sAttackPos.uwY, 0);
	if(pOther && pOther != pWarrior && isPositionCollidingWithWarrior(sAttackPos, pOther)) {
		warriorSetAnim(pOther, ANIM_HURT);
		pOther->sPushDelta = *pPushDelta;
		return;
	}

	pOther = warriorGetAtPos(sAttackPos.uwX, 0, sAttackPos.uwY, 1);
	if(pOther && pOther != pWarrior && isPositionCollidingWithWarrior(sAttackPos, pOther)) {
		warriorSetAnim(pOther, ANIM_HURT);
		pOther->sPushDelta = *pPushDelta;
		return;
	}

	pOther = warriorGetAtPos(sAttackPos.uwX, 1, sAttackPos.uwY, 1);
	if(pOther && pOther != pWarrior && isPositionCollidingWithWarrior(sAttackPos, pOther)) {
		warriorSetAnim(pOther, ANIM_HURT);
		pOther->sPushDelta = *pPushDelta;
		return;
	}
}

static void warriorProcess(tWarrior *pWarrior) {
	steerProcess(&pWarrior->sSteer);

	if(pWarrior->eAnim == ANIM_HURT && pWarrior->ubAnimFrame != getFrameCountForAnim(ANIM_HURT) - 1) {
		// Pushback
		warriorTryMoveBy(pWarrior, pWarrior->sPushDelta.bX, pWarrior->sPushDelta.bY);
	}
	else if(pWarrior->eAnim == ANIM_ATTACK && pWarrior->ubAnimFrame != getFrameCountForAnim(ANIM_ATTACK) - 1) {
		if(pWarrior->ubAnimFrame == getFrameCountForAnim(ANIM_ATTACK) - 2) {
			// Do the actual hit
			warriorStrike(pWarrior);
		}
	}
	else {
		if(steerDirCheck(&pWarrior->sSteer, DIRECTION_FIRE)) {
			// Start swinging
			warriorSetAnim(pWarrior, ANIM_ATTACK);
		}
		else {
			BYTE bDeltaX = 0, bDeltaY = 0;
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
				warriorSetAnimOnce(pWarrior, ANIM_WALK);
				warriorTryMoveBy(pWarrior, bDeltaX, bDeltaY);
			}
			else {
				warriorSetAnimOnce(pWarrior, ANIM_IDLE);
			}
		}
	}

	--pWarrior->ubFrameCooldown;
	if (!pWarrior->ubFrameCooldown) {
		if (++pWarrior->ubAnimFrame >= getFrameCountForAnim(pWarrior->eAnim)) {
			pWarrior->ubAnimFrame = 0;
		}
		pWarrior->ubFrameCooldown = FRAME_COOLDOWN;
		tFrameOffsets *pOffsets = &s_pFrameOffsets[pWarrior->eDirection][pWarrior->eAnim][pWarrior->ubAnimFrame];
		bobNewSetFrame(&pWarrior->sBob, pOffsets->pBitmap, pOffsets->pMask);
	}

	bobNewPush(&pWarrior->sBob);
}

//------------------------------------------------------------------- PUBLIC FNS

void warriorsDrawLookup(tBitMap *pBuffer) {
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

void warriorsCreate(void) {
	initFrameOffsets();
	resetWarriorLookup();

	for(UBYTE i = 0; i < WARRIOR_COUNT; ++i) {
		s_pWarriors[i] = memAllocFast(sizeof(*s_pWarriors[i]));
		warriorAdd(
			s_pWarriors[i],
			100 + 32 * (i % WARRIORS_PER_ROW),
			100 + 32 * (i / WARRIORS_PER_ROW),
			i == 0 ? steerInitKey(KEYMAP_WSAD) : (
				i == 1 ? steerInitKey(KEYMAP_ARROWS) : steerInitIdle()
			)
		);
	}
}

void warriorsProcess(void) {
	for(UBYTE i = 0; i < WARRIOR_COUNT; ++i) {
		warriorProcess(s_pWarriors[i]);
	}

	tWarrior **pPrev = &s_pWarriors[0];
	for(UBYTE i = 1; i < WARRIOR_COUNT; ++i) {
		if(s_pWarriors[i]->sPos.ulYX < (*pPrev)->sPos.ulYX) {
			tWarrior *pTemp = *pPrev;
			*pPrev = s_pWarriors[i];
			s_pWarriors[i] = pTemp;
		}
		pPrev = &s_pWarriors[i];
	}
}

void warriorsDestroy(void) {
	for(UBYTE i = 0; i < WARRIOR_COUNT; ++i) {
		memFree(s_pWarriors[i], sizeof(*s_pWarriors[i]));
	}
}
