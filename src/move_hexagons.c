
#include <math.h>

#include "game.h"
#include "game_error.h"
#include "clickables.h"
#include "move_hexagons.h"

#include "stdio.h"
#include "stdlib.h"

bool tileRecentlyMerged = false;
bool tileRecentlyCompacted = false;

float radius = 3;

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
	GAME_ASSERT(axial2.q <= MAX_Q);
	GAME_ASSERT(axial2.r <= MAX_R);
	return axial2;
}


bool cartesianInBounds(AxialHex axial) {
	return (
			(axial.q >= -2) &&
			(axial.r >= -2) &&
			(axial.q <= 2)  &&
			(axial.r <= 2)
		);

	return axialManhattanDistance((AxialHex){0,0}, axial) <= 2;
}

// pos2 is the furthest of the two from the iteration's start
void visitPairCart(AxialHex pos1Cart, AxialHex pos2Cart, int mode) {
	GAME_ASSERT(cartesianInBounds(pos1Cart));
	GAME_ASSERT(cartesianInBounds(pos2Cart));

	AxialHex pos1 = axialCartesianToArray(pos1Cart);
	AxialHex pos2 = axialCartesianToArray(pos2Cart);

	if (!hexTileUseable(pos1) || !hexTileUseable(pos2)) {
		return;
	}


	float one = hexTileGet(pos1);
	float two = hexTileGet(pos2);
	bool tilesEqual = one == two;
	bool oneOccupied = hexTileOccupied(pos1);

	if (mode == 1) {
		// merge
		if (tilesEqual && oneOccupied) {
			hexTileSet(pos1, TILE_EMPTY);
			hexTileSet(pos2, one * 2);
			tileRecentlyMerged = true;
		}
	} else {
		// compact
		one = hexTileGet(pos1);
		two = hexTileGet(pos2);
		if (oneOccupied && two == TILE_EMPTY) {
			hexTileSet(pos1, TILE_EMPTY);
			hexTileSet(pos2, one);
			tileRecentlyCompacted = true;
		}
	}
}

//
//
void walkLine(AxialHex pos, LibHexDirection iterDirection) {
	AxialHex posCopy = pos;

	for (int i = 0; i < 6; i++) {
		AxialHex nextPos = axialDirectNeighbour(pos, iterDirection, 1);
		while (cartesianInBounds(pos) && cartesianInBounds(nextPos)) {
			visitPairCart(pos, nextPos,1);

			pos = nextPos;
			nextPos = axialDirectNeighbour(nextPos, iterDirection, 1) ;
		}
	}

	for (int i = 0; i < 6; i++) {
		pos = posCopy;
		AxialHex nextPos = axialDirectNeighbour(pos, iterDirection, 1);
		while (cartesianInBounds(pos) && cartesianInBounds(nextPos)) {
			visitPairCart(pos, nextPos,0);

			pos = nextPos;
			nextPos = axialDirectNeighbour(nextPos, iterDirection, 1) ;
		}
	}

}
//
// void walkFromEdge(AxialHex pos, LibHexDirection edgeWalkDirection, LibHexDirection innerDirection) {
// 	if (innerDirection == HEXN_E) {
// 		printf("east\n");
// 	}
//
// 	while (cartesianInBounds(pos)) {
// 		walkLine(pos, innerDirection);
//
// 		pos = axialDirectNeighbour(pos, edgeWalkDirection, 1);
// 	}
// }

/*
 *
* function cube_scale(hex, factor):
	return Cube(hex.q * factor, hex.r * factor, hex.s * factor)

function cube_ring(center, radius):
	var results = []
	// this code doesn't work for radius == 0; can you see why?
	var hex = cube_add(center,
						cube_scale(cube_direction(4), radius))
	for each 0 ≤ i < 6:
		for each 0 ≤ j < radius:
			results.append(hex)
			hex = cube_neighbor(hex, i)
	return results
 *
 */

int directionEdgeOne(LibHexDirection dir) {
	return (int) dir;
}

int directionEdgeTwo(LibHexDirection dir) {
	return (int) (dir + 1) % 6;
}

