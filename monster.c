#include "game.h"
#include "world.h"

extern game_state_t* gs;

void M_Move() {
	for ( int i = 0; i < sizeof(gs->ms) / sizeof(gs->ms[0]); ++i ) {
		monster_state_t* m = &gs->ms[i];
		if ( m->type ) {
			if ( !m->npx && !m->npy ) {
				m->npx = gs->levels[gs->current_level].path[m->path_index];
				m->npy = gs->levels[gs->current_level].path[m->path_index + 1];
				m->path_index += 2;
			}
			// end of path
			if ( m->npx == (int8_t)0xea && m->npy == (int8_t)0xea ) {
				m->npx = gs->levels[gs->current_level].path[0];
				m->npy = gs->levels[gs->current_level].path[1];
				m->path_index = 2;
			}
			// move
			if ( m->npx < 0 ) { m->px -= 1; m->npx++; }
			if ( m->npx > 0 ) { m->px += 1; m->npx--; }
			if ( m->npy < 0 ) { m->py -= 1; m->npy++; }
			if ( m->npy > 0 ) { m->py += 1; m->npy--; }
			// update tile pos
			m->tx = m->px / TILE_SIZE;
			m->ty = m->py / TILE_SIZE;
		}
	}
}

void M_Fire() {
	if ( !gs->mbullet_px && !gs->mbullet_py ) {
		for ( int i = 0; i < sizeof(gs->ms) / sizeof(gs->ms[0]); ++i ) {
			monster_state_t* m = &gs->ms[i];
			if ( m->type && W_IsVisible( m->px ) ) {
				gs->mbullet_dir = gs->ps.px < m->px ? -1 : 1;
				if ( gs->mbullet_dir == 1 )
					gs->mbullet_px = m->px + 18;
				if ( gs->mbullet_dir == -1 )
					gs->mbullet_px = m->px - 8;
				gs->mbullet_py = m->py + 8;
			}
		}
	}
}

void M_UpdateBullet() {
	if ( !gs->mbullet_px || !gs->mbullet_py ) return;
	if ( !W_IsClear( gs->mbullet_px, gs->mbullet_py, 0 ) || !W_IsVisible( gs->mbullet_px ) )
		gs->mbullet_px = gs->mbullet_py = 0;
	if ( gs->mbullet_px )
		gs->mbullet_px += gs->mbullet_dir * 4;
}

