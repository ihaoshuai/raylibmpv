#include "video.h"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>
#include "mpv/client.h"
#include "mpv/render_gl.h"
#include "raylib.h"
#include "ui.h"

mpv_handle* mpv;
mpv_render_context* mpv_gl;
RenderTexture mpv_tx;
bool redraw;
VideoInfo videoInfo{.speed = 1.0};

uint64_t propertyFirstSet = 0x0;

const uint64_t MPV_PROPERTY_PAUSE = 0x1;
const uint64_t MPV_PROPERTY_PERCENTPOS = 0x2;
const uint64_t MPV_PROPERTY_VOLUME = 0x4;
const uint64_t MPV_PROPERTY_SPEED = 0x8;
const uint64_t MPV_PROPERTY_TRACK_LIST = 0x10;

std::atomic<bool> needCheckEvents{ false };

std::vector<SubTrack> GetSubTracks();
void UpdateVideoTracks(mpv_node* node);

static void* get_proc_address(void *ctx, const char *name) {
    return rlGetProcAddress(name);
}


void on_mpv_events(void *ctx)
{
    needCheckEvents.store(true, std::memory_order_relaxed);
}

void on_mpv_render_update(void *ctx)
{
    if(mpv_render_context_update(mpv_gl) & MPV_RENDER_UPDATE_FRAME)
    {
        redraw = true;
    }    
}

