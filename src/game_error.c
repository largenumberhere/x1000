#include "game_error.h"
#include <stdbool.h>
#include <raylib.h>
#include <stdio.h>
#include "game.h"

#include "stdlib.h"

void assertFail(const char* file, int line) {
    return; // no assertions for this build

    gameErrored = true;
    sprintf(gameErrorBuff, "Assertion failed. Version '%s'. At %s:%i\n", gameVersion, file, line);

    TraceLog(LOG_ERROR, gameErrorBuff);
    fprintf(stdout ,"%s", gameErrorBuff);
    fflush(stdout);

#ifndef PLATFORM_WEB

    asm("int3");
#endif

    // exit(1);
}

char gameErrorBuff[512] = {0};
bool gameErrored = false;

void drawError() {
    BeginDrawing();
    ClearBackground(WHITE);
    DrawText(gameErrorBuff, 0, 0, 10, RED);
    EndDrawing();
}