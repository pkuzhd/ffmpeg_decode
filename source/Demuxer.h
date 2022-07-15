//
// Created by zhd on 2022/4/2.
//

#ifndef FFMPEG_TEST_DEMUXER_H
#define FFMPEG_TEST_DEMUXER_H

#include "status.h"
#include "Packet.h"

namespace net_video {

    class Demuxer {
    public:
        Demuxer(std::string url);

        ~Demuxer();

        Status open();

        Status close();

        std::shared_ptr<Packet> *getPacket();

    };
}


#endif //FFMPEG_TEST_DEMUXER_H
