#ifndef _LMDAVE_H
#define _LMDAVE_H

#include <SDL.h>
//#include <SDL/SDL_mixer.h>

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

	// enemy bullet
	uint16_t ebullet_px, ebullet_py;
	int8_t ebullet_dir;

	level_t levels[NUM_EXE_LEVELS]; // copied from exe util's GetLevel
} game_state_t;

// game assets
typedef struct {
	// tiles as textures converted from util's tile surfaces
	SDL_Texture* tile_tx[NUM_EXE_TILES];
	// sfx
	//Mix_Chunk* sfx[2];
	// music
	//Mix_Music* mus;
} game_assets_t;

// level tile size in pixels
#define TILE_SIZE 16
// fixed frame delay
#define FRAME_DELAY 33

#endif
