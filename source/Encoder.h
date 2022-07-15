//
// Created by zhd on 2022/4/1.
//

#ifndef FFMPEG_TEST_ENCODER_H
#define FFMPEG_TEST_ENCODER_H

#include "status.h"
#include "ffmpeg_headers.h"

#include <string>

namespace net_video {

    class Encoder {
    public:
        Encoder(std::string format);

        ~Encoder();

        Status open();

        Status close();

        Status setOption(const std::string &key, const std::string &value);

    public:
        std::string format;
        AVCodecContext *context;
    };
}

#endif //FFMPEG_TEST_ENCODER_H
