#include "game.h"
#include "world.h"

// global game state
game_state_t* gs = NULL;
// global game assets
game_assets_t* g_assets = NULL;

// initialize a new game state
static void G_Init() {
	gs = malloc( sizeof(game_state_t) );
	memset( gs, 0, sizeof(game_state_t) );

	// clean player state
	memset( &gs->ps, 0, sizeof(player_state_t) );
}

// initialize game assets
static void G_InitAssets( SDL_Renderer* r ) {
	// load resources from DAVE.EXE
	Util_LoadTiles();
	Util_LoadLevels();

	g_assets = malloc( sizeof(game_assets_t) );
	memset( g_assets, 0, sizeof(game_assets_t) );

	// copy all levels loaded from exe by util
	for ( int i = 0; i < NUM_EXE_LEVELS; ++i )
		Util_GetLevel( i, &gs->levels[i] );

	// get loaded tile surface array from util lib
	SDL_Surface** tileSfcs = Util_GetTileSurfaces();

	// convert each surface from util's array to g_asset texture array
	for ( int i = 0; i < NUM_EXE_TILES; ++i ) {
		// mask dave sprites
		if ( (i >= 53 && i <= 59) || (i >= 67 && i <= 68)
			 || (i >= 71 && i <= 73) || (i >= 77 && i <= 82) )
			SDL_SetColorKey( tileSfcs[i], 1, SDL_MapRGB( tileSfcs[i]->format, 0, 0, 0 ) );
		g_assets->tile_tx[i] = SDL_CreateTextureFromSurface( r, tileSfcs[i] );
	}

	// tile surfaces should be converted as textures inside g_assets now
	Util_FreeTileSurfaces();

	// sfx
	//g_assets->sfx[0] = S_LoadChunk( "res/jumpd8.wav" ); //Mix_LoadWAV_RW( rw, 1 );
	//g_assets->sfx[1] = S_LoadChunk( "res/pickupd8.wav" ); //Mix_LoadWAV_RW( rw, 1 );
}

// poll input
static void G_CheckInput() {
	SDL_Event ev;

	while ( SDL_PollEvent( &ev ) ) {

		// events
		switch ( ev.type ) {

		case SDL_QUIT: gs->quit = 1; break;

		case SDL_KEYDOWN: {
			if ( ev.key.repeat ) break;
			SDL_Keycode key = ev.key.keysym.sym;
			// android back
			if ( key == SDLK_AC_BACK ) { gs->quit = 1; }
			// jump event
			if ( key == SDLK_UP || key == SDLK_z ) { gs->ps.try_jump = 1; }
			// fire
			if ( key == SDLK_LCTRL || key == SDLK_x ) { gs->ps.try_fire = 1; }
			// jetpack
			if ( key == SDLK_LALT || key == SDLK_c ) { gs->ps.try_jetpack = 1; }
			// level select
			if ( key >= SDLK_0 && key <= SDLK_9 ) {
				gs->current_level = key - SDLK_0;
				W_ResetLevel();
			}
		} break;

		default: break;
		} // switch
	}

	// real-time keystate
	const uint8_t* keys = SDL_GetKeyboardState( NULL );

	// attempt dave movement by setting try_ flags
	if ( keys[SDL_SCANCODE_RIGHT] ) gs->ps.try_right = 1;
	if ( keys[SDL_SCANCODE_LEFT] ) gs->ps.try_left = 1;
	if ( keys[SDL_SCANCODE_DOWN] ) gs->ps.try_down = 1;
	if ( keys[SDL_SCANCODE_UP] ) gs->ps.try_up = 1;
}

// clear all try input flags at end of frame
static void G_ClearInput() {
	gs->ps.try_left = 0;
	gs->ps.try_right = 0;
	gs->ps.try_jump = 0;
	gs->ps.try_fire = 0;
	gs->ps.try_jetpack = 0;
	gs->ps.try_up = 0;
	gs->ps.try_down = 0;
}

// main update routine
static void G_Update() {
	// update collision point flags
	P_UpdateCollision();
	// pickups
	P_PickupItem();
	// player bullet
	P_UpdateBullet();
	// verify input try flags
	P_VerifyInput();
	// apply player movement
	P_Move();
	// game view scrolling
	W_ScrollView();
	// player gravity
	P_ApplyGravity();
	// update level-wide state
	W_Update();
	// reset input flags
	G_ClearInput();
}

// draw level at current view
void Draw_World( SDL_Renderer* r ) {
	SDL_Rect dst = {0,0,320,200};
	// solid BG fill
	SDL_RenderCopy( r, g_assets->tile_tx[0], NULL, &dst );
	// draw level view 20x10 tiles at 16x16px offset one tile in y
	for ( int j = 0; j < 10; ++j ) {
		dst.y = TILE_SIZE + j * TILE_SIZE;
		dst.w = TILE_SIZE; dst.h = TILE_SIZE;
		for ( int i = 0; i < 20; ++i ) {
			dst.x = i * TILE_SIZE;
			uint8_t til = gs->levels[gs->current_level].tiles[j * 100 + gs->view_x + i];
			if ( til == 0 ) continue;
			SDL_RenderCopy( r, g_assets->tile_tx[til], NULL, &dst );
		}
	}
}

