#include "game.h"

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
}

// sets new beginning state based on current level
void W_StartLevel() {
	gs->view_x = 0;
	gs->view_y = 0;
	gs->scroll_x = 0;

	gs->ps.jump_timer = 0;
	gs->ps.on_ground = 0;
	gs->ps.do_jump = 0;

	// hardcoded player starts
	switch ( gs->current_level ) {
	case 0: case 4: case 5: case 7: case 9: gs->ps.tx = 2; gs->ps.ty = 8; break;
	case 1: gs->ps.tx = 1; gs->ps.ty = 8; break;
	case 2: gs->ps.tx = 2; gs->ps.ty = 5; break;
	case 3: gs->ps.tx = 1; gs->ps.ty = 5; break;
	case 6: gs->ps.tx = 1; gs->ps.ty = 2; break;
	case 8: gs->ps.tx = 6; gs->ps.ty = 1; break;
	default: break;
	}

	gs->ps.px = gs->ps.tx * TILE_SIZE;
	gs->ps.py = gs->ps.ty * TILE_SIZE;
	// reset items
	gs->ps.gun = 0;
	gs->ps.jetpack = 0;
	gs->ps.trophy = 0;
	gs->ps.check_door = 0;
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
		} break;

		default: break;
		} // switch
	}

	// real-time keystate
	const uint8_t* keys = SDL_GetKeyboardState( NULL );

	// attempt dave movement by setting try_ flags
	if ( keys[SDL_SCANCODE_RIGHT] ) gs->ps.try_right = 1;
	if ( keys[SDL_SCANCODE_LEFT] ) gs->ps.try_left = 1;
	//if ( keys[SDL_SCANCODE_UP] ) gs->ps.try_jump = 1;
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

// pickup functionality and remove from world
void P_PickupItem() {
	uint8_t tx = gs->ps.check_pickup_x;
	uint8_t ty = gs->ps.check_pickup_y;

	if ( !tx || !ty ) return;

	uint8_t til = gs->levels[gs->current_level].tiles[ty * 100 + tx];

	// pickup functionality here
	if ( til == 4 ) gs->ps.jetpack = 0xff;
	if ( til == 10 ) {
		gs->ps.score += 1000;
		gs->ps.trophy = 1;
	}
	if ( til == 20 ) gs->ps.gun = 1;

	// remove
	gs->levels[gs->current_level].tiles[ty * 100 + tx] = 0;
	gs->ps.check_pickup_x = 0;
	gs->ps.check_pickup_y = 0;
}

// returns 1 if passed pixel point is not within a solid tile
uint8_t W_IsClear( uint16_t px, uint16_t py ) {
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
	if ( til == 10 || til == 4 || til == 20 || (til >= 47 && til <= 52) ) {
		gs->ps.check_pickup_x = tx;
		gs->ps.check_pickup_y = ty;
	}

	// door
	if ( til == 2 ) {
		gs->ps.check_door = 1;
	}

	return 1;
}

// update collision point clear flags
void P_UpdateCollision() {
	// 8 points of collision; relative to top left of tile 56 neutral frame (20x16)
	// 0, 1 = top left, top right
	gs->ps.col_point[0] = W_IsClear( gs->ps.px + 4, gs->ps.py - 0 );
	gs->ps.col_point[1] = W_IsClear( gs->ps.px + 10, gs->ps.py - 0 );
	// 2, 3 = right edge
	gs->ps.col_point[2] = W_IsClear( gs->ps.px + 12, gs->ps.py + 2 );
	gs->ps.col_point[3] = W_IsClear( gs->ps.px + 12, gs->ps.py + 14 );
	// 4, 5 = bottom edge
	gs->ps.col_point[4] = W_IsClear( gs->ps.px + 10, gs->ps.py + 16 );
	gs->ps.col_point[5] = W_IsClear( gs->ps.px + 4, gs->ps.py + 16 );
	// 6, 7 = left edge
	gs->ps.col_point[6] = W_IsClear( gs->ps.px + 2, gs->ps.py + 14 );
	gs->ps.col_point[7] = W_IsClear( gs->ps.px + 2, gs->ps.py + 2 );
	// update on_ground flag if a bottom point (4,5) is not clear
	gs->ps.on_ground = (!gs->ps.col_point[4] || !gs->ps.col_point[5]);
}

// validate input whose try flags were set
void P_VerifyInput() {
	// right; col points 2, 3
	if ( gs->ps.try_right && gs->ps.col_point[2] && gs->ps.col_point[3] ) {
		gs->ps.do_right = 1;
	}
	// left; col points 6, 7
	if ( gs->ps.try_left && gs->ps.col_point[6] && gs->ps.col_point[7] ) {
		gs->ps.do_left = 1;
	}
	// jump; on_ground and col points 0, 1
	if ( gs->ps.try_jump && gs->ps.on_ground && !gs->ps.do_jump
		 && (gs->ps.col_point[0] && gs->ps.col_point[1]) ) {
		gs->ps.do_jump = 1;
	}
	// reset jump timer if contact a ground while still "jumping"
	if ( gs->ps.try_jump && gs->ps.on_ground && gs->ps.jump_timer )
		gs->ps.jump_timer = 0;
}

