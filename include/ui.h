#pragma once

#include "raylib.h"

extern Texture2D thumbTexture;

bool DrawProgress(Rectangle rect, double percent, double* targetPercentage);
void DrawMsg(const char* msg, int x, int y, int fontSize, int margin);


