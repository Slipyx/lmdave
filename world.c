#include "game.h"
#include "world.h"
//#include "player.h"

extern game_state_t* gs;

// sets new beginning state for current level
void W_StartLevel() {
	P_Spawn();

	// reset monster bullet
	gs->ebullet_px = 0;
	gs->ebullet_py = 0;

	// reset items
	gs->ps.gun = 0;
	gs->ps.jetpack = 0;
	gs->ps.trophy = 0;
	gs->ps.check_door = 0;
}

// hard reset current level from original data
void W_ResetLevel() {
	Util_GetLevel( gs->current_level, &gs->levels[gs->current_level] );
	W_StartLevel();
}

// returns 1 if passed pixel point is not within a solid tile, is_player non zero to execute functionality
uint8_t W_IsClear( uint16_t px, uint16_t py, uint8_t is_player ) {
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

	// player collision functionality
	if ( is_player ) {
		// kill tiles
		if ( til == 6 || til == 25 || til == 36 ) {
			P_Spawn();
		}

		// pickups
		if ( til == 10 || til == 4 || til == 20 || (til >= 47 && til <= 52) ) {
			gs->ps.check_pickup_x = tx;
			gs->ps.check_pickup_y = ty;
		}

		// door
		if ( til == 2 ) {
			gs->ps.check_door = 1;
		}
	}

	return 1;
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
