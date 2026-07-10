#include "../include/game.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#include "raylib.h"
#include "raymath.h"
#include <string.h>
#include "../include/game_error.h"
#include "../include/get_mouse_pos_rel.h"
#ifndef PLATFORM_DESKTOP
	#include "emscripten/emscripten.h"
#endif
#include "../include/game.h"
#include "../include/hexagons.h"


Color clrYellow  = {224,243,89, 255}  ;
Color clrOrange     = {236,143,18, 255}  ;
Color clrDarkGreen  = {89,140,10, 255}   ;
Color clrLightGreen   = {111,223,77, 255}  ;
Color clrRed        = {200,13,77, 255}   ;

RenderTexture2D gameRenderTexture = {0};
Rectangle gameRenderTextureSize = {.x=0, .y=0,.width= 1000, .height=1000};
Rectangle gameDestinationScreenSize = {0, 0, 720, 720};

bool gameTerminateWindowImmediately = false;
const char* gameVersion = "0.3";


void drawArrow(Vector2 headSpot, float angle, float length) {
	float thickness = 10;
	angle -= 180;
	Vector2 other = {0, length};
	float radiansFactor = (PI / 180);
	float radians = angle * radiansFactor;


	Vector2 rotated = Vector2Rotate(other, radians);
	Vector2 b = Vector2Add(headSpot, rotated);
	DrawLineEx(headSpot, b, thickness, clrOrange);


	Vector2 leftWedge = {0, 40};
	leftWedge = Vector2Rotate(leftWedge, radians);
	leftWedge = Vector2Rotate(leftWedge , 30*radiansFactor);
	leftWedge = Vector2Add(headSpot, leftWedge);
	DrawLineEx(headSpot, leftWedge, thickness, clrOrange);

	Vector2 rightWedge = {0, 40};
	rightWedge = Vector2Rotate(rightWedge, radians);
	rightWedge = Vector2Rotate(rightWedge , -30*radiansFactor);
	rightWedge = Vector2Add(headSpot, rightWedge);

	DrawLineEx(headSpot, rightWedge, thickness, clrOrange);

}


typedef enum {
	CLICK_MOVE_NW,
	CLICK_MOVE_E,
	CLICK_MOVE_NE,
	CLICK_MOVE_SE,
	CLICK_MOVE_W,
} ClickMove;


typedef struct {
	Rectangle position;
	ClickMove direction;
	float tickCooldownCurrent;
	float tickCooldownMax;
} GameClickable;


typedef struct {
	bool tickSpawnRandomHexTile;

} GameState;

GameState gameState = {0};


#define  MAX_Q 5
#define  MAX_R 5
typedef struct {
	float hex[MAX_Q][MAX_R];
} HexTilesQr;

const float TILE_UNUSED = -1;	// hidden corner
const float TILE_EMPTY = 0;
const float TILE_MIN_POPULATED = 1;


AxialHex newTile = {0};
AxialHex prevNewTile = {0};
float deltaSinceNewTile = 0;

static HexTilesQr hexTiles = {0};
void initHexTiles() {
	for (int q = 0; q < MAX_Q; q++) {
		for (int r = 0; r < MAX_R; r++) {
			hexTiles.hex[q][r] = (float)1;
			AxialHex hex = {q, r};
			AxialHex centre = {2, 2};
			float distance = axialManhattanDistance(centre, hex);
			if (distance > 2) {
				hexTiles.hex[q][r] = TILE_UNUSED;
			} else {
				hexTiles.hex[q][r] = 0;
			}
		}
	}
}

#define MAX_CLICKABLE 64
static GameClickable clickables[MAX_CLICKABLE];
static int64_t clickableCount = 0;


void initClickables() {
	GameClickable seArrow = {.direction = CLICK_MOVE_SE, .position = {700, 900, 90, 90}, .tickCooldownCurrent = 1000, .tickCooldownMax = 1000};
	GameClickable nwArrow = {.direction = CLICK_MOVE_NW, .position = {20, 100, 90, 90}, .tickCooldownCurrent = 1000, .tickCooldownMax = 1000};
	GameClickable neArrow = {.direction = CLICK_MOVE_E, .position = {20, 200, 90, 90}, .tickCooldownCurrent = 1000, .tickCooldownMax = 1000};
	GameClickable swArrow = {.direction = CLICK_MOVE_W, .position = {20, 300, 90, 90}, .tickCooldownCurrent = 1000, .tickCooldownMax = 1000};

	clickables[clickableCount++] = seArrow;
	clickables[clickableCount++] = nwArrow;

	clickables[clickableCount++] = neArrow;
	clickables[clickableCount++] = swArrow;
}

