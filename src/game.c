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


int debug_box_x = 0;
bool gameTerminateWindowImmediately = false;


typedef enum clickableKind_t {
	GameDirectionUp =		1,
	GameDirectionDown =		2,
	GameDirectionLeft =		3,
	GameDirectionRight =	4,
} GameDirection;

typedef struct {
	int tileValue[6][6];
} TileValues;

static TileValues tile_values = {0};
int getTileValue(int x, int y) {
	return tile_values.tileValue[y][x];
}

int* getTileValuePtr(int x, int y) {
	return (tile_values.tileValue[y]) + x;
}

void setTileValue(int x, int y, int value) {
	tile_values.tileValue[y][x] = value;
}

void initTiles() {
	setTileValue(1, 3, 0x1);
	setTileValue(1,4, 0x1);

}

void tickTiles() {

}

void moveTilesUp() {}

void mergeTilesDown() {

}

void moveTilesDown() {
	for (int x = 0; x < 6; x++) {
		int top = 0;
		int btm = 6-1;
		while (top < 6) {
			int topVal = getTileValue(x, top);
			int btmVal =getTileValue(x, btm);
			if (btmVal == 0 && topVal != 0) {
				int btmCopy = btmVal;
				btmVal = topVal;
				topVal = btmCopy;

				setTileValue(x, top, topVal);
				setTileValue(x, btm, btmVal);

				btm --;
			}


			top ++;
		}


	}
}
void moveTilesLeft() {

}
void moveTilesRight() {}

void shiftTiles(GameDirection direction) {
	switch (direction) {
		case GameDirectionUp:
			moveTilesUp();
			break;
		case GameDirectionDown:
			moveTilesDown();
			mergeTilesDown();
			break;
		case GameDirectionLeft:
			moveTilesLeft();
			break;
		case GameDirectionRight:
			moveTilesRight();
			break;
		default: GAME_ASSERT(false);
	}
}


typedef struct {
	Rectangle position;
	GameDirection direction;
	float tickCooldownCurrent;
	float tickCooldownMax;
} GameClickable;


#define MAX_CLICKABLE 64
static GameClickable clickables[MAX_CLICKABLE];
static int64_t clickableCount = 0;


void initClickables() {
	GameClickable upArrow = {.direction = GameDirectionUp, .position = {0, 0, 90, 90}, .tickCooldownCurrent = 1000, .tickCooldownMax = 1000};
	GameClickable downArrow = {.direction = GameDirectionDown, .position = {0, 100, 90, 90}, .tickCooldownCurrent = 1000, .tickCooldownMax = 1000};
	GameClickable leftArrow = {.direction = GameDirectionLeft, .position = {0, 200, 90, 90}, .tickCooldownCurrent = 1000, .tickCooldownMax = 1000};
	GameClickable rightArrow = {.direction = GameDirectionRight, .position = {0, 300, 90, 90}, .tickCooldownCurrent = 1000, .tickCooldownMax = 1000};

	clickables[clickableCount++] = upArrow;
	clickables[clickableCount++] = downArrow;
	clickables[clickableCount++] = leftArrow;
	clickables[clickableCount++] = rightArrow;
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
			case GameDirectionUp:
				DrawText("^", pos.x, pos.y, size, WHITE);

				break;
			case GameDirectionDown:
				DrawText("v", pos.x, pos.y, size, WHITE);

				break;
			case GameDirectionLeft:
				DrawText("<", pos.x, pos.y, size, WHITE);

				break;
			case GameDirectionRight:
				DrawText(">", pos.x, pos.y, size, WHITE);

				break;
			default: GAME_ASSERT(false);
		}
	}

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
	initTiles();
}


void drawTiles() {
	float offX = 100;
	float offY = 100;
	float width = 120;
	float height = 120;
	float padx = 15;
	float pady = 15;
	for (int y = 0; y < 6; y++) {
		for (int x = 0; x < 6; x++) {
			if (getTileValue(x, y) > 0) {
				Rectangle rec = {.x = (float)x * width + offX + (float)x * padx, .y = (float)y * height + offY + (float)y * pady, width, height};
				DrawRectangleRec(rec, BLUE);
			}
		}
	}
}
void drawGrid() {
	float offX = 100;
	float offY = 100;
	float width = 125;
	float height = 125;
	float padx = 10;
	float pady = 10;
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			Rectangle rec = {.x = (float)i * width + offX + (float)i * padx, .y = (float)j * height + offY + (float)j * pady, width, height};
			DrawRectangleRec(rec, WHITE);
		}
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
Vector2 hexagonCorner(Vector2 centerPos, float size, float sideNumber) {
	float degrees = 60 * sideNumber - 30;
	float radians = PI / 180 * degrees;

	Vector2 point = {centerPos.x + size * cos(radians),
		centerPos.y + size * sin(radians)
	};
	return point;
}

