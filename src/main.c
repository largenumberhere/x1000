#include "raylib.h"
#include "../include/game.h"
#ifdef PLATFORM_WEB
	#include "emscripten/emscripten.h"

#endif



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
	emscripten_set_main_loop(gameUpdate, 60, true);

#else
	SetTargetFPS(60);
	while (!WindowShouldClose() && !gameTerminateWindowImmediately) gameUpdate();
	CloseWindow();


#endif
	return 0;
}