void drawCardinalArrow(ClickMove clickMoveDirection, Vector2 origin, float offsetX, float offsetY) {
	float angle = 180;	// the origin points down for the arrow's implementation;
	switch (clickMoveDirection) {
		case CLICK_MOVE_NW:
			angle += 310;
			break;
		case CLICK_MOVE_E:
			angle += 90;
			break;
		case CLICK_MOVE_NE:
			angle += 45;
			break;
		case CLICK_MOVE_SE:
			angle += 135;
			break;
		case CLICK_MOVE_W:
			angle += 270;
			break;
		default:
		GAME_ASSERT(false);
	}

	Vector2 offset = {offsetX, offsetY};
	Vector2 pos = Vector2Add(origin, offset);
	drawArrow(pos, angle, 100);
}

void drawClickables() {
	for (int i = 0; i < clickableCount; i++) {
		if (clickables[i].tickCooldownCurrent < clickables[i].tickCooldownMax) {
			DrawRectangleRec(clickables[i].position, clrYellow);
		} else {
			DrawRectangleRec(clickables[i].position, clrDarkGreen);
		}

		int size = 80;
		Vector2 pos = { clickables[i].position.x, clickables[i].position.y};

		switch (clickables[i].direction) {
			case CLICK_MOVE_SE:
				DrawText("SE", pos.x, pos.y, size, WHITE);
				drawCardinalArrow(clickables[i].direction, pos, 170, 70);
				break;
			case CLICK_MOVE_NW:
				DrawText("NW", pos.x, pos.y, size, WHITE);
				drawCardinalArrow(clickables[i].direction, pos, 100, 20);

				break;
			case CLICK_MOVE_E:
				DrawText("E", pos.x, pos.y, size, WHITE);
				drawCardinalArrow(clickables[i].direction, pos, 200, 50);
				break;
			case CLICK_MOVE_W:
				DrawText("W", pos.x, pos.y, size, WHITE);
				drawCardinalArrow(clickables[i].direction, pos, 100, 50);
				break;

			// case GameDirectionDown:
			// 	DrawText("v", pos.x, pos.y, size, WHITE);
			//
			// 	break;
			// case GameDirectionLeft:
			// 	DrawText("<", pos.x, pos.y, size, WHITE);
			//
			// 	break;
			// case GameDirectionRight:
			// 	DrawText(">", pos.x, pos.y, size, WHITE);
			//
			// 	break;
			default: GAME_ASSERT(false);
		}
	}

}

bool canSpawnTile() {
	for (int q = 0; q < MAX_Q; q++) {
		for (int r = 0; r < MAX_R; r++) {
			if (hexTiles.hex[q][r] == TILE_EMPTY) {
				return true;
			}
		}
	}

	return false;
}

void spawnRandomHexTile() {
	if (!canSpawnTile()) {
		return;
	}


	int q = 0;
	int r = 0;
	do {
		int qr = GetRandomValue(0, MAX_Q * MAX_R -1);
		q = qr % MAX_Q;
		r = qr / MAX_R;

		GAME_ASSERT(q < MAX_Q);
		GAME_ASSERT(r < MAX_R);
		GAME_ASSERT(q >= 0);
		GAME_ASSERT(r >= 0);

	} while (
		hexTiles.hex[q][r] == TILE_UNUSED ||
		hexTiles.hex[q][r] >= TILE_MIN_POPULATED
	);

	hexTiles.hex[q][r] = 0x1;
	AxialHex axial = {q, r};
	newTile = axial;
	prevNewTile = newTile;
	deltaSinceNewTile = 0;
}

void drawGameRenderTexture() {
	Rectangle flippedSource = gameRenderTextureSize;

	// rendering seems to start from bottom instead of top, flipping is needed
	flippedSource.height = -flippedSource.height;
	DrawTexturePro(gameRenderTexture.texture, flippedSource, gameDestinationScreenSize, Vector2Zero(), 0, WHITE);
}

