/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "menu.h"
#include <ace/managers/blit.h>
#include <ace/managers/game.h>
#include <ace/managers/key.h>
#include <mini_std/stdio.h>
#include "display.h"
#include "assets.h"
#include "menu_list.h"
#include "chaos_arena.h"
#include "steer.h"

//---------------------------------------------------------------------- DEFINES

#define MENU_WIDTH 192
#define MENU_HEIGHT 168
#define MENU_DISPLAY_START_X ((DISPLAY_WIDTH - MENU_WIDTH) / 2)
#define MENU_DISPLAY_START_Y ((DISPLAY_HEIGHT - MENU_HEIGHT) / 2)
#define MENU_DISPLAY_END_Y (MENU_DISPLAY_START_Y + MENU_HEIGHT)
#define MENU_COLOR_BG 5
#define MENU_COLOR_INACTIVE 4
#define MENU_COLOR_ACTIVE 7
#define MENU_COLOR_TITLE 7
#define APPEAR_ANIM_SPEED 4

//-------------------------------------------------------------------------TYPES

typedef enum tSteerKind {
	STEER_KIND_JOY1,
	STEER_KIND_JOY2,
	STEER_KIND_JOY3,
	STEER_KIND_JOY4,
	STEER_KIND_WSAD,
	STEER_KIND_ARROWS,
	STEER_KIND_OFF,
	STEER_KIND_COUNT,
} tSteerKind;

//---------------------------------------------------------------- PRIVATE DECLS

static void onStart(void);
static void onExit(void);
static void onUndraw(UWORD uwX, UWORD uwY, UWORD uwWidth, UWORD uwHeight);
static void onDrawPos(
	UWORD uwX, UWORD uwY, const char *szCaption, const char *szText,
	UBYTE isActive, UWORD *pUndrawWidth
);

//----------------------------------------------------------------- PRIVATE VARS

static tSimpleBufferManager *s_pVpManager;
static tBitMap *s_pMenuBitmap;
static UBYTE s_pPlayerSteerKinds[6] = {
	STEER_KIND_JOY1, STEER_KIND_JOY2, STEER_KIND_JOY3,
	STEER_KIND_JOY4, STEER_KIND_ARROWS, STEER_KIND_WSAD
};
static tSteer s_pMenuSteers[6];
static UBYTE s_ubLastDrawEnd[2];
static UBYTE s_isOdd;

static const char *s_pSteerEnumLabels[] = {
	"JOY 1",
	"JOY 2",
	"JOY 3",
	"JOY 4",
	"WSAD",
	"ARROWS",
	"OFF",
};

static tMenuListOption s_pMenuOptions[] = {
	{.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK, .sOptCb = {.cbSelect = onStart}},
	{.eOptionType = MENU_LIST_OPTION_TYPE_UINT8, .sOptUb = {
		.isCyclic = 1, .pEnumLabels = s_pSteerEnumLabels, .pVar = &s_pPlayerSteerKinds[0],
		.ubMin = 0, .ubMax = STEER_KIND_COUNT - 1
	}},
	{.eOptionType = MENU_LIST_OPTION_TYPE_UINT8, .sOptUb = {
		.isCyclic = 1, .pEnumLabels = s_pSteerEnumLabels, .pVar = &s_pPlayerSteerKinds[1],
		.ubMin = 0, .ubMax = STEER_KIND_COUNT - 1
	}},
	{.eOptionType = MENU_LIST_OPTION_TYPE_UINT8, .sOptUb = {
		.isCyclic = 1, .pEnumLabels = s_pSteerEnumLabels, .pVar = &s_pPlayerSteerKinds[2],
		.ubMin = 0, .ubMax = STEER_KIND_COUNT - 1
	}},
	{.eOptionType = MENU_LIST_OPTION_TYPE_UINT8, .sOptUb = {
		.isCyclic = 1, .pEnumLabels = s_pSteerEnumLabels, .pVar = &s_pPlayerSteerKinds[3],
		.ubMin = 0, .ubMax = STEER_KIND_COUNT - 1
	}},
	{.eOptionType = MENU_LIST_OPTION_TYPE_UINT8, .sOptUb = {
		.isCyclic = 1, .pEnumLabels = s_pSteerEnumLabels, .pVar = &s_pPlayerSteerKinds[4],
		.ubMin = 0, .ubMax = STEER_KIND_COUNT - 1
	}},
	{.eOptionType = MENU_LIST_OPTION_TYPE_UINT8, .sOptUb = {
		.isCyclic = 1, .pEnumLabels = s_pSteerEnumLabels, .pVar = &s_pPlayerSteerKinds[5],
		.ubMin = 0, .ubMax = STEER_KIND_COUNT - 1
	}},
	{.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK, .sOptCb = {.cbSelect = onExit}},
};
#define MENU_OPTION_COUNT (sizeof(s_pMenuOptions) / sizeof(s_pMenuOptions[0]))

const char *s_pMenuCaptions[MENU_OPTION_COUNT] = {
	"BEGIN CHAOS",
	"PLAYER 1",
	"PLAYER 2",
	"PLAYER 3",
	"PLAYER 4",
	"PLAYER 5",
	"PLAYER 6",
	"EXIT",
};

//------------------------------------------------------------------ PRIVATE FNS

