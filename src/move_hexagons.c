
#include <math.h>

#include "game.h"
#include "game_error.h"
#include "clickables.h"
#include "move_hexagons.h"

#include "stdio.h"


bool tileRecentlyMerged = false;
bool tileRecentlyCompacted = false;


// Cartesian here is somewhat of a misnomer. It just means that the origin is 0,0 and negative numbers are allowed.
// Array coordinates always start at zero and do not reflect the way the grid is displayed.
AxialHex axialArrayToCartesian(AxialHex axial) {
	GAME_ASSERT(axial.q >= 0);
	GAME_ASSERT(axial.r >= 0);
	GAME_ASSERT(axial.q <= 4);
	GAME_ASSERT(axial.r <= 4);

	AxialHex axial2 = axialHexAdd(axial, (AxialHex){-2, -2});

	GAME_ASSERT(axial2.q >= -2);
	GAME_ASSERT(axial2.r >= -2);
	GAME_ASSERT(axial2.q <= 2);
	GAME_ASSERT(axial2.r <= 2);

	return axial2;
}

AxialHex axialCartesianToArray(AxialHex axial) {
	GAME_ASSERT(axial.q >= -2);
	GAME_ASSERT(axial.r >= -2);
	GAME_ASSERT(axial.q <= 2);
	GAME_ASSERT(axial.r <= 2);

	AxialHex axial2 = axialHexAdd(axial, (AxialHex){2, 2});

	GAME_ASSERT(axial2.q >= 0);
	GAME_ASSERT(axial2.r >= 0);
	GAME_ASSERT(axial2.q <= 4);
	GAME_ASSERT(axial2.r <= 4);
	return axial2;
}


bool cartesianInBounds(AxialHex axial) {
	return axialManhattanDistance((AxialHex){0,0}, axial) <= 2;
}

// pos2 is the furthest of the two from the iteration's start
void visitPairCart(AxialHex pos1Cart, AxialHex pos2Cart) {
	GAME_ASSERT(cartesianInBounds(pos1Cart));
	GAME_ASSERT(cartesianInBounds(pos2Cart));

	AxialHex pos1 = axialCartesianToArray(pos1Cart);
	AxialHex pos2 = axialCartesianToArray(pos2Cart);

	if (!hexTileUseable(pos1) || !hexTileUseable(pos2)) {
		return;
	}


	if (hexTileUseable(pos1) && hexTileUseable(pos2)) {
		float one = hexTileGet(pos1);
		float two = hexTileGet(pos2);

		// merge
		if (one == two && two != TILE_EMPTY) {
			hexTileSet(pos1, TILE_EMPTY);
			hexTileSet(pos2, one * 2);
		}

		// compact
		one = hexTileGet(pos1);
		two = hexTileGet(pos2);
		if (one >= TILE_MIN_POPULATED && two == TILE_EMPTY) {
			hexTileSet(pos1, TILE_EMPTY);
			hexTileSet(pos2, one);
		}

	}
}


void walkLine(AxialHex pos, LibHexDirection iterDirection) {
	AxialHex nextPos = axialDirectNeighbour(pos, iterDirection, 1);
	while (cartesianInBounds(pos) && cartesianInBounds(nextPos)) {
		visitPairCart(pos, nextPos);

		pos = nextPos;
		nextPos = axialDirectNeighbour(nextPos, iterDirection, 1) ;
	}

}

void walkFromEdge(AxialHex pos, LibHexDirection edgeWalkDirection, LibHexDirection innerDirection) {
	if (innerDirection == HEXN_E) {
		printf("east\n");
	}

	while (cartesianInBounds(pos)) {
		walkLine(pos, innerDirection);

		pos = axialDirectNeighbour(pos, edgeWalkDirection, 1);
	}
}


void move2(ClickableKind clickableKind) {
	LibHexDirection dir =  clickableToHexDir(clickableKind);
	if (dir == HEXN_E) {
		printf("east2!\n");
	}

	LibHexDirection left = hexDirCClockwise(dir, 1);
	LibHexDirection right = hexdirClockwise(dir, 1);

	AxialHex pos = {0, 0};
	AxialHex leftCorner = axialDirectNeighbour(pos, left, 2);
	AxialHex middleCorner = axialDirectNeighbour(pos, dir ,2);
	AxialHex rightCorner = axialDirectNeighbour(pos, right, 2);


	LibHexDirection leftEdgeDirection = hexDirCClockwise(left, 2);
	LibHexDirection rightEdgeDirection = hexdirClockwise(right, 2);

	LibHexDirection innerDir = dir;

	for (int i = 0; i < 4; i++) {
		walkFromEdge(leftCorner, leftEdgeDirection, innerDir);
		walkFromEdge(rightCorner, rightEdgeDirection, innerDir);
	}
}



