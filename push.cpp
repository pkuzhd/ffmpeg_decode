#include <iostream>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <chrono>

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
using namespace std;

int main() {
    int ret;
    Status status;
    char video_url[] = "/home/zhd/010.2.mp4";
    int width = 1920;
    int height = 1080;
    AVPixelFormat pix_fmt = AV_PIX_FMT_YUV420P;
    Video2frame video =
            // {"/data/zhanghaodan/web/demo3/1.bg.mp4", width, height, pix_fmt};
            {"rtmp://172.31.204.119:1935/live/1", width, height, pix_fmt};
    video.run();

    const char *out_filename = "rtmp://172.31.204.119:1935/live/push";
    out_filename = "rtmp://127.0.0.1:1935/live/push";
    out_filename = "/home/zhd/test.flv";

    shared_ptr<Frame2video> video_out = make_shared<Frame2video>(out_filename, "flv", "libx264");
    std::shared_ptr<Muxer> muxer = video_out->muxer;
    std::shared_ptr<Encoder> encoder = video_out->encoder;

    AVFormatContext *out_ctx = muxer->context;
    AVCodecContext *codec_ctx = encoder->context;

    codec_ctx->bit_rate = 1000000;
    codec_ctx->width = width;
    codec_ctx->height = height;
    codec_ctx->time_base = (AVRational) {1, 1000};
    codec_ctx->framerate = (AVRational) {25, 1};
    codec_ctx->gop_size = 25;
    codec_ctx->max_b_frames = 1;
    codec_ctx->pix_fmt = pix_fmt;
    codec_ctx->sample_aspect_ratio = (AVRational) {1, 1};
    if (out_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    video_out->open();

    av_dump_format(out_ctx, 0, out_filename, 1);


    AVFrame *frame = av_frame_alloc();
    AVPacket *pkt = av_packet_alloc();

    // frame->format = pix_fmt;
    // frame->width = width;
    // frame->height = height;
    // uint8_t *buffer = (uint8_t *) av_malloc(width * height * 3 / 2);
    // av_image_fill_arrays(frame->data,
    //                      frame->linesize,
    //                      buffer,
    //                      pix_fmt,
    //                      width,
    //                      height,
    //                      1);

    auto start_all = chrono::high_resolution_clock::now();
    int frame_cnt = 0;

    for (int i = 0; i < 25 * 10; ++i) {
        Frame *f;
        while ((f = video.getFrame()) == NULL);

        av_frame_ref(frame, f->frame);
        frame->pts = f->pts * 1000;

        video_out->writeFrame(frame);
        
        auto end_all = chrono::high_resolution_clock::now();
        auto duration = chrono::duration<double, milli>(end_all - start_all).count() / 1000;
        printf("frame: %8d, fps: %4.0lf\n", video_out->frame_cnt, video_out->frame_cnt / duration);
        
        av_frame_unref(frame);
        delete f;
    }
    video_out->writeFrame(nullptr);
    video_out->close();

    auto end_all = chrono::high_resolution_clock::now();
    auto duration = chrono::duration<double, milli>(end_all - start_all).count() / 1000;
    printf("frame: %8d, fps: %4.0lf\n", video_out->frame_cnt, video_out->frame_cnt / duration);

    return 0;
}