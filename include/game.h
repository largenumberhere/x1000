#pragma once
#include "hexagons.h"
#include "raylib.h"
#include "stdbool.h"

extern const char* gameVersion;

extern Rectangle gameDestinationScreenSize;
extern Rectangle gameRenderTextureSize;

extern RenderTexture2D gameRenderTexture;
extern bool gameTerminateWindowImmediately;

void gamePreInit1();
void gamePreInit2();
void gamePreInit3();

void gameUpdate();

void spawnRandomHexTile();

typedef enum {
    CLICK_MOVE_NW,
    CLICK_MOVE_E,
    CLICK_MOVE_NE,
    CLICK_MOVE_SE,
    CLICK_MOVE_W,
} ClickMove;

// tile grid sizes
#define  MAX_Q 5

// tile grid sizes
#define  MAX_R 5

typedef struct {
    float hex[MAX_Q][MAX_R];
} HexTilesQr;

extern HexTilesQr hexTiles;

const extern float TILE_UNUSED;	// hidden corner
const extern float TILE_EMPTY;
const extern float TILE_MIN_POPULATED;

extern Color clrYellow;
extern Color clrOrange;
extern Color clrDarkGreen;
extern Color clrLightGreen;
extern Color clrRed;

extern bool debugTiles;


extern AxialHex newTile;
extern AxialHex prevNewTile;
extern float deltaSinceNewTile;