// draws a hexagon by triangles and points
void drawHexagon(Vector2 centrePoint, float size) {
	// Draw 6 triangles to compose the shape, starting from the centre point.
	// Wraps around for the first one to always have 3 valid points for every single valid triangle
	for (int i = 1; i < 6 + 1; i++) {
		Vector2 leftPoint = hexagonCorner(centrePoint, size, i -1);
		Vector2 rightPoint = hexagonCorner(centrePoint, size, i);

		DrawTriangle(centrePoint, rightPoint, leftPoint, BROWN);
	}

	// Draw outline
	// (A big choppy on corners, but shhh it's fine with small line tickness.)
	for (int i = 1; i < 6 + 1; i++) {
		Vector2 leftPoint = hexagonCorner(centrePoint, size, i - 1);
		Vector2 rightPoint = hexagonCorner(centrePoint, size, i);
		DrawLineEx(leftPoint, rightPoint, 3, DARKGRAY);
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

// // arbitrary number root of a number
// float froot(float value, float root_number) {
// 	return powf(value, 1 / root_number);
// }
//
// float root3(float value) {
// 	return froot(value, 3);
// }

void tileHexagonHorizEvenRow(int count, Vector2 centrePoint, float size) {
	float horiz = sqrtf(3) * size;
	float vert = ((float)3/(float)2) * size;
	for (int i = 0; i < count; i++) {
		Vector2 point = centrePoint;
		point.x += (i * horiz);
		drawHexagon(point, size);
	}

	// for horizontal tesselation purposes, the width of a hexagon can be considered root3 of its radius

}

float hexVerticalDiff(float size) {
	return (float)3/(float)2 * size;
}

float hexHorizDiff(float size) {
	return sqrtf(3) * size;
}

void tileHexagonVertOddRow(int count, Vector2 centrePoint, float size) {
	float horiz = sqrtf(3) * size;
	for (int i = 0; i < count; i++) {
		Vector2 point = centrePoint;
		point.x += (i * horiz);
		drawHexagon(point, size);
	}

}





void gameDraw() {
	BeginTextureMode(gameRenderTexture);
	{
		// drawGrid();
		// drawTiles();
		// drawClickables();
		// DrawText("Title", 450, 25, 55, RED);
		//
		//
		ClearBackground(RAYWHITE);
		// DrawRectangle(debug_box_x, 10, 30, 30, ORANGE);
		Vector2 point = {500, 500};
		// drawHexagon(point, 100);
		tileHexagonHorizEvenRow(4, point, 50);

		Vector2 point2 = {500 + hexHorizDiff(50)/2 , 500+ hexVerticalDiff(50)};
		tileHexagonVertOddRow(4, point2, 50);

	}
	EndTextureMode();


	BeginDrawing();
	{
		drawGameRenderTexture();
	}
	EndDrawing();
}




void tickClickables () {
	Vector2 mouse = getMousePosRel();

	// other mouse functions don't work on wasm! IsMouseButtonPressed(), IsMouseButtonReleased()
	bool wasLeftclickPressed = IsMouseButtonDown(MOUSE_LEFT_BUTTON);

	for (int i = 0; i < clickableCount; i++) {
		bool isButtonHovered = CheckCollisionPointRec(mouse, clickables[i].position);
		if (wasLeftclickPressed) {
			if (isButtonHovered) {
				clickables[i].tickCooldownCurrent = 0;
				shiftTiles(clickables[i].direction);
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

void gameTick() {
	debug_box_x += 10;
	tickClickables();

	// debugging utility on desktop
	#ifndef PLATFORM_WEB
	if (IsKeyDown(KEY_ESCAPE)) {
		gameTerminateWindowImmediately = true;
	}
	#endif

}




void gameUpdate() {
#ifndef PLATFORM_DESKTOP
	if (gameTerminateWindowImmediately) {
		emscripten_cancel_main_loop();
		BeginDrawing();
		ClearBackground(WHITE);
		DrawText("Game terminated. Please refresh window", 0, 0, 30, BLACK);
		EndDrawing();
		return;
	}
#endif


	if (gameErrored) {
		drawError();
		return;
	}

	gameDraw();
	PollInputEvents();
	gameTick();
}
