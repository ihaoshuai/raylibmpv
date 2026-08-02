#include "ui.h"
#include "video.h"
#include "window.h"
#include "mpv/render.h"
#include "raylib.h"
#include "mpv/client.h"
#include "rlgl.h"
#include <cmath>
#include <spdlog/spdlog.h>
#include <math.h>



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
void AdjustWindow();


// raylib ToggleFullScreen has a bug, when you exit fullscreen, the height of the window will change
// maybe wayland env cause it: 
// WARNING: GLFW: Error: 65548 Description: Wayland: The platform does not provide the window position
struct FullscreenHelper{
    int width;
    int height;
    int frameCount;
};
FullscreenHelper fullscreenHelper{0, 0, 2};
void CustomToggleFullscreen();

bool IsMouseButtonDoubleClicked(int button);

void play(const char* video) {

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIDDEN);
    InitWindow(100, 100, "cvp");
    SetTargetFPS(60);
    SetExitKey(KEY_Q);

    rlDisableBackfaceCulling();

    MpvInit(video);

    AdjustWindow();
    ClearWindowState(FLAG_WINDOW_HIDDEN);

    

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
                SetWindowTitle(videoInfo.title);
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

        if(fullscreenHelper.frameCount < 2)
        {
            if(fullscreenHelper.frameCount == 1)
            {
                SetWindowSize(fullscreenHelper.width, fullscreenHelper.height);
                ResizeWindowCallback();
            }
            fullscreenHelper.frameCount++;
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
    Vector2 mouseWheelMove = GetMouseWheelMoveV();
    if(IsKeyPressed(KEY_DOWN) || mouseWheelMove.y < 0)
        ChangeVolume(-5);
    if(IsKeyPressed(KEY_UP) || mouseWheelMove.y > 0)
        ChangeVolume(5);

    if(IsKeyPressed(KEY_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_SIDE))
        Seek(-5);
    if(IsKeyPressed(KEY_RIGHT) || IsMouseButtonPressed(MOUSE_BUTTON_EXTRA))
        Seek(5);

    if(IsKeyPressed(KEY_F) || IsMouseButtonDoubleClicked(MOUSE_BUTTON_LEFT))
    {
        CustomToggleFullscreen();
    }

}

void AdjustWindow()
{
    int monitor = GetCurrentMonitor();
    const int MONITOR_WIDTH = GetMonitorWidth(monitor);
    const int MONITOR_HEIGHT = GetMonitorHeight(monitor);

    int screenWidth = MONITOR_WIDTH * 0.8;
    int screenHeight = MONITOR_HEIGHT * 0.8;
    
    SetWindowSize(screenWidth, screenHeight);
    SetWindowPosition((MONITOR_WIDTH-screenWidth)/2, (MONITOR_HEIGHT-screenHeight)/2);
}

void CustomToggleFullscreen()
{
    if(IsWindowFullscreen()) {
        ToggleFullscreen();
        fullscreenHelper.frameCount = 0;
    }else {
        fullscreenHelper.width = GetScreenWidth();
        fullscreenHelper.height = GetScreenHeight();   
        ToggleFullscreen();
    }
}

double Vector2Distance(Vector2 point, Vector2 other)
{
    return std::sqrt(std::pow(point.x - other.x, 2) + std::pow(point.y-other.y, 2));
}

bool IsMouseButtonDoubleClicked(int button)
{
    static double lastClickTime = -1.0f;
    static Vector2 lastClickPos = {0.f, 0.f};
    const double delay = 0.25;

    if(IsMouseButtonPressed(button))
    {
        double curTime = GetTime();
        Vector2 curPos = GetMousePosition();

        double timieDiff = curTime - lastClickTime;
        double posDistance = Vector2Distance(curPos, lastClickPos);

        if(timieDiff < delay && posDistance < 5.0)
        {
            lastClickTime = 0.0f;
            return true;
        }

        lastClickTime = curTime;
        lastClickPos = curPos;
    }
    
    return false;
    
}

}