//
// Created by zhd on 2022/4/2.
//

#include "Frame.h"

namespace net_video {
    Frame::Frame() {
        avFrame = av_frame_alloc();
    }

    Frame::~Frame() {
        av_frame_free(&avFrame);
    }
}