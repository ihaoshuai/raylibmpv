#include "raylib.h"
#include "screen.h"
#include "ui.h"
#include <spdlog/spdlog.h>

std::string drop_file_path;


void InitIdleScreen()
{
    drop_file_path.clear();
    osdMsg.msg.clear();
}

void UpdateIdleScreen()
{
    if(IsFileDropped())
    {
        FilePathList drop_file = LoadDroppedFiles();
        if(drop_file.count > 1) {
            SetMsg("only play one video");
        }else {
            drop_file_path = drop_file.paths[0];
        }
        UnloadDroppedFiles(drop_file);
    }
}

void DrawIdleScreen()
{
    const char* hint_text = "drop video file to play";
    int font_size = 30;
    int text_width = MeasureText(hint_text, font_size);
    int render_width = GetRenderWidth();
    int render_height = GetRenderHeight();

    DrawText(hint_text, (render_width-text_width)/2, (render_height-font_size)/2, font_size, WHITE);

    if(GetTime() < osdMsg.lastShowTime && !osdMsg.msg.empty())
    {
        DrawMsg(osdMsg.msg.c_str(), 10, 10, 20, 10);
    }
}

void UnloadIdleScreen()
{

}

int FinishIdleScreen(const char** videoPath)
{
    if(drop_file_path.empty())
        return 0;
    else
    {
        *videoPath = drop_file_path.c_str();
        return 1;
    }
}
