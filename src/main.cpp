


#include "raylib.h"
#include "screen.h"
#include "window.h"
#include "spdlog/spdlog.h"
#include <spdlog/common.h>


// it did not work
// Disable the console in Windows releases
// # if defined(WIN32) && !defined(_DEBUG)
// #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
// #endif

// it did not work
// #if defined(WIN32) && defined(_MSC_VER) && defined(RELEASE_BUILD)
// #if defined(RELEASE_BUILD)
// #pragma comment(linker, "/entry:mainCRTStartup")
// #endif

ScreenType currentScreen;
void UpdateScreen();
void DrawScreen();
void ChangeToScreen(ScreenType screen, void* data);

int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::debug);
    
    if (argc > 2) {
        spdlog::error("the count of argument must less 2");
        return 1;
    }

    WindowInit();
    
    if(argc == 1)
    {
        currentScreen = ScreenType::Idle;
        InitIdleScreen();
    }
    else
    {
        currentScreen = ScreenType::VideoPlay;
        InitVideoPlayScreen(argv[1]);
    }
    

    while (!WindowShouldClose()) {
        HandleResizeWindow();
        UpdateScreen();
        

        BeginDrawing();
            ClearBackground(BLACK);
            DrawScreen();
        EndDrawing();
    }
        
    
    switch (currentScreen) {
        case ScreenType::Idle:
            UnloadIdleScreen();
            break;
        case ScreenType::VideoPlay:
            UnloadVideoPlayScreen();
            break;
    }

    WindowFinish();

    return 0;
}


void UpdateScreen()
{
    const char* videoPath = nullptr;
    switch (currentScreen) {
        case ScreenType::Idle:
            UpdateIdleScreen();
            if(FinishIdleScreen(&videoPath) == 1) 
            {
                ChangeToScreen(ScreenType::VideoPlay, (void*)videoPath); 
            }
            break;
        case ScreenType::VideoPlay:
            UpdateVideoPlayScreen();
            break;
    }
}

void DrawScreen()
{

    switch (currentScreen) {
        case ScreenType::Idle:
            DrawIdleScreen();
            break;
        case ScreenType::VideoPlay:
            DrawVideoPlayScreen();
            break;
    }
}

void ChangeToScreen(ScreenType screen, void* data)
{
    switch (currentScreen) {
        case ScreenType::Idle:
            UnloadIdleScreen();
            break;
        case ScreenType::VideoPlay:
            UnloadVideoPlayScreen();
            break;
    }

    currentScreen = screen;

    switch (screen) {
        case ScreenType::Idle:
            InitIdleScreen();
            break;
        case ScreenType::VideoPlay:
            InitVideoPlayScreen((const char*)data);
            break;
    }
}
