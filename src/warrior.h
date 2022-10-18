/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INCLUDE_WARRIOR_H
#define INCLUDE_WARRIOR_H

#include "bob_new.h"
#include "steer.h"
#include "anim.h"

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
	UBYTE ubAnimFrame;
	UBYTE ubFrameCooldown;
	UBYTE ubStunCooldown;
	UBYTE isDead;
	tControl eControl;
	tAnim eAnim;
	tAnimDirection eDirection;
	tSteer sSteer;
	tBCoordYX sPushDelta;
} tWarrior;

void warriorsCreate(void);

void warriorsProcess(void);

void warriorsDestroy(void);

void warriorsDrawLookup(tBitMap *pBuffer);

UBYTE warriorsGetAliveCount(void);

UBYTE warriorsGetAlivePlayerCount(void);

void warriorsEnableMove(UBYTE isEnabled);

tWarrior *warriorGetStrikeTarget(
	const tWarrior *pWarrior, tAnimDirection eDirection
);

#endif // INCLUDE_WARRIOR_H