static void menuGsCreate(void) {
	s_pMenuBitmap = bitmapCreate(MENU_WIDTH, MENU_HEIGHT, DISPLAY_BPP, BMF_INTERLEAVED);
	s_pVpManager = displayGetManager();
	UBYTE isParallel = joyIsParallelEnabled();

	s_pMenuSteers[0] = steerInitFromMode(STEER_MODE_JOY_1, 0);
	s_pMenuSteers[1] = steerInitFromMode(STEER_MODE_JOY_2, 0);
	s_pMenuSteers[2] = steerInitFromMode(isParallel ? STEER_MODE_JOY_3 : STEER_MODE_IDLE, 0);
	s_pMenuSteers[3] = steerInitFromMode(isParallel ? STEER_MODE_JOY_4 : STEER_MODE_IDLE, 0);
	s_pMenuSteers[4] = steerInitFromMode(STEER_MODE_KEY_WSAD, 0);
	s_pMenuSteers[5] = steerInitFromMode(STEER_MODE_KEY_ARROWS, 0);

	menuListInit(
		s_pMenuOptions, s_pMenuCaptions, MENU_OPTION_COUNT, g_pFontMenu,
		0, 30, onUndraw, onDrawPos
	);

	blitRect(s_pMenuBitmap, 0, 0, MENU_WIDTH, MENU_HEIGHT, MENU_COLOR_BG);
	fontDrawStr(
		g_pFontMenu, s_pMenuBitmap, MENU_WIDTH / 2, 10, "CHAOS ARENA",
		MENU_COLOR_TITLE, FONT_COOKIE | FONT_SHADOW | FONT_HCENTER, g_pTextBitmap
	);
	menuListDraw();
	s_ubLastDrawEnd[0] = 0;
	s_ubLastDrawEnd[1] = 0;
	s_isOdd = 0;
}

static void menuGsLoop(void) {
	if(keyUse(KEY_ESCAPE)) {
		gameExit();
		return;
	}

	if(s_ubLastDrawEnd[s_isOdd] < MENU_HEIGHT) {
		UBYTE ubStartRow = s_ubLastDrawEnd[s_isOdd];
		UBYTE ubEndRow = MIN(s_ubLastDrawEnd[!s_isOdd] + APPEAR_ANIM_SPEED, MENU_HEIGHT);
		blitCopyAligned(
			s_pMenuBitmap, 0, ubStartRow, s_pVpManager->pBack,
			MENU_DISPLAY_START_X, MENU_DISPLAY_START_Y + ubStartRow,
			MENU_WIDTH, ubEndRow - ubStartRow
		);
		s_ubLastDrawEnd[s_isOdd] = ubEndRow;
		s_isOdd = !s_isOdd;
		return;
	}

	if(keyUse(KEY_RETURN)) {
		menuListEnter();
		return;
	}

	for(UBYTE ubPlayer = 0; ubPlayer < 6; ++ubPlayer) {
		tSteer *pSteer = &s_pMenuSteers[ubPlayer];
		steerProcess(pSteer);
		if(steerDirUse(pSteer, DIRECTION_UP)) {
			menuListNavigate(-1);
		}
		else if(steerDirUse(pSteer, DIRECTION_DOWN)) {
			menuListNavigate(+1);
		}
		else if(steerDirUse(pSteer, DIRECTION_LEFT)) {
			menuListToggle(-1);
		}
		else if(steerDirUse(pSteer, DIRECTION_RIGHT)) {
			menuListToggle(+1);
		}
		else if (steerDirUse(pSteer, DIRECTION_FIRE)) {
			menuListEnter();
			return;
		}
	}

	menuListDraw();

	blitCopyAligned(
		s_pMenuBitmap, 0, 0, s_pVpManager->pBack,
		MENU_DISPLAY_START_X, MENU_DISPLAY_START_Y,
		MENU_WIDTH, MENU_HEIGHT
	);
}

static void menuGsDestroy(void) {
	bitmapDestroy(s_pMenuBitmap);
}

static void onStart(void) {
	stateChange(g_pStateMachineGame, &g_sStateGame);
}

static void onExit(void) {
	gameExit();
}

static void onUndraw(UWORD uwX, UWORD uwY, UWORD uwWidth, UWORD uwHeight) {
	blitRect(s_pMenuBitmap, uwX, uwY, uwWidth, uwHeight, MENU_COLOR_BG);
}

static void onDrawPos(
	UWORD uwX, UWORD uwY, const char *szCaption, const char *szText,
	UBYTE isActive, UWORD *pUndrawWidth
) {
	UWORD uwTextWidth = fontMeasureText(g_pFontMenu, szText).uwX;
	fontDrawStr(
		g_pFontMenu, s_pMenuBitmap, uwX + MENU_WIDTH / 2, uwY, szText,
		isActive ? MENU_COLOR_ACTIVE : MENU_COLOR_INACTIVE,
		FONT_SHADOW | FONT_COOKIE | FONT_HCENTER, g_pTextBitmap
	);
	*pUndrawWidth = (MENU_WIDTH + uwTextWidth) / 2;
}

//------------------------------------------------------------------ PUBLIC VARS

tState g_sStateMenu = {
	.cbCreate = menuGsCreate, .cbLoop = menuGsLoop, .cbDestroy = menuGsDestroy
};
