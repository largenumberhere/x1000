#pragma once


#include <stdbool.h>
#include <stdint.h>
#include "raylib.h"

typedef enum {
    CLICKABLE_DIR_NW,
    CLICKABLE_DIR_E,
    CLICKABLE_DIR_SW,
    CLICKABLE_DIR_SE,
    CLICKABLE_DIR_W,
} ClickableDir;

typedef struct {
    Rectangle position;
    ClickableDir direction;
    float tickCooldownCurrent;
    float tickCooldownMax;
    bool hovered;
} GameClickable;


#define MAX_CLICKABLE 64
static GameClickable clickables[MAX_CLICKABLE];
static int64_t clickableCount = 0;

void initClickables();
void drawClickables();
void tickClickables();
