//
// Created by zhd on 2022/3/28.
//

#ifndef FFMPEG_TEST_VIDEO2FRAME_H
#define FFMPEG_TEST_VIDEO2FRAME_H

#include <iostream>
#include <cstring>
#include <queue>
#include <thread>
#include <mutex>

#ifdef __cplusplus
extern "C" {
#endif
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#ifdef __cplusplus
};
#endif

class Frame {
public:
    float pts;
    int64_t raw_pts;
    uint8_t *data = NULL;
    AVFrame *frame;
    size_t size;

    Frame(float pts, AVFrame *frame, size_t size)
            : pts(pts), size(size), frame(frame) {
        data = frame->data[0];
    }

    ~Frame() {
        av_frame_free(&frame);
    }
};

class Video2frame {
public:
    std::string url;
    std::queue<Frame *> buffer;
    std::queue<AVPacket *> pkt_buffer;
    AVFormatContext *format_ctx;
    AVFrame *frame_raw, *frame_out;
    AVCodecContext *codec_ctx;
    SwsContext *sws_ctx = NULL;
    std::mutex m;
    std::thread *thread;
    bool term = false;
    int frame_size;

    bool is_finish = false;

    int video_stream_idx;

    int width;
    int height;
    AVPixelFormat pix_fmt;

    Video2frame(std::string url, int width, int height, AVPixelFormat pix_fmt);

    ~Video2frame();

    void run();

    Frame *getFrame();

    float getPTS();

    int getBufferSize();

    int getPacketBufferSize();
};


#endif //FFMPEG_TEST_VIDEO2FRAME_H