CubeHex findEdgeEndCubeCart(int i) {
	GAME_ASSERT(i >= 0);
	GAME_ASSERT(i < 6);

	int radius = 3;
	AxialHex centreAxialCart = {0,0};
	CubeHex centreCubeCart = axialToCube(centreAxialCart);

	// start in far SW corner
	CubeHex cube = cubeAdd(centreCubeCart, cubeScale(cubeDirection(4), radius-1));
	const int edgesCount = 6;
	for (int edge = 0; edge < edgesCount; edge++) {

		// advances at every corner
		for (int j = 0; j < radius-1; j++) {
			// position = cube

			// next neighbour
			cube = cubeNeighbour(cube, edge);
			if (j == radius-2 && edge == i) {
				return cube;
			}
		}
	}

	return (CubeHex) {0, 0, 0};
}

CubeHex findEdgeDirectionOffsetCubeCart(int i) {
	GAME_ASSERT(i >= 0);
	GAME_ASSERT(i < 6);

	int radius = 3;
	AxialHex centreAxialCart = {0,0};
	CubeHex centreCubeCart = axialToCube(centreAxialCart);

	// start in far SW corner
	CubeHex cube = cubeAdd(centreCubeCart, cubeScale(cubeDirection(4), radius-1));
	const int edgesCount = 6;
	for (int edge = 0; edge < edgesCount; edge++) {

		// advances at every corner
		for (int j = 0; j < radius-1; j++) {
			// position = cube
			if (j == 0 && edge == i) {
				CubeHex nextPos = cubeNeighbour(cube, edge);
				CubeHex diff = cubeAdd((CubeHex){-nextPos.q, -nextPos.r, -nextPos.s}, cube);
				CubeHex opposite = (CubeHex){-diff.q, -diff.r, -diff.s};
				CubeHex rounded = cubeRound(opposite);
				return  rounded;
			}\
			// next neighbour
			cube = cubeNeighbour(cube, edge);
		}
	}

	return (CubeHex) {0, 0, 0};
}
CubeHex findEdgeStartCubeCart(int i) {
	GAME_ASSERT(i >= 0);
	GAME_ASSERT(i < 6);

	int radius = 3;
	AxialHex centreAxialCart = {0,0};
	CubeHex centreCubeCart = axialToCube(centreAxialCart);

	// start in far SW corner
	CubeHex cube = cubeAdd(centreCubeCart, cubeScale(cubeDirection(4), radius-1));
	const int edgesCount = 6;
	for (int edge = 0; edge < edgesCount; edge++) {

		// advances at every corner
		for (int j = 0; j < radius-1; j++) {
			// position = cube
			if (j == 0 && edge == i) {
				return cube;
			}
			// next neighbour
			cube = cubeNeighbour(cube, edge);
		}
	}

	return (CubeHex) {0, 0, 0};
}


