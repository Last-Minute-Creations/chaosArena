/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "steer.h"
#include <ace/managers/log.h>
#include <ace/managers/joy.h>
#include <ace/managers/key.h>

#define STEER_RECORD_FILE "steers.bin"

#if defined(STEER_RECORD_KEYPRESSES) || defined(STEER_REPLAY_KEYPRESSES)
#include <ace/managers/system.h>
#include <ace/utils/file.h>
#define STEER_RECORD_MEMORY (4*1024*1024)
#define STEER_RECORD_MAX_ENTRIES (STEER_RECORD_MEMORY / DIRECTION_COUNT)
#define STEER_RECORD_MAX (STEER_RECORD_MAX_ENTRIES * DIRECTION_COUNT)

static UBYTE *s_pRecordedInputs;
static ULONG s_ulRecordLength;
#endif

#if defined(STEER_REPLAY_KEYPRESSES)
static ULONG s_ulReplayPos;
#endif

//------------------------------------------------------------------ PRIVATE FNS

static void onJoy(tSteer *pSteer) {
	// Joy direction enum is not in same order as steer enum
	// There is a reason why it's ordered that way in game, so it needs remapping
	static const UBYTE pDirToJoy[] = {
		[DIRECTION_UP] = JOY_UP,
		[DIRECTION_DOWN] = JOY_DOWN,
		[DIRECTION_LEFT] = JOY_LEFT,
		[DIRECTION_RIGHT] = JOY_RIGHT,
		[DIRECTION_FIRE] = JOY_FIRE
	};

	UBYTE ubJoy = pSteer->ubJoy;

	for(tDirection eDir = 0; eDir < DIRECTION_COUNT; ++eDir) {
		if(joyCheck(ubJoy + pDirToJoy[eDir])) {
			if(pSteer->pDirectionStates[eDir] == STEER_DIR_STATE_INACTIVE) {
				pSteer->pDirectionStates[eDir] = STEER_DIR_STATE_ACTIVE;
			}
		}
		else {
			pSteer->pDirectionStates[eDir] = STEER_DIR_STATE_INACTIVE;
		}
	}
}

static void onKey(tSteer *pSteer) {
	static const UBYTE pDirsWsad[] = {
		[DIRECTION_UP] = KEY_W,
		[DIRECTION_DOWN] = KEY_S,
		[DIRECTION_LEFT] = KEY_A,
		[DIRECTION_RIGHT] = KEY_D,
		[DIRECTION_FIRE] = KEY_LSHIFT
	};
	static const UBYTE pDirsArrows[] = {
		[DIRECTION_UP] = KEY_UP,
		[DIRECTION_DOWN] = KEY_DOWN,
		[DIRECTION_LEFT] = KEY_LEFT,
		[DIRECTION_RIGHT] = KEY_RIGHT,
		[DIRECTION_FIRE] = KEY_RSHIFT
	};

	const UBYTE *pDirToKey = (pSteer->eKeymap == STEER_KEYMAP_WSAD) ? pDirsWsad : pDirsArrows;

	for(tDirection eDir = 0; eDir < DIRECTION_COUNT; ++eDir) {
		if(keyCheck(pDirToKey[eDir])) {
			if(pSteer->pDirectionStates[eDir] == STEER_DIR_STATE_INACTIVE) {
				pSteer->pDirectionStates[eDir] = STEER_DIR_STATE_ACTIVE;
			}
		}
		else {
			pSteer->pDirectionStates[eDir] = STEER_DIR_STATE_INACTIVE;
		}
	}
}

