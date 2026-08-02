#include "ui.h"
#include "video.h"
#include "window.h"
#include "mpv/render.h"
#include "raylib.h"
#include "mpv/client.h"
#include "rlgl.h"
#include <spdlog/spdlog.h>



namespace window 
{

Rectangle renderRect = { 0, 0, 0, 0};

const float progressHeight = 50;
Rectangle progressRect = { 0, 0, 0, 0};

//debouce when window size change
const double RESIZE_INTERVAL = 0.1;
double lastResizeTime = 0.0;

void ResizeWindowCallback();
void HandleInput();

void play(const char* video) {
    int screenWidth=1500, screenHeight=600;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "cvp");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    rlDisableBackfaceCulling();

    MpvInit(video);

    bool isFirstFrame = true;
    lastResizeTime = 0.0;

    while (!WindowShouldClose()) {
        double now = GetTime();

        if(redraw) {
            redraw = false;
            if(isFirstFrame)
            {
                isFirstFrame = false;
                VideoInit();
                mpv_tx = LoadRenderTexture(videoInfo.width, videoInfo.height);
                ResizeWindowCallback();
                lastResizeTime = GetTime();
            }
            MpvRender();
        }

        if(IsWindowResized())
        {
            if(now - lastResizeTime > RESIZE_INTERVAL)
            {
                ResizeWindowCallback();
                lastResizeTime = now;
            }
        }

        HandleInput();


        BeginDrawing();
            ClearBackground(BLACK);
            

            DrawTexturePro(mpv_tx.texture, Rectangle{0, 0, (float)mpv_tx.texture.width, (float)mpv_tx.texture.height}, renderRect, Vector2{0, 0}, 0.f, WHITE);

            if(now < osdMsg.lastShowTime && !osdMsg.msg.empty())
            {
                DrawMsg(osdMsg.msg.c_str(), 10, 10, 20, 10);
            }
            double percent;
            if((percent = DrawProgress(progressRect, videoInfo.percentPos)) >= 0)
            {
                Jump(percent);
            }
            // DrawFPS(GetScreenWidth() - 100, 10);
        EndDrawing();
    }

    mpv_render_context_free(mpv_gl);
    mpv_destroy(mpv);
    CloseWindow();
    
}

void ResizeWindowCallback()
{
    int renderWidth = GetRenderWidth();
    int renderHeight = GetRenderHeight();

    float videoRatio = ((float)videoInfo.width)/(float)videoInfo.height;
    float renderRatio = ((float)renderWidth)/(float)renderHeight;

    //consider the type of render size, decide video render area
    if(renderRatio > videoRatio)
    {
        renderRect.height = renderHeight;
        renderRect.width = mpv_tx.texture.width * (((float)renderHeight)/mpv_tx.texture.height);
        renderRect.x = (renderWidth - renderRect.width)/2;
        renderRect.y  = 0;
    } else {
        renderRect.height = mpv_tx.texture.height * (((float)renderWidth)/mpv_tx.texture.width);
        renderRect.width = renderWidth;
        renderRect.x = 0;
        renderRect.y = (renderHeight - renderRect.height)/2;
    }

    progressRect = { 0, renderHeight-progressHeight, (float)renderWidth, progressHeight};
}

void HandleInput()
{
    if(IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
        TogglePause();

    //volume control
    if(IsKeyPressed(KEY_DOWN))
        ChangeVolume(-5);
    if(IsKeyPressed(KEY_UP))
        ChangeVolume(5);

    if(IsKeyPressed(KEY_LEFT))
        Seek(-5);
    if(IsKeyPressed(KEY_RIGHT))
        Seek(5);

    if(IsKeyPressed(KEY_F))
        ToggleFullscreen();
}


}