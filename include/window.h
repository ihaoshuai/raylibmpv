#pragma once



void WindowInit();
void WindowFinish();
void CustomToggleFullscreen();
bool IsMouseButtonDoubleClicked(int button);
void SetResizeWindowCallback(void (*callback)(void));
void HandleResizeWindow();
