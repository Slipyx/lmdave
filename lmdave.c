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

	// dave state at level 1;
	memset( &gs->ds, 0, sizeof(dave_state_t) );
	gs->ds.tx = 2;
	gs->ds.ty = 8;
	gs->ds.px = gs->ds.tx * TILE_SIZE;
	gs->ds.py = gs->ds.ty * TILE_SIZE;
	gs->ds.jump_timer = 0;
	gs->ds.on_ground = 0;

	// returns array of NUM_EXE_LEVELS that was loaded by the util lib
	gs->levels = GetLevels();
}

// initialize game assets
void init_assets( SDL_Renderer* r ) {
	// get loaded tile surface array from util lib
	SDL_Surface** tileSfcs = GetTileSurfaces();

	// convert each surface from util's array to g_asset texture array
	for ( int i = 0; i < NUM_EXE_TILES; ++i ) {
		// mask dave sprites
		if ( (i >= 53 && i <= 59) || (i >= 67 && i <= 68)
			 || (i >= 71 && i <= 73) || (i >= 77 && i <= 82) )
			SDL_SetColorKey( tileSfcs[i], 1, SDL_MapRGB( tileSfcs[i]->format, 0, 0, 0 ) );
		g_assets->tile_tx[i] = SDL_CreateTextureFromSurface( r, tileSfcs[i] );
	}
}

// poll input
void check_input() {
	SDL_Event ev;
	SDL_PollEvent( &ev );
	// real-time keystate
	const uint8_t* keys = SDL_GetKeyboardState( NULL );

	// attempt dave movement by setting try_ flags
	if ( keys[SDL_SCANCODE_RIGHT] ) gs->ds.try_right = 1;
	if ( keys[SDL_SCANCODE_LEFT] ) gs->ds.try_left = 1;
	if ( keys[SDL_SCANCODE_UP] ) gs->ds.try_jump = 1;

	// events
	switch ( ev.type ) {

	case SDL_QUIT: gs->quit = 1; break;

	case SDL_KEYDOWN: {
		if ( ev.key.repeat ) break;
		SDL_Keycode key = ev.key.keysym.sym;
		// android back
		if ( key == SDLK_AC_BACK ) gs->quit = 1;
	}
	} // switch
}

// clear all try input flags at end of frame
void clear_input() {
	gs->ds.try_left = 0;
	gs->ds.try_right = 0;
	gs->ds.try_jump = 0;
}

// remove item from world
void pickup_item( uint8_t tx, uint8_t ty ) {
	if ( !tx || !ty ) return;

	uint8_t til = gs->levels[gs->current_level].tiles[ty * 100 + tx];
	// pickup functionality here
	(void)til;

	// remove
	gs->levels[gs->current_level].tiles[ty * 100 + tx] = 0;
	gs->ds.check_pickup_x = 0;
	gs->ds.check_pickup_y = 0;
}

// returns 1 if passed pixel point is not within a solid tile
uint8_t is_clear( uint16_t px, uint16_t py ) {
	uint8_t tx, ty; // tile pos
	uint8_t til; // tile index

	// pixel point to tile pos
	tx = px / TILE_SIZE; ty = py / TILE_SIZE;
	// tile index at level's tx, ty pos
	til = gs->levels[gs->current_level].tiles[ty * 100 + tx];

	// solid tiles
	if ( til == 1 || til == 3 || til == 5 ) return 0;
	if ( til >= 15 && til <= 19 ) return 0;
	if ( til >= 21 && til <= 24 ) return 0;
	if ( til >= 29 && til <= 30 ) return 0;

	// pickups
	if ( til == 10 || (til >= 47 && til <= 52) ) {
		gs->ds.check_pickup_x = tx;
		gs->ds.check_pickup_y = ty;
	}

	return 1;
}

// update collision point clear flags
void check_collision() {
	// 8 points of collision; relative to top left of tile 56 neutral frame (20x16)
	// 0, 1 = top left, top right, above sprite
	gs->ds.col_point[0] = is_clear( gs->ds.px + 4, gs->ds.py - 0 );
	gs->ds.col_point[1] = is_clear( gs->ds.px + 10, gs->ds.py - 0 );
	// 2, 3 = right edge
	gs->ds.col_point[2] = is_clear( gs->ds.px + 12, gs->ds.py + 2 );
	gs->ds.col_point[3] = is_clear( gs->ds.px + 12, gs->ds.py + 14 );
	// 4, 5 = bottom edge
	gs->ds.col_point[4] = is_clear( gs->ds.px + 10, gs->ds.py + 16 );
	gs->ds.col_point[5] = is_clear( gs->ds.px + 4, gs->ds.py + 16 );
	// 6, 7 = left edge
	gs->ds.col_point[6] = is_clear( gs->ds.px + 2, gs->ds.py + 14 );
	gs->ds.col_point[7] = is_clear( gs->ds.px + 2, gs->ds.py + 2 );
	// update on_ground flag if a bottom point (4,5) is not clear
	gs->ds.on_ground = (!gs->ds.col_point[4] || !gs->ds.col_point[5]);
}

