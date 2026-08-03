#pragma once

#include "mpv/render.h"
#include "raylib.h"
#include "rlgl.h"
#include <cstdint>
#include <string>

extern mpv_handle* mpv;
extern mpv_render_context* mpv_gl;
extern RenderTexture mpv_tx;
extern bool redraw;

struct OsdMsg
{
    std::string msg;
    double lastShowTime;
};

extern OsdMsg osdMsg;

struct VideoInfo
{
    const char* title;
    int64_t width;
    int64_t height;    
    double percentPos;
    double speed;
};

extern VideoInfo videoInfo;

void MpvInit(const char* videoPath);
void MpvRender();
void VideoInit();
void MpvFinish();

//video control
void TogglePause();
void Jump(double percent);
void Seek(int second);
void ChangeVolume(int step);
void SetSpeed(double speed);

void PollMpvEvent();
