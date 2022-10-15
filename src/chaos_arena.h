/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INCLDE_CHAOS_ARENA_H
#define INCLDE_CHAOS_ARENA_H

#include <ace/managers/rand.h>
#include <ace/managers/state.h>

extern tStateManager *g_pStateMachineGame;
extern tRandManager g_sRandManager;

extern tState g_sStateMenu;
extern tState g_sStateGame;

#endif // INCLDE_CHAOS_ARENA_H
