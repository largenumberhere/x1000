#pragma once
#include "raylib.h"

extern Rectangle gameDestinationScreenSize;
extern Rectangle gameRenderTextureSize;

extern RenderTexture2D gameRenderTexture;
extern bool gameTerminateWindowImmediately;

void gamePreInit1();
void gamePreInit2();
void gamePreInit3();

void gameUpdate();
