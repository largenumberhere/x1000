#pragma once

#include "math.h"
#include "raylib.h"
#include "game_error.h"

#include "stdlib.h"

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

// the origin is facing down to south, each change moves counter-clockwise
typedef enum {
    HEXN_SE = 0,
    HEXN_E = 1,
    HEXN_NE = 2,
    HEXN_NW = 3,
    HEXN_W = 4,
    HEXN_SW = 5,

}  LibHexDirection;


LibHexDirection hexDirCClockwise(LibHexDirection dir, int count);

LibHexDirection hexdirClockwise(LibHexDirection dir, int count);


AxialHex cubeToAxial(CubeHex cubeHex);

CubeHex axialToCube(AxialHex axialHex);


Vector2 hexToVec2(AxialHex hex, float size);

CubeHex cubeRound(CubeHex value);


AxialHex vec2ToHex(Vector2 vec, float size);


float axialManhattanDistance(AxialHex axial1, AxialHex axial2);


AxialHex axialDiagonalNeighbour(AxialHex axial, LibHexDirection direction, int count);


AxialHex axialDirectNeighbour(AxialHex axial, LibHexDirection direction, int count);

AxialHex axialHexAdd(AxialHex one, AxialHex two);
