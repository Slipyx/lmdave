// extract tilesxxx.bmp from uncompressed DAVE.EXE
#include <stdint.h>
#include <inttypes.h>

#include <SDL.h>

// all tiles in the exe
SDL_Surface* tile_sfc[158];
//SDL_Texture* tile_tx[158];

// level info
void LoadLevels();
void SaveLevels();
void CreateWorldMap();

// export all tiles to bmp
void SaveTiles() {
	// Save out the all tile surfaces
	for ( int curtil = 0; curtil < 158; ++curtil ) {
		SDL_Surface* sfc = tile_sfc[curtil];
		printf( "Saving tile%03d.bmp (%ux%u)...\n", curtil, sfc->w, sfc->h );
		char fname[1024];
		snprintf( fname, 1024, "../tiles/tile%03d.bmp", curtil );
		SDL_SaveBMP( sfc, fname );
	}
}

// convert all loaded tile surfaces to textures
/*void ConvertTiles( SDL_Renderer* r ) {
	for ( int i = 0; i < 158; ++i ) {
		tile_tx[i] = SDL_CreateTextureFromSurface( r, tile_sfc[i] );
		SDL_FreeSurface( tile_sfc[i] );
	}
}*/

// fill global tile array with tiles from exe
void LoadTiles() {
	const uint32_t vga_data_addr = 0x120f0;
	const uint32_t vga_pal_addr = 0x26b0a;
	// exe assumed uncompressed
	SDL_RWops* ddexe = SDL_RWFromFile( "../DAVE.EXE", "rb" );
	if ( ddexe != NULL ) printf( "SUCCESS! (%"PRIi64" bytes)\n", SDL_RWsize( ddexe ) );

	// tileset
	SDL_RWseek( ddexe, vga_data_addr, RW_SEEK_SET );
	uint32_t tssz;
	SDL_RWread( ddexe, &tssz, 4, 1 );
	printf( "Uncompressed tileset size: %u\n", tssz );
	uint8_t* tsdat = malloc( tssz );
	memset( tsdat, 0, tssz );
	uint8_t bytebuf = 0;
	uint8_t* tsbyte = tsdat; // current output byte to be written to
	// uncompress RLE
	while ( (tsbyte - tsdat) < tssz ) {
		SDL_RWread( ddexe, &bytebuf, 1, 1 );

		if ( bytebuf & 0x80 ) { // unchanged
			uint32_t cnt = ((bytebuf & 0x7f) + 1);

			while ( cnt-- ) {
				SDL_RWread( ddexe, &bytebuf, 1, 1 );
				*tsbyte = bytebuf; tsbyte++;
			}
		} else { // repeated
			uint32_t cnt = bytebuf + 3;
			SDL_RWread( ddexe, &bytebuf, 1, 1 ); // byte to be repeated
			while ( cnt-- ) {
				*tsbyte = bytebuf; tsbyte++;
			}
		}
	}

	// palette
	SDL_RWseek( ddexe, vga_pal_addr, RW_SEEK_SET );
	uint8_t paldat[768];
	memset( paldat, 0, 768 );
	// reach each color and convert from 6bit to 8bit
	for ( int i = 0; i < 256; ++i ) {
		SDL_RWread( ddexe, paldat + (i * 3), 3, 1 );
		paldat[i*3] <<= 2;
		paldat[i*3+1] <<= 2;
		paldat[i*3+2] <<= 2;
	}

	// note: extra byte every 65536 bytes
	// total tile count
	SDL_RWops* tsfil = SDL_RWFromMem( tsdat, tssz );
	uint32_t tscnt;
	SDL_RWread( tsfil, &tscnt, 4, 1 );
	printf( "Tile count: %u\n", tscnt );

	// tile offsets
	uint32_t tsoffs[tscnt+1];
	for ( int i = 0; i < tscnt; ++i ) {
		SDL_RWread( tsfil, &tsoffs[i], 4, 1 );
		//printf( "ts%u: %u -- ", i, tsoffs[i] );
	}
	// last tile ends at EOF
	tsoffs[tscnt] = tssz;

	uint16_t tw, th;
	uint32_t curbyte;
	uint8_t curtil;
	// read each tile
	for ( curtil = 0; curtil < tscnt; curtil++ ) {
		//curtilbyte = 0;
		curbyte = tsoffs[curtil];
		tw = 16; th = 16;
		// skip extra byte
		if ( curbyte > 65280 )
			curbyte++;
		// curbyte is now actual offset to raw image data
		SDL_RWseek( tsfil, curbyte, RW_SEEK_SET );

		// check if dimension thats not 16x16, assumed if current chunk size is not 256
		if ( tsoffs[curtil+1] - tsoffs[curtil] != 256 ) {
			SDL_RWread( tsfil, &tw, 2, 1 );
			SDL_RWread( tsfil, &th, 2, 1 );
			curbyte += 4;
		}

		// fill a new 32bpp surface using palette
		SDL_Surface* tilsfc = SDL_CreateRGBSurface( 0, tw, th, 32, 0,0,0,0 );//SDL_PIXELFORMAT_RGBA32 );
		SDL_LockSurface( tilsfc );
		uint8_t* pxp = (uint8_t*)(tilsfc->pixels);

		for ( int p = 0; p < tw * th; ++p ) {
				uint8_t ix;
				SDL_RWread( tsfil, &ix, 1, 1 );
				size_t pos = p * 4;
				// bgra
				pxp[pos] = paldat[ix*3+2];
				pxp[pos+1] = paldat[ix*3+1];
				pxp[pos+2] = paldat[ix*3];
				pxp[pos+3] = 255;//(ix == 0) ? 0 : 255;
		}
		SDL_UnlockSurface( tilsfc );
		// store surface in global tile array
		tile_sfc[curtil] = tilsfc;

		//SDL_FreeSurface( tilsfc );

		//printf( "ts%u (%ux%u) @ %u -- ", curtil, tw, th, curbyte );
	}

	SDL_RWclose( tsfil );
	free( tsdat );

	// done with exe
	SDL_RWclose( ddexe );
}

// free all loaded exe's tile surfaces
void FreeTileSurfaces() {
	for ( int i = 0; i < 158; ++i ) {
		SDL_FreeSurface( tile_sfc[i] );
	}
}

int main( int argc, char** argv ) {
	// high scores
	SDL_RWops* dscor = SDL_RWFromFile( "../DSCORES.DAV", "rb" );
	uint8_t lscor[9];
	while ( SDL_RWread( dscor, lscor, 9, 1 ) ) {
		printf( "LVL %u - SCORE: %u%u%u%u%u - NAM: %.3s\n", lscor[0],
			lscor[1], lscor[2], lscor[3], lscor[4], lscor[5], (char*)&lscor[6] );
	}
	SDL_RWclose( dscor );

	LoadTiles();
	//SaveTiles();
	//ConvertTiles();

	// LEVELS
	LoadLevels();
	//SaveLevels();
	CreateWorldMap();

	FreeTileSurfaces();

	return 0;
}

