#pragma once

#include "raylib.h"
#include <string>
#include <vector>

struct OsdMsg
{
    std::string msg;
    double lastShowTime;
};

extern OsdMsg osdMsg;
void SetMsg(const std::string& msg);

extern Texture2D thumbTexture;

bool DrawProgress(Rectangle rect, double percent, double* targetPercentage);
void DrawMsg(const char* msg, int x, int y, int fontSize, int margin);

bool DrawButton(Rectangle rect, const char* text, Color baseColor = Color{25, 26, 27, 128}, Color hoverColor = Color{44, 45, 46, 255});

struct MenuItem
{
    const char* text;
    bool is_selected;
};

int DrawMenu(std::vector<MenuItem>& menu);



