/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INCLUDE_TILE_H
#define INCLUDE_TILE_H

#include <ace/utils/bitmap.h>

#define MAP_TILE_SIZE 16

void tilesInit(void);

void tilesDrawOn(tBitMap *pDestination);

UBYTE tileIsSolid(UBYTE ubTileX, UBYTE ubTileY);

#endif // INCLUDE_TILE_H
