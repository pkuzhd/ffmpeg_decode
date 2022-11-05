//
// Created by zhd on 2022/3/29.
//

#ifndef FFMPEG_TEST_FRAME2VIDEO_H
#define FFMPEG_TEST_FRAME2VIDEO_H

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

#include "source/status.h"
#include "source/Muxer.h"
#include "source/Encoder.h"

class Frame2video {
public:
    std::shared_ptr<net_video::Muxer> muxer = nullptr;
    std::shared_ptr<net_video::Encoder> encoder = nullptr;

    int frame_cnt = 0;

    Frame2video(std::string filename, std::string format, std::string codec);
    
    ~Frame2video();

    Status open();

    Status writeFrame(AVFrame *f);

    Status close();

};


#endif //FFMPEG_TEST_FRAME2VIDEO_H
