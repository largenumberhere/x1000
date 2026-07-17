#include "game.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#include "raylib.h"
#include "raymath.h"
#include "game_error.h"
#include "get_mouse_pos_rel.h"
#ifndef PLATFORM_DESKTOP
	#include "emscripten/emscripten.h"
#endif
#include "game.h"
#include "hexagons.h"
#include "draw_hex_tiles.h"
#include "move_hexagons.h"
#include "clickables.h"
#include "../include/ray_wrap/rectangle_shrink.h"
#include "../include/ray_wrap/rectangle_centre.h"

HexTilesQr hexTiles = {0};

bool hexTileUseable(AxialHex axial) {
	return  hexTileInRange(axial) && hexTileGet(axial) >= TILE_EMPTY;
}

bool hexTileOccupied(AxialHex axial) {
	return hexTileGet(axial) >= 1;
}

float hexTileGet(AxialHex axial) {
	GAME_ASSERT(hexTileInRange(axial));
	return hexTiles.hex[(int) axial.q][(int) axial.r];
}


void hexTileSet(AxialHex axial, float value) {
	GAME_ASSERT(hexTileInRange(axial));
	hexTiles.hex[(int) axial.q][(int) axial.r] = value;
}

bool hexTileInRange(AxialHex axial) {
	float q = axial.q;
	float r = axial.r;

	return (q >= 0) &&
		(r >= 0) &&
		(q < MAX_Q) &&
		(r < MAX_R);
}



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
float deltaSinceNewTile = 0;


bool gameTerminateWindowImmediately = false;
const char* gameVersion = "1.1";


bool canSpawnTile();	// fwd declaration of local symbol


void spawnRandomHexTile() {
	if (!canSpawnTile()) {
		return;
	}

	AxialHex axial = {0, 0};
	do {
		int qr = GetRandomValue(0, MAX_Q * MAX_R -1);
		axial.q = qr % MAX_Q;
		axial.r = qr / MAX_R;


		GAME_ASSERT(hexTileInRange(axial));
	} while (
		(!hexTileUseable(axial)) | hexTileOccupied(axial)
	);

	hexTileSet(axial, 0x1);
	newTile = axial;
	deltaSinceNewTile = 0;
}


bool movedHexagonsRecently = false;

float gameVolumeAdjustment = 1;

// --- end of header implementation --- //



bool gameEnded = false;

bool checkGameOver() {
	// notify if no spaces are left
	int emptyCount = 0;
	for (int q = 0; q < MAX_Q; q++) {
		for (int r = 0; r < MAX_R; r++) {
			if (hexTiles.hex[q][r] == TILE_EMPTY) {
				emptyCount += 1;
			}
		}
	}

	if (emptyCount == 0) {
		return true;
	}

	return false;
}


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

	// workaround for noise stuttering on mobile
	// https://github.com/raysan5/raylib/issues/4491
	SetAudioStreamBufferSizeDefault(4096 * 16);

}


Sound popSound = {0};
Sound shiftSound = {0};
Sound bottlePopSound = {0};
Music backgroundMusic = {0};

void updateSoundVolumes() {
	float factor =  gameVolumeAdjustment;
	SetSoundVolume(popSound, 0.2 * factor);
	SetSoundVolume(shiftSound, 0.5 * factor);
	SetSoundVolume(bottlePopSound, 0.3f * factor);
	SetMusicVolume(backgroundMusic, 0.2 * factor);
}

void gamePreInit3() {
	// load any resources that depend on raylib
	loadGameRenderTexture();
	initClickables();
	initHexTiles();
	for (int i = 0; i < 2; i++) {
		spawnRandomHexTile();
	}
	InitAudioDevice();
	while (!IsAudioDeviceReady()) {}

	popSound = LoadSound(ASSET_PATH "/pop5b.mp3");
	shiftSound = LoadSound(ASSET_PATH "/shifting3.mp3");
	bottlePopSound = LoadSound(ASSET_PATH "/freesound_community-plastic-bottle-pop-90006.mp3");
	backgroundMusic = LoadMusicStream(ASSET_PATH "/amurich-amurich-substance-without-shape-353916.mp3");
	PlayMusicStream(backgroundMusic);
	updateSoundVolumes();
}

