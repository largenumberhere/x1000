#include "../include/game_render_texture.h"
#include "../include/game.h"
#include "raymath.h"


void loadGameRenderTexture() {
    gameRenderTexture = LoadRenderTexture(gameRenderTextureSize.width, gameRenderTextureSize.height);
}
