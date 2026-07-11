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
#include "../include/draw_hex_tiles.h"
#include "../include/move_hexagons.h"

HexTilesQr hexTiles = {0};

const float TILE_UNUSED = -1;	// hidden corner
const float TILE_EMPTY = 0;
const float TILE_MIN_POPULATED = 1;

Color clrYellow  = {224,243,89, 255}  ;
Color clrOrange     = {236,143,18, 255}  ;
Color clrDarkGreen  = {89,140,10, 255}   ;
Color clrLightGreen   = {111,223,77, 255}  ;
Color clrRed        = {200,13,77, 255}   ;

RenderTexture2D gameRenderTexture = {0};
Rectangle gameRenderTextureSize = {.x=0, .y=0,.width= 1000, .height=1000};
Rectangle gameDestinationScreenSize = {0, 0, 720, 720};

bool debugTiles = false;

AxialHex newTile = {0};
AxialHex prevNewTile = {0};
float deltaSinceNewTile = 0;


bool gameTerminateWindowImmediately = false;
const char* gameVersion = "0.3";


bool canSpawnTile();	// fwd declaration of local symbol

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



// --- end of header implementation --- //

typedef struct {
	Rectangle position;
	ClickMove direction;
	float tickCooldownCurrent;
	float tickCooldownMax;
} GameClickable;


typedef struct {
	bool tickSpawnRandomHexTile;

} GameState;



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


GameState gameState = {0};

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
