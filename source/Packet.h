//
// Created by zhd on 2022/4/2.
//

#ifndef FFMPEG_TEST_PACKET_H
#define FFMPEG_TEST_PACKET_H

#include "ffmpeg_headers.h"

namespace net_video {

    class Packet {
    public:
        Packet();

        ~Packet();

    public:
        AVPacket *avPacket;
    };
}

#endif //FFMPEG_TEST_PACKET_H
