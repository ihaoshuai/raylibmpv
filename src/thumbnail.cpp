#include "thumbnail.h"
#include "raylib.h"
#include "spdlog/spdlog.h"
#include <cmath>

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/ffversion.h>
    #include <libswscale/swscale.h>
    #include "libavcodec/codec.h"
    #include "libavcodec/codec_par.h"
    #include "libavcodec/packet.h"
    #include "libavutil/avutil.h"
    #include "libavutil/frame.h"
    #include "libavutil/rational.h"
}


bool isThumbnailReady = false;

AVFormatContext *fmtCtx;
int videoStreamIdx = -1;
AVCodecContext *codeCtx;

void ThumbnailInit(const char *videoPath)
{
    isThumbnailReady = false;
    fmtCtx = nullptr;

    // open video file
    if(avformat_open_input(&fmtCtx, videoPath, nullptr, nullptr) < 0)
    {
        spdlog::error("Failed to open video file : {}", videoPath);
        return;
    }

    // get stream infi
    if(avformat_find_stream_info(fmtCtx, nullptr) < 0)
    {
        spdlog::error("Failed to find stream info");
        return;
    }

    // find video stream
    AVCodecParameters *codeParams = nullptr;
    for(unsigned int i=0; i<fmtCtx->nb_streams; i++)
    {
        if(fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) 
        {
            videoStreamIdx = i;
            codeParams = fmtCtx->streams[i]->codecpar;
            break;
        }
    }

    if(videoStreamIdx == -1)
    {
        spdlog::error("No found video stream");
        avformat_close_input(&fmtCtx);
        return;
    }

    // find decoder
    const AVCodec *decoder = avcodec_find_decoder(codeParams->codec_id);
    if(!decoder)
    {
        spdlog::error("Codec not found");
        avformat_close_input(&fmtCtx);
        return;
    }

    codeCtx = avcodec_alloc_context3(decoder);
    avcodec_parameters_to_context(codeCtx, codeParams);

    if(avcodec_open2(codeCtx, decoder, nullptr) < 0)
    {
        spdlog::error("Failed to open codec");
        avcodec_free_context(&codeCtx);
        avformat_close_input(&fmtCtx);
        return;
    }

    isThumbnailReady = true;
}


Image GetThumbnail(double percentage, int targetWidth)
{
    Image emptyImage{0};
    if(!isThumbnailReady)
        return emptyImage;
    //jump to specified time point
    AVStream *stream = fmtCtx->streams[videoStreamIdx];
    int64_t targetTimestamp = 0;
    if (stream->duration != AV_NOPTS_VALUE) {
        targetTimestamp = static_cast<int64_t>(stream->duration * percentage);
    } else if (fmtCtx->duration != AV_NOPTS_VALUE) {
        // fmtCtx->duration 单位是 AV_TIME_BASE (微秒)，需要转换为该视频流的 time_base
        double durationInSeconds = static_cast<double>(fmtCtx->duration) / AV_TIME_BASE;
        targetTimestamp = static_cast<int64_t>((durationInSeconds * percentage) / av_q2d(stream->time_base));
    }


    // seek forward to find the nearest key frame
    if(av_seek_frame(fmtCtx, videoStreamIdx, targetTimestamp, AVSEEK_FLAG_BACKWARD) < 0)
    {
        spdlog::error("Error seeking to timestamp");
    }

    // flush decoder buffer, prepare to receive the seek frame
    avcodec_flush_buffers(codeCtx);

    // decode seek frame
    AVPacket *packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    bool frameDecoded = false;

    while(av_read_frame(fmtCtx, packet) >= 0)
    {
        if(packet->stream_index == videoStreamIdx)
        {
            if(avcodec_send_packet(codeCtx, packet) == 0)
            {
                if(avcodec_receive_frame(codeCtx, frame) == 0)
                {
                    frameDecoded = true;
                    av_packet_unref(packet);
                    break;
                }
            }
        }
        av_packet_unref(packet);
    }
    av_packet_free(&packet);

    if(!frameDecoded)
    {
        spdlog::error("Failed to decode frame at target time");
        av_frame_free(&frame);
        avcodec_free_context(&codeCtx);
        avformat_close_input(&fmtCtx);
        return emptyImage;
    }

    int targetHeight = static_cast<int>(targetWidth * (static_cast<double>(frame->height)/frame->width));
    
    size_t dataSize = targetWidth * targetHeight * 3;
    unsigned char* rgbPixels = (unsigned char*)RL_MALLOC(dataSize);
    SwsContext *swsCtx = sws_getContext(frame->width, frame->height, codeCtx->pix_fmt, targetWidth, targetHeight, AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);

    uint8_t *destSlice[1] = {rgbPixels };
    int destStride[1] = { targetWidth * 3};

    sws_scale(swsCtx, frame->data, frame->linesize, 0, frame->height, destSlice, destStride);

    sws_freeContext(swsCtx);
    av_frame_free(&frame);

    Image raylibImage = {
        .data = rgbPixels,
        .width = targetWidth,
        .height = targetHeight,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8
    };

    return raylibImage;

}


void ThumbnailFinish()
{
    if(isThumbnailReady) {
        avcodec_free_context(&codeCtx);
        avformat_close_input(&fmtCtx);
    }
}