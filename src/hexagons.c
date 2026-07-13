#include "hexagons.h"

AxialHex axialDirectionVectors[6] = {
    {+1, 0},
    {+1, -1},
    {0, -1},
    {-1, 0},
    {-1, +1},
    {0, +1},
};

CubeHex cubeHexDiagonalDirectionalVectors[6] = {
    {2.0f, -1.0f, -1.0f},
    {+1, -2, +1},
    {-1, -1, +2},
    {-2, +1, +1},
    {-1, +2, -1},
    {+1, +1, -2}
};

AxialHex cubeToAxial(CubeHex cubeHex) {
    // discard the implied 's' factor
    AxialHex axial = {.q = cubeHex.q, .r = cubeHex.r};
    return  axial;
}

CubeHex axialToCube(AxialHex axialHex) {
    // re-construct the implied s field
    CubeHex cubeHex = {.q = axialHex.q, .r = axialHex.r, .s = - axialHex.q - axialHex.r};
    return cubeHex;
}


Vector2 hexToVec2(AxialHex hex, float size) {
    float x = sqrtf(3.0f) * hex.q + sqrtf(3.0f)/2.0f * hex.r;
    float y = 3.0f / 2.0f * hex.r;
    x = x * size;
    y = y * size;

    Vector2 point = {x, y};
    return  point;
}

CubeHex cubeRound(CubeHex value) {
    // round the values and then correct for any skew of the invariants of q, r, s
    float q = roundf(value.q);
    float r = roundf(value.r);
    float s = roundf(value.s);

    float q_diff = fabsf(q - value.q);
    float r_diff = fabsf(r - value.r);
    float s_diff = fabsf(s - value.s);

    if (q_diff > r_diff && q_diff > s_diff) {
        q = -r -s;
    } else if (r_diff > s_diff) {
        r = -q - s;
    } else {
        s = -q -r;
    }

    CubeHex cubeHex = {q, r, s};
    return cubeHex;

}


AxialHex vec2ToHex(Vector2 vec, float size) {
    float x = vec.x / size;
    float y = vec.y / size;

    float q = (sqrtf(3.0f)/(float)3 * x - (float)1/(float)3 * y);
    float r = ((float)2/(float)3 * y);

    AxialHex coords = {q, r};

    CubeHex cubeCoords = axialToCube(coords);
    cubeCoords = cubeRound(cubeCoords);

    coords = cubeToAxial(cubeCoords);
    return coords;
}


float axialManhattanDistance(AxialHex axial1, AxialHex axial2) {
    CubeHex a = axialToCube(axial1);
    CubeHex b = axialToCube(axial2);

    CubeHex diff = {a.q - b.q, a.r - b.r, a.s - b.s};
    float distance = (fabsf(diff.q) + fabsf(diff.r) + fabsf(diff.s)) / 2;
    return distance;
}


AxialHex axialDiagonalNeighbour(AxialHex axial, HexDirection direction, int count) {
    GAME_ASSERT(direction < 6);
    CubeHex cube = cubeHexDiagonalDirectionalVectors[direction];
    AxialHex vector = cubeToAxial(cube);
    axial.r += vector.r * (float)count;
    axial.q += vector.q * (float)count;

    return axial;
}


AxialHex axialDirectNeighbour(AxialHex axial, HexDirection direction, int count) {
    GAME_ASSERT(direction < 6);
    AxialHex vector = axialDirectionVectors[direction];
    axial.r += vector.r * (float)count;
    axial.q += vector.q * (float)count;

    return axial;
}


HexDirection hexDirectionOposite(HexDirection hd) {
    int directionsCount = 6;
    return (hd + (directionsCount / 2) ) % directionsCount;
}

