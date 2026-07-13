#include "game.h"
#include "get_mouse_pos_rel.h"
#include <stdio.h>


float fclamp(float value, float min, float max) {
    if (value < min) {return min;};
    if (value > max) {return max;};
    return value;
}

// the mouse position from the perspective of the rendertexture used to draw to
Vector2 getMousePosRel() {
    // emscripten's console sometimes gives slightly out of range values like -0.1. The excessive clamping aims to fix that

    Vector2 absPos = GetMousePosition();
    absPos.x = fclamp(absPos.x, 0, gameDestinationScreenSize.width);
    absPos.y = fclamp(absPos.y, 0, gameDestinationScreenSize.height);

    float unitx = absPos.x / gameDestinationScreenSize.width;
    float unity = absPos.y / gameDestinationScreenSize.height;
    unitx = fclamp(unitx, 0, 1);
    unity = fclamp(unity, 0, 1);

    float relx = unitx * gameRenderTextureSize.width;
    float rely = unity * gameRenderTextureSize.height;
    relx = fclamp(relx, 0, gameRenderTextureSize.width);
    rely = fclamp(rely, 0, gameRenderTextureSize.height);

    Vector2 relPos = {relx, rely};
    return relPos;
}