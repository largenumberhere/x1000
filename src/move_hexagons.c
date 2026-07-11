#include "../include/move_hexagons.h"

#include <math.h>

#include "../include/game.h"
#include "../include/game_error.h"

bool tileRecentlyMerged = false;


void moveHexagons(ClickMove direction) {
	tileRecentlyMerged = false;
	bool tileMerged[MAX_Q][MAX_R] = {0};


	if (direction == CLICK_MOVE_SE) {
		// hacky compactions
		for (int i = 0; i < fmax(MAX_Q, MAX_R); i++) {
			for (int q = 0; q < MAX_Q; q++) {
				for (int r = 0; r < MAX_R - 1; r++) {
					// move all r to r-1 when r+1 is empty and r is occupied
					float one = hexTiles.hex[q][r];
					float two = hexTiles.hex[q][r+1];

					// remove gap
					if (one >= TILE_MIN_POPULATED && two == TILE_EMPTY) {
						hexTiles.hex[q][r+1] = one;
						hexTiles.hex[q][r] = two;
					}
				}
			}
		}

		// merge neighbours
		for (int q = 0; q < MAX_Q; q++) {
			for (int r = 0; r < MAX_R - 1; r++) {
				// move all r to r-1 when r+1 is empty and r is occupied
				float one = hexTiles.hex[q][r];
				float two = hexTiles.hex[q][r+1];

				// remove gap
				bool isSameNonzeroKind = (one >= TILE_MIN_POPULATED) && two == one;
				if (isSameNonzeroKind) {
					if (tileMerged[q][r] || tileMerged[q][r+1]) {
						continue;
					}

					tileMerged[q][r+1] = true;

					hexTiles.hex[q][r+1] = one + one;
					hexTiles.hex[q][r] = TILE_EMPTY;
				}
			}
		}

		// hacky compactions
		for (int i = 0; i < fmaxf(MAX_Q, MAX_R); i++) {
			for (int q = 0; q < MAX_Q; q++) {
				for (int r = 0; r < MAX_R - 1; r++) {
					// move all r to r-1 when r+1 is empty and r is occupied
					float one = hexTiles.hex[q][r];
					float two = hexTiles.hex[q][r+1];

					// remove gap
					if (one >= TILE_MIN_POPULATED && two == TILE_EMPTY) {
						hexTiles.hex[q][r+1] = one;
						hexTiles.hex[q][r] = two;
					}
				}
			}
		}


	} else if (direction == CLICK_MOVE_NW) {
		// hacky compactions
		for (int i = 0; i < fmaxf(MAX_Q, MAX_R); i++) {
			for (int q = 0; q < MAX_Q; q++) {
				for (int r = 0; r < MAX_R - 1; r++) {
					// move all r to r-1 when r+1 is empty and r is occupied
					float one = hexTiles.hex[q][r+1];
					float two = hexTiles.hex[q][r];

					// remove gap
					if (one >= TILE_MIN_POPULATED && two == TILE_EMPTY) {
						hexTiles.hex[q][r] = one;
						hexTiles.hex[q][r+1] = two;
					}
				}
			}
		}


		// merge neighbours
		for (int q = 0; q < MAX_Q; q++) {
			for (int r = 0; r < MAX_R - 1; r++) {
				// move all r+1 to r when r is empty and r+1 is occupied
				float one = hexTiles.hex[q][r+1];
				float two = hexTiles.hex[q][r];

				// remove gap
				bool isSameNonzeroKind = (two >= TILE_MIN_POPULATED) && two == one;
				if (isSameNonzeroKind) {
					if (tileMerged[q][r] || tileMerged[q][r+1]) {
						continue;
					}

					tileMerged[q][r] = true;

					hexTiles.hex[q][r] = one + one;
					hexTiles.hex[q][r+1] = TILE_EMPTY;
				}
			}
		}

		// hacky compactions
		for (int i = 0; i < fmaxf(MAX_Q, MAX_R); i++) {
			for (int q = 0; q < MAX_Q; q++) {
				for (int r = 0; r < MAX_R - 1; r++) {
					// move all r to r-1 when r+1 is empty and r is occupied
					float one = hexTiles.hex[q][r+1];
					float two = hexTiles.hex[q][r];

					// remove gap
					if (one >= TILE_MIN_POPULATED && two == TILE_EMPTY) {
						hexTiles.hex[q][r] = one;
						hexTiles.hex[q][r+1] = two;
					}
				}
			}
		}

	}

	else if (direction == CLICK_MOVE_W) {
		// hacky compactions
		for (int i = 0; i < fmaxf(MAX_Q, MAX_R); i++) {
			for (int r = 0; r < MAX_R; r++) {
				for (int q = 0; q < MAX_Q - 1; q++) {

					float one = hexTiles.hex[q+1][r];
					float two = hexTiles.hex[q][r];

					// remove gap
					if (one >= TILE_MIN_POPULATED && two == TILE_EMPTY) {
						hexTiles.hex[q][r] = one;
						hexTiles.hex[q+1][r] = two;
					}
				}
			}
		}

		// merge neighbours
		for (int r = 0; r < MAX_R - 1; r++) {
			for (int q = 0; q < MAX_Q -1; q++) {
				// move all r+1 to r when r is empty and r+1 is occupied
				float one = hexTiles.hex[q+1][r];
				float two = hexTiles.hex[q][r];

				// remove gap
				bool isSameNonzeroKind = (two >= TILE_MIN_POPULATED) && two == one;
				if (isSameNonzeroKind) {
					if (tileMerged[q][r] || tileMerged[q+1][r]) {
						continue;
					}

					tileMerged[q][r] = true;

					hexTiles.hex[q][r] = one + one;
					hexTiles.hex[q+1][r] = TILE_EMPTY;
				}
			}
		}

		// hacky compactions
		for (int i = 0; i < fmaxf(MAX_Q, MAX_R); i++) {
			for (int r = 0; r < MAX_R; r++) {
				for (int q = 0; q < MAX_Q - 1; q++) {

					float one = hexTiles.hex[q+1][r];
					float two = hexTiles.hex[q][r];

					// remove gap
					if (one >= TILE_MIN_POPULATED && two == TILE_EMPTY) {
						hexTiles.hex[q][r] = one;
						hexTiles.hex[q+1][r] = two;
					}
				}
			}
		}

	} else if (direction == CLICK_MOVE_E) {
		// hacky compactions
		for (int i = 0; i < fmaxf(MAX_Q, MAX_R); i++) {
			for (int r = 0; r < MAX_R; r++) {
				for (int q = 0; q < MAX_Q - 1; q++) {

					float one = hexTiles.hex[q][r];
					float two = hexTiles.hex[q+1][r];

					// remove gap
					if (one >= TILE_MIN_POPULATED && two == TILE_EMPTY) {
						hexTiles.hex[q+1][r] = one;
						hexTiles.hex[q][r] = two;
					}
				}
			}
		}

		// merge neighbours
		for (int r = 0; r < MAX_R - 1; r++) {
			for (int q = 0; q < MAX_Q -1; q++) {
				// move all r+1 to r when r is empty and r+1 is occupied
				float one = hexTiles.hex[q][r];
				float two = hexTiles.hex[q+1][r];

				// remove gap
				bool isSameNonzeroKind = (two >= TILE_MIN_POPULATED) && two == one;
				if (isSameNonzeroKind) {
					if (tileMerged[q][r] || tileMerged[q][r]) {
						continue;
					}

					tileMerged[q+1][r] = true;

					hexTiles.hex[q+1][r] = one + one;
					hexTiles.hex[q][r] = TILE_EMPTY;
				}
			}
		}


	} else {
		GAME_ASSERT(false);
	}

	// notify of one or more tile merges
	for (int q = 0; q < MAX_Q; q++) {
		for (int r = 0; r < MAX_R; r++) {
			if (tileMerged[q][r]) {
				tileRecentlyMerged = true;
			}
		}
	}



	spawnRandomHexTile();
}
