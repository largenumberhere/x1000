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

void drawTextCentred(Vector2 centrePoint, Font font, const char* str, float textSize, Color color) {
	float spcaing = 0;

	Vector2 size = MeasureTextEx(font, str, textSize, spcaing);

	Vector2 two = {2,  2};
	Vector2 middle = Vector2Divide(size, two);

	Vector2 position = {centrePoint.x - middle.x, centrePoint.y - middle.y};

	DrawTextPro(GetFontDefault(), str, position, Vector2Zero(), 0, textSize, spcaing, color);
}



// --- end of header implementation --- //

typedef struct {
	Rectangle position;
	ClickMove direction;
	float tickCooldownCurrent;
	float tickCooldownMax;
	bool hovered;
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
	Color color = clrLightGreen;


	Vector2 rotated = Vector2Rotate(other, radians);
	Vector2 b = Vector2Add(headSpot, rotated);
	DrawLineEx(headSpot, b, thickness, color);


	Vector2 leftWedge = {0, 40};
	leftWedge = Vector2Rotate(leftWedge, radians);
	leftWedge = Vector2Rotate(leftWedge , 30*radiansFactor);
	leftWedge = Vector2Add(headSpot, leftWedge);
	DrawLineEx(headSpot, leftWedge, thickness, color);

	Vector2 rightWedge = {0, 40};
	rightWedge = Vector2Rotate(rightWedge, radians);
	rightWedge = Vector2Rotate(rightWedge , -30*radiansFactor);
	rightWedge = Vector2Add(headSpot, rightWedge);

	DrawLineEx(headSpot, rightWedge, thickness, color);
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
	GameClickable defaultClickable = {
		.direction = CLICK_MOVE_SE,
		.position = {750, 600, 250, 90},
		.tickCooldownCurrent = 1000,
		.tickCooldownMax = 300,
		.hovered = false
	};

	GameClickable seArrow = defaultClickable;
	GameClickable nwArrow = defaultClickable;
		nwArrow.direction = CLICK_MOVE_NW;
		nwArrow.position = (Rectangle) {15, 250, 250, 90};
	GameClickable eArrow = defaultClickable;
		eArrow.direction = CLICK_MOVE_E;
		eArrow.position = (Rectangle) {830, 420, 150, 90};

	GameClickable wArrow = defaultClickable;
		wArrow.direction = CLICK_MOVE_W;
		wArrow.position = (Rectangle) {20, 420, 150, 90};

	clickables[clickableCount++] = seArrow;
	clickables[clickableCount++] = nwArrow;

	clickables[clickableCount++] = eArrow;
	clickables[clickableCount++] = wArrow;

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
	drawArrow(pos, angle, 70);
}

Rectangle rectangleShrink(Rectangle rectangle, float pixels) {
	rectangle.x += pixels ;
	rectangle.y += pixels ;
	rectangle.width -= pixels * 2;
	rectangle.height -= pixels * 2;

	return rectangle;
}

Vector2 rectangleCentre(Rectangle rectangle) {
	float x = rectangle.width / 2 + rectangle.x;
	float y = rectangle.height /2 + rectangle.y;
	Vector2 vec = {x, y};

	return vec;
}

void drawClickables() {
	float roundedness = 0.8;
	int segments = 1;

	for (int i = 0; i < clickableCount; i++) {
		DrawRectangleRounded(clickables[i].position, roundedness, segments, clrDarkGreen);

		Rectangle r1 = rectangleShrink(clickables[i].position, 4);
		DrawRectangleRounded(r1, roundedness, segments, clrYellow);

		Rectangle r2 = rectangleShrink(clickables[i].position, 8);

		if (clickables[i].tickCooldownCurrent < clickables[i].tickCooldownMax) {
			DrawRectangleRounded(r2, roundedness, segments, WHITE);
			DrawRectangleRounded(r2, roundedness, segments, ColorAlpha(clrDarkGreen, 0.8));

		} else if (clickables[i].hovered) {
			DrawRectangleRounded(r2, roundedness, segments, clrYellow);
		} else {
			DrawRectangleRounded(r2, roundedness, segments, clrDarkGreen);
		}





		int size = 80;
		Vector2 pos = { clickables[i].position.x, clickables[i].position.y};

		Rectangle textSubRec = clickables[i].position;
		textSubRec.width -= 60;

		Color color = clrOrange;
		switch (clickables[i].direction) {
			case CLICK_MOVE_SE:
				drawTextCentred(rectangleCentre(textSubRec), GetFontDefault(), "SE", size, color);
				drawCardinalArrow(clickables[i].direction, pos, 170, 70);

				break;
			case CLICK_MOVE_NW:
				drawTextCentred(rectangleCentre(textSubRec), GetFontDefault(), "NW", size, color);
				drawCardinalArrow(clickables[i].direction, pos, 160, 20);

				break;
			case CLICK_MOVE_E:
				drawTextCentred(rectangleCentre(textSubRec), GetFontDefault(), "E", size, color);
				drawCardinalArrow(clickables[i].direction, pos, 137, 45);

				break;
			case CLICK_MOVE_W:
				drawTextCentred(rectangleCentre(textSubRec), GetFontDefault(), "W", size, color);
				drawCardinalArrow(clickables[i].direction, pos, 80, 45);

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

float frameAccum = 0;
void gameDraw() {
	frameAccum += GetFrameTime();
	BeginTextureMode(gameRenderTexture);
	{

		ClearBackground(WHITE);

		{
			// draw bg 'clouds'
			Vector2 tileOffset = {-8000, -9000};
			Vector2 reallyFarAway = {2000, 900};
			tileOffset = Vector2Lerp(tileOffset, reallyFarAway, fmodf(frameAccum / 500, 0.5));
			float tileSize = 200;
			for (int r = 0; r < 100; r++) {
				for (int q = 0; q < 100; q++) {
					AxialHex hexUnit = {q, r};
					Vector2 px = hexToVec2(hexUnit, tileSize);
					px = Vector2Add(px, tileOffset);


					if ((r * q) % 2 == 0) {
						Color color = ColorAlpha(clrLightGreen, 0.4);
						drawHexagonB(px, tileSize, color);
					} else {
						Color color = ColorAlpha(clrDarkGreen, 0.4);
						drawHexagonB(px, tileSize, color);
					}
				}
			}
		}

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
		if (isButtonHovered) {
			clickables[i].hovered = true;
		} else {
			clickables[i].hovered = false;
		}

		if (isButtonHovered && wasLeftclickPressed && clickables[i].tickCooldownCurrent) {
			clickables[i].tickCooldownCurrent = 0;
			moveHexagons(clickables[i].direction);
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
	tickControls();
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