static void onAi(tSteer *pSteer) {
	tAi *pAi = &pSteer->sAi;
	tDirection eDir = aiProcess(pAi);

	// // Fire is a special button - can be pressed for more than one process
	// if(pAi->isPressingFire) {
	// 	if(pSteer->pDirectionStates[DIRECTION_FIRE] == STEER_DIR_STATE_INACTIVE) {
	// 		logWrite("Pressing fire\n");
	// 		pSteer->pDirectionStates[DIRECTION_FIRE] = STEER_DIR_STATE_ACTIVE;
	// 	}
	// }
	// else {
	// 	if(pSteer->pDirectionStates[DIRECTION_FIRE] != STEER_DIR_STATE_INACTIVE) {
	// 		logWrite("Releasing fire\n");
	// 	}
	// 	pSteer->pDirectionStates[DIRECTION_FIRE] = STEER_DIR_STATE_INACTIVE;
	// }

	if(eDir != DIRECTION_COUNT) {
		// Push the selected button
		if(pSteer->pDirectionStates[eDir] == STEER_DIR_STATE_INACTIVE) {
			// logWrite("Pressing dir %hhu\n", eDir);
			pSteer->pDirectionStates[eDir] = STEER_DIR_STATE_ACTIVE;
		}
	}

	// Unpress the previous action
	if(pSteer->ePrevDirection != eDir && pSteer->ePrevDirection != DIRECTION_COUNT) {
		// logWrite("Releasing dir %hhu\n", pAi->ePrevDir);
		pSteer->pDirectionStates[pSteer->ePrevDirection] = STEER_DIR_STATE_INACTIVE;
	}

	// Save the current action for later unpressing
	pSteer->ePrevDirection = eDir;
}

static void onIdle(UNUSED_ARG tSteer *pSteer) {
	// Do nothing
}

//------------------------------------------------------------------- PUBLIC FNS

void steerManagerCreate(void) {
#if defined(STEER_RECORD_KEYPRESSES)
	s_ulRecordLength = 0;
	s_pRecordedInputs = memAllocFast(STEER_RECORD_MAX);
#elif defined(STEER_REPLAY_KEYPRESSES)
	systemUse();
	tFile *pFileRecording = fileOpen(STEER_RECORD_FILE, "rb");
	fileRead(pFileRecording, &s_ulRecordLength, sizeof(s_ulRecordLength));
	s_pRecordedInputs = memAllocFast(s_ulRecordLength);
	fileRead(pFileRecording, s_pRecordedInputs, s_ulRecordLength);
	fileClose(pFileRecording);
	s_ulReplayPos = 0;
	logWrite("Replaying %lu inputs", s_ulRecordLength);
	systemUnuse();
#endif
}

void steerManagerDestroy(void) {
#if defined(STEER_RECORD_KEYPRESSES)
	systemUse();
	tFile *pFileRecording = fileOpen(STEER_RECORD_FILE, "wb");
	fileWrite(pFileRecording, &s_ulRecordLength, sizeof(s_ulRecordLength));
	fileWrite(pFileRecording, s_pRecordedInputs, s_ulRecordLength);
	fileClose(pFileRecording);
	memFree(s_pRecordedInputs, STEER_RECORD_MAX);
	logWrite(
		"Recorded %lu/%d (%lu%%) inputs",
		s_ulRecordLength, STEER_RECORD_MAX,
		(s_ulRecordLength * 100) / STEER_RECORD_MAX
	);
	systemUnuse();
#elif defined(STEER_REPLAY_KEYPRESSES)
	memFree(s_pRecordedInputs, s_ulRecordLength);
#endif
}

tSteer steerInitFromMode(tSteerMode eMode, struct tWarrior *pWarrior) {
	switch(eMode) {
		case STEER_MODE_JOY_1:
			return steerInitJoy(JOY1);
		case STEER_MODE_JOY_2:
			return steerInitJoy(JOY2);
		case STEER_MODE_JOY_3:
			return steerInitJoy(JOY3);
		case STEER_MODE_JOY_4:
			return steerInitJoy(JOY4);
		case STEER_MODE_KEY_ARROWS:
			return steerInitKey(STEER_KEYMAP_ARROWS);
		case STEER_MODE_KEY_WSAD:
			return steerInitKey(STEER_KEYMAP_WSAD);
		case STEER_MODE_AI:
			return steerInitAi(pWarrior);
		default:
			return steerInitIdle();
	}
}

