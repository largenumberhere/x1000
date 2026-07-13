#include "ray_wrap/draw_text_centred.h"

#include "raymath.h"
#include "raylib.h"

void drawTextCentred(Vector2 centrePoint, Font font, const char* str, float textSize, Color color) {
    float spcaing = 0;

    Vector2 size = MeasureTextEx(font, str, textSize, spcaing);

    Vector2 two = {2,  2};
    Vector2 middle = Vector2Divide(size, two);

    Vector2 position = {centrePoint.x - middle.x, centrePoint.y - middle.y};

    DrawTextPro(GetFontDefault(), str, position, Vector2Zero(), 0, textSize, spcaing, color);
}
