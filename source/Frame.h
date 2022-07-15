//
// Created by zhd on 2022/4/2.
//

#ifndef FFMPEG_TEST_FRAME_H
#define FFMPEG_TEST_FRAME_H

#include "ffmpeg_headers.h"

namespace net_video {

    class Frame {
        Frame();

        ~Frame();

    public:
        AVFrame *avFrame;
    };
}


#endif //FFMPEG_TEST_FRAME_H
