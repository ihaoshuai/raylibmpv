#pragma once

#include "raylib.h"

extern bool isThumbnailReady;

void ThumbnailInit(const char* videoPath);
Image GetThumbnail(double percentage, int targetWidth);
void ThumbnailFinish();