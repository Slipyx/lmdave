#ifndef __SOUND_H
#define __SOUND_H

#include <SDL.h>
#include <SDL/SDL_mixer.h>

// sfx names
typedef enum {
	SFX_JUMP,
	SFX_PICKUP,
	NUM_SFX
} SFX_NAME;

// custom chunk loader for emscripten support
Mix_Chunk* S_LoadChunk( const char* f );

#endif

