#pragma once

#include "mpv/render.h"
#include "raylib.h"
#include "rlgl.h"
#include <cstdint>
#include <string>
#include <vector>

extern mpv_handle* mpv;
extern mpv_render_context* mpv_gl;
extern RenderTexture mpv_tx;
extern bool redraw;

struct SubTrack
{
    int64_t sid;
    std::string lang;
    std::string title;
    bool is_selected;
};

struct VideoInfo
{
    const char* title;
    int64_t width;
    int64_t height;    
    double percentPos;
    double speed;
    std::vector<SubTrack> subs;
};

extern VideoInfo videoInfo;


void MpvInit();
void LoadVideo(const char* videoPath);
void MpvRender();
void VideoInit();
void MpvFinish();

//video control
void TogglePause();
void Jump(double percent);
void Seek(int second);
void ChangeVolume(int step);
void SetSpeed(double speed);
void SetSub(int64_t sid);

void PollMpvEvent();
