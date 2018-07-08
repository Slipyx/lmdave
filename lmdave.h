#ifndef _LMDAVE_H
#define _LMDAVE_H

#include <SDL.h>

// for dealing with resources in EXE
#include "util/util.h"

// global game state
typedef struct {
	uint8_t quit;
	uint8_t current_level;
	// view and scroll are per tile
	uint8_t view_x, view_y;
	int8_t scroll_x;

	level_t* levels; // grabbed from util, NUM_EXE_LEVELS count
} game_state_t;

// game assets
typedef struct {
	// tiles as textures converted from util's tile surfaces
	SDL_Texture* tile_tx[NUM_EXE_TILES];
} game_assets_t;

#endif