void loadGameRenderTexture() {
	gameRenderTexture = LoadRenderTexture(gameRenderTextureSize.width, gameRenderTextureSize.height);
}

void gamePreInit1() {
	// anything needed before raylib starts
}

void gamePreInit2() {
	// boot up raylib
	InitWindow(gameDestinationScreenSize.width, gameDestinationScreenSize.height, "game");
}

void gamePreInit3() {
	// load any resources that depend on raylib
	loadGameRenderTexture();
	initClickables();
	initHexTiles();
	for (int i = 0; i < 2; i++) {
		spawnRandomHexTile();
	}
}

/* For flat hexagons. Computes the outer edge coordinate from the inner edge and a count of degrees.
 * Adapted from https://www.redblobgames.com/grids/hexagons/
 * Supports over 360 degrees somehow
 *
 *	  *****
 *	*      *
 * *		*
 *	*	   *
 *	  *****
 *	( ^ a flat-top hexagon  )
 */
Vector2 hexagonCorner(Vector2 centerPos, float size, float sideNumber, float offsetRoation) {
	float degrees = 60 * sideNumber - 30 + offsetRoation;
	float radians = PI / 180 * degrees;

	Vector2 point = {centerPos.x + size * cos(radians),
		centerPos.y + size * sin(radians)
	};
	return point;
}



float hexVerticalDiff(float size) {
	return (float)3/(float)2 * size;
}

float hexHorizDiff(float size) {
	return sqrtf(3) * size;
}


void drawHexagonOutline(Vector2 centrePoint, float size, Color color, float roationOffset) {
	// Draw outline
	// (A big choppy on corners, but shhh it's fine with small line tickness.)
	for (int i = 1; i < 6 + 1; i++) {
		Vector2 leftPoint = hexagonCorner(centrePoint, size, i - 1, roationOffset);
		Vector2 rightPoint = hexagonCorner(centrePoint, size, i, roationOffset);
		DrawLineEx(leftPoint, rightPoint, 3, color);
	}
}


void drawHexagon(Vector2 centrePoint, float size, Color color1, Color color2) {
	// Draw 6 triangles to compose the shape, starting from the centre point.
	// Wraps around for the first one to always have 3 valid points for every single valid triangle
	for (int i = 1; i < 6 + 1; i++) {
		Vector2 leftPoint = hexagonCorner(centrePoint, size, i -1, 0);
		Vector2 rightPoint = hexagonCorner(centrePoint, size, i, 0);

		DrawTriangle(centrePoint, rightPoint, leftPoint, color1);
	}

	drawHexagonOutline(centrePoint, size, color2, 0);

}

void drawHexagonR(Vector2 centrePoint, float size, Color color1, Color color2, float roation) {
	for (int i = 1; i < 6 + 1; i++) {
		Vector2 leftPoint = hexagonCorner(centrePoint, size, i -1, roation);
		Vector2 rightPoint = hexagonCorner(centrePoint, size, i, roation);

		DrawTriangle(centrePoint, rightPoint, leftPoint, color1);
	}

	drawHexagonOutline(centrePoint, size, color2, roation);
}


// Vector2 textMiddle(const char* text, float size) {
// 	Vector2 textSize = MeasureTextEx(GetFontDefault(), text, size, 0);
//
//
// }



