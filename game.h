#ifndef _LMDAVE_H
#define _LMDAVE_H

#include <SDL.h>

// for dealing with resources in EXE
#include "util/util.h"

// dave player state
typedef struct {
	uint8_t tx, ty; // tile pos
	uint16_t px, py; // pixel pos
	uint16_t score;
	uint8_t lives;
	// on ground flag
	uint8_t on_ground;

	// input flags
	uint8_t try_right;
	uint8_t try_left;
	uint8_t try_jump;
	uint8_t try_fire;
	uint8_t try_jetpack;
	uint8_t try_up, try_down; // jetpack/climbing

	uint8_t do_right;
	uint8_t do_left;
	uint8_t do_jump;
	uint8_t do_fire;
	uint8_t do_jetpack;
	uint8_t do_up, do_down;

	uint8_t jump_timer;
	// pickup tile pos
	uint8_t check_pickup_x;
	uint8_t check_pickup_y;
	// door was hit, check if passable
	uint8_t check_door;
	// item flags; jetpack is also fuel count
	uint8_t trophy, gun, jetpack;

	// collision point clear flags; 1 = clear
	uint8_t col_point[8];
} player_state_t;

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
const uint8_t TILE_SIZE = 16;
// fixed frame delay
const uint8_t FRAME_DELAY = 33;

#endif

