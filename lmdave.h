#ifndef _LMDAVE_H
#define _LMDAVE_H

#include <SDL.h>

// level structure
// byte[256] path, two signed 8bit relative movement, 0xea 0xea for end
// byte[100x10] tile index data, 100 by 10, so more than one level could fit in a chunk
// byte[24] unsed padding
// note: player start and monster starts are hardcoded
typedef struct {
	int8_t path[256];
	uint8_t tiles[1000];
	uint8_t pad[24];
} level_t;

// global game state
typedef struct {
	uint8_t quit;
	uint8_t current_level;
	uint8_t view_x, view_y;

	level_t levels[10];
} game_state_t;

// game assets
typedef struct {
	SDL_Texture* tile_tx[158];
} game_assets_t;

#endif

