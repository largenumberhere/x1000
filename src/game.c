#include "../include/game.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#include "raylib.h"
#include "raymath.h"
#include "../include/game_error.h"
#include "../include/get_mouse_pos_rel.h"
#ifndef PLATFORM_DESKTOP
	#include "emscripten/emscripten.h"
#endif
#include "../include/game.h"
#include "../include/hexagons.h"

RenderTexture2D gameRenderTexture = {0};
Rectangle gameRenderTextureSize = {.x=0, .y=0,.width= 1000, .height=1000};
Rectangle gameDestinationScreenSize = {0, 0, 720, 720};

bool gameTerminateWindowImmediately = false;
const char* gameVersion = "0.2";

typedef struct {
	Rectangle position;
	HexDirection direction;
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
	GameClickable upArrow = {.direction = HEXN_SE, .position = {0, 0, 90, 90}, .tickCooldownCurrent = 1000, .tickCooldownMax = 1000};
	// GameClickable downArrow = {.direction = GameDirectionDown, .position = {0, 100, 90, 90}, .tickCooldownCurrent = 1000, .tickCooldownMax = 1000};
	// GameClickable leftArrow = {.direction = GameDirectionLeft, .position = {0, 200, 90, 90}, .tickCooldownCurrent = 1000, .tickCooldownMax = 1000};
	// GameClickable rightArrow = {.direction = GameDirectionRight, .position = {0, 300, 90, 90}, .tickCooldownCurrent = 1000, .tickCooldownMax = 1000};

	clickables[clickableCount++] = upArrow;
	// clickables[clickableCount++] = downArrow;
	// clickables[clickableCount++] = leftArrow;
	// clickables[clickableCount++] = rightArrow;
}

void drawClickables() {
	for (int i = 0; i < clickableCount; i++) {
		if (clickables[i].tickCooldownCurrent < clickables[i].tickCooldownMax) {
			DrawRectangleRec(clickables[i].position, BLUE);
		} else {
			DrawRectangleRec(clickables[i].position, BROWN);
		}

		int size = 80;
		Vector2 pos = { clickables[i].position.x, clickables[i].position.y};
		switch (clickables[i].direction) {
			case HEXN_SE:
				DrawText("South East \\", pos.x, pos.y, size, WHITE);
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

void spawnRandomHexTile() {
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
Vector2 hexagonCorner(Vector2 centerPos, float size, float sideNumber) {
	float degrees = 60 * sideNumber - 30;
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



void drawHexagon(Vector2 centrePoint, float size, Color color1, Color color2) {
	// Draw 6 triangles to compose the shape, starting from the centre point.
	// Wraps around for the first one to always have 3 valid points for every single valid triangle
	for (int i = 1; i < 6 + 1; i++) {
		Vector2 leftPoint = hexagonCorner(centrePoint, size, i -1);
		Vector2 rightPoint = hexagonCorner(centrePoint, size, i);

		DrawTriangle(centrePoint, rightPoint, leftPoint, color1);
	}

	// Draw outline
	// (A big choppy on corners, but shhh it's fine with small line tickness.)
	for (int i = 1; i < 6 + 1; i++) {
		Vector2 leftPoint = hexagonCorner(centrePoint, size, i - 1);
		Vector2 rightPoint = hexagonCorner(centrePoint, size, i);
		DrawLineEx(leftPoint, rightPoint, 3, color2);
	}
}



/*
*  Next we want to put several hexagons together.
*  The spacing will depend on both the outer circle's radius (size) and the inner circle's radius (inradius).

In the pointy orientation, the horizontal distance between adjacent hexagon centers is horiz = width == sqrt(3) * size == 2 * inradius.
The vertical distance is vert = 3/4 * height == 3/2 * size.
 *
 *
 */


void moveHexagons(HexDirection direction) {
	if (direction == HEXN_SE) {
		// hacky compactions
		for (int i = 0; i < fmaxf(MAX_Q, MAX_R); i++) {
			for (int q = 0; q < MAX_Q; q++) {
				for (int r = 0; r < MAX_R - 1; r++) {
					// move all r to r-1 when r+1 is empty and r is occupied
					float one = hexTiles.hex[q][r];
					float two = hexTiles.hex[q][r+1];

					if (one >= TILE_MIN_POPULATED && two == TILE_EMPTY) {
						hexTiles.hex[q][r+1] = one;
						hexTiles.hex[q][r] = two;
					}
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
	Vector2 tilesInset = {40, 200};
	float tileSize = 75;

	// draw grid
	char buff[1024] = {0};
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
				drawHexagon(px, tileSize, BROWN, BLACK);
			} else {
				if (debugTiles) {
					drawHexagon(px, tileSize, ORANGE, BLACK);
				}
			}
			buff[0] = '\0';
		}
	}

	// draw grid spiral
	AxialHex centre = {2, 2};
	for (int i = 0; i < 6; i++) {
		AxialHex axial = axialDirectNeighbour(centre, i, 1);

		Vector2 px = hexToVec2(axial, tileSize);
		px.x += tilesInset.x; // offset from the top left
		px.y += tilesInset.y;

		px.x += (hexHorizDiff(tileSize) / 2);
		px.y += (hexVerticalDiff(tileSize) / 2);

		drawHexagon(px, tileSize, GRAY, BLACK);
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


			drawHexagon(px, tileSize - 20, BLUE, BLACK);
			buff[0] = '\0';
		}
	}
}




int clickCount = 0;
char fmtBuff [512] = {0};
void gameDraw() {
	BeginTextureMode(gameRenderTexture);
	{
		ClearBackground(RAYWHITE);
		drawClickables();
		drawHexTiles();
		sprintf(fmtBuff, "%i", clickCount);
		DrawText(fmtBuff, 1, 1, 40, RED);

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
				clickCount += 1;
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


float tSinceLastSpawn = 0;
void gameTick() {
	tickClickables();
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