// void walkEdgePair(int n) {
// 	printf("pair start\n");
// 	int radius = 3;
// 	AxialHex centreAxialCart = {0,0};
// 	CubeHex centreCubeCart = axialToCube(centreAxialCart);
//
// 	// start in far SW corner
// 	CubeHex cube = cubeAdd(centreCubeCart, cubeScale(cubeDirection(4), radius));
// 	const int edgesCount = 6;
// 	for (int edge = 0; edge < edgesCount; edge++) {
// 		// advances at every corner
// 		for (int j = 0; j < radius; j++) {
//
// 			bool isAtEdge1 = edge == n;
// 			bool isAtEdge2 = edge == n + 1;
//
// 			// position = cube
// 			if (isAtEdge1 || isAtEdge2) {
// 				printf("(%.0f, %.0f, %.0f)\n", cube.q, cube.r, cube.s);
// 			}
//
// 			// next neighbour
// 			cube = cubeNeighbour(cube, edge);
// 			if (j == radius-1 && isAtEdge2) {
// 				// include trailing coner too
// 				printf("(%.0f, %.0f, %.0f)\n", cube.q, cube.r, cube.s);
// 			}
// 		}
//
// 	}
// }
//
// void iterRing() {
// 	int radius = 3;
// 	AxialHex centreAxialCart = {0,0};
// 	CubeHex centreCubeCart = axialToCube(centreAxialCart);
//
// 	// start in far SW corner
// 	CubeHex cube = cubeAdd(centreCubeCart, cubeScale(cubeDirection(4), radius));
// 	const int edgesCount = 6;
// 	for (int edge = 0; edge < edgesCount; edge++) {
//
// 		// advances at every corner
// 		for (int j = 0; j < radius; j++) {
// 			// position = cube
// 			printf("(%.0f, %.0f, %.0f)\n", cube.q, cube.r, cube.s);
//
// 			// next neighbour
// 			cube = cubeNeighbour(cube, edge);
//
// 			if (j == radius-1) {
// 				// include trailing coner too
// 				printf("(%.0f, %.0f, %.0f)\n", cube.q, cube.r, cube.s);
// 			}
// 		}
//
// 		// backtrack one
// 		// cube = cubeNeighbour(cube, edgesCount - edge);
// 	}
// }
//
// LibHexDirection lhsEdgeWalkDir(LibHexDirection dir) {
// 	LibHexDirection dir2 = {0};
// 	switch (dir) {
// 		case HEXN_SE:
// 			dir2 = HEXN_NE;
// 			break;
// 		case HEXN_E:
// 			dir2 = HEXN_NW;
// 			break;
// 		case HEXN_NE:
// 			dir2 = HEXN_W;
// 			break;
// 		case HEXN_NW:
// 			dir2 = HEXN_SW;
// 			break;
// 		case HEXN_W:
// 			dir2 = HEXN_SE;
// 			break;
// 		case HEXN_SW:
// 			dir2 = HEXN_E;
// 			break;
// 		default:
// 			GAME_ASSERT(false);
// 	}
//
// 	return dir2;
// }
// LibHexDirection rhsEdgeWalkDir(LibHexDirection dir) {
// 	// note: left and rights may be mixed up.
//
// 	LibHexDirection dir2 = {0};
// 	switch (dir) {
// 		case HEXN_SE:
// 			dir2 = HEXN_W;
// 			break;
// 		case HEXN_E:
// 			dir2 = HEXN_SW;
// 			break;
// 		case HEXN_NE:
// 			dir2 = HEXN_SE;
// 			break;
// 		case HEXN_NW:
// 			dir2 = HEXN_NE;
// 			break;
// 		case HEXN_W:
// 			dir2 = HEXN_NE;
// 			break;
// 		case HEXN_SW:
// 			dir2 = HEXN_NW;
// 			break;
// 		default:
// 			GAME_ASSERT(false);
// 	}
//
// 	return dir2;
// }
//
// void move2(ClickableKind clickableKind) {
// 	// iterRing();
// 	for (int i = 0; i < 6; i++) {
// 		CubeHex cubeCart = findEdgeStartCubeCart(i);
// 		printf("%.0f %.0f %.0f\n", cubeCart.q, cubeCart.r, cubeCart.s);
// 		// walkEdgePair(i);
// 	}
//
// 	printf("\n\n");
// 	for (int i = 0; i < 6; i++) {
// 		CubeHex cubeCart = findEdgeDirectionOffsetCubeCart(i);
// 		printf("%.0f %.0f %.0f\n", cubeCart.q, cubeCart.r, cubeCart.s);
// 	}
//
// 	printf("\n\n");
// 	for (int i = 0; i < 6; i++) {
// 		CubeHex cubeCart = findEdgeEndCubeCart(i);
// 		printf("%.0f %.0f %.0f\n", cubeCart.q, cubeCart.r, cubeCart.s);
// 	}
//
//
// 	LibHexDirection dir =  clickableToHexDir(clickableKind);
// 	if (dir == HEXN_E) {
// 		printf("east2!\n");
// 	}
//
// 	LibHexDirection left = hexDirCClockwise(dir, 1);
// 	LibHexDirection right = hexdirClockwise(dir, 1);
//
// 	AxialHex pos = {0, 0};
// 	AxialHex leftCorner = axialDirectNeighbour(pos, left, 2);
// 	AxialHex middleCorner = axialDirectNeighbour(pos, dir ,2);
// 	AxialHex rightCorner = axialDirectNeighbour(pos, right, 2);
//
//
// 	// LibHexDirection leftEdgeDirection = hexDirCClockwise(left, 2);
// 	// LibHexDirection rightEdgeDirection = hexdirClockwise(right, 2);
//
//
// 	LibHexDirection leftEdgeDirection = lhsEdgeWalkDir(dir);
// 	LibHexDirection rightEdgeDirection = rhsEdgeWalkDir(dir);
//
// 	LibHexDirection innerDir = dir;
//
//
// 	walkFromEdge(leftCorner, leftEdgeDirection, innerDir);
// 	walkFromEdge(rightCorner, rightEdgeDirection, innerDir);
//
//
// 	movedHexagonsRecently = true;
// 	tileRecentlyCompacted = false;
// }


