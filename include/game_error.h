#pragma once
#include <stdbool.h>

#define GAME_ASSERT(condition) \
    do {	\
        if (!(condition)) {	\
            assertFail(__FILE__, __LINE__);\
        };	\
    } while (0);


void assertFail(const char* file, int line);
void drawError();

extern bool gameErrored;
extern char gameErrorBuff[512];


