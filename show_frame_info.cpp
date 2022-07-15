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

using std::cout;
using std::endl;

int main() {
    int ret;
    char video_url[] = "/home/zhd/010.2.mp4";
    int width = 480;
    int height = 270;
    AVPixelFormat pix_fmt = AV_PIX_FMT_YUV420P;
    Video2frame video =
            {"/home/zhd/test.mp4", width, height, pix_fmt};
    video.run();
    while (true);

    AVCodec *codec = avcodec_find_encoder_by_name("libx264");
    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);

    codec_ctx->bit_rate = 1000000;
    codec_ctx->width = width;
    codec_ctx->height = height;
    codec_ctx->time_base = (AVRational) {1, 1000};
    codec_ctx->framerate = (AVRational) {50, 1};
    codec_ctx->gop_size = 10;
    codec_ctx->max_b_frames = 1;
    codec_ctx->pix_fmt = pix_fmt;
    codec_ctx->sample_aspect_ratio = (AVRational) {1, 1};

    ret = avcodec_open2(codec_ctx, codec, NULL);

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
    char *out_filename = "/home/zhd/test.mp4";
    AVFormatContext *out_ctx;
    avformat_alloc_output_context2(&out_ctx, NULL, NULL, out_filename);

    AVStream *stream = avformat_new_stream(out_ctx, NULL);
    stream->time_base = codec_ctx->time_base;
    avcodec_parameters_from_context(stream->codecpar, codec_ctx);

    av_dump_format(out_ctx, 0, out_filename, 1);
    avio_open(&out_ctx->pb, out_filename, AVIO_FLAG_WRITE);

    avformat_write_header(out_ctx, NULL);


    for (int i = 0; i < 100; ++i) {
        av_frame_make_writable(frame);

        Frame *f;
        while ((f = video.getFrame()) == NULL);

        memcpy(frame->data[0], f->data, f->size);
        frame->pts = i * 20000;

        ret = avcodec_send_frame(codec_ctx, frame);
        while (true) {
            ret = avcodec_receive_packet(codec_ctx, pkt);
            if (ret != 0)
                break;
            cout << i << " " << pkt->dts << " " << pkt->pts << " " << pkt->size << endl;
            av_interleaved_write_frame(out_ctx, pkt);

        }
    }
    avcodec_send_frame(codec_ctx, NULL);
    while (true) {
        ret = avcodec_receive_packet(codec_ctx, pkt);
        if (ret != 0)
            break;
        cout << -1 << " " << pkt->dts << " " << pkt->pts << " " << pkt->size << endl;
        av_interleaved_write_frame(out_ctx, pkt);
    }

    av_write_trailer(out_ctx);
    avio_closep(&out_ctx->pb);

    return 0;
}






//1
//3
//3
//3
//2
//3
//3
//3
//2
//3
//3
//3
//2
//3
//3
//3
//2
//3
//3
//3
//2
//3
//3
//3
//2


//1
//3
//2
//3
//2
//3
//2
//3
//2
//3
//2
//3
//2
//3
//2
//3
//2
//3
//2
//3
//2
//3
//2
//3
//2
//1
//3
//2
//3
//2
//3
//2
//3
//2
//3
//2
//3
//2
//3
//2
//3
//2
//3
//2
//3
//2
//3
//2
//3
//2
