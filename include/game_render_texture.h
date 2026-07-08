#pragma once
#include "raylib.h"

static Rectangle gameRenderTextureSize = {.x=0, .y=0,.width= 1000, .height=1000};
static RenderTexture2D gameRenderTexture = {0};


void loadGameRenderTexture();
void drawGameRenderTexture(Rectangle screenSize);
