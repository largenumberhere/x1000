#pragma once

#include "math.h"
#include "raylib.h"
#include "game_error.h"

// https://www.redblobgames.com/grids/hexagons/

// Axial hexagon coordinates
typedef struct {
    float q;
    float r;

} AxialHex;

// cube-like hexagon coordinates
typedef struct {
    float q;
    float r;
    float s;
}  CubeHex;



extern AxialHex axialDirectionVectors[6];



extern CubeHex cubeHexDiagonalDirectionalVectors[6];

typedef enum {
    HEXN_NE =   0,
    HEXN_N =    1,
    HEXN_NW =   2,
    HEXN_SW =   3,
    HEXN_S =    4,
    HEXN_SE =   5,
}  HexDirection;


AxialHex cubeToAxial(CubeHex cubeHex);

CubeHex axialToCube(AxialHex axialHex);


Vector2 hexToVec2(AxialHex hex, float size);

CubeHex cubeRound(CubeHex value);


AxialHex vec2ToHex(Vector2 vec, float size);


float axialManhattanDistance(AxialHex axial1, AxialHex axial2);


AxialHex axialDiagonalNeighbour(AxialHex axial, HexDirection direction, int count);


AxialHex axialDirectNeighbour(AxialHex axial, HexDirection direction, int count);

