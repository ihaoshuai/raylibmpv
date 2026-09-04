#include "window.h"
#include "raylib.h"
#include "rlgl.h"
#include <cmath>

// raylib ToggleFullScreen has a bug, when you exit fullscreen, the height of the window will change
// maybe wayland env cause it: 
// WARNING: GLFW: Error: 65548 Description: Wayland: The platform does not provide the window position
struct FullscreenHelper{
    int width;
    int height;
    int frameCount;
};
FullscreenHelper fullscreenHelper{0, 0, 2};

//debouce when window size change
const double RESIZE_INTERVAL = 0.1;
double lastResizeTime = 0.0;


void (*resize_window_callback)(void) = NULL;


void AdjustWindow();
int* GenerateChineseCodepoints(int *outCount);


void WindowInit()
{
    #if defined(RELEASE_BUILD)
        ChangeDirectory(GetApplicationDirectory());
    #endif
    

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIDDEN | FLAG_WINDOW_HIGHDPI | FLAG_VSYNC_HINT);
    
    
    InitWindow(100, 100, "cvp");

    Image icon = LoadImage("icon.PNG");
    ImageResize(&icon, 64, 64);
    SetWindowIcon(icon);
    UnloadImage(icon);


    SetTargetFPS(60);
    SetExitKey(KEY_Q);

    rlDisableBackfaceCulling();
    AdjustWindow();


    ClearWindowState(FLAG_WINDOW_HIDDEN);

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

#if defined(_WIN32)
    void CustomToggleFullscreen()
    {
        static bool isFullscreen = false;
        static int screenWidth;
        static int screenHeight;
        static Vector2 windowPos;

        if(isFullscreen) {
            ClearWindowState(FLAG_WINDOW_TOPMOST | FLAG_WINDOW_UNDECORATED);
            SetWindowSize(screenWidth, screenHeight);
            SetWindowPosition(windowPos.x, windowPos.y);
           if(resize_window_callback)
                resize_window_callback();
            isFullscreen = false;
        }else {
            screenHeight = GetScreenHeight();
            screenWidth = GetScreenWidth();
            windowPos = GetWindowPosition();
            isFullscreen = true;
            int monitor = GetCurrentMonitor();
            SetWindowState(FLAG_WINDOW_TOPMOST | FLAG_WINDOW_UNDECORATED);
            SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
            SetWindowPosition(0, 0);
            if(resize_window_callback)
                resize_window_callback();
        }
    }
#else
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
#endif


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

void SetResizeWindowCallback(void (*callback)(void))
{
    resize_window_callback = callback;
}


void HandleResizeWindow()
{
    double now = GetTime();
    if(IsWindowResized())
    {
        if(now - lastResizeTime > RESIZE_INTERVAL)
        {
            if(resize_window_callback)
                resize_window_callback();
            lastResizeTime = now;
        }
    }

    #if !defined(_WIN32)
        if(fullscreenHelper.frameCount < 2)
        {
            if(fullscreenHelper.frameCount == 1)
            {
                SetWindowSize(fullscreenHelper.width, fullscreenHelper.height);
                if(resize_window_callback)
                    resize_window_callback();
            }
            fullscreenHelper.frameCount++;
        }
    #endif

}


int* GenerateChineseCodepoints(int *outCount) {
    // 1. 定义需要包含的字符区间
    // ASCII 范围: 32 - 126
    int asciiCount = 126 - 32 + 1;
    // CJK 统一表意文字基本区 (常用汉字主要集中在此范围: 0x4E00 - 0x9FA5, 共 20902 字)
    // 为了性能和显存，可以加载常用范围，或选取前 N 个高频字范围
    int cjkStart = 0x4E00;
    int cjkEnd = 0x9FA5; 
    int cjkCount = cjkEnd - cjkStart + 1;

    int totalCount = asciiCount + cjkCount;
    int *codepoints = (int *)RL_MALLOC(totalCount * sizeof(int));

    int index = 0;
    // 填充 ASCII
    for (int i = 32; i <= 126; i++) {
        codepoints[index++] = i;
    }
    // 填充 CJK 汉字
    for (int i = cjkStart; i <= cjkEnd; i++) {
        codepoints[index++] = i;
    }

    *outCount = totalCount;
    return codepoints;
}


void WindowFinish()
{
    CloseWindow();
}

