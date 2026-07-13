#include "game_error.h"
#include <stdbool.h>
#include <raylib.h>
#include <stdio.h>
#include "game.h"


void assertFail(const char* file, int line) {
    gameErrored = true;
    sprintf(gameErrorBuff, "Assertion failed. Version '%s'. At %s:%i\n", gameVersion, file, line);

    TraceLog(LOG_ERROR, gameErrorBuff);
}

char gameErrorBuff[512] = {0};
bool gameErrored = false;

void drawError() {
    BeginDrawing();
    ClearBackground(WHITE);
    DrawText(gameErrorBuff, 0, 0, 10, RED);
    EndDrawing();
}