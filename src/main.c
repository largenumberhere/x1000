#include <stdint.h>
#include <stdio.h>
#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <stdbool.h>


#ifdef PLATFORM_WEB
	#include "emscripten/emscripten.h"
	#include "../emsdk/upstream/emscripten/cache/sysroot/include/emscripten/emscripten.h"

#endif


#ifndef lassert


#define lassert(condition) \
do {	\
	if (!condition) {	\
		assertFail(__FILE__, __LINE__);\
	};	\
} while (0);
#endif

static bool errored = false;
static char errorBuff[512] = {0};

void assertFail(const char* file, int line) {
	errored = true;
	sprintf(errorBuff, "Assertion failed at %s:%i\n", file, line);

	TraceLog(LOG_ERROR, errorBuff);
	#ifdef PLATFORM_WEB
	#else

	#endif
}


int debug_box_x = 0;

/*
	https://itch.io/jam/raylib-6x-gamejam
	The goal of this gamejam is making a game with raylib in 6 DAYS with some technical constraints and following proposed theme.
	Theme: hex + merge

	Hard constraints (mandatory requirements):

		Game MUST be 720x720 pixels resolution!
		Game MUST be run on Web with WebAssembly technology
		Game MUST be under 64MB (full package: .wasm + .data)


 */

// #ifdef PLATFORM_WEB
//     #include <emscripten/emscripten.h>
// #endif

// typedef struct Grid {
//
// };



static Rectangle backingTextureSize = {.x=0, .y=0,.width= 1000, .height=1000};
static Rectangle destinationScreenSize = {0, 0, 720, 720};

static RenderTexture2D backingTexture = {0};

typedef enum clickableKind_t {
	ClickableKindUp =		1,
	ClickableKindDown =		2,
	ClickableKindLeft =		3,
	ClickableKindRight =	4,
} ClickableKind;

typedef struct {
	Rectangle position;
	ClickableKind kind;
	float tickCooldownCurrent;
	float tickCooldownMax;
} Clickable;


#define MAX_CLICKABLE 64
static Clickable clickables[MAX_CLICKABLE];
static int64_t clickableCount = 0;


void initClickables() {
	Clickable upArrow = {.kind = ClickableKindUp, .position = {0, 0, 20, 20}, .tickCooldownCurrent = 1000, .tickCooldownMax = 1000};
	Clickable downArrow = {.kind = ClickableKindDown, .position = {0, 30, 20, 20}, .tickCooldownCurrent = 1000, .tickCooldownMax = 1000};
	Clickable leftArrow = {.kind = ClickableKindLeft, .position = {0, 60, 20, 20}, .tickCooldownCurrent = 1000, .tickCooldownMax = 1000};
	Clickable rightArrow = {.kind = ClickableKindRight, .position = {0, 90, 20, 20}, .tickCooldownCurrent = 1000, .tickCooldownMax = 1000};

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

void preinit() {

}

void rayinit() {
	InitWindow(destinationScreenSize.width, destinationScreenSize.height, "game");
}

void init() {
	backingTexture = LoadRenderTexture(backingTextureSize.width, backingTextureSize.height);
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


void draw() {

	// sprintf(errorBuff, "%f %f\n", mouse.x, mouse.y);
	// errored = true;

	if (errored) {

		ClearBackground(WHITE);
		BeginDrawing();
		DrawText(errorBuff, 0, 0, 20, RED);

		EndDrawing();
		return;
	}


	BeginTextureMode(backingTexture);
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
		Rectangle flippedSource = backingTextureSize;
		flippedSource.height = -flippedSource.height;
		DrawTexturePro(backingTexture.texture, flippedSource, destinationScreenSize, Vector2Zero(), 0, WHITE);
	}
	EndDrawing();


}

float fclamp(float value, float min, float max) {
	if (value < min) {return min;};
	if (value > max) {return max;};
	return value;
}

// the mouse position from the perspective of the rendertexture used to draw to
Vector2 getMousePosRel() {
	// emscripten's console sometimes gives slightly out of range values like -0.1. The excessive clamping aims to fix that

	Vector2 absPos = GetMousePosition();
	absPos.x = fclamp(absPos.x, 0, destinationScreenSize.width);
	absPos.y = fclamp(absPos.y, 0, destinationScreenSize.height);

	printf(errorBuff, "%f %f\n", absPos.x, absPos.y);
	float unitx = absPos.x / destinationScreenSize.width;
	float unity = absPos.y / destinationScreenSize.height;
	unitx = fclamp(unitx, 0, 1);
	unity = fclamp(unity, 0, 1);

	float relx = unitx * backingTextureSize.width;
	float rely = unity * backingTextureSize.height;
	relx = fclamp(relx, 0, backingTextureSize.width);
	rely = fclamp(rely, 0, backingTextureSize.height);

	Vector2 relPos = {relx, rely};


	return relPos;
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

void tick() {
	debug_box_x += 10;
	tickClickables();
}


void update() {
	draw();
	PollInputEvents();



	if (!errored) {
		tick();
	}


}

// bootstrap the game with different frontentds
#ifndef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif
int main() {
	preinit();
	rayinit();
	init();
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
#ifdef PLATFORM_WEB
	// tell emscripten's js runtime to periodically call the function.
	// Ensures that the runtime is never blocked
	emscripten_set_main_loop(update, 60, true);
#else
	SetTargetFPS(60);
	while (!WindowShouldClose()) update();
#endif
	return 0;
}
