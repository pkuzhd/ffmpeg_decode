//
// Created by zhd on 2022/3/29.
//

#include "Frame2video.h"

using namespace std;
using namespace net_video;

Frame2video::Frame2video(std::string filename, std::string format, std::string codec) {
    muxer = make_shared<Muxer>(filename, "flv");
    encoder = make_shared<Encoder>("libx264");
}

Status Frame2video::open() {
    Status status;
    status = encoder->open();
    if (status != OK) {
        av_log(encoder->context, AV_LOG_INFO, "open error\n");
        return status;
    }

    status = muxer->addStream(encoder);
    if (status != OK) {
        av_log(muxer->context, AV_LOG_INFO, "addStream error\n");
        return status;
    }

    status = muxer->open();
    if (status != OK) {
        av_log(muxer->context, AV_LOG_INFO, "open error\n");
        return status;
    }
    return status;
}

Status Frame2video::writeFrame(AVFrame *_frame) {
    Status status;
    int ret;

    AVFrame *frame = nullptr;
    if (_frame) {
        frame = av_frame_alloc();
        av_frame_ref(frame, _frame);
    }

    ret = avcodec_send_frame(encoder->context, frame);

    AVPacket *pkt = av_packet_alloc();
    while (true) {
        ret = avcodec_receive_packet(encoder->context, pkt);
        if (ret != 0)
            break;

        pkt->stream_index = 0;

        av_packet_rescale_ts(pkt, encoder->context->time_base, muxer->context->streams[0]->time_base);
        ret = av_interleaved_write_frame(muxer->context, pkt);
        if (ret < 0) {
            av_log(NULL, AV_LOG_INFO, "av_interleaved_write_frame(), %d\n", ret);
        }
        ++frame_cnt;
        av_packet_unref(pkt);
    }

    av_packet_free(&pkt);
    av_frame_free(&frame);
    return status;
}

Status Frame2video::close() {
    return muxer->close();
}

Frame2video::~Frame2video() {
    
}