// draw player
void Draw_Player( SDL_Renderer* r ) {
	SDL_Rect dst;
	// relative to view
	dst.x = gs->ps.px - gs->view_x * TILE_SIZE;
	dst.y = TILE_SIZE + gs->ps.py;
	// tile 56 neutral, 53 right, 57 left 20x16px
	uint8_t til = 56;
	dst.w = 20; dst.h = 16;

	// jetpack tile
	if ( gs->ps.do_jetpack )
		til = gs->ps.last_dir >= 0 ? 77 : 80;
	// grounded walk tile
	else if ( gs->ps.on_ground ) {
		til = gs->ps.last_dir >= 0 ? 53 : 57;
	}
	// jump tile
	else if ( gs->ps.do_jump || !gs->ps.on_ground )
		til = gs->ps.last_dir >= 0 ? 67 : 68;

	// render
	// grounded debug
	if ( gs->ps.on_ground ) {
		SDL_SetRenderDrawColor( r, 255, 0, 255, 255 );
		SDL_RenderDrawLine( r, dst.x, dst.y+dst.h, dst.x+dst.w, dst.y+dst.h );
	}
	SDL_RenderCopy( r, g_assets->tile_tx[til], NULL, &dst );
}

// draw player bullet
void Draw_Bullet( SDL_Renderer* r ) {
	SDL_Rect dst;
	if ( gs->ps.bullet_px && gs->ps.bullet_py ) {
		// relative to view
		dst.x = gs->ps.bullet_px - gs->view_x * TILE_SIZE;
		dst.y = TILE_SIZE + gs->ps.bullet_py;
		// tile 127 right, 128 left
		uint8_t til = gs->ps.bullet_dir > 0 ? 127 : 128;
		dst.w = 12; dst.h = 3;

		// render
		SDL_RenderCopy( r, g_assets->tile_tx[til], NULL, &dst );
	}
}

// main drawing routine
static void G_Draw( SDL_Renderer* r ) {
	// clear backbuffer
	SDL_SetRenderDrawColor( r, 0, 40, 80, 0xff );
	SDL_RenderClear( r );

	Draw_World( r );
	Draw_Player( r );
	Draw_Bullet( r );

	// flip buffers
	SDL_RenderPresent( r );
}

// main loop function for emscripten support
#ifdef __EMSCRIPTEN__
#include <emscripten.h>

static void emloop( void* p ) {
	SDL_Renderer* r = (SDL_Renderer*)p;
	G_CheckInput();
	G_Update();
	G_Draw( r );
}
#endif

int main( int argc, char** argv ) {
	// initialize SDL
	if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) )
		SDL_Log( "SDL Init error: %s\n", SDL_GetError() );
	// mixer
	//int res = Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 1024 );
	//if ( res < 0 ) fprintf( stderr, "Mix_OpenAudio error: %s\n", Mix_GetError() );
	//Mix_AllocateChannels( 8 );

	// create window and renderer
	int winw = 1280, winh = 720;
	// emscripten canvas size
#ifdef __EMSCRIPTEN__
	winw = 1024; winh = 576;
#endif
	SDL_Window* window = SDL_CreateWindow( "lmdave", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		winw, winh, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL );
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
	// scaling hint
	//SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "linear" );

	// initialize game state and assets
	G_Init();
	G_InitAssets( renderer );

	// clear initial frame
	SDL_SetRenderDrawColor( renderer, 0, 40, 80, 0xff );
	SDL_RenderClear( renderer );

	// set state for first level
	W_StartLevel();
	// start bg music
	//Mix_PlayMusic( g_assets->mus, -1 );

	// main loop 
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop_arg( emloop, renderer, 1000 / FRAME_DELAY, 1 );
	//emscripten_set_main_loop_timing( EM_TIMING_RAF, 2 ); // vsync/2
#else
	while ( gs->quit == 0 ) {
		// fixed timestep
		uint32_t st = SDL_GetTicks();

		G_CheckInput();
		G_Update();
		G_Draw( renderer );

		uint32_t delay = FRAME_DELAY - (SDL_GetTicks() - st);
		delay = delay > FRAME_DELAY ? 0 : delay;
		SDL_Delay( delay );
	}
#endif
	// destroy each tile texture
	for ( int i = 0; i < NUM_EXE_TILES; ++i )
		SDL_DestroyTexture( g_assets->tile_tx[i] );
	free( g_assets );
	free( gs );
	// free audio
	//for ( int i = 0; i < sizeof(g_assets->sfx) / sizeof(g_assets->sfx[0]) )
		//Mix_FreeChunk( g_assets->sfx[i] );
	//Mix_FreeMusic( g_assets->mus );
	//Mix_CloseAudio();

	// cleanup SDL
	SDL_DestroyRenderer( renderer );
	SDL_DestroyWindow( window );
	SDL_Quit();

	// an explicit exit for android
#if defined( __ANDROID__ ) || defined( ANDROID )
	exit( EXIT_SUCCESS );
#endif

	return EXIT_SUCCESS;
}

