#pragma once

enum class ScreenType 
{
    Idle, VideoPlay
};

extern ScreenType currentScreen;


void InitIdleScreen();
void UpdateIdleScreen();
void DrawIdleScreen();
void UnloadIdleScreen();
int FinishIdleScreen(const char** videoPath);

void InitVideoPlayScreen(const char* videoPath);
void UpdateVideoPlayScreen();
void DrawVideoPlayScreen();
void UnloadVideoPlayScreen();
int FinishVideoPlayScreen();
