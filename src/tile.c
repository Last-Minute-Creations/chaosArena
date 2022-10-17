/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "tile.h"
#include <mini_std/stdlib.h>
#include <ace/types.h>
#include <ace/generic/screen.h>
#include <ace/managers/blit.h>
#include <ace/managers/rand.h>
#include "assets.h"
#include "chaos_arena.h"
#include "display.h"

#define TILE_WIDTH (DISPLAY_WIDTH / MAP_TILE_SIZE)
#define TILE_HEIGHT (DISPLAY_HEIGHT / MAP_TILE_SIZE)
#define SPAWNS_MAX 30
#define CRUMBLES_MAX 4
#define CRUMBLE_COOLDOWN 4
#define CRUMBLE_ADD_COOLDOWN 15
#define TILE_QUEUE_SIZE (CRUMBLES_MAX * 2)

#define SFX_PRIORITY_CRUMBLE 8

typedef enum tTile {
	TILE_VOID,
	TILE_WALL1,
	TILE_FLOOR1,
	TILE_FLOOR1_C1, // crumble state
	TILE_FLOOR1_C2,
	TILE_FLOOR1_C3,
	TILE_FLOOR1_C4,
	TILE_FLOOR1_C5,
	TILE_FLOOR1_C6,
	TILE_FLOOR1_C7,
	TILE_FLOOR1_C8,
	TILE_FLOOR1_C9,
	TILE_FLOOR1_C10,
	TILE_COUNT
} tTile;

typedef struct tCrumble {
	tTile *pTile;
	UBYTE ubTileX;
	UBYTE ubTileY;
	UBYTE ubCooldown;
} tCrumble;

typedef struct tTileDrawQueueEntry {
	UWORD uwX;
	UWORD uwY;
	UWORD uwTileOffset;
	UBYTE ubDrawCount;
} tTileDrawQueueEntry;

typedef struct tTileCrumbleOrderEntry {
	tUbCoordYX sPos;
	UWORD uwSortOrder;
} tTileCrumbleOrderEntry;

static tTile s_pTilesXy[TILE_WIDTH][TILE_HEIGHT];
static tCrumble s_pCrumbleList[CRUMBLES_MAX];
static tUwCoordYX s_pSpawns[SPAWNS_MAX];
static UBYTE s_ubSpawnCount;
static UBYTE s_ubActiveCrumbles;
static UBYTE s_ubCrumbleAddCooldown;

static tTileDrawQueueEntry s_pTileRedrawQueue[TILE_QUEUE_SIZE];
static UBYTE s_ubRedrawPushPos;
static UBYTE s_ubRedrawPopPos;
static tTileCrumbleOrderEntry s_pTileCrumbleOrder[TILE_WIDTH * TILE_HEIGHT];

static UWORD s_uwTileCount;
static UWORD s_uwCurrentTileCrumble;

/**
 * @brief The easy to read tile layout.
 * Note that indices are reversed here, but the ascii is in correct order.
 */
static const char s_pMapPatternYx[TILE_HEIGHT][TILE_WIDTH + 1] = {
	"@@@@@@@@@@@@@@@@@@@@@@@@",
	"@@@@@@@@@@@@@@@@@@@@@@@@",
	"@@@@@@@@@@@@@@@@@@@@@@@@",
	"@@@@@@@@@@@@@@@@@@@@@@@@",
	"@@@.....L......J.....@@@",
	"@@@.1......9.......3.@@@",
	"@@@....@@@....@@@....@@@",
	"@@@.G..@@@..5.@@@..D.@@@",
	"@@@....@@@....@@@....@@@",
	"@@@.....B...M......8.@@@",
	"@@@.7......N...C.....@@@",
	"@@@....@@@....@@@....@@@",
	"@@@.E..@@@.6..@@@..F.@@@",
	"@@@....@@@....@@@....@@@",
	"@@@.4.......A......2.@@@",
	"@@@.....H......K.....@@@",
	"@@@@@@@@@@@@@@@@@@@@@@@@",
	"@@@@@@@@@@@@@@@@@@@@@@@@",
	"@@@@@@@@@@@@@@@@@@@@@@@@",
	"@@@@@@@@@@@@@@@@@@@@@@@@"
};

//------------------------------------------------------------------ PRIVATE FNS

