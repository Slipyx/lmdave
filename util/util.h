// utility functions for dealing with resources in the dave.exe file
#ifndef _UTIL_H
#define _UTIL_H

#include <SDL.h>

// constant exe resource counts
#define NUM_EXE_TILES 158
#define NUM_EXE_LEVELS 10

// tiles
void LoadTiles();
SDL_Surface** GetTileSurfaces();
void SaveTiles();
void FreeTileSurfaces();

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

// levels
void LoadLevels();
level_t* GetLevels();
void SaveLevels();
void CreateWorldMap();

#endif

