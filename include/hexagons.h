#pragma once
#include "raylib.h"

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

extern AxialHex axialDirectionVectors[6];
AxialHex cubeToAxial(CubeHex cubeHex);
AxialHex vec2ToHex(Vector2 vec, float size);
AxialHex axialDiagonalNeighbour(AxialHex axial, LibHexDirection direction, int count);
AxialHex axialDirectNeighbour(AxialHex axial, LibHexDirection direction, int count);
AxialHex axialHexAdd(AxialHex one, AxialHex two);

extern CubeHex cubeDirectionVectors[6];
extern CubeHex cubeHexDiagonalDirectionalVectors[6];
CubeHex axialToCube(AxialHex axialHex);
CubeHex cubeRound(CubeHex value);
CubeHex cubeDirection(int direction);
CubeHex cubeAdd(CubeHex cube1, CubeHex cubeOffset);
CubeHex cubeNeighbour(CubeHex cube, int direction);
CubeHex cubeScale(CubeHex cube, float multiple);

float axialManhattanDistance(AxialHex axial1, AxialHex axial2);

Vector2 hexToVec2(AxialHex hex, float size);

