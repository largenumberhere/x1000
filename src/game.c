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
	ClickableKindUp =		1,
	ClickableKindDown =		2,
	ClickableKindLeft =		3,
	ClickableKindRight =	4,
} GameClickableKind;

typedef struct {
	Rectangle position;
	GameClickableKind kind;
	float tickCooldownCurrent;
	float tickCooldownMax;
} GameClickable;


#define MAX_CLICKABLE 64
static GameClickable clickables[MAX_CLICKABLE];
static int64_t clickableCount = 0;


void initClickables() {
	GameClickable upArrow = {.kind = ClickableKindUp, .position = {0, 0, 20, 20}, .tickCooldownCurrent = 1000, .tickCooldownMax = 1000};
	GameClickable downArrow = {.kind = ClickableKindDown, .position = {0, 30, 20, 20}, .tickCooldownCurrent = 1000, .tickCooldownMax = 1000};
	GameClickable leftArrow = {.kind = ClickableKindLeft, .position = {0, 60, 20, 20}, .tickCooldownCurrent = 1000, .tickCooldownMax = 1000};
	GameClickable rightArrow = {.kind = ClickableKindRight, .position = {0, 90, 20, 20}, .tickCooldownCurrent = 1000, .tickCooldownMax = 1000};

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
	}

}

void drawRenderTexture() {
	Rectangle flippedSource = gameRenderTextureSize;
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
	// load any resources the depend on raylib
	loadGameRenderTexture();
	initClickables();
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
		drawClickables();
		DrawText("Title", 450, 25, 55, RED);


		ClearBackground(GREEN);
		DrawRectangle(debug_box_x, 10, 30, 30, ORANGE);
	}
	EndTextureMode();


	BeginDrawing();
	{
		drawRenderTexture();
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
