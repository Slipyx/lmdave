#include "game.h"
#include "world.h"

extern game_state_t* gs;

// sets player position to current level's player start
void P_Spawn() {
	// reset view
	gs->view_x = 0;
	gs->view_y = 0;
	gs->scroll_x = 0;

	// reset player jump
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
