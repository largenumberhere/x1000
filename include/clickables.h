#pragma once


#include <stdbool.h>
#include <stdint.h>
#include "raylib.h"
#include "hexagons.h"

typedef enum {


    CLICKABLE_DIR_NW =  1,
    CLICKABLE_DIR_E =   1 << 1,
    CLICKABLE_DIR_SW =  1 << 2,
    CLICKABLE_DIR_SE =  1 << 3,
    CLICKABLE_DIR_W =   1 << 4,
    CLICKABLE_DIR_NE =   1 << 5,

    CLICKABLE_MOVE_HEXAGONS = 1 << 6,
    CLICKABLE_HOVER = 1 << 7,

    CLICKABLE_VOL_FIST = 1 << 12,
    CLICKABLE_VOL_PLUS = 1 << 13,
    CLICKABLE_VOL_MINUS = 1 << 14,
    CLICKABLE_VOL_LAST = 1 << 15,
} ClickableKind;

typedef struct {
    Rectangle position;
    ClickableKind kind;
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

bool clickableKindHasFlag(ClickableKind clickable, ClickableKind kind);
LibHexDirection clickableToHexDir(ClickableKind kind);

