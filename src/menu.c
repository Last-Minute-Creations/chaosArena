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

//---------------------------------------------------------------------- DEFINES

#define MENU_WIDTH 192
#define MENU_HEIGHT 168
#define MENU_DISPLAY_START_X ((DISPLAY_WIDTH - MENU_WIDTH) / 2)
#define MENU_DISPLAY_START_Y ((DISPLAY_HEIGHT - MENU_HEIGHT) / 2)
#define MENU_DISPLAY_END_Y (MENU_DISPLAY_START_Y + MENU_HEIGHT)
#define MENU_COLOR_BG 5
#define MENU_COLOR_INACTIVE 6
#define MENU_COLOR_ACTIVE 7
#define MENU_COLOR_TITLE 7

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

static tMenuListOption s_pMenuOptions[] = {
	{.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK, .sOptCb = {.cbSelect = onStart}},
	{.eOptionType = MENU_LIST_OPTION_TYPE_CALLBACK, .sOptCb = {.cbSelect = onExit}},
};
#define MENU_OPTION_COUNT (sizeof(s_pMenuOptions) / sizeof(s_pMenuOptions[0]))

const char *s_pMenuCaptions[MENU_OPTION_COUNT] = {
	"BEGIN CHAOS",
	"EXIT",
};

//------------------------------------------------------------------ PRIVATE FNS

static void menuGsCreate(void) {
	s_pMenuBitmap = bitmapCreate(MENU_WIDTH, MENU_HEIGHT, DISPLAY_BPP, BMF_INTERLEAVED);
	s_pVpManager = displayGetManager();

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
}

static void menuGsLoop(void) {
	if(keyUse(KEY_ESCAPE)) {
		gameExit();
	}

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
