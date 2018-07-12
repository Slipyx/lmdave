#ifndef _LMDAVE_H
#define _LMDAVE_H

#include <SDL.h>

// for dealing with resources in EXE
#include "util/util.h"

// dave player state
typedef struct {
	uint8_t tx, ty; // tile pos
	uint16_t px, py; // pixel pos

	// input flags
	uint8_t try_right;
	uint8_t try_left;
	uint8_t try_jump;

	uint8_t do_right;
	uint8_t do_left;
	uint8_t do_jump;
} dave_state_t;

// global game state
typedef struct {
	uint8_t quit;
	uint8_t current_level;
	// view and scroll are per tile
	uint8_t view_x, view_y;
	int8_t scroll_x;

	// dave player state
	dave_state_t ds;

	level_t* levels; // grabbed from util, NUM_EXE_LEVELS count
} game_state_t;

// game assets
typedef struct {
	// tiles as textures converted from util's tile surfaces
	SDL_Texture* tile_tx[NUM_EXE_TILES];
} game_assets_t;

// level tile size in pixels
#define TILE_SIZE 16

#endif
