/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INCLUDE_DISPLAY_H
#define INCLUDE_DISPLAY_H

#include <ace/generic/screen.h>
#include <ace/managers/viewport/simplebuffer.h>

#define DISPLAY_BPP 4
#define DISPLAY_TILE_MARGIN 2
#define DISPLAY_WIDTH (SCREEN_PAL_WIDTH + 2 * DISPLAY_TILE_MARGIN * 16)
#define DISPLAY_HEIGHT (SCREEN_PAL_HEIGHT + 2 * DISPLAY_TILE_MARGIN * 16)

void displayCreate(void);

void displayDestroy(void);

void displayProcess(void);

void displayOn(void);

void displayOff(void);

tSimpleBufferManager *displayGetManager(void);

#endif // INCLUDE_DISPLAY_H