void MpvRender()
{
    mpv_opengl_fbo fbo = {
        .fbo = (int)mpv_tx.id,
        .w = mpv_tx.texture.width,
        .h = mpv_tx.texture.height,
    };
    int flip_y = 0;
    mpv_render_param params[] = {
        // Specify the default framebuffer (0) as target. This will
        // render onto the entire screen. If you want to show the video
        // in a smaller rectangle or apply fancy transformations, you'll
        // need to render into a separate FBO and draw it manually.
        {MPV_RENDER_PARAM_OPENGL_FBO, &fbo},
        // Flip rendering (needed due to flipped GL coordinate system).
        {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
        {(mpv_render_param_type)0}
    };
    BeginTextureMode(mpv_tx);
        mpv_render_context_render(mpv_gl, params);
        // GenTextureMipmaps(&mpv_tx.texture);
        // SetTextureFilter(mpv_tx.texture, TEXTURE_FILTER_TRILINEAR);
        rlEnableColorBlend();
    EndTextureMode();
} 
void MpvInit()
{
    mpv = mpv_create();
    if(!mpv)
    {
        spdlog::error("failed to create mpv instance");
        exit(1);
    }
    mpv_set_option_string(mpv, "vo", "libmpv");
    mpv_set_option_string(mpv, "hwdec", "auto");
    // mpv_set_option_string(mpv, "msg-level", "all=v");
    // mpv_set_option_string(mpv, "terminal", "yes");
    mpv_initialize(mpv);

    mpv_opengl_init_params gl_params{ .get_proc_address = get_proc_address };
    int mpv_render_param_advanced_control = 1;
    mpv_render_param params[] = {
        { MPV_RENDER_PARAM_API_TYPE, (void*)MPV_RENDER_API_TYPE_OPENGL },
        { MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_params },
        { MPV_RENDER_PARAM_ADVANCED_CONTROL, &mpv_render_param_advanced_control},
        { (mpv_render_param_type)0 }
    };
    mpv_render_context_create(&mpv_gl, mpv, params);

    mpv_set_wakeup_callback(mpv, on_mpv_events, mpv);
    mpv_render_context_set_update_callback(mpv_gl, on_mpv_render_update, NULL);


}

void LoadVideo(const char *videoPath)
{
    const char* cmd[] = { "loadfile", videoPath, NULL};
    mpv_command_async(mpv, 0, cmd);
    propertyFirstSet = 0x0;
}

void VideoInit()
{
    mpv_get_property(mpv, "media-title", MPV_FORMAT_STRING, &videoInfo.title);
    mpv_get_property(mpv, "dwidth", MPV_FORMAT_INT64, &videoInfo.width);
    mpv_get_property(mpv, "dheight", MPV_FORMAT_INT64, &videoInfo.height);

    //observe property
    mpv_observe_property(mpv, MPV_PROPERTY_PAUSE, "pause", MPV_FORMAT_FLAG);
    mpv_observe_property(mpv, MPV_PROPERTY_PERCENTPOS, "percent-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, MPV_PROPERTY_VOLUME, "volume", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, MPV_PROPERTY_SPEED, "speed", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, MPV_PROPERTY_TRACK_LIST, "track-list", MPV_FORMAT_NODE);

    // videoInfo.subs = GetSubTracks();
}

void MpvFinish()
{
    mpv_unobserve_property(mpv, MPV_PROPERTY_PAUSE);
    mpv_unobserve_property(mpv, MPV_PROPERTY_PERCENTPOS);
    mpv_unobserve_property(mpv, MPV_PROPERTY_VOLUME);
    mpv_unobserve_property(mpv, MPV_PROPERTY_SPEED);
    mpv_unobserve_property(mpv, MPV_PROPERTY_TRACK_LIST);

    mpv_render_context_free(mpv_gl);
    UnloadRenderTexture(mpv_tx);
    mpv_destroy(mpv);
}

void TogglePause()
{
    int paused = 0;
    if (mpv_get_property(mpv, "pause", MPV_FORMAT_FLAG, &paused) >= 0) {
        const char *new_state = paused ? "no" : "yes";
        mpv_set_property_string(mpv, "pause", new_state);
    }
}


void Jump(double percent)
{
    std::string percent_str = std::to_string(percent*100);
    const char* cmd[] = { "seek", percent_str.c_str(), "absolute-percent", NULL };
    mpv_command_async(mpv, 0, cmd);
}

void Seek(int second)
{
    std::string sec_str = std::to_string(second);
    const char* cmd[] = { "seek", sec_str.c_str(), NULL };
    mpv_command_async(mpv, 0, cmd);
}

void ChangeVolume(int step)
{
    std::string step_str = std::to_string(step);
    const char* cmd[] = { "add", "volume", step_str.c_str(), NULL };
    mpv_command_async(mpv, 0, cmd);
}

void SetSpeed(double speed)
{
    mpv_set_property_async(mpv, 0, "speed", MPV_FORMAT_DOUBLE, &speed);
}

void PollMpvEvent()
{
    if(!needCheckEvents.exchange(false))
        return;
    while (true) {
        mpv_event *event = mpv_wait_event(mpv, 0);
        if(event->event_id == MPV_EVENT_NONE)
            break;
        if(event->event_id == MPV_EVENT_LOG_MESSAGE)
        {
            mpv_event_log_message *msg = (mpv_event_log_message*)event->data;
            if(strstr(msg->text, "DR image"))
                spdlog::info("MPV_LOG: {}", msg->text);
            continue;
        }
        if(event->event_id == MPV_EVENT_PROPERTY_CHANGE)
        {
            mpv_event_property *prop = (mpv_event_property*)event->data;
            if(event->reply_userdata == MPV_PROPERTY_PAUSE)
            {
                if((propertyFirstSet & MPV_PROPERTY_PAUSE) == 0)
                {
                    propertyFirstSet = propertyFirstSet | MPV_PROPERTY_PAUSE;
                    continue;
                }
                if(prop->format == MPV_FORMAT_FLAG)
                {
                    int isPause = *(int*)prop->data;
                    if(isPause)
                        SetMsg("Pause");
                    else
                        SetMsg("Play");
                }
            }else if (event->reply_userdata == MPV_PROPERTY_PERCENTPOS) {
                if(prop->format == MPV_FORMAT_DOUBLE)
                {
                    videoInfo.percentPos = (*(double*)prop->data)/100;
                }
            }else if (event->reply_userdata == MPV_PROPERTY_VOLUME) {
                if((propertyFirstSet & MPV_PROPERTY_VOLUME) == 0)
                {
                    propertyFirstSet = propertyFirstSet | MPV_PROPERTY_VOLUME;
                    continue;
                }
                if(prop->format == MPV_FORMAT_DOUBLE)
                {
                    double vol = *(double*)prop->data;
                    SetMsg(fmt::format("Volume: {}", vol));
                }
            }else if(event->reply_userdata == MPV_PROPERTY_SPEED) {
                if(prop->format == MPV_FORMAT_DOUBLE)
                {
                    videoInfo.speed = *(double*)prop->data;
                }
            }else if(event->reply_userdata == MPV_PROPERTY_TRACK_LIST) {
                if(prop->format == MPV_FORMAT_NODE)
                {
                    spdlog::debug("track list change");
                    UpdateVideoTracks((mpv_node*)prop->data);
                    mpv_free_node_contents((mpv_node*)prop->data);
                }
            }
        }
        //TODO handle other mpv event
    }
}


std::vector<SubTrack> GetSubTracks()
{
    std::vector<SubTrack> subs;
    mpv_node node;
    if((mpv_get_property(mpv, "track-list", MPV_FORMAT_NODE, &node)) >= 0) 
    {
        if(node.format == MPV_FORMAT_NODE_ARRAY)
        {
            mpv_node_list *list = node.u.list;
            for(int i=0; i<list->num; i++)
            {
                if(list->values[i].format == MPV_FORMAT_NODE_MAP)
                {
                    mpv_node_list *track = list->values[i].u.list;
                    
                    char *type = NULL;
                    int64_t id = -1;
                    char *lang = NULL;
                    char *title = NULL;
                    bool selected = false;

                    for(int j=0; j<track->num; j++)
                    {
                        char* key = track->keys[j];
                        mpv_node *val = &track->values[j];
                        if (strcmp(key, "type") == 0 && val->format == MPV_FORMAT_STRING) {
                            type = val->u.string;
                        } else if (strcmp(key, "id") == 0 && val->format == MPV_FORMAT_INT64) {
                            id = val->u.int64;
                        } else if (strcmp(key, "lang") == 0 && val->format == MPV_FORMAT_STRING) {
                            lang = val->u.string;
                        } else if (strcmp(key, "title") == 0 && val->format == MPV_FORMAT_STRING) {
                            title = val->u.string;
                        } else if (strcmp(key, "selected") == 0 && val->format == MPV_FORMAT_FLAG) {
                            selected = val->u.flag;
                        }
                    }

                    if(type && strcmp(type, "sub") == 0)
                    {
                        subs.push_back({
                            .sid = id, .lang = lang?lang:"unknown", .title = title?title:"unknown", .is_selected = selected
                        });
                    }
                }
            }
        }    
    }
    mpv_free_node_contents(&node);
    return subs;
}

void SetSub(int64_t sid)
{
    mpv_set_property_async(mpv, 0, "sid", MPV_FORMAT_INT64, &sid);
}

void DisableSub()
{
    mpv_set_property_string(mpv, "sid", "no");
}


void UpdateVideoTracks(mpv_node* node)
{
    videoInfo.subs.clear();
    if((*node).format == MPV_FORMAT_NODE_ARRAY)
    {
        mpv_node_list *list = (*node).u.list;
        for(int i=0; i<list->num; i++)
        {
            if(list->values[i].format == MPV_FORMAT_NODE_MAP)
            {
                mpv_node_list *track = list->values[i].u.list;
                
                char *type = NULL;
                int64_t id = -1;
                char *lang = NULL;
                char *title = NULL;
                bool selected = false;

                for(int j=0; j<track->num; j++)
                {
                    char* key = track->keys[j];
                    mpv_node *val = &track->values[j];
                    if (strcmp(key, "type") == 0 && val->format == MPV_FORMAT_STRING) {
                        type = val->u.string;
                    } else if (strcmp(key, "id") == 0 && val->format == MPV_FORMAT_INT64) {
                        id = val->u.int64;
                    } else if (strcmp(key, "lang") == 0 && val->format == MPV_FORMAT_STRING) {
                        lang = val->u.string;
                    } else if (strcmp(key, "title") == 0 && val->format == MPV_FORMAT_STRING) {
                        title = val->u.string;
                    } else if (strcmp(key, "selected") == 0 && val->format == MPV_FORMAT_FLAG) {
                        selected = val->u.flag;
                    }
                }

                if(type && strcmp(type, "sub") == 0)
                {
                    videoInfo.subs.push_back({
                        .sid = id, .lang = lang?lang:"unknown", .title = title?title:"unknown", .is_selected = selected
                    });
                }
            }
        }
    }
}


void AddSub(const char* sub_path)
{
    const char* cmd[] = { "sub-add", sub_path, "select", "external sub", NULL };
    mpv_command_async(mpv, 0, cmd);
}
