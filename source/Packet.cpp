//
// Created by zhd on 2022/4/2.
//

#include "Packet.h"

namespace net_video {

    Packet::Packet() {
        avPacket = av_packet_alloc();
    }

    Packet::~Packet() {
        av_packet_free(&avPacket);
    }
}