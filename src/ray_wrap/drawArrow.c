#include "ray_wrap/draw_arrow.h"

#include "raylib.h"
#include "raymath.h"
#include "game.h"
void drawArrow(Vector2 headSpot, float angle, float length) {
    float thickness = 10;
    angle -= 180;
    Vector2 other = {0, length};
    float radiansFactor = (PI / 180);
    float radians = angle * radiansFactor;
    Color color = clrLightGreen;


    Vector2 rotated = Vector2Rotate(other, radians);
    Vector2 b = Vector2Add(headSpot, rotated);
    DrawLineEx(headSpot, b, thickness, color);


    Vector2 leftWedge = {0, 40};
    leftWedge = Vector2Rotate(leftWedge, radians);
    leftWedge = Vector2Rotate(leftWedge , 30*radiansFactor);
    leftWedge = Vector2Add(headSpot, leftWedge);
    DrawLineEx(headSpot, leftWedge, thickness, color);

    Vector2 rightWedge = {0, 40};
    rightWedge = Vector2Rotate(rightWedge, radians);
    rightWedge = Vector2Rotate(rightWedge , -30*radiansFactor);
    rightWedge = Vector2Add(headSpot, rightWedge);

    DrawLineEx(headSpot, rightWedge, thickness, color);
}