tSteer steerInitJoy(UBYTE ubJoy) {
	tSteer sSteer = {
		.cbProcess = onJoy,
		.ubJoy = ubJoy
	};
	return sSteer;
}

tSteer steerInitKey(tSteerKeymap eKeymap) {
	tSteer sSteer = {
		.cbProcess = onKey,
		.eKeymap = eKeymap
	};
	return sSteer;
}

tSteer steerInitAi(struct tWarrior *pWarrior) {
	tSteer sSteer = {
		.cbProcess = onAi
	};

	aiInit(&sSteer.sAi, pWarrior);
	return sSteer;
}

tSteer steerInitIdle(void) {
	tSteer sSteer = {
		.cbProcess = onIdle
	};
	return sSteer;
}

void steerProcess(tSteer *pSteer) {
	if(pSteer->cbProcess) {
#if defined(STEER_REPLAY_KEYPRESSES)
		if(s_ulReplayPos + DIRECTION_COUNT <= s_ulRecordLength) {
			for(tDirection eDir = 0; eDir < DIRECTION_COUNT; ++eDir) {
				pSteer->pDirectionStates[eDir] = s_pRecordedInputs[s_ulReplayPos++];
			}
		}
		else {
			logWrite("ERR: Replay time exceeded!\n");
		}
#elif defined(STEER_RECORD_KEYPRESSES)
		pSteer->cbProcess(pSteer);
		if(s_ulRecordLength + DIRECTION_COUNT <= STEER_RECORD_MAX) {
			for(tDirection eDir = 0; eDir < DIRECTION_COUNT; ++eDir) {
				s_pRecordedInputs[s_ulRecordLength++] = pSteer->pDirectionStates[eDir];
			}
		}
		else {
			logWrite("ERR: Recording time exceeded!\n");
		}
#else
		pSteer->cbProcess(pSteer);
#endif
	}
}

UBYTE steerIsPlayer(const tSteer *pSteer) {
	return (pSteer->cbProcess == onJoy || pSteer->cbProcess == onKey);
}

UBYTE steerIsArrows(const tSteer *pSteer) {
	UBYTE isArrows = (pSteer->cbProcess == onKey && pSteer->eKeymap == STEER_KEYMAP_ARROWS);
	return isArrows;
}

UBYTE steerDirCheck(const tSteer *pSteer, tDirection eDir) {
	return pSteer->pDirectionStates[eDir] != STEER_DIR_STATE_INACTIVE;
}

UBYTE steerDirUse(tSteer *pSteer, tDirection eDir) {
	if(pSteer->pDirectionStates[eDir] == STEER_DIR_STATE_ACTIVE) {
		pSteer->pDirectionStates[eDir] = STEER_DIR_STATE_USED;
		return 1;
	}
	return 0;
}

tDirection steerGetPressedDir(const tSteer *pSteer) {
	if(pSteer->pDirectionStates[DIRECTION_UP] != STEER_DIR_STATE_INACTIVE) {
		return DIRECTION_UP;
	}
	if(pSteer->pDirectionStates[DIRECTION_DOWN] != STEER_DIR_STATE_INACTIVE) {
		return DIRECTION_DOWN;
	}
	if(pSteer->pDirectionStates[DIRECTION_LEFT] != STEER_DIR_STATE_INACTIVE) {
		return DIRECTION_LEFT;
	}
	if(pSteer->pDirectionStates[DIRECTION_RIGHT] != STEER_DIR_STATE_INACTIVE) {
		return DIRECTION_RIGHT;
	}
	return DIRECTION_COUNT;
}

void steerResetAi(tSteer *pSteer) {
	if(pSteer->cbProcess == onAi) {
		aiInit(&pSteer->sAi, pSteer->sAi.pWarrior);
	}
}

const char *g_pSteerModeLabels[STEER_MODE_COUNT] = {
	"JOY 1", "JOY 2", "JOY 3", "JOY 4", "WSAD", "ARROWS", "CPU", "IDLE", "OFF"
};
