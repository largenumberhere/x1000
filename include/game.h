#pragma once
#include "raylib.h"

static Rectangle gameDestinationScreenSize = {0, 0, 720, 720};
static Rectangle gameRenderTextureSize = {.x=0, .y=0,.width= 1000, .height=1000};

static RenderTexture2D gameRenderTexture = {0};

void gamePreInit1();
void gamePreInit2();
void gamePreInit3();

void gameUpdate();
