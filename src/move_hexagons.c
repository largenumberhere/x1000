
#include <math.h>

#include "game.h"
#include "game_error.h"
#include "clickables.h"
#include "move_hexagons.h"


bool tileRecentlyMerged = false;
bool tileRecentlyCompacted = false;


void moveHexagons(ClickableDir direction) {
	tileRecentlyMerged = false;
	bool recentCompaction = false;
	bool tileMerged[MAX_Q][MAX_R] = {0};


	if (direction == CLICKABLE_DIR_SE) {
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
						recentCompaction = true;
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
						recentCompaction = true;
					}
				}
			}
		}


	} else if (direction == CLICKABLE_DIR_NW) {
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
						recentCompaction = true;
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
						recentCompaction = true;
					}
				}
			}
		}

	}

	else if (direction == CLICKABLE_DIR_W) {
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
						recentCompaction = true;
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
						recentCompaction = true;
					}
				}
			}
		}

	} else if (direction == CLICKABLE_DIR_E) {
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
						recentCompaction = true;
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
						recentCompaction = true;
					}
				}
			}
		}

	} else if (direction == CLICKABLE_DIR_SW) {

		// hacky compactions
		for (int i = 0; i < fmaxf(MAX_Q, MAX_R); i++) {
			for (int r = 0; r < MAX_R; r++) {
				for (int q = 0; q < MAX_Q; q++) {
					AxialHex here = {q, r};
					AxialHex next = axialDirectNeighbour(here,  HEXN_SW, 1);
					if ((here.q >= MAX_Q || here.r >= MAX_R || here.q < 0 || here.r < 0) ||
						(next.q >= MAX_Q || next.r >= MAX_R || next.q < 0 || next.r < 0)
					) {
						continue;
					}

					float hereVal = hexTiles.hex[(int)here.q][(int)here.r];
					float nextVal = hexTiles.hex[(int)next.q][(int)next.r];
					if (nextVal == TILE_EMPTY && hereVal >= TILE_MIN_POPULATED) {
						hexTiles.hex[(int)next.q][(int)next.r] = hereVal;
						hexTiles.hex[(int)here.q][(int)here.r] = TILE_EMPTY;
						recentCompaction = true;
					}
				}
			}
		}

		// merge neighbours
		for (int r = 0; r < MAX_R; r++) {
			// for (int s = 0; s < MAX_R -1 /*probably fine?*/; s++) {
			for (int q = 0; q < MAX_Q; q++) {

				// nb: the board is displayed with a skew, so direction values are one different
				AxialHex here = {q, r};
				AxialHex next = axialDirectNeighbour(here,  HEXN_SW, 1);
				if ((here.q >= MAX_Q || here.r >= MAX_R || here.q < 0 || here.r < 0) ||
					(next.q >= MAX_Q || next.r >= MAX_R || next.q < 0 || next.r < 0)
				) {
					continue;
				}

				// move all r+1 to r when r is empty and r+1 is occupied
				float one = hexTiles.hex[(int)here.q][(int)here.r];
				float two = hexTiles.hex[(int)next.q][(int)next.r];

				// remove gap
				bool isSameNonzeroKind = (two >= TILE_MIN_POPULATED) && two == one;
				if (isSameNonzeroKind) {
					if (tileMerged[(int) here.q][(int) here.r] || tileMerged[(int) next.q][(int) next.r]) {
						continue;
					}

					tileMerged[(int) here.q][(int) here.r] = true;

					hexTiles.hex[(int) next.q][(int) next.r] = one + one;
					hexTiles.hex[(int) here.q][(int) here.r] = TILE_EMPTY;
				}
			}

		}

		// hacky compactions
		for (int i = 0; i < fmaxf(MAX_Q, MAX_R); i++) {
			for (int r = 0; r < MAX_R; r++) {
				for (int q = 0; q < MAX_Q; q++) {
					AxialHex here = {q, r};
					AxialHex next = axialDirectNeighbour(here,  HEXN_SW, 1);
					if ((here.q >= MAX_Q || here.r >= MAX_R || here.q < 0 || here.r < 0) ||
						(next.q >= MAX_Q || next.r >= MAX_R || next.q < 0 || next.r < 0)
					) {
						continue;
					}

					float hereVal = hexTiles.hex[(int)here.q][(int)here.r];
					float nextVal = hexTiles.hex[(int)next.q][(int)next.r];
					if (nextVal == TILE_EMPTY && hereVal >= TILE_MIN_POPULATED) {
						hexTiles.hex[(int)next.q][(int)next.r] = hereVal;
						hexTiles.hex[(int)here.q][(int)here.r] = TILE_EMPTY;
						recentCompaction = true;
					}
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

	if (recentCompaction) {
		tileRecentlyCompacted = true;
	}

	spawnRandomHexTile();
}
