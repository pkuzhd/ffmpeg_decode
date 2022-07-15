#include <iostream>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#ifdef __cplusplus
};
#endif

#include "Video2frame.h"
#include "Frame2video.h"
#include "source/Muxer.h"
#include "source/Encoder.h"

using std::cout;
using std::endl;
using namespace net_video;

int main() {
    int ret;
    Status status;
    char video_url[] = "/home/zhd/010.2.mp4";
    int width = 3840;
    int height = 2160;
    AVPixelFormat pix_fmt = AV_PIX_FMT_YUV420P;
    Video2frame video =
            {"http://172.31.203.194:8765/hls/1.m3u8", width, height, pix_fmt};
    video.run();

    const char *out_filename = "rtmp://172.31.203.194:1935/hls/1.2";

    std::shared_ptr<Muxer> muxer = std::make_shared<Muxer>(out_filename, "flv");
    std::shared_ptr<Encoder> encoder = std::make_shared<Encoder>("libx264");

    AVFormatContext *out_ctx = muxer->context;
    AVCodecContext *codec_ctx = encoder->context;

    codec_ctx->bit_rate = 1000000;
    codec_ctx->width = width;
    codec_ctx->height = height;
    codec_ctx->time_base = (AVRational) {1, 1000};
    codec_ctx->framerate = (AVRational) {50, 1};
    codec_ctx->gop_size = 25;
    codec_ctx->max_b_frames = 1;
    codec_ctx->pix_fmt = pix_fmt;
    codec_ctx->sample_aspect_ratio = (AVRational) {1, 1};
    if (out_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    status = encoder->open();
    if (status != OK) {
        av_log(encoder->context, AV_LOG_INFO, "open error\n");
    }

    status = muxer->addStream(encoder);
    if (status != OK) {
        av_log(muxer->context, AV_LOG_INFO, "addStream error\n");
    }

    status = muxer->open();
    if (status != OK) {
        av_log(muxer->context, AV_LOG_INFO, "open error\n");
    }

    av_dump_format(out_ctx, 0, out_filename, 1);


    AVFrame *frame = av_frame_alloc();
    AVPacket *pkt = av_packet_alloc();

    frame->format = AV_PIX_FMT_YUV420P;
    frame->width = width;
    frame->height = height;
//    ret = av_frame_get_buffer(frame, 0);
    uint8_t *buffer = (uint8_t *) av_malloc(width * height * 3 / 2);
    av_image_fill_arrays(frame->data,
                         frame->linesize,
                         buffer,
                         pix_fmt,
                         width,
                         height,
                         1);
//    char *out_filename = "/home/zhd/test.flv";





    for (int i = 0; i < 50 * 1000; ++i) {
        Frame *f;
        while ((f = video.getFrame()) == NULL);

        memcpy(frame->data[0], f->data, f->size);
        frame->pts = f->pts * 1000;

        ret = avcodec_send_frame(codec_ctx, frame);
        while (true) {
            av_packet_unref(pkt);
            ret = avcodec_receive_packet(codec_ctx, pkt);
            if (ret != 0)
                break;

            pkt->stream_index = 0;

            av_packet_rescale_ts(pkt, codec_ctx->time_base, out_ctx->streams[0]->time_base);
            ret = av_interleaved_write_frame(out_ctx, pkt);
            if (ret < 0) {
                av_log(NULL, AV_LOG_INFO, "av_interleaved_write_frame(), %d\n", ret);
            }
        }
        delete f;
    }
    avcodec_send_frame(codec_ctx, NULL);
    while (true) {
        ret = avcodec_receive_packet(codec_ctx, pkt);
        if (ret != 0)
            break;
//        cout << -1 << " " << pkt->dts << " " << pkt->pts << " " << pkt->size << endl;
        av_packet_rescale_ts(pkt, codec_ctx->time_base, out_ctx->streams[0]->time_base);
        av_interleaved_write_frame(out_ctx, pkt);
    }

    muxer->close();

    return 0;
}