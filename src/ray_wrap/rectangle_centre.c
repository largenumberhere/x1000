#include "ray_wrap/rectangle_centre.h"

#include "raylib.h"


Vector2 rectangleCentre(Rectangle rectangle) {
    float x = rectangle.width / 2 + rectangle.x;
    float y = rectangle.height /2 + rectangle.y;
    Vector2 vec = {x, y};

    return vec;
}