bool compactHexagons(AxialHex offset1, AxialHex offset2) {
	bool compaction = false;

	for (int i = -1; i < fmax(MAX_Q, MAX_R)+1; i++) {
		for (int q = -1; q < MAX_Q+1; q++) {
			for (int r = 0; r < MAX_R - 1; r++) {
				AxialHex axial = {q, r};

				AxialHex a1 = axialHexAdd(axial, offset1);
				AxialHex a2 = axialHexAdd(axial, offset2);
				if (!hexTileInRange(a1) || !hexTileInRange(a2)) {
					continue;
				}

				float one = hexTileGet(a1);
				float two = hexTileGet(a2);

				// remove gap
				if (one >= TILE_MIN_POPULATED && two == TILE_EMPTY) {
					hexTileSet(a2, one);
					hexTileSet(a1, two);
					compaction = true;
				}
			}
		}
	}

	return compaction;
}

void moveHexagons(ClickableKind direction) {
	move2(direction);
	return;


	tileRecentlyMerged = false;
	bool recentCompaction = false;
	bool tileMerged[MAX_Q][MAX_R] = {0};


	if (clickableKindHasFlag(direction, CLICKABLE_DIR_SE)) {
		// hacky compactions
		recentCompaction |= compactHexagons((AxialHex){0, 0}, (AxialHex){0, 1});

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

		recentCompaction |= compactHexagons((AxialHex){0, 0}, (AxialHex){0, 1});
		// // hacky compactions
		// for (int i = 0; i < fmaxf(MAX_Q, MAX_R); i++) {
		// 	for (int q = 0; q < MAX_Q; q++) {
		// 		for (int r = 0; r < MAX_R - 1; r++) {
		// 			// move all r to r-1 when r+1 is empty and r is occupied
		// 			float one = hexTiles.hex[q][r];
		// 			float two = hexTiles.hex[q][r+1];
		//
		// 			// remove gap
		// 			if (one >= TILE_MIN_POPULATED && two == TILE_EMPTY) {
		// 				hexTiles.hex[q][r+1] = one;
		// 				hexTiles.hex[q][r] = two;
		// 				recentCompaction = true;
		// 			}
		// 		}
		// 	}
		// }


	} else if (clickableKindHasFlag(direction, CLICKABLE_DIR_NW)) {
		recentCompaction |= compactHexagons((AxialHex){0, 1}, (AxialHex){0, 0});
		// // hacky compactions
		// for (int i = 0; i < fmaxf(MAX_Q, MAX_R); i++) {
		// 	for (int q = 0; q < MAX_Q; q++) {
		// 		for (int r = 0; r < MAX_R - 1; r++) {
		// 			// move all r to r-1 when r+1 is empty and r is occupied
		// 			float one = hexTiles.hex[q][r+1];
		// 			float two = hexTiles.hex[q][r];
		//
		// 			// remove gap
		// 			if (one >= TILE_MIN_POPULATED && two == TILE_EMPTY) {
		// 				hexTiles.hex[q][r] = one;
		// 				hexTiles.hex[q][r+1] = two;
		// 				recentCompaction = true;
		// 			}
		// 		}
		// 	}
		// }


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

		recentCompaction |= compactHexagons((AxialHex){0, 1}, (AxialHex){0, 0});
		// // hacky compactions
		// for (int i = 0; i < fmaxf(MAX_Q, MAX_R); i++) {
		// 	for (int q = 0; q < MAX_Q; q++) {
		// 		for (int r = 0; r < MAX_R - 1; r++) {
		// 			// move all r to r-1 when r+1 is empty and r is occupied
		// 			float one = hexTiles.hex[q][r+1];
		// 			float two = hexTiles.hex[q][r];
		//
		// 			// remove gap
		// 			if (one >= TILE_MIN_POPULATED && two == TILE_EMPTY) {
		// 				hexTiles.hex[q][r] = one;
		// 				hexTiles.hex[q][r+1] = two;
		// 				recentCompaction = true;
		// 			}
		// 		}
		// 	}
		// }

	}

	else if (clickableKindHasFlag(direction, CLICKABLE_DIR_W)) {
		recentCompaction |= compactHexagons((AxialHex){1, 1}, (AxialHex){0, 0});
		// hacky compactions
		// for (int i = 0; i < fmaxf(MAX_Q, MAX_R); i++) {
		// 	for (int r = 0; r < MAX_R; r++) {
		// 		for (int q = 0; q < MAX_Q - 1; q++) {
		//
		// 			float one = hexTiles.hex[q+1][r];
		// 			float two = hexTiles.hex[q][r];
		//
		// 			// remove gap
		// 			if (one >= TILE_MIN_POPULATED && two == TILE_EMPTY) {
		// 				hexTiles.hex[q][r] = one;
		// 				hexTiles.hex[q+1][r] = two;
		// 				recentCompaction = true;
		// 			}
		// 		}
		// 	}
		// }

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

		recentCompaction |= compactHexagons((AxialHex){1, 1}, (AxialHex){0, 0});

		// // hacky compactions
		// for (int i = 0; i < fmaxf(MAX_Q, MAX_R); i++) {
		// 	for (int r = 0; r < MAX_R; r++) {
		// 		for (int q = 0; q < MAX_Q - 1; q++) {
		//
		// 			float one = hexTiles.hex[q+1][r];
		// 			float two = hexTiles.hex[q][r];
		//
		// 			// remove gap
		// 			if (one >= TILE_MIN_POPULATED && two == TILE_EMPTY) {
		// 				hexTiles.hex[q][r] = one;
		// 				hexTiles.hex[q+1][r] = two;
		// 				recentCompaction = true;
		// 			}
		// 		}
		// 	}
		// }

	} else if (clickableKindHasFlag(direction, CLICKABLE_DIR_E)) {
		recentCompaction |= compactHexagons((AxialHex){0,0}, (AxialHex){1,0});

		// // hacky compactions
		// for (int i = 0; i < fmaxf(MAX_Q, MAX_R); i++) {
		// 	for (int r = 0; r < MAX_R; r++) {
		// 		for (int q = 0; q < MAX_Q - 1; q++) {
		//
		// 			float one = hexTiles.hex[q][r];
		// 			float two = hexTiles.hex[q+1][r];
		//
		// 			// remove gap
		// 			if (one >= TILE_MIN_POPULATED && two == TILE_EMPTY) {
		// 				hexTiles.hex[q+1][r] = one;
		// 				hexTiles.hex[q][r] = two;
		// 				recentCompaction = true;
		// 			}
		// 		}
		// 	}
		// }

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

		recentCompaction |= compactHexagons((AxialHex){0,0}, (AxialHex){1,0});

		//
		// // hacky compactions
		// for (int i = 0; i < fmaxf(MAX_Q, MAX_R); i++) {
		// 	for (int r = 0; r < MAX_R; r++) {
		// 		for (int q = 0; q < MAX_Q - 1; q++) {
		//
		// 			float one = hexTiles.hex[q][r];
		// 			float two = hexTiles.hex[q+1][r];
		//
		// 			// remove gap
		// 			if (one >= TILE_MIN_POPULATED && two == TILE_EMPTY) {
		// 				hexTiles.hex[q+1][r] = one;
		// 				hexTiles.hex[q][r] = two;
		// 				recentCompaction = true;
		// 			}
		// 		}
		// 	}
		// }

	} else if (clickableKindHasFlag(direction, CLICKABLE_DIR_SW)) {
		recentCompaction |= compactHexagons((AxialHex){1,0}, (AxialHex){0,0});


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

		recentCompaction |= compactHexagons((AxialHex){1,0}, (AxialHex){0,0});


		// // hacky compactions
		// for (int i = 0; i < fmaxf(MAX_Q, MAX_R); i++) {
		// 	for (int r = 0; r < MAX_R; r++) {
		// 		for (int q = 0; q < MAX_Q; q++) {
		// 			AxialHex here = {q, r};
		// 			AxialHex next = axialDirectNeighbour(here,  HEXN_SW, 1);
		// 			if ((here.q >= MAX_Q || here.r >= MAX_R || here.q < 0 || here.r < 0) ||
		// 				(next.q >= MAX_Q || next.r >= MAX_R || next.q < 0 || next.r < 0)
		// 			) {
		// 				continue;
		// 			}
		//
		// 			float hereVal = hexTiles.hex[(int)here.q][(int)here.r];
		// 			float nextVal = hexTiles.hex[(int)next.q][(int)next.r];
		// 			if (nextVal == TILE_EMPTY && hereVal >= TILE_MIN_POPULATED) {
		// 				hexTiles.hex[(int)next.q][(int)next.r] = hereVal;
		// 				hexTiles.hex[(int)here.q][(int)here.r] = TILE_EMPTY;
		// 				recentCompaction = true;
		// 			}
		// 		}
		// 	}
		// }


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
