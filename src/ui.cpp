#include "ui.h"
#include "raylib.h"
#include <spdlog/spdlog.h>


OsdMsg osdMsg;
const double MSG_DURATION_TIME = 1.5;

void SetMsg(const std::string& msg)
{
    osdMsg.msg = msg;
    osdMsg.lastShowTime = GetTime() + MSG_DURATION_TIME;
}


bool DrawProgress(Rectangle rect, double percent, double* mousePercentage)
{
    Vector2 mousePos = GetMousePosition();
    Color backgroundColor{0, 0, 0, 128};
    Color foregroundColor{255, 255, 255, 255};
    DrawRectangleRec(rect, backgroundColor);
    Rectangle completedRect = rect;
    completedRect.width = rect.width * percent;
    DrawRectangleRec(completedRect, foregroundColor);

    if(CheckCollisionPointRec(mousePos, rect))
    {
        float mouseRectWidth = 6;
        Color mouseColor{210, 168, 255, 255};
        Rectangle mouseRect{ mousePos.x - mouseRectWidth/2, rect.y, mouseRectWidth, rect.height};
        DrawRectangleRec(mouseRect, mouseColor);
        *mousePercentage = (mousePos.x-rect.x)/rect.width;
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            return true;
        }
    }

    return false;
}

void DrawMsg(const char* msg, int x, int y, int fontSize, int margin)
{
    int textWidth = MeasureText(msg, fontSize);
    DrawRectangle(x, y, textWidth+margin*2, fontSize+margin*2, Color{0, 0, 0, 128});
    DrawText(msg, x+margin, y+margin, fontSize, WHITE);
}

bool DrawButton(Rectangle rect, const char* text, Color baseColor, Color hoverColor)
{
    Vector2 mousePos = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mousePos, rect);
    bool isClicked = isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    Color curColor = isHovered ? hoverColor : baseColor;

    DrawRectangleRec(rect, curColor);
    // DrawRectangleLinesEx(rect, 2, WHITE);

    int fontSize = 18;
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, rect.x + (rect.width - textWidth)/2, rect.y + (rect.height - fontSize)/2, fontSize, WHITE);

    return isClicked;
}

int DrawMenu(std::vector<MenuItem>& menu)
{
    int render_width = GetRenderWidth();
    int render_height = GetRenderHeight();
    int max_text_width = 0;
    int font_size = 20;

    for(MenuItem& item : menu)
    {   
        int text_width = MeasureText(item.text, font_size);
        if(text_width > max_text_width)
            max_text_width = text_width;
    }

    Color ItemBaseColor{25, 26, 27, 128 };
    Color ItemHoverColor{ 44, 45, 46, 255};
    Color ItemSelectedColor{33, 34, 35, 255};
    const float margin = 10;
    float item_width = max_text_width + margin*2;
    float item_height = font_size + margin*2;
    Rectangle rect{(render_width-item_width)/2, (render_height-item_height*menu.size())/2, item_width, item_height};
    Vector2 mousePos = GetMousePosition();
    int item_size = menu.size();
    for(int i=0; i<item_size; i++)
    {
        if(CheckCollisionPointRec(mousePos, rect))
        {
            DrawRectangleRec(rect, ItemHoverColor);
            if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
                return i;
        }else if(menu[i].is_selected)
        {
            DrawRectangleRec(rect, ItemSelectedColor);
        }else 
        {
            DrawRectangleRec(rect, ItemBaseColor);
        }
        DrawText(menu[i].text, rect.x+margin, rect.y+margin, font_size, WHITE);
        // DrawTextEx(font, menu[i].text, Vector2{rect.x+margin, rect.y+margin}, font_size, 0, WHITE);
        rect.y += item_height;

    }
    rect.y -= item_height*item_size;
    rect.height = item_height*item_size;
    if(!CheckCollisionPointRec(mousePos, rect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        return -2;
    
    return -1;
}
