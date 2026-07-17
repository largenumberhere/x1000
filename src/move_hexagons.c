
#include <math.h>

#include "game.h"
#include "game_error.h"
#include "clickables.h"
#include "move_hexagons.h"

#include "stdio.h"
#include "stdlib.h"

bool tileRecentlyMerged = false;
bool tileRecentlyCompacted = false;

// Cartesian here is somewhat of a misnomer. It just means that the origin is 0,0 and negative numbers are allowed.
// Array coordinates always start at zero and do not directly reflect the way the grid is displayed.
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


void walkLine(AxialHex pos, LibHexDirection iterDirection) {
	AxialHex posCopy = pos;

	for (int i = 0; i < 6; i++) {
		int visitMode = 1;
		AxialHex nextPos = axialDirectNeighbour(pos, iterDirection, 1);
		while (cartesianInBounds(pos) && cartesianInBounds(nextPos)) {
			visitPairCart(pos, nextPos,visitMode);

			pos = nextPos;
			nextPos = axialDirectNeighbour(nextPos, iterDirection, 1) ;
		}
	}

	for (int i = 0; i < 6; i++) {
		pos = posCopy;
		int visitMode = 0;
		AxialHex nextPos = axialDirectNeighbour(pos, iterDirection, 1);
		while (cartesianInBounds(pos) && cartesianInBounds(nextPos)) {
			visitPairCart(pos, nextPos,visitMode);

			pos = nextPos;
			nextPos = axialDirectNeighbour(nextPos, iterDirection, 1) ;
		}
	}

}

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

bool cubeEql(CubeHex one, CubeHex two) {
	return (one.q == two.q && one.r == two.r && one.s == two.s);
}


void moveHexagons(ClickableKind clickableKind) {
		LibHexDirection arrowDirection = clickableToHexDir(clickableKind);

		LibHexDirection dir = (arrowDirection + 3) % 6;

		int one = directionEdgeOne(dir);
		int two = directionEdgeTwo(dir);
		CubeHex startOneCubeCart = findEdgeStartCubeCart(one);
		CubeHex startTwoCubeCart = findEdgeStartCubeCart(two);

		CubeHex endOneCubeCart = findEdgeEndCubeCart(one);
		CubeHex endTwoCubeCart = findEdgeEndCubeCart(two);

		CubeHex dirOneCubeCart = findEdgeDirectionOffsetCubeCart(one);
		CubeHex dirTwoCubeCart = findEdgeDirectionOffsetCubeCart(two);

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