//
// bool compactHexagons(AxialHex offset1, AxialHex offset2) {
// 	bool compaction = false;
//
// 	for (int i = -1; i < fmax(MAX_Q, MAX_R)+1; i++) {
// 		for (int q = -1; q < MAX_Q+1; q++) {
// 			for (int r = 0; r < MAX_R - 1; r++) {
// 				AxialHex axial = {q, r};
//
// 				AxialHex a1 = axialHexAdd(axial, offset1);
// 				AxialHex a2 = axialHexAdd(axial, offset2);
// 				if (!hexTileInRange(a1) || !hexTileInRange(a2)) {
// 					continue;
// 				}
//
// 				float one = hexTileGet(a1);
// 				float two = hexTileGet(a2);
//
// 				// remove gap
// 				if (one >= TILE_MIN_POPULATED && two == TILE_EMPTY) {
// 					hexTileSet(a1, two);
// 					hexTileSet(a2, one);
// 					compaction = true;
// 				}
// 			}
// 		}
// 	}
//
// 	return compaction;
// }

bool cubeEql(CubeHex one, CubeHex two) {
	return (one.q == two.q && one.r == two.r && one.s == two.s);
}

CubeHex cubeRotateCartCCOne(CubeHex cube) {
	/*
	https://www.redblobgames.com/grids/hexagons/#rings
	A rotation 60° left (counter-clockwise ↺) shoves each coordinate one slot to the right →:
				 [ q,  r,  s]
		 to  [-s, -q, -r]
	  to [r,  s,  q]
	*/
	CubeHex cube2 = {- cube.s, - cube.q, - cube.r};
	return cube2;
}



