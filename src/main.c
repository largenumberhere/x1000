#include "raylib.h"
#include "../include/game.h"
#ifdef PLATFORM_WEB
	#include "emscripten/emscripten.h"

#endif




void webGameUpdate() {
#ifndef PLATFORM_DESKTOP
	if (gameTerminateWindowImmediately) {
		// Emscripten does not close the window... ever,
		// so let's make it clear when it does happen

		emscripten_cancel_main_loop();
		BeginDrawing();
		ClearBackground(WHITE);
		DrawText("Game terminated. Please refresh window", 0, 0, 30, BLACK);
		EndDrawing();
		return;
	}
#endif

	gameUpdate();
}

void desktopGameUpdate() {
	// debugging utility on desktop
#ifndef PLATFORM_WEB
	if (IsKeyDown(KEY_ESCAPE)) {
		gameTerminateWindowImmediately = true;
	}
#endif

	gameUpdate();
}

// bootstrap the game with different frontentds
#ifndef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif
int main() {
	gamePreInit1();
	gamePreInit2();
	gamePreInit3();
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
#ifdef PLATFORM_WEB
	// tell emscripten's js runtime to periodically call the function.
	// Ensures that the runtime is never blocked (mostly.. loging is a bit broken)
	emscripten_set_main_loop(webGameUpdate, 60, true);

#else
	SetTargetFPS(60);
	while (!WindowShouldClose() && !gameTerminateWindowImmediately) desktopGameUpdate();
	CloseWindow();


#endif
	return 0;
}
