#pragma once
#include "../emsdk/upstream/emscripten/cache/sysroot/include/math.h"

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

    float q = (sqrt(3)/(float)3 * x - (float)1/(float)3 * y);
    float r = ((float)2/(float)3 * y);

    AxialHex coords = {q, r};

    CubeHex cubeCoords = axialToCube(coords);
    cubeCoords = cubeRound(cubeCoords);

    coords = cubeToAxial(cubeCoords);
    return coords;
}

