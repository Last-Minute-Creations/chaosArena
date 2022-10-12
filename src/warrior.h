/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INCLUDE_WARRIOR_H
#define INCLUDE_WARRIOR_H

#include "bob_new.h"
#include "steer.h"

typedef enum tAnimDirection {
	ANIM_DIRECTION_S,
	ANIM_DIRECTION_SE,
	ANIM_DIRECTION_E,
	ANIM_DIRECTION_NE,
	ANIM_DIRECTION_N,
	ANIM_DIRECTION_NW,
	ANIM_DIRECTION_W,
	ANIM_DIRECTION_SW,
	ANIM_DIRECTION_COUNT,
} tAnimDirection;

typedef enum tAnim {
	ANIM_IDLE,
	ANIM_WALK,
	ANIM_ATTACK,
	ANIM_HURT,
	ANIM_COUNT,
} tAnim;

typedef enum tControl {
	CONTROL_JOY1,
	CONTROL_JOY2,
	CONTROL_JOY3,
	CONTROL_JOY4,
	CONTROL_KEY1,
	CONTROL_KEY2,
	CONTROL_CPU,
	CONTROL_OFF,
} tControl;

typedef struct tWarrior {
	tUwCoordYX sPos;
	tBobNew sBob;
	UBYTE ubStunCooldown;
	UBYTE ubAnimFrame;
	UBYTE ubFrameCooldown;
	tControl eControl;
	tAnim eAnim;
	tAnimDirection eDirection;
	tSteer sSteer;
} tWarrior;

void warriorInit(void);

void warriorAdd(tWarrior *pWarrior, UWORD uwSpawnX, UWORD uwSpawnY, tSteer sSteer);

void warriorProcess(tWarrior *pWarrior);

void warriorDrawLookup(tBitMap *pBuffer);

#endif // INCLUDE_WARRIOR_H
