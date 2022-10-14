#include "tile.h"
#include <ace/types.h>
#include <ace/generic/screen.h>
#include <ace/managers/blit.h>
#include <ace/managers/rand.h>
#include "assets.h"
#include "chaos_arena.h"

#define TILE_WIDTH (SCREEN_PAL_WIDTH / MAP_TILE_SIZE)
#define TILE_HEIGHT (SCREEN_PAL_HEIGHT / MAP_TILE_SIZE)
#define SPAWNS_MAX 30

typedef enum tTile {
	TILE_VOID,
	TILE_WALL_1,
	TILE_FLOOR_1,
	TILE_COUNT
} tTile;

static tTile s_pTilesXy[TILE_WIDTH][TILE_HEIGHT];
static tUwCoordYX s_pSpawns[SPAWNS_MAX];
static UBYTE s_ubSpawnCount;

/**
 * @brief The easy to read tile layout.
 * Note that indices are reversed here, but the ascii is in correct order.
 */
static const char s_pMapPatternYx[TILE_HEIGHT][TILE_WIDTH + 1] = {
	"@@@@@@@@@@@@@@@@@@@@",
	"@@@@@@@@@@@@@@@@@@@@",
	"@.....L......J.....@",
	"@.1......9.......3.@",
	"@....@@@....@@@....@",
	"@.G..@@@..5.@@@..D.@",
	"@....@@@....@@@....@",
	"@.....B...M......8.@",
	"@.7......N...C.....@",
	"@....@@@....@@@....@",
	"@.E..@@@.6..@@@..F.@",
	"@....@@@....@@@....@",
	"@.4.......A......2.@",
	"@.....H......K.....@",
	"@@@@@@@@@@@@@@@@@@@@",
	"@@@@@@@@@@@@@@@@@@@@"
};

//------------------------------------------------------------------- PUBLIC FNS

void tilesInit(void) {
	s_ubSpawnCount = 0;
	for(UBYTE ubY = 0; ubY < TILE_HEIGHT; ++ubY) {
		for(UBYTE ubX = 0; ubX < TILE_WIDTH; ++ubX) {
			s_pTilesXy[ubX][ubY] = (
				s_pMapPatternYx[ubY][ubX] == '@' ? TILE_VOID : TILE_FLOOR_1
			);

			if(s_pMapPatternYx[ubY][ubX] != '@' && s_pMapPatternYx[ubY][ubX] != '.') {
				s_pSpawns[s_ubSpawnCount++] = (tUwCoordYX){
					.uwX = ubX * MAP_TILE_SIZE + (MAP_TILE_SIZE / 2),
					.uwY = ubY * MAP_TILE_SIZE + (MAP_TILE_SIZE / 2)
				};
			}

			// 3d effect
			if (
				ubY > 0 && s_pTilesXy[ubX][ubY] == TILE_VOID &&
				s_pTilesXy[ubX][ubY - 1] == TILE_FLOOR_1
			) {
				s_pTilesXy[ubX][ubY] = TILE_WALL_1;
			}
		}
	}

	logWrite("Loaded %hhu spawn points\n", s_ubSpawnCount);
}

void tileShuffleSpawns(void) {
	for(UBYTE ubShuffle = 0; ubShuffle < 50; ++ubShuffle) {
		UBYTE ubA = randUwMax(&g_sRandManager, s_ubSpawnCount);
		UBYTE ubB = randUwMax(&g_sRandManager, s_ubSpawnCount);

		tUwCoordYX sTmp = {.ulYX = s_pSpawns[ubA].ulYX};
		s_pSpawns[ubA].ulYX = s_pSpawns[ubB].ulYX;
		s_pSpawns[ubB].ulYX = sTmp.ulYX;
	}
}

const tUwCoordYX *tileGetSpawn(UBYTE ubIndex) {
	return &s_pSpawns[ubIndex];
}

void tilesDrawOn(tBitMap *pDestination) {
	for(UBYTE ubY = 0; ubY < TILE_HEIGHT; ++ubY) {
		for(UBYTE ubX = 0; ubX < TILE_WIDTH; ++ubX) {
			blitCopyAligned(
				g_pTileset, 0, s_pTilesXy[ubX][ubY] * 16,
				pDestination, ubX * 16, ubY * 16, 16, 16
			);
		}
	}
}

UBYTE tileIsSolid(UBYTE ubTileX, UBYTE ubTileY) {
	return s_pTilesXy[ubTileX][ubTileY] >= TILE_FLOOR_1;
}
