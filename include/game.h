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

void drawTextCentred(Vector2 centrePoint, Font font, const char* str, float textSize, Color color);



// tile grid sizes
#define  MAX_Q 5

// tile grid sizes
#define  MAX_R 5

typedef struct {
    float hex[MAX_Q][MAX_R];
} HexTilesQr;

extern HexTilesQr hexTiles;

bool hexTileInRange(AxialHex axial);
bool hexTileOccupied(AxialHex axial);
bool hexTileUseable(AxialHex axial);
float hexTileGet(AxialHex axial);
void hexTileSet(AxialHex axial, float value);

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
extern float deltaSinceNewTile;
extern bool movedHexagonsRecently;
extern float gameVolumeAdjustment;

void updateSoundVolumes();