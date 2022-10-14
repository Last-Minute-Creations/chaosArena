#include "tile.h"
#include <ace/types.h>
#include <ace/generic/screen.h>
#include <ace/managers/blit.h>
#include "assets.h"

#define TILE_WIDTH (SCREEN_PAL_WIDTH / TILE_SIZE)
#define TILE_HEIGHT (SCREEN_PAL_HEIGHT / TILE_SIZE)

typedef enum tTile {
	TILE_VOID,
	TILE_WALL_1,
	TILE_FLOOR_1,
	TILE_COUNT
} tTile;

static tTile s_pTilesXy[TILE_WIDTH][TILE_HEIGHT];

/**
 * @brief The easy to read tile layout.
 * Note that indices are reversed here, but the ascii is in correct order.
 */
static const char s_pMapPatternYx[TILE_HEIGHT][TILE_WIDTH + 1] = {
	"....................",
	"....................",
	".##################.",
	".##################.",
	".####...####...####.",
	".####...####...####.",
	".####...####...####.",
	".##################.",
	".##################.",
	".####...####...####.",
	".####...####...####.",
	".####...####...####.",
	".##################.",
	".##################.",
	"....................",
	"...................."
};

//------------------------------------------------------------------- PUBLIC FNS

void tilesInit(void) {
	for(UBYTE ubY = 0; ubY < TILE_HEIGHT; ++ubY) {
		for(UBYTE ubX = 0; ubX < TILE_WIDTH; ++ubX) {
			s_pTilesXy[ubX][ubY] = (
				s_pMapPatternYx[ubY][ubX] == '#' ? TILE_FLOOR_1 : TILE_VOID
			);

			// 3d effect
			if (
				ubY > 0 && s_pTilesXy[ubX][ubY] == TILE_VOID &&
				s_pTilesXy[ubX][ubY - 1] == TILE_FLOOR_1
			) {
				s_pTilesXy[ubX][ubY] = TILE_WALL_1;
			}
		}
	}
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
