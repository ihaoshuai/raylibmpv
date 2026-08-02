#include "ui.h"
#include "raylib.h"
#include <spdlog/spdlog.h>

double DrawProgress(Rectangle rect, double percent)
{


    Vector2 mousePos = GetMousePosition();
    if(CheckCollisionPointRec(mousePos, rect))
    {
        Color backgroundColor{0, 0, 0, 128};
        Color foregroundColor{255, 255, 255, 255};
        DrawRectangleRec(rect, backgroundColor);
        Rectangle completedRect = rect;
        completedRect.width = rect.width * percent;
        DrawRectangleRec(completedRect, foregroundColor);


        float mouseRectWidth = 6;
        Color mouseColor{210, 168, 255, 255};
        Rectangle mouseRect{ mousePos.x - mouseRectWidth/2, rect.y, mouseRectWidth, rect.height};
        DrawRectangleRec(mouseRect, mouseColor);

        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            return (mousePos.x-rect.x)/rect.width;
        }
    }

    return -1;
}

void DrawMsg(const char* msg, int x, int y, int fontSize, int margin)
{
    int textWidth = MeasureText(msg, fontSize);
    DrawRectangle(x, y, textWidth+margin*2, fontSize+margin*2, Color{0, 0, 0, 128});
    DrawText(msg, x+margin, y+margin, fontSize, WHITE);
}
