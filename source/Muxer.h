//
// Created by zhd on 2022/4/1.
//

#ifndef FFMPEG_TEST_MUXER_H
#define FFMPEG_TEST_MUXER_H

#include "status.h"
#include "ffmpeg_headers.h"
#include "Packet.h"
#include "Encoder.h"

#include <string>
#include <memory>

namespace net_video {
    class Muxer {
    public:
        Muxer(std::string url, std::string format = std::string());

        ~Muxer();

        Status open();

        Status close();

        Status addStream(std::shared_ptr<Encoder> encoder);

//        std::shared_ptr<Packet> *getPacket();
    public:
        std::string url;
        std::string format;
        AVFormatContext *context{};
    };
}


#endif //FFMPEG_TEST_MUXER_H