void moveHexagons(ClickMove direction) {
	if (direction == CLICK_MOVE_SE) {
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

		bool tileMerged[MAX_Q][MAX_R] = {0};
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


		bool tileMerged[MAX_Q][MAX_R] = {0};
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

		bool tileMerged[MAX_Q][MAX_R] = {0};
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
		bool tileMerged[MAX_Q][MAX_R] = {0};
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

	spawnRandomHexTile();
}

bool debugTiles = false;
void drawHexTiles() {
	// draw bg hexagon
	{;

		float size = 400;
		Vector2 px = {gameRenderTextureSize.width / 2, gameDestinationScreenSize.height / 2};
		px.y += 100;


		drawHexagonR(px, size, clrYellow, clrDarkGreen, 30);
		drawHexagonR(px, size -10, clrDarkGreen, clrDarkGreen, 30);
	}

	Vector2 tilesInset = {40, 200};
	float tileSize = 75;



	// draw plain grid
	for (int r = 0; r < MAX_R; r++) {
		for (int q = 0; q < MAX_Q; q++) {
			AxialHex hexUnit = {q, r};
			bool isUnusedTile = hexTiles.hex[q][r] == TILE_UNUSED;

			Vector2 px = hexToVec2(hexUnit, tileSize);

			// centre the hex of hexes according to the normal display characteristics of a single hexagon.
			// this is not very precise and doesn't take into account the non-horizontal layout but it's nice
			px.x += (hexHorizDiff(tileSize) / 2);
			px.y += (hexVerticalDiff(tileSize) / 2);

			px.x += tilesInset.x; // offset from the top left
			px.y += tilesInset.y;


			if (!isUnusedTile) {
				drawHexagon(px, tileSize, clrLightGreen, BLACK);
			} else {
				if (debugTiles) {
					drawHexagon(px, tileSize, ORANGE, BLACK);
				}
			}
		}
	}


	{
		// draw dots in centres
		for (int r = 0; r < MAX_R; r++) {
			for (int q = 0; q < MAX_Q; q++) {
				AxialHex hexUnit = {q, r};

				Vector2 px = hexToVec2(hexUnit, tileSize);
				px.x += (hexHorizDiff(tileSize) / 2);
				px.y += (hexVerticalDiff(tileSize) / 2);

				px.x += tilesInset.x; // offset from the top left
				px.y += tilesInset.y;


				bool isUnusedTile = hexTiles.hex[q][r] == TILE_UNUSED;

				if (!isUnusedTile || debugTiles) {

					Color invisible = {0, 0, 0, 0};
					drawHexagon(px, 30, clrDarkGreen, clrDarkGreen);
					drawHexagon(px, 25, clrLightGreen, clrLightGreen);
				}
			}
		}
	}


	AxialHex centre = {2, 2};
	{
		// draw centre tile
		Vector2 px = hexToVec2(centre, tileSize);
		px.x += tilesInset.x; // offset from the top left
		px.y += tilesInset.y;

		px.x += (hexHorizDiff(tileSize) / 2);
		px.y += (hexVerticalDiff(tileSize) / 2);
		drawHexagon(px, tileSize, clrYellow, BLACK);
	}

	// draw tiles with non-zero value
	for (int r = 0; r < MAX_R; r++) {
		for (int q = 0; q < MAX_Q; q++) {
			AxialHex hexUnit = {q, r};
			if (!(hexTiles.hex[q][r] >= 1)) {
				continue;
			}

			Vector2 px = hexToVec2(hexUnit, tileSize);

			// centre the hex of hexes according to the normal display characteristics of a single hexagon.
			// this is not very precise and doesn't take into account the non-horizontal layout but it's nice
			px.x += (hexHorizDiff(tileSize) / 2);
			px.y += (hexVerticalDiff(tileSize) / 2);

			px.x += tilesInset.x; // offset from the top left
			px.y += tilesInset.y;


			drawHexagon(px, tileSize - 20, clrDarkGreen, clrOrange);
		}
	}

	char hexHexBuff[16] = {0};
	// draw text on hexagons
	for (int r = 0; r < MAX_R; r++) {
		for (int q = 0; q < MAX_Q; q++) {
			bool isTileEmpty = hexTiles.hex[q][r] == TILE_EMPTY;
			bool isUnusedTile = hexTiles.hex[q][r] == TILE_UNUSED;

			AxialHex hexUnit = {q, r};
			Vector2 px = hexToVec2(hexUnit, tileSize);

			// centre the hex of hexes according to the normal display characteristics of a single hexagon.
			// this is not very precise and doesn't take into account the non-horizontal layout but it's nice
			px.x += (hexHorizDiff(tileSize) / 2);
			px.y += (hexVerticalDiff(tileSize) / 2);

			px.x += tilesInset.x; // offset from the top left
			px.y += tilesInset.y;


			float value = hexTiles.hex[q][r];

			// float to string
			sprintf(hexHexBuff, "0x%x", (int) value);
			if (value == -1) {
			  hexHexBuff[0] = '\0';
				strcat(hexHexBuff, "-0x1");
			}
			//

			if ((!isUnusedTile || debugTiles) && !isTileEmpty) {
				DrawText(hexHexBuff, px.x, px.y, 25, GREEN);
			}
		}
	}

	// draw dots on the edges to visually indicate tile magnitude
	for (int r = 0; r < MAX_R; r++) {
		for (int q = 0; q < MAX_Q; q++) {
			AxialHex hexUnit = {q, r};

			Vector2 px = hexToVec2(hexUnit, tileSize);
			px.x += (hexHorizDiff(tileSize) / 2);
			px.y += (hexVerticalDiff(tileSize) / 2);

			px.x += tilesInset.x; // offset from the top left
			px.y += tilesInset.y;


			bool isUnusedTile = hexTiles.hex[q][r] == TILE_UNUSED;

			if (!isUnusedTile || debugTiles) {
				float tileValue = hexTiles.hex[q][r];
				int exponent =  (int) sqrtf(tileValue);
				exponent -= 1;
				exponent = Clamp(exponent, 0, 6);

				int max = (int) fminf(exponent, 6);
				for (int i = 0; i < max; i++) {

					Vector2 insetCorner = hexagonCorner(px, tileSize-25, i, 0);


					Vector2 insetCorner2 = hexagonCorner(px, tileSize-25, i+1, 0);

					Vector2 point1 = Vector2Lerp(insetCorner, insetCorner2, 0.25);
					Vector2 point2 = Vector2Lerp(insetCorner, insetCorner2, 0.75);


					DrawLineEx(point1, point2, 13, clrOrange);
				}
			}
		}
	}

	// draw new tile fade-in
	{
		Vector2 px = hexToVec2(newTile, tileSize);
		px.x += (hexHorizDiff(tileSize) / 2);
		px.y += (hexVerticalDiff(tileSize) / 2);

		px.x += tilesInset.x; // offset from the top left
		px.y += tilesInset.y;
		float animDurationSecs = 1.0;
		float ratio = deltaSinceNewTile / animDurationSecs;
		float alpha = Lerp(1, 0, ratio);

		drawHexagon(px, tileSize-25 ,ColorAlpha(ORANGE ,alpha), clrDarkGreen );
	}


}




void gameDraw() {
	BeginTextureMode(gameRenderTexture);
	{
		ClearBackground(RAYWHITE);

		drawClickables();
		drawHexTiles();


	}
	EndTextureMode();


	BeginDrawing();
	{
		drawGameRenderTexture();
	}
	EndDrawing();
}


float lmbCooldownGoal = 0.5f;
float lmbCooldown = 1.0f;

void tickClickables () {
	Vector2 mouse = getMousePosRel();

	// other mouse functions don't work on wasm! IsMouseButtonPressed(), IsMouseButtonReleased()
	// hacky button debounce logic
	bool wasLeftclickPressed = false;
	bool buttonDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
	lmbCooldown += GetFrameTime();
	if (buttonDown) {
		// assume if the user holds down the button, they want to spam it,
		// otherwise assume 0.5 seconds between valid clicks
		if (lmbCooldown >= lmbCooldownGoal) {
			wasLeftclickPressed = true;
			lmbCooldown = 0;
		}
	}


	for (int i = 0; i < clickableCount; i++) {
		bool isButtonHovered = CheckCollisionPointRec(mouse, clickables[i].position);
		if (wasLeftclickPressed && clickables[i].tickCooldownCurrent) {
			if (isButtonHovered) {
				clickables[i].tickCooldownCurrent = 0;
				moveHexagons(clickables[i].direction);
			}
		}
	}

	float frameMilis = ((float)GetFrameTime()) * 1000;

	for (int i = 0; i < clickableCount; i++) {
		if (clickables[i].tickCooldownCurrent < clickables[i].tickCooldownMax) {
			clickables[i].tickCooldownCurrent += frameMilis;
		}
	}

}

void tickControls() {
	/*
	 *
	* we (NW NE)
		ad (W E)
		yx (SW SE)
	 *
	 */
	// moveHexagons(clickables[i].direction);

}


float tSinceLastSpawn = 0;
void gameTick() {
	tickClickables();
	deltaSinceNewTile += GetFrameTime();
}



void gameUpdate() {
	PollInputEvents();	// apparently handles some wasm stuff too
	if (gameErrored) {
		drawError();
		return;
	}

	gameDraw();
	gameTick();
}
