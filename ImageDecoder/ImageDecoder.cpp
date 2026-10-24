#include "ImageDecoder.h"
#include <QDebug>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

ImageFrame ImageDecoder::decode(const QString& filePath) {
    ImageFrame result;
    AVFormatContext* fmtCtx = nullptr;
    AVCodecContext* codecCtx = nullptr;
    SwsContext* swsCtx = nullptr;
    AVPacket* packet = nullptr;
    AVFrame* frame = nullptr;
    AVFrame* frameRGB = nullptr;
    uint8_t* rgbBuffer = nullptr;

    std::string pathStr = filePath.toStdString();
    if (avformat_open_input(&fmtCtx, pathStr.c_str(), nullptr, nullptr) != 0) {
        qDebug() << "ImageDecoder: 打开文件失败:" << filePath;
        return result;
    }

    if (avformat_find_stream_info(fmtCtx, nullptr) < 0) {
        avformat_close_input(&fmtCtx);
        return result;
    }

    // 寻找视频/图片流
    int streamIdx = -1;
    for (unsigned int i = 0; i < fmtCtx->nb_streams; ++i) {
        if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            streamIdx = i;
            break;
        }
    }

    if (streamIdx == -1) {
        avformat_close_input(&fmtCtx);
        return result;
    }

    AVCodecParameters* codecPar = fmtCtx->streams[streamIdx]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecPar->codec_id);
    if (!codec) {
        avformat_close_input(&fmtCtx);
        return result;
    }

    codecCtx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecCtx, codecPar);
    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
        avcodec_free_context(&codecCtx);
        avformat_close_input(&fmtCtx);
        return result;
    }

    packet = av_packet_alloc();
    frame = av_frame_alloc();
    frameRGB = av_frame_alloc();

    while (av_read_frame(fmtCtx, packet) >= 0) {
        if (packet->stream_index == streamIdx) {
            if (avcodec_send_packet(codecCtx, packet) == 0) {
                if (avcodec_receive_frame(codecCtx, frame) == 0) {
                    result.width = frame->width;
                    result.height = frame->height;

                    // 转成标准 RGB24 格式供 OpenGL 上传
                    swsCtx = sws_getContext(
                        result.width, result.height, (AVPixelFormat)frame->format,
                        result.width, result.height, AV_PIX_FMT_RGB24,
                        SWS_BILINEAR, nullptr, nullptr, nullptr
                    );

                    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, result.width, result.height, 1);
                    result.rgbData.resize(numBytes);

                    av_image_fill_arrays(frameRGB->data, frameRGB->linesize, result.rgbData.data(),
                        AV_PIX_FMT_RGB24, result.width, result.height, 1);

                    sws_scale(swsCtx, (const uint8_t* const*)frame->data, frame->linesize,
                        0, result.height, frameRGB->data, frameRGB->linesize);

                    result.isValid = true;
                    av_packet_unref(packet);
                    break;
                }
            }
        }
        av_packet_unref(packet);
    }

    // 释放 FFmpeg 结构
    if (frameRGB) av_frame_free(&frameRGB);
    if (frame) av_frame_free(&frame);
    if (packet) av_packet_free(&packet);
    if (swsCtx) sws_freeContext(swsCtx);
    if (codecCtx) avcodec_free_context(&codecCtx);
    if (fmtCtx) avformat_close_input(&fmtCtx);

    return result;
}