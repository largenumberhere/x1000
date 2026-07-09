#include "../include/game_error.h"
#include <stdbool.h>
#include <raylib.h>
#include <stdio.h>


void assertFail(const char* file, int line) {
    gameErrored = true;
    sprintf(gameErrorBuff, "Assertion failed at %s:%i\n", file, line);

    TraceLog(LOG_ERROR, gameErrorBuff);
#ifdef PLATFORM_WEB
#else

#endif
}

char gameErrorBuff[512] = {0};
bool gameErrored = false;

void drawError() {
    BeginDrawing();
    ClearBackground(WHITE);
    DrawText(gameErrorBuff, 0, 0, 10, RED);
    EndDrawing();
}