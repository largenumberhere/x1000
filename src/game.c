#include "../include/game.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#include "raylib.h"
#include "raymath.h"
#include "../include/game_error.h"
#include "../include/get_mouse_pos_rel.h"

int debug_box_x = 0;



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




void gameDraw() {
	BeginTextureMode(gameRenderTexture);
	{
		drawGrid();
		drawTiles();
		drawClickables();
		DrawText("Title", 450, 25, 55, RED);


		ClearBackground(GREEN);
		DrawRectangle(debug_box_x, 10, 30, 30, ORANGE);
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
}




void gameUpdate() {
	if (gameErrored) {
		drawError();
		return;
	}

	gameDraw();
	PollInputEvents();
	gameTick();
}
