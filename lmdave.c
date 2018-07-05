#include "lmdave.h"

// global game state
game_state_t* gs = NULL;
// global game assets
game_assets_t* g_assets = NULL;

// initialize a new game state
void init_game() {
	gs->quit = 1;
	gs->current_level = 1;
}

// initialize assets
void init_assets( SDL_Renderer* r ) {
}

// poll input
void check_input() {
}

// update game logic
void update_game() {
}

// draw to renderer
void render( SDL_Renderer* r ) {
}

// rendering scale
const uint8_t R_SCALE = 3;

int main( int argc, char** argv ) {
	gs = malloc( sizeof(game_state_t) );
	init_game();

	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;
	SDL_CreateWindowAndRenderer( 320 * R_SCALE, 200 * R_SCALE, 0, &window, &renderer );

	g_assets = malloc( sizeof(game_assets_t) );
	init_assets( renderer );

	// main loop
	while ( !gs->quit ) {
		check_input();
		update_game();
		render( renderer );
	}

	free( g_assets );
	free( gs );
}