// validate input whose try flags were set
void verify_input() {
	// right; col points 2, 3
	if ( gs->ds.try_right && gs->ds.col_point[2] && gs->ds.col_point[3] ) {
		gs->ds.do_right = 1;
	}
	// left; col points 6, 7
	if ( gs->ds.try_left && gs->ds.col_point[6] && gs->ds.col_point[7] ) {
		gs->ds.do_left = 1;
	}
	// jump; on_ground and col points 0, 1
	if ( gs->ds.try_jump && gs->ds.on_ground && !gs->ds.do_jump
		 && (gs->ds.col_point[0] && gs->ds.col_point[1]) ) {
		gs->ds.do_jump = 1;
	}
}

// apply validated dave movement
void move_dave() {
	// update dave's tile pos
	gs->ds.tx = gs->ds.px / TILE_SIZE;
	gs->ds.ty = gs->ds.py / TILE_SIZE;

	if ( gs->ds.do_right ) {
		gs->ds.px += 2;
		gs->ds.do_right = 0;
	}
	if ( gs->ds.do_left ) {
		gs->ds.px -= 2;
		gs->ds.do_left = 0;
	}
	if ( gs->ds.do_jump ) {
		if ( !gs->ds.jump_timer )
			gs->ds.jump_timer = 19;

		if ( gs->ds.col_point[0] && gs->ds.col_point[1] ) {
			if ( gs->ds.jump_timer > 5 ) gs->ds.py -= 2;
			else gs->ds.py -= 1;
			gs->ds.jump_timer--;
		} else {
			gs->ds.jump_timer = 0;
		}

		if ( !gs->ds.jump_timer )
			gs->ds.do_jump = 0;
	}
}

// apply gravity to dave
void apply_gravity() {
	if ( !gs->ds.do_jump && !gs->ds.on_ground ) {
		// check below sprite
		if ( is_clear( gs->ds.px + 4, gs->ds.py + 17 ) && is_clear( gs->ds.px + 10, gs->ds.py + 17 ) )
			gs->ds.py += 2;
		else { // align to tile
			uint8_t not_align = gs->ds.py % TILE_SIZE;
			if ( not_align ) {
				gs->ds.py = not_align < (TILE_SIZE / 2) ?
					gs->ds.py - not_align : gs->ds.py + TILE_SIZE - not_align;
			}
		}
	}
}

// update game view based on set scroll values
void scroll_screen() {
	if ( gs->scroll_x > 0 ) {
		if ( gs->view_x == 80 ) gs->scroll_x = 0;
		else { gs->view_x++; gs->scroll_x--; }
	}
	if ( gs->scroll_x < 0 ) {
		if ( gs->view_x == 0 ) gs->scroll_x = 0;
		else { gs->view_x--; gs->scroll_x++; }
	}
}

// update game logic
void update_game() {
	/*if ( gs->current_level == 0xff ) gs->current_level = 0;
	if ( gs->current_level > NUM_EXE_LEVELS - 1 )
		gs->current_level = NUM_EXE_LEVELS - 1;*/

	// update collision point flags
	check_collision();
	// pickups
	pickup_item( gs->ds.check_pickup_x, gs->ds.check_pickup_y );
	// verify input try flags
	verify_input();
	// apply dave movement
	move_dave();
	// game view scrolling
	scroll_screen();
	apply_gravity();
	// reset input flags
	clear_input();
}

// draw level at current view
void draw_world( SDL_Renderer* r ) {
	SDL_Rect dst;
	// draw level view 20x10 tiles at 16x16px
	for ( int j = 0; j < 10; ++j ) {
		dst.y = j * TILE_SIZE;
		dst.w = TILE_SIZE; dst.h = TILE_SIZE;
		for ( int i = 0; i < 20; ++i ) {
			dst.x = i * TILE_SIZE;
			uint8_t til = gs->levels[gs->current_level].tiles[j * 100 + gs->view_x + i];
			SDL_RenderCopy( r, g_assets->tile_tx[til], NULL, &dst );
		}
	}
}

// draw dave
void draw_dave( SDL_Renderer* r ) {
	SDL_Rect dst;
	dst.x = gs->ds.px;
	dst.y = gs->ds.py;
	// tile 56 neutral; 20x16px
	dst.w = 20; dst.h = 16;

	// render
	// grounded debug
	/*if ( gs->ds.on_ground ) {
		SDL_SetRenderDrawColor( r, 255, 0, 255, 255 );
		SDL_RenderDrawRect( r, &dst );
	}*/
	SDL_RenderCopy( r, g_assets->tile_tx[53], NULL, &dst );
}

// draw to renderer
void render( SDL_Renderer* r ) {
	// clear backbuffer
	SDL_SetRenderDrawColor( r, 0, 40, 80, 0xff );
	SDL_RenderClear( r );

	draw_world( r );
	draw_dave( r );

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
	SDL_SetRenderDrawColor( renderer, 0, 40, 80, 0xff );
	SDL_RenderClear( renderer );

	// main loop
	while ( !gs->quit ) {
		// fixed timestep
		uint32_t st = SDL_GetTicks();

		check_input();
		update_game();
		render( renderer );

		//uint32_t et = SDL_GetTicks();
		uint32_t delay = FRAME_DELAY - (SDL_GetTicks() - st);
		delay = delay > FRAME_DELAY ? 0 : delay;
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

	// an explicit exit for android
#ifdef __ANDROID__
	exit( EXIT_SUCCESS );
#endif

	return EXIT_SUCCESS;
}
