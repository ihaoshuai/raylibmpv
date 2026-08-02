#include "ui.h"
#include "raylib.h"
#include "thumbnail.h"
#include <spdlog/spdlog.h>


const int THUMB_WIDTH = 300;
Texture2D thumbTexture{0};

bool DrawProgress(Rectangle rect, double percent, double* mousePercentage)
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

        *mousePercentage = (mousePos.x-rect.x)/rect.width;

        if(isThumbnailReady)
        {
            Image thumbImage = GetThumbnail(*mousePercentage, THUMB_WIDTH);
            if(thumbImage.data != nullptr)
            {
                if(thumbTexture.id == 0)
                {
                    thumbTexture = LoadTextureFromImage(thumbImage);
                }else {
                    UpdateTexture(thumbTexture, thumbImage.data);
                }
                UnloadImage(thumbImage);
            }
            int posX = mousePos.x - (double)thumbTexture.width/2;
            if(posX < 0)
                posX = 0;
            else if(posX > rect.x + rect.width - thumbTexture.width)
                posX = rect.x + rect.width - thumbTexture.width;
            int posY = rect.y - thumbImage.height;
            DrawTexture(thumbTexture, posX, posY, WHITE);
            
        }


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