void moveHexagons(ClickableKind clickableKind) {
		LibHexDirection arrowDirection = clickableToHexDir(clickableKind);

		LibHexDirection dir = (arrowDirection + 3) % 6;

		// move2(direction);
		int one = directionEdgeOne(dir);
		int two = directionEdgeTwo(dir);
		CubeHex startOneCubeCart = findEdgeStartCubeCart(one);
		CubeHex startTwoCubeCart = findEdgeStartCubeCart(two);

		CubeHex endOneCubeCart = findEdgeEndCubeCart(one);
		CubeHex endTwoCubeCart = findEdgeEndCubeCart(two);

		CubeHex dirOneCubeCart = findEdgeDirectionOffsetCubeCart(one);
		CubeHex dirTwoCubeCart = findEdgeDirectionOffsetCubeCart(two);


		CubeHex innerDir = cubeRotateCartCCOne(dirTwoCubeCart);

		tileRecentlyCompacted = false;
		tileRecentlyMerged = false;

		{
			CubeHex edge1 = startOneCubeCart;
			do {
				LibHexDirection dir2 = (dir + 3) % 6;
				walkLine(cubeToAxial(edge1), dir2);

				edge1 = cubeAdd(edge1, dirOneCubeCart);
			} while (!cubeEql(edge1, endOneCubeCart));
		};


		{
				CubeHex edge1 = startTwoCubeCart;
				do {
					LibHexDirection dir2 = (dir + 3) % 6;
					walkLine(cubeToAxial(edge1), dir2);

					edge1 = cubeAdd(edge1, dirTwoCubeCart);
				} while (!cubeEql(edge1, endTwoCubeCart));
		};

		spawnRandomHexTile();
}

	//
	// tileRecentlyMerged = false;
	// bool recentCompaction = false;
	// bool tileMerged[MAX_Q][MAX_R] = {0};
	//
	//
	// if (clickableKindHasFlag(direction, CLICKABLE_DIR_SE)) {
	// 	// hacky compactions
	// 	recentCompaction |= compactHexagons((AxialHex){0, 0}, (AxialHex){0, 1});
	//
	// 	// merge neighbours
	// 	for (int q = 0; q < MAX_Q; q++) {
	// 		for (int r = 0; r < MAX_R - 1; r++) {
	// 			// move all r to r-1 when r+1 is empty and r is occupied
	// 			float one = hexTiles.hex[q][r];
	// 			float two = hexTiles.hex[q][r+1];
	//
	// 			// remove gap
	// 			bool isSameNonzeroKind = (one >= TILE_MIN_POPULATED) && two == one;
	// 			if (isSameNonzeroKind) {
	// 				if (tileMerged[q][r] || tileMerged[q][r+1]) {
	// 					continue;
	// 				}
	//
	// 				tileMerged[q][r+1] = true;
	//
	// 				hexTiles.hex[q][r+1] = one + one;
	// 				hexTiles.hex[q][r] = TILE_EMPTY;
	// 			}
	// 		}
	// 	}
	//
	// 	recentCompaction |= compactHexagons((AxialHex){0, 0}, (AxialHex){0, 1});
	// 	// // hacky compactions
	// 	// for (int i = 0; i < fmaxf(MAX_Q, MAX_R); i++) {
	// 	// 	for (int q = 0; q < MAX_Q; q++) {
	// 	// 		for (int r = 0; r < MAX_R - 1; r++) {
	// 	// 			// move all r to r-1 when r+1 is empty and r is occupied
	// 	// 			float one = hexTiles.hex[q][r];
	// 	// 			float two = hexTiles.hex[q][r+1];
	// 	//
	// 	// 			// remove gap
	// 	// 			if (one >= TILE_MIN_POPULATED && two == TILE_EMPTY) {
	// 	// 				hexTiles.hex[q][r+1] = one;
	// 	// 				hexTiles.hex[q][r] = two;
	// 	// 				recentCompaction = true;
	// 	// 			}
	// 	// 		}
	// 	// 	}
	// 	// }
	//
	//
	// } else if (clickableKindHasFlag(direction, CLICKABLE_DIR_NW)) {
	// 	recentCompaction |= compactHexagons((AxialHex){0, 1}, (AxialHex){0, 0});
	// 	// // hacky compactions
	// 	// for (int i = 0; i < fmaxf(MAX_Q, MAX_R); i++) {
	// 	// 	for (int q = 0; q < MAX_Q; q++) {
	// 	// 		for (int r = 0; r < MAX_R - 1; r++) {
	// 	// 			// move all r to r-1 when r+1 is empty and r is occupied
	// 	// 			float one = hexTiles.hex[q][r+1];
	// 	// 			float two = hexTiles.hex[q][r];
	// 	//
	// 	// 			// remove gap
	// 	// 			if (one >= TILE_MIN_POPULATED && two == TILE_EMPTY) {
	// 	// 				hexTiles.hex[q][r] = one;
	// 	// 				hexTiles.hex[q][r+1] = two;
	// 	// 				recentCompaction = true;
	// 	// 			}
	// 	// 		}
	// 	// 	}
	// 	// }
	//
	//
	// 	// merge neighbours
	// 	for (int q = 0; q < MAX_Q; q++) {
	// 		for (int r = 0; r < MAX_R - 1; r++) {
	// 			// move all r+1 to r when r is empty and r+1 is occupied
	// 			float one = hexTiles.hex[q][r+1];
	// 			float two = hexTiles.hex[q][r];
	//
	// 			// remove gap
	// 			bool isSameNonzeroKind = (two >= TILE_MIN_POPULATED) && two == one;
	// 			if (isSameNonzeroKind) {
	// 				if (tileMerged[q][r] || tileMerged[q][r+1]) {
	// 					continue;
	// 				}
	//
	// 				tileMerged[q][r] = true;
	//
	// 				hexTiles.hex[q][r] = one + one;
	// 				hexTiles.hex[q][r+1] = TILE_EMPTY;
	// 			}
	// 		}
	// 	}
	//
	// 	recentCompaction |= compactHexagons((AxialHex){0, 1}, (AxialHex){0, 0});
	// 	// // hacky compactions
	// 	// for (int i = 0; i < fmaxf(MAX_Q, MAX_R); i++) {
	// 	// 	for (int q = 0; q < MAX_Q; q++) {
	// 	// 		for (int r = 0; r < MAX_R - 1; r++) {
	// 	// 			// move all r to r-1 when r+1 is empty and r is occupied
	// 	// 			float one = hexTiles.hex[q][r+1];
	// 	// 			float two = hexTiles.hex[q][r];
	// 	//
	// 	// 			// remove gap
	// 	// 			if (one >= TILE_MIN_POPULATED && two == TILE_EMPTY) {
	// 	// 				hexTiles.hex[q][r] = one;
	// 	// 				hexTiles.hex[q][r+1] = two;
	// 	// 				recentCompaction = true;
	// 	// 			}
	// 	// 		}
	// 	// 	}
	// 	// }
	//
	// }
	//
	// else if (clickableKindHasFlag(direction, CLICKABLE_DIR_W)) {
	// 	recentCompaction |= compactHexagons((AxialHex){1, 1}, (AxialHex){0, 0});
	// 	// hacky compactions
	// 	// for (int i = 0; i < fmaxf(MAX_Q, MAX_R); i++) {
	// 	// 	for (int r = 0; r < MAX_R; r++) {
	// 	// 		for (int q = 0; q < MAX_Q - 1; q++) {
	// 	//
	// 	// 			float one = hexTiles.hex[q+1][r];
	// 	// 			float two = hexTiles.hex[q][r];
	// 	//
	// 	// 			// remove gap
	// 	// 			if (one >= TILE_MIN_POPULATED && two == TILE_EMPTY) {
	// 	// 				hexTiles.hex[q][r] = one;
	// 	// 				hexTiles.hex[q+1][r] = two;
	// 	// 				recentCompaction = true;
	// 	// 			}
	// 	// 		}
	// 	// 	}
	// 	// }
	//
	// 	// merge neighbours
	// 	for (int r = 0; r < MAX_R - 1; r++) {
	// 		for (int q = 0; q < MAX_Q -1; q++) {
	// 			// move all r+1 to r when r is empty and r+1 is occupied
	// 			float one = hexTiles.hex[q+1][r];
	// 			float two = hexTiles.hex[q][r];
	//
	// 			// remove gap
	// 			bool isSameNonzeroKind = (two >= TILE_MIN_POPULATED) && two == one;
	// 			if (isSameNonzeroKind) {
	// 				if (tileMerged[q][r] || tileMerged[q+1][r]) {
	// 					continue;
	// 				}
	//
	// 				tileMerged[q][r] = true;
	//
	// 				hexTiles.hex[q][r] = one + one;
	// 				hexTiles.hex[q+1][r] = TILE_EMPTY;
	// 			}
	// 		}
	// 	}
	//
	// 	recentCompaction |= compactHexagons((AxialHex){1, 1}, (AxialHex){0, 0});
	//
	// 	// // hacky compactions
	// 	// for (int i = 0; i < fmaxf(MAX_Q, MAX_R); i++) {
	// 	// 	for (int r = 0; r < MAX_R; r++) {
	// 	// 		for (int q = 0; q < MAX_Q - 1; q++) {
	// 	//
	// 	// 			float one = hexTiles.hex[q+1][r];
	// 	// 			float two = hexTiles.hex[q][r];
	// 	//
	// 	// 			// remove gap
	// 	// 			if (one >= TILE_MIN_POPULATED && two == TILE_EMPTY) {
	// 	// 				hexTiles.hex[q][r] = one;
	// 	// 				hexTiles.hex[q+1][r] = two;
	// 	// 				recentCompaction = true;
	// 	// 			}
	// 	// 		}
	// 	// 	}
	// 	// }
	//
	// } else if (clickableKindHasFlag(direction, CLICKABLE_DIR_E)) {
	// 	recentCompaction |= compactHexagons((AxialHex){0,0}, (AxialHex){1,0});
	//
	// 	// // hacky compactions
	// 	// for (int i = 0; i < fmaxf(MAX_Q, MAX_R); i++) {
	// 	// 	for (int r = 0; r < MAX_R; r++) {
	// 	// 		for (int q = 0; q < MAX_Q - 1; q++) {
	// 	//
	// 	// 			float one = hexTiles.hex[q][r];
	// 	// 			float two = hexTiles.hex[q+1][r];
	// 	//
	// 	// 			// remove gap
	// 	// 			if (one >= TILE_MIN_POPULATED && two == TILE_EMPTY) {
	// 	// 				hexTiles.hex[q+1][r] = one;
	// 	// 				hexTiles.hex[q][r] = two;
	// 	// 				recentCompaction = true;
	// 	// 			}
	// 	// 		}
	// 	// 	}
	// 	// }
	//
	// 	// merge neighbours
	// 	for (int r = 0; r < MAX_R - 1; r++) {
	// 		for (int q = 0; q < MAX_Q -1; q++) {
	// 			// move all r+1 to r when r is empty and r+1 is occupied
	// 			float one = hexTiles.hex[q][r];
	// 			float two = hexTiles.hex[q+1][r];
	//
	// 			// remove gap
	// 			bool isSameNonzeroKind = (two >= TILE_MIN_POPULATED) && two == one;
	// 			if (isSameNonzeroKind) {
	// 				if (tileMerged[q][r] || tileMerged[q][r]) {
	// 					continue;
	// 				}
	//
	// 				tileMerged[q+1][r] = true;
	//
	// 				hexTiles.hex[q+1][r] = one + one;
	// 				hexTiles.hex[q][r] = TILE_EMPTY;
	// 			}
	// 		}
	// 	}
	//
	// 	recentCompaction |= compactHexagons((AxialHex){0,0}, (AxialHex){1,0});
	//
	// 	//
	// 	// // hacky compactions
	// 	// for (int i = 0; i < fmaxf(MAX_Q, MAX_R); i++) {
	// 	// 	for (int r = 0; r < MAX_R; r++) {
	// 	// 		for (int q = 0; q < MAX_Q - 1; q++) {
	// 	//
	// 	// 			float one = hexTiles.hex[q][r];
	// 	// 			float two = hexTiles.hex[q+1][r];
	// 	//
	// 	// 			// remove gap
	// 	// 			if (one >= TILE_MIN_POPULATED && two == TILE_EMPTY) {
	// 	// 				hexTiles.hex[q+1][r] = one;
	// 	// 				hexTiles.hex[q][r] = two;
	// 	// 				recentCompaction = true;
	// 	// 			}
	// 	// 		}
	// 	// 	}
	// 	// }
	//
	// } else if (clickableKindHasFlag(direction, CLICKABLE_DIR_SW)) {
	// 	recentCompaction |= compactHexagons((AxialHex){1,0}, (AxialHex){0,0});
	//
	//
	// 	// hacky compactions
	// 	for (int i = 0; i < fmaxf(MAX_Q, MAX_R); i++) {
	// 		for (int r = 0; r < MAX_R; r++) {
	// 			for (int q = 0; q < MAX_Q; q++) {
	// 				AxialHex here = {q, r};
	// 				AxialHex next = axialDirectNeighbour(here,  HEXN_SW, 1);
	// 				if ((here.q >= MAX_Q || here.r >= MAX_R || here.q < 0 || here.r < 0) ||
	// 					(next.q >= MAX_Q || next.r >= MAX_R || next.q < 0 || next.r < 0)
	// 				) {
	// 					continue;
	// 				}
	//
	// 				float hereVal = hexTiles.hex[(int)here.q][(int)here.r];
	// 				float nextVal = hexTiles.hex[(int)next.q][(int)next.r];
	// 				if (nextVal == TILE_EMPTY && hereVal >= TILE_MIN_POPULATED) {
	// 					hexTiles.hex[(int)next.q][(int)next.r] = hereVal;
	// 					hexTiles.hex[(int)here.q][(int)here.r] = TILE_EMPTY;
	// 					recentCompaction = true;
	// 				}
	// 			}
	// 		}
	// 	}
	//
	// 	// merge neighbours
	// 	for (int r = 0; r < MAX_R; r++) {
	// 		// for (int s = 0; s < MAX_R -1 /*probably fine?*/; s++) {
	// 		for (int q = 0; q < MAX_Q; q++) {
	//
	// 			// nb: the board is displayed with a skew, so direction values are one different
	// 			AxialHex here = {q, r};
	// 			AxialHex next = axialDirectNeighbour(here,  HEXN_SW, 1);
	// 			if ((here.q >= MAX_Q || here.r >= MAX_R || here.q < 0 || here.r < 0) ||
	// 				(next.q >= MAX_Q || next.r >= MAX_R || next.q < 0 || next.r < 0)
	// 			) {
	// 				continue;
	// 			}
	//
	// 			// move all r+1 to r when r is empty and r+1 is occupied
	// 			float one = hexTiles.hex[(int)here.q][(int)here.r];
	// 			float two = hexTiles.hex[(int)next.q][(int)next.r];
	//
	// 			// remove gap
	// 			bool isSameNonzeroKind = (two >= TILE_MIN_POPULATED) && two == one;
	// 			if (isSameNonzeroKind) {
	// 				if (tileMerged[(int) here.q][(int) here.r] || tileMerged[(int) next.q][(int) next.r]) {
	// 					continue;
	// 				}
	//
	// 				tileMerged[(int) here.q][(int) here.r] = true;
	//
	// 				hexTiles.hex[(int) next.q][(int) next.r] = one + one;
	// 				hexTiles.hex[(int) here.q][(int) here.r] = TILE_EMPTY;
	// 			}
	// 		}
	//
	// 	}
	//
	// 	recentCompaction |= compactHexagons((AxialHex){1,0}, (AxialHex){0,0});
	//
	//
	// 	// // hacky compactions
	// 	// for (int i = 0; i < fmaxf(MAX_Q, MAX_R); i++) {
	// 	// 	for (int r = 0; r < MAX_R; r++) {
	// 	// 		for (int q = 0; q < MAX_Q; q++) {
	// 	// 			AxialHex here = {q, r};
	// 	// 			AxialHex next = axialDirectNeighbour(here,  HEXN_SW, 1);
	// 	// 			if ((here.q >= MAX_Q || here.r >= MAX_R || here.q < 0 || here.r < 0) ||
	// 	// 				(next.q >= MAX_Q || next.r >= MAX_R || next.q < 0 || next.r < 0)
	// 	// 			) {
	// 	// 				continue;
	// 	// 			}
	// 	//
	// 	// 			float hereVal = hexTiles.hex[(int)here.q][(int)here.r];
	// 	// 			float nextVal = hexTiles.hex[(int)next.q][(int)next.r];
	// 	// 			if (nextVal == TILE_EMPTY && hereVal >= TILE_MIN_POPULATED) {
	// 	// 				hexTiles.hex[(int)next.q][(int)next.r] = hereVal;
	// 	// 				hexTiles.hex[(int)here.q][(int)here.r] = TILE_EMPTY;
	// 	// 				recentCompaction = true;
	// 	// 			}
	// 	// 		}
	// 	// 	}
	// 	// }
	//
	//
	// } else {
	// 	GAME_ASSERT(false);
	// }
	//
	// // notify of one or more tile merges
	// for (int q = 0; q < MAX_Q; q++) {
	// 	for (int r = 0; r < MAX_R; r++) {
	// 		if (tileMerged[q][r]) {
	// 			tileRecentlyMerged = true;
	// 		}
	// 	}
	// }
	//
	// if (recentCompaction) {
	// 	tileRecentlyCompacted = true;
	// }

// }