float endFadeInAccum = 0;
float frameAccum = 0;

void drawBorderedRectangle(Rectangle rec) {
	DrawRectangleRec(rectangleShrink(rec, 0), clrDarkGreen);
	DrawRectangleRec(rectangleShrink(rec, 4), clrYellow);
	DrawRectangleRec(rectangleShrink(rec, 8), clrDarkGreen);
}



void gameDraw() {
	frameAccum += GetFrameTime();
	BeginTextureMode(gameRenderTexture);
	{

		ClearBackground(WHITE);

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


		drawClickables();
		drawHexTiles();

		// draw the score
		float score = 0;
		for (int q = 0; q < MAX_Q; q++) {
			for (int r = 0; r < MAX_R; r++) {
				float value = hexTiles.hex[q][r];
				value = value < 0 ? 0 : value;
				score += value;
			}
		}
		char buff[128] = {0};
		sprintf(buff, "Score: 0x% 5.0X0", (int)score);
		Rectangle scoreBgBox = {70, 825, 900, 140};
		drawBorderedRectangle(scoreBgBox);
		DrawText(buff, 160, 850, 100, clrOrange);

		// draw title
		drawBorderedRectangle((Rectangle){300, 40, 390, 140});
		drawTextCentred((Vector2){500, 110}, GetFontDefault(), "x1000", 80, clrOrange);


		if (gameEnded) {
			endFadeInAccum += GetFrameTime();
			float alpha = Lerp(0, 1, endFadeInAccum /10);
			Rectangle rect = rectangleShrink((Rectangle) {0, 0, 1000, 1000}, 150);

			DrawRectangleRec(rectangleShrink(rect, -4), ColorAlpha(clrDarkGreen, alpha));
			DrawRectangleRec(rect, ColorAlpha(clrYellow, alpha));
			DrawRectangleRec(rectangleShrink(rect, 4), ColorAlpha(clrDarkGreen, alpha));
			Rectangle r1 = rect;
			r1.y -= 500;
			r1.height += 500;

			drawTextCentred(rectangleCentre(r1), GetFontDefault(), "Hexing isn't it?", 85, ColorAlpha(clrYellow, alpha));
			Rectangle r2 = r1;
			r2.height -= 200;
			r2.y += 200;
			drawTextCentred(rectangleCentre(r2), GetFontDefault(), "The game is ended now...", 65, ColorAlpha(clrOrange, alpha));

			Rectangle r3 = r2;
			r3.height -= 300;
			r3.y += 300;

			drawTextCentred(rectangleCentre(r3), GetFontDefault(), "You have won, but you\n were always going to\n weren't you?", 65, ColorAlpha(clrOrange, alpha));

			Rectangle r4 = r3;
			r4.height -= 400;
			r4.y += 400;
			drawTextCentred(rectangleCentre(r4), GetFontDefault(), "Go outside and feel\nthe wind in your hair.", 65, ColorAlpha(clrOrange, alpha));
		}


	}
	EndTextureMode();


	BeginDrawing();
	{
		// workaround for flickering on mobile
		// https://www.reddit.com/r/raylib/comments/wcvsiw/why_the_screen_flicker_if_clearbackground_isnt/
		ClearBackground(WHITE);

		drawGameRenderTexture();
	}
	EndDrawing();
}


void tickControls() {
	// maybe handle keyboard controls?
}

void tickAudio() {
	if (tileRecentlyMerged) {
		tileRecentlyMerged = false;
		tileRecentlyCompacted = false;
		movedHexagonsRecently = false;
		PlaySound(popSound);
	} else if (tileRecentlyCompacted) {
		PlaySound(shiftSound);
		tileRecentlyMerged = false;
		tileRecentlyCompacted = false;
		movedHexagonsRecently = false;

	} else if (movedHexagonsRecently) {
		PlaySound(bottlePopSound);
		tileRecentlyMerged = false;
		tileRecentlyCompacted = false;
		movedHexagonsRecently = false;

	}

	UpdateMusicStream(backgroundMusic);
}

float tSinceLastSpawn = 0;
void gameTick() {
	tickClickables();
	deltaSinceNewTile += GetFrameTime();
	tickControls();
	tickAudio();
	gameEnded = checkGameOver();

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