static void tileQueueAddEntry(UBYTE ubTileX, UBYTE ubTileY, tTile eTile) {
	s_pTilesXy[ubTileX][ubTileY] = eTile;
	tTileDrawQueueEntry *pEntry = &s_pTileRedrawQueue[s_ubRedrawPushPos];
	pEntry->uwTileOffset = eTile * MAP_TILE_SIZE;
	pEntry->uwX = ubTileX * MAP_TILE_SIZE;
	pEntry->uwY = ubTileY * MAP_TILE_SIZE;
	pEntry->ubDrawCount = 2;

	if(++s_ubRedrawPushPos == TILE_QUEUE_SIZE) {
		s_ubRedrawPushPos = 0;
	}
	if(s_ubRedrawPushPos == s_ubRedrawPopPos) {
		logWrite("ERR: Ring buffer overflow");
	}
}

static void tileQueueProcess(tBitMap *pBuffer) {
	tTileDrawQueueEntry *pEntry = &s_pTileRedrawQueue[s_ubRedrawPopPos];
	if(pEntry->ubDrawCount == 0) {
		return;
	}

	blitCopyAligned(
		g_pTileset, 0, pEntry->uwTileOffset,
		pBuffer, pEntry->uwX, pEntry->uwY, MAP_TILE_SIZE, MAP_TILE_SIZE
	);

	if(--pEntry->ubDrawCount == 0) {
		if(++s_ubRedrawPopPos == TILE_QUEUE_SIZE) {
			s_ubRedrawPopPos = 0;
		}
	}
}

static UBYTE tileQueueHasSpace(void) {
	// UBYTE ubPopPos = s_ubRedrawPopPos;
	// if(ubPopPos <= s_ubRedrawPushPos) {
	// 	ubPopPos += TILE_QUEUE_SIZE;
	// }
	// return (ubPopPos - s_ubRedrawPushPos) > 2;
	return s_ubRedrawPopPos == s_ubRedrawPushPos;
}

static int onTileCrumbleSort(const void *pLhs, const void *pRhs) {
	const tTileCrumbleOrderEntry *pLhsCoord = (tTileCrumbleOrderEntry*)pLhs;
	const tTileCrumbleOrderEntry *pRhsCoord = (tTileCrumbleOrderEntry*)pRhs;
	return pRhsCoord->uwSortOrder - pLhsCoord->uwSortOrder;
}

static void tileCrumbleAddNext(void) {
	if(s_uwCurrentTileCrumble >= s_uwTileCount) {
		return;
	}

	tUbCoordYX sPos = {.uwYX = s_pTileCrumbleOrder[s_uwCurrentTileCrumble].sPos.uwYX};
	tTile *pTile = &s_pTilesXy[sPos.ubX][sPos.ubY];
	if(*pTile != TILE_FLOOR1) {
		return;
	}

	for(UBYTE i = 0; i < CRUMBLES_MAX; ++i) {
		if(!s_pCrumbleList[i].pTile) {
			s_pCrumbleList[i].pTile = pTile;
			s_pCrumbleList[i].ubCooldown = CRUMBLE_COOLDOWN;
			s_pCrumbleList[i].ubTileX = sPos.ubX;
			s_pCrumbleList[i].ubTileY = sPos.ubY;
			++s_ubActiveCrumbles;
			++s_uwCurrentTileCrumble;
			return;
		}
	}
}

//------------------------------------------------------------------- PUBLIC FNS

