#ifndef _LMDAVE_H
#define _LMDAVE_H

#include <SDL.h>

// for dealing with resources in EXE
#include "util/util.h"

#include "player.h"

// global game state
typedef struct {
	uint8_t quit;
	uint8_t current_level;
	// view and scroll are per tile
	uint8_t view_x, view_y;
	int8_t scroll_x;

	// player state
	player_state_t ps;

	level_t levels[NUM_EXE_LEVELS]; // copied from exe util's GetLevel
} game_state_t;

// game assets
typedef struct {
	// tiles as textures converted from util's tile surfaces
	SDL_Texture* tile_tx[NUM_EXE_TILES];
} game_assets_t;

// level tile size in pixels
#define TILE_SIZE 16
// fixed frame delay
#define FRAME_DELAY 33

#endif
