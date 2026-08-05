#include "raylib.h"
#include "screen.h"
#include "thumbnail.h"
#include "video.h"
#include "window.h"
#include "ui.h"
#include <cstddef>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <vector>

bool is_first_frame;
Rectangle renderRect = { 0, 0, 0, 0};
Rectangle progressRect{0, 0, 0, 0};
Rectangle menuRect{0, 0, 0, 0};

Texture2D thumbTexture{0};

const int THUMBNAIL_WIDTH = 300;
const float PROGRESS_HEIGHT = 50;
const float MENU_HEIGHT = 60;

bool is_menu_show;

void HandleInput();
static void ResizeWindowCallback();

Shader down_shader;
Shader up_shader;

void InitVideoPlayScreen(const char* videoPath)
{
    ThumbnailInit(videoPath);
    MpvInit();
    LoadVideo(videoPath);
    is_first_frame = true;
    is_menu_show = false;
    thumbTexture = {0};
    osdMsg.msg.clear();
    SetResizeWindowCallback(ResizeWindowCallback);

    down_shader = LoadShader(NULL, "shader/lanczos3.fs");
    up_shader = LoadShader(NULL, "shader/bicubic.fs");
    


}


void UpdateVideoPlayScreen()
{
    if(redraw) {
        redraw = false;
        if(is_first_frame)
        {
            is_first_frame = false;
            VideoInit();
            mpv_tx = LoadRenderTexture(videoInfo.width, videoInfo.height);
    
            SetWindowTitle(videoInfo.title);
            ResizeWindowCallback();
        }
        MpvRender();
    }

    PollMpvEvent();

    if(IsFileDropped())
    {
        FilePathList drop_file = LoadDroppedFiles();
        if(drop_file.count == 1) {
            char* drop_file_path = drop_file.paths[0];
            AddSub(drop_file_path);
        }
        UnloadDroppedFiles(drop_file);
    }


    HandleInput();

}


void DrawVideoPlayScreen()
{
    Vector2 resolution{(float)mpv_tx.texture.width, (float)mpv_tx.texture.height};
    if(videoInfo.width > renderRect.width)
    {
        //缩小
        // spdlog::debug("use down shader");
        int resolution_loc = GetShaderLocation(down_shader, "u_resolution");
        SetShaderValue(down_shader, resolution_loc, &resolution, SHADER_UNIFORM_VEC2);
        BeginShaderMode(down_shader);
            DrawTexturePro(mpv_tx.texture, Rectangle{0, 0, (float)mpv_tx.texture.width, (float)mpv_tx.texture.height}, renderRect, Vector2{0, 0}, 0.f, WHITE);
        EndShaderMode();
    }
    else if(videoInfo.width < renderRect.width)
    {
        //放大        
        // spdlog::debug("use up shader");
        int resolution_loc = GetShaderLocation(up_shader, "u_resolution");
        SetShaderValue(up_shader, resolution_loc, &resolution, SHADER_UNIFORM_VEC2);
        BeginShaderMode(up_shader);
            DrawTexturePro(mpv_tx.texture, Rectangle{0, 0, (float)mpv_tx.texture.width, (float)mpv_tx.texture.height}, renderRect, Vector2{0, 0}, 0.f, WHITE);
        EndShaderMode();
    }
    else
    {
        // spdlog::debug("no use shader");
        DrawTexturePro(mpv_tx.texture, Rectangle{0, 0, (float)mpv_tx.texture.width, (float)mpv_tx.texture.height}, renderRect, Vector2{0, 0}, 0.f, WHITE);
    }

    if(GetTime() < osdMsg.lastShowTime && !osdMsg.msg.empty())
    {
        DrawMsg(osdMsg.msg.c_str(), 10, 10, 20, 10);
    }
    if(videoInfo.speed != 1.0)
    {
        std::string msg = fmt::format("Speed: {}", videoInfo.speed);
        DrawMsg(msg.c_str(), 10, 60, 20, 10);
    }
    if(!IsCursorOnScreen())
        return;
    double mousePercent = -1.f;
    Vector2 mousePos = GetMousePosition();
    if(is_menu_show)
    {
        std::vector<MenuItem> menu;
        for(SubTrack& sub_track : videoInfo.subs)
        {
            menu.push_back({.text = sub_track.title.c_str(), .is_selected = sub_track.is_selected});
        }

        int item_id = DrawMenu(menu);
        if(item_id == -2)
            is_menu_show = false;
        if(item_id >= 0)
        {
            is_menu_show = false;
            if(videoInfo.subs[item_id].is_selected)
                DisableSub();
            else
                SetSub(videoInfo.subs[item_id].sid);
            
        }
        return;
    }
    bool should_show_btns = videoInfo.subs.size()>0;
    if((should_show_btns && CheckCollisionPointRec(mousePos, menuRect)) || CheckCollisionPointRec(mousePos, progressRect))
    {
        if(DrawProgress(progressRect, videoInfo.percentPos, &mousePercent))
        {
            Jump(mousePercent);
        }

        if(!CheckCollisionPointRec(mousePos, progressRect)) {
            if(videoInfo.subs.size() > 0)
            {
                if(DrawButton(Rectangle{10, menuRect.y+5, 50, 50}, "sub"))
                {
                    is_menu_show = true;
                }
            }
        }else {
            if(isThumbnailReady)
            {
                Image thumbImage = GetThumbnail(mousePercent, THUMBNAIL_WIDTH);
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
                else if(posX > progressRect.x + progressRect.width - thumbTexture.width)
                    posX = progressRect.x + progressRect.width - thumbTexture.width;
                int posY = progressRect.y - thumbImage.height;
                DrawTexture(thumbTexture, posX, posY, WHITE);
            }
        }
    }

}


void UnloadVideoPlayScreen()
{
    ThumbnailFinish();
    UnloadTexture(thumbTexture);
    MpvFinish();
    SetResizeWindowCallback(NULL);
    UnloadShader(down_shader);
    UnloadShader(up_shader);
}

int FinishVideoPlayScreen()
{
    return 0;
}


void HandleInput()
{
    if(IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) || IsKeyReleased(KEY_SPACE))
        TogglePause();

    //volume control
    Vector2 mouseWheelMove = GetMouseWheelMoveV();
    if(IsKeyPressed(KEY_DOWN) || mouseWheelMove.y < 0)
        ChangeVolume(-5);
    if(IsKeyPressed(KEY_UP) || mouseWheelMove.y > 0)
        ChangeVolume(5);

    if(IsKeyPressed(KEY_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_SIDE))
    {
        Seek(-5);
    }
    if(IsMouseButtonPressed(MOUSE_BUTTON_EXTRA))
    {
        Seek(5); 
    }
        if(IsKeyPressed(KEY_F) || IsMouseButtonDoubleClicked(MOUSE_BUTTON_LEFT))
    {
        CustomToggleFullscreen();
    }

    static bool isKeyRightRepeat = false;
    if(IsKeyPressedRepeat(KEY_RIGHT))
    {
        isKeyRightRepeat = true;
        SetSpeed(3.0);
    }else if(IsKeyReleased(KEY_RIGHT))
    {
        if(isKeyRightRepeat) {
            SetSpeed(1.0);
        }else {
            Seek(5);
        }
        isKeyRightRepeat = false;
    }

}

static void ResizeWindowCallback()
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

    progressRect = { 0, renderHeight-PROGRESS_HEIGHT, (float)renderWidth, PROGRESS_HEIGHT};
    menuRect = { 0, renderHeight-PROGRESS_HEIGHT-MENU_HEIGHT, (float)renderWidth, MENU_HEIGHT};


}