// apply validated player movement
void P_Move() {
	// update player's tile pos
	// sample x towards the center
	gs->ps.tx = (gs->ps.px + TILE_SIZE / 2) / TILE_SIZE;
	gs->ps.ty = gs->ps.py / TILE_SIZE;

	if ( gs->ps.do_right ) {
		gs->ps.px += 2;
		gs->ps.do_right = 0;
	}
	if ( gs->ps.do_left ) {
		gs->ps.px -= 2;
		gs->ps.do_left = 0;
	}
	if ( gs->ps.do_jump ) {
		if ( !gs->ps.jump_timer )
			gs->ps.jump_timer = 25;

		if ( gs->ps.col_point[0] && gs->ps.col_point[1] ) {
			if ( gs->ps.jump_timer > 12 )
				gs->ps.py -= 2;
			if ( gs->ps.jump_timer >= 7 && gs->ps.jump_timer <= 12 )
				gs->ps.py -= 1;
			gs->ps.jump_timer--;
		} else gs->ps.jump_timer = 0;

		//gs->ps.jump_timer--;

		if ( !gs->ps.jump_timer )
			gs->ps.do_jump = 0;
	}
}

// apply gravity to player
void P_ApplyGravity() {
	if ( !gs->ps.do_jump && !gs->ps.on_ground ) {
		// check below sprite
		if ( W_IsClear( gs->ps.px + 4, gs->ps.py + 17 ) && W_IsClear( gs->ps.px + 10, gs->ps.py + 17 ) )
			gs->ps.py += 2;
		else { // align to tile
			uint8_t not_align = gs->ps.py % TILE_SIZE;
			if ( not_align ) {
				gs->ps.py = not_align < (TILE_SIZE / 2) ?
					gs->ps.py - not_align : gs->ps.py + TILE_SIZE - not_align;
			}
		}
	}
}

// level-wide state update
void W_Update() {
	// check if at door and has trophy
	if ( gs->ps.check_door ) {
		if ( gs->ps.trophy ) {
			if ( gs->current_level < 9 ) {
				gs->current_level++;
				W_StartLevel();
			} else {
				// finshed level 10
				gs->quit = 1;
			}
		} else { // no trophy
			gs->ps.check_door = 0;
		}
	}
}

// update game view based on set scroll values
void W_ScrollView() {
	// scroll view if dave is about to move off view
	if ( gs->ps.tx - gs->view_x >= 18 )
		gs->scroll_x = 15;
	if ( gs->ps.tx - gs->view_x < 2 )
		gs->scroll_x = -15;

	// do the scroll
	if ( gs->scroll_x > 0 ) {
		if ( gs->view_x == 80 ) gs->scroll_x = 0;
		else { gs->view_x++; gs->scroll_x--; }
	}
	if ( gs->scroll_x < 0 ) {
		if ( gs->view_x == 0 ) gs->scroll_x = 0;
		else { gs->view_x--; gs->scroll_x++; }
	}
}

// main update routine
static void G_Update() {
	// update collision point flags
	P_UpdateCollision();
	// pickups
	P_PickupItem();
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

// draw player
void Draw_Player( SDL_Renderer* r ) {
	SDL_Rect dst;
	// relative to view
	dst.x = gs->ps.px - gs->view_x * TILE_SIZE;
	dst.y = gs->ps.py;
	// tile 56 neutral; 20x16px
	uint8_t til = 53;
	dst.w = 20; dst.h = 16;

	// render
	// grounded debug
	if ( gs->ps.on_ground ) {
		SDL_SetRenderDrawColor( r, 255, 0, 255, 255 );
		SDL_RenderDrawRect( r, &dst );
	}
	SDL_RenderCopy( r, g_assets->tile_tx[til], NULL, &dst );
}

// main drawing routine
static void G_Draw( SDL_Renderer* r ) {
	// clear backbuffer
	SDL_SetRenderDrawColor( r, 0, 40, 80, 0xff );
	SDL_RenderClear( r );

	Draw_World( r );
	Draw_Player( r );

	// flip buffers
	SDL_RenderPresent( r );
}

// rendering scale
//const uint8_t R_SCALE = 3;

int main( int argc, char** argv ) {
	// initialize SDL
	if ( SDL_Init( SDL_INIT_VIDEO ) )
		SDL_Log( "SDL Init error: %s\n", SDL_GetError() );

	// create window and renderer
	int winw = 1280, winh = 720;
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

	// main loop
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
#if defined( __ANDROID__ ) || defined( ANDROID )
	exit( EXIT_SUCCESS );
#endif

	return EXIT_SUCCESS;
}

