#include "lmdave.h"

// global game state
game_state_t* gs = NULL;
// global game assets
game_assets_t* g_assets = NULL;

// initialize a new game state
void init_game() {
	gs->quit = 0;
	gs->current_level = 0;
	gs->view_x = 0;
	gs->view_y = 0;
	gs->scroll_x = 0;

	// returns array of NUM_EXE_LEVELS that was loaded by the util lib
	gs->levels = GetLevels();
}

// initialize game assets
void init_assets( SDL_Renderer* r ) {
	// get loaded tile surface array from util lib
	SDL_Surface** tileSfcs = GetTileSurfaces();

	// convert each surface from util's array to g_asset texture array
	for ( int i = 0; i < NUM_EXE_TILES; ++i ) {
		g_assets->tile_tx[i] = SDL_CreateTextureFromSurface( r, tileSfcs[i] );
	}
}

// poll input
void check_input() {
	SDL_Event ev;
	SDL_PollEvent( &ev );

	if ( ev.type == SDL_QUIT ) gs->quit = 1;

	if ( ev.type == SDL_KEYDOWN ) {
		switch ( ev.key.keysym.sym ) {
		case SDLK_RIGHT:
			gs->scroll_x = 15; break;
		case SDLK_LEFT:
			gs->scroll_x = -15; break;
		case SDLK_DOWN:
			gs->current_level++; break;
		case SDLK_UP:
			gs->current_level--; break;
		}
	}
}

// update game logic
void update_game() {
	if ( gs->current_level == 0xff ) gs->current_level = 0;
	if ( gs->current_level > NUM_EXE_LEVELS - 1 )
		gs->current_level = NUM_EXE_LEVELS - 1;

	// scrolling
	if ( gs->scroll_x > 0 ) {
		if ( gs->view_x == 80 ) gs->scroll_x = 0;
		else { gs->view_x++; gs->scroll_x--; }
	}
	if ( gs->scroll_x < 0 ) {
		if ( gs->view_x == 0 ) gs->scroll_x = 0;
		else { gs->view_x--; gs->scroll_x++; }
	}
}

// draw to renderer
void render( SDL_Renderer* r ) {
	// clear backbuffer
	SDL_RenderClear( r );

	SDL_Rect dst;
	// draw level 20x10 tiles at 16x16px
	for ( int j = 0; j < 10; ++j ) {
		dst.y = j * 16;
		dst.w = 16; dst.h = 16;
		for ( int i = 0; i < 20; ++i ) {
			dst.x = i * 16;
			uint8_t til = gs->levels[gs->current_level].tiles[j * 100 + gs->view_x + i];
			SDL_RenderCopy( r, g_assets->tile_tx[til], NULL, &dst );
		}
	}

	// flip buffers
	SDL_RenderPresent( r );
}

// rendering scale
//const uint8_t R_SCALE = 3;

int main( int argc, char** argv ) {
	// load resources from DAVE.EXE
	LoadTiles();
	LoadLevels();

	gs = malloc( sizeof(game_state_t) );
	init_game();

	if ( SDL_Init( SDL_INIT_VIDEO ) )
		SDL_Log( "SDL Init error: %s\n", SDL_GetError() );

	// SDL_Window* window = NULL;
	// SDL_Renderer* renderer = NULL;
	// if ( SDL_CreateWindowAndRenderer( 960, 720, SDL_WINDOW_RESIZABLE, &window, &renderer ) )
	// 	SDL_Log( "SDL WindowAndRenderer error: %s\n", SDL_GetError() );

	// create window and renderer
	SDL_Window* window = SDL_CreateWindow( "lmdave", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL );
	if ( window == NULL ) {
		SDL_Log( "SDL_CreateWindow error: %s\n", SDL_GetError() );
		SDL_Quit(); return EXIT_FAILURE;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED
		| SDL_RENDERER_PRESENTVSYNC );
	if ( renderer == NULL ) {
		SDL_DestroyWindow( window );
		SDL_Log( "SDL_CreateRenderer error: %s\n", SDL_GetError() );
		SDL_Quit(); return EXIT_FAILURE;
	}

	// set internal rendering size
	SDL_RenderSetLogicalSize( renderer, 320, 200 );
	//SDL_RenderSetScale( renderer, R_SCALE, R_SCALE );
	// scaling hint
	//SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "linear" );

	g_assets = malloc( sizeof(game_assets_t) );
	init_assets( renderer );
	// tile surfaces should be converted as textures inside g_assets now
	FreeTileSurfaces();

	// clear initial frame
	SDL_SetRenderDrawColor( renderer, 0, 0, 0, 0xff );
	SDL_RenderClear( renderer );

	// main loop
	while ( !gs->quit ) {
		// fixed timestep
		uint32_t st = SDL_GetTicks();

		check_input();
		update_game();
		render( renderer );

		//uint32_t et = SDL_GetTicks();
		uint32_t delay = 16 - (SDL_GetTicks() - st);
		delay = delay > 16 ? 0 : delay;
		SDL_Delay( delay );
	}

	// destroy each tile texture
	for ( int i = 0; i < NUM_EXE_TILES; ++i )
		SDL_DestroyTexture( g_assets->tile_tx[i] );
	free( g_assets );
	free( gs );

	// cleanup SDL
	SDL_DestroyRenderer( renderer );
	SDL_DestroyWindow( window );
	SDL_Quit();

	return EXIT_SUCCESS;
}