void tilesInit(void) {
	s_ubSpawnCount = 0;
	s_uwTileCount = 0;
	s_uwCurrentTileCrumble = 0;
	s_ubCrumbleAddCooldown = CRUMBLE_ADD_COOLDOWN;
	for(UBYTE ubY = 0; ubY < TILE_HEIGHT; ++ubY) {
		for(UBYTE ubX = 0; ubX < TILE_WIDTH; ++ubX) {
			s_pTilesXy[ubX][ubY] = (
				s_pMapPatternYx[ubY][ubX] == '@' ? TILE_VOID : TILE_FLOOR1
			);

			if(s_pMapPatternYx[ubY][ubX] != '@' && s_pMapPatternYx[ubY][ubX] != '.') {
				s_pSpawns[s_ubSpawnCount++] = (tUwCoordYX){
					.uwX = ubX * MAP_TILE_SIZE + (MAP_TILE_SIZE / 2),
					.uwY = ubY * MAP_TILE_SIZE + (MAP_TILE_SIZE / 2)
				};
				logWrite(
					"Loaded spawn at %hhu,%hhu: %hu,%hu\n", ubX, ubY,
					s_pSpawns[s_ubSpawnCount-1].uwX, s_pSpawns[s_ubSpawnCount-1].uwY
				);
			}

			if(s_pTilesXy[ubX][ubY] == TILE_FLOOR1) {
				s_pTileCrumbleOrder[s_uwTileCount++] = (tTileCrumbleOrderEntry){
					.sPos = {.ubX = ubX, .ubY = ubY},
					.uwSortOrder = (
						ABS(DISPLAY_WIDTH / 2 - ((ubX * MAP_TILE_SIZE) + HALF_TILE_SIZE)) +
						ABS(DISPLAY_HEIGHT / 2 - ((ubY * MAP_TILE_SIZE) + HALF_TILE_SIZE))
					)
				};
			}

			// 3d effect
			if (
				ubY > 0 && s_pTilesXy[ubX][ubY] == TILE_VOID &&
				s_pTilesXy[ubX][ubY - 1] == TILE_FLOOR1
			) {
				s_pTilesXy[ubX][ubY] = TILE_WALL1;
			}
		}
	}

	logWrite("Loaded %hhu spawn points\n", s_ubSpawnCount);
	s_ubRedrawPushPos = 0;
	s_ubRedrawPopPos = 0;
	s_ubActiveCrumbles = 0;
	for(UBYTE i = 0; i < CRUMBLES_MAX; ++i) {
		s_pCrumbleList[i].pTile = 0;
	}

	qsort(
		s_pTileCrumbleOrder, s_uwTileCount, sizeof(s_pTileCrumbleOrder[0]),
		onTileCrumbleSort
	);
	logWrite("Tiles: %hu\n", s_uwTileCount);
}

void tileCrumbleProcess(tBitMap *pBuffer) {
	if(--s_ubCrumbleAddCooldown == 0) {
		tileCrumbleAddNext();
		s_ubCrumbleAddCooldown = CRUMBLE_ADD_COOLDOWN;
	}

	tileQueueProcess(pBuffer);
	if(!tileQueueHasSpace()) {
		return;
	}

	tCrumble *pCrumble = &s_pCrumbleList[0];
	for(UBYTE i = 0; i < CRUMBLES_MAX; ++i, ++pCrumble) {
		if(!pCrumble->pTile) {
			continue;
		}
		if(--pCrumble->ubCooldown == 0) {
			tTile eNewTile;
			if(*pCrumble->pTile == TILE_FLOOR1_C10) {
				eNewTile = (
					tileIsSolid(pCrumble->ubTileX, pCrumble->ubTileY - 1) ?
					TILE_WALL1 : TILE_VOID
				);

				if(s_pTilesXy[pCrumble->ubTileX][pCrumble->ubTileY + 1] == TILE_WALL1) {
					tileQueueAddEntry(pCrumble->ubTileX, pCrumble->ubTileY + 1, TILE_VOID);
				}
				pCrumble->pTile = 0;
				ptplayerSfxPlay(g_pSfxCrumble, 2, 64, SFX_PRIORITY_CRUMBLE);
			}
			else {
				eNewTile = *pCrumble->pTile + 1;
			}
			pCrumble->ubCooldown = CRUMBLE_COOLDOWN;
			tileQueueAddEntry(pCrumble->ubTileX, pCrumble->ubTileY, eNewTile);
		}
	}
}

void tileShuffleSpawns(void) {
	for(UBYTE ubShuffle = 0; ubShuffle < 50; ++ubShuffle) {
		UBYTE ubA = randUwMax(&g_sRandManager, s_ubSpawnCount - 1);
		UBYTE ubB = randUwMax(&g_sRandManager, s_ubSpawnCount - 1);

		if(ubA == ubB) {
			continue;
		}

		tUwCoordYX sTmp = {.ulYX = s_pSpawns[ubA].ulYX};
		s_pSpawns[ubA].ulYX = s_pSpawns[ubB].ulYX;
		s_pSpawns[ubB].ulYX = sTmp.ulYX;
	}
}

const tUwCoordYX *tileGetSpawn(UBYTE ubIndex) {
	return &s_pSpawns[ubIndex];
}

void tilesDrawAllOn(tBitMap *pDestination) {
	for(UBYTE ubX = 0; ubX < TILE_WIDTH; ++ubX) {
		for(UBYTE ubY = 0; ubY < TILE_HEIGHT; ++ubY) {
			blitCopyAligned(
				g_pTileset, 0, s_pTilesXy[ubX][ubY] * 16,
				pDestination, ubX * 16, ubY * 16, 16, 16
			);
		}
	}
}

UBYTE tileIsSolid(UBYTE ubTileX, UBYTE ubTileY) {
	return s_pTilesXy[ubTileX][ubTileY] >= TILE_FLOOR1;
}
