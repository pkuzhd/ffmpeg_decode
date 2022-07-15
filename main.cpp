#include <iostream>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

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

using std::cout;
using std::endl;

int main() {
    char video_url[] = "/home/zhd/010.2.mp4";
    int width = 960;
    int height = 540;
//    Frame *f;
//    Video2frame v("http://172.31.203.194:8765/hls/1.m3u8", width, height, AV_PIX_FMT_BGR24);
//    Video2frame v2("http://172.31.203.194:8765/hls/2.m3u8", width, height, AV_PIX_FMT_BGR24);
//    v.run();
//    v2.run();
//    float last = 0;
//    cv::Mat rgbImg;
//    rgbImg.create(height, width, CV_8UC3);
//    char filename[256];
//
//    while (v.getBufferSize() == 0);
//    while (v2.getBufferSize() == 0);
//
//    while (v.getPTS() - v2.getPTS() < -0.01) {
//        while ((f = v.getFrame()) == NULL);
//        delete f;
//        while (v.getBufferSize() == 0);
//    }
//    while (v.getPTS() - v2.getPTS() > 0.01) {
//        while ((f = v2.getFrame()) == NULL);
//        delete f;
//        while (v2.getBufferSize() == 0);
//    }
//    FILE *f1 = fopen("/home/zhd/hls1.log", "w+");
//    FILE *f2 = fopen("/home/zhd/hls2.log", "w+");
//
//    while (1) {
//        while ((f = v.getFrame()) == NULL);
////        printf("%.3f %.3f\n", f->pts, f->pts - last);
//        memcpy(rgbImg.data, f->data, height * width * 3 * sizeof(unsigned char));
//        sprintf(filename, "/home/zhd/CLionProjects/ffmpeg_test/png/%.3f-%d.png", f->pts, 1);
//        cv::imwrite(filename, rgbImg);
////        last = f->pts;
//        fprintf(f1, "%.3f\n", f->pts);
//        delete f;
//
//        while ((f = v2.getFrame()) == NULL);
//        memcpy(rgbImg.data, f->data, height * width * 3 * sizeof(unsigned char));
//        sprintf(filename, "/home/zhd/CLionProjects/ffmpeg_test/png/%.3f-%d.png", f->pts, 2);
//        cv::imwrite(filename, rgbImg);
//        fprintf(f2, "%.3f\n", f->pts);
//        delete f;
//
//    }
//    return 0;
    Video2frame video[5] = {
            {"http://172.31.203.194:8765/hls/1.m3u8", width, height, AV_PIX_FMT_BGR24},
            {"http://172.31.203.194:8765/hls/2.m3u8", width, height, AV_PIX_FMT_BGR24},
            {"http://172.31.203.194:8765/hls/3.m3u8", width, height, AV_PIX_FMT_BGR24},
            {"http://172.31.203.194:8765/hls/4.m3u8", width, height, AV_PIX_FMT_BGR24},
            {"http://172.31.203.194:8765/hls/5.m3u8", width, height, AV_PIX_FMT_BGR24}
            // {"/home/zhd/test.mp4", 480, 270, AV_PIX_FMT_YUV420P},
            // {"/home/zhd/test.mp4", 480, 270, AV_PIX_FMT_YUV420P},
            // {"/home/zhd/test.mp4", 480, 270, AV_PIX_FMT_YUV420P},
            // {"/home/zhd/test.mp4", 480, 270, AV_PIX_FMT_YUV420P},
            // {"/home/zhd/test.mp4", 480, 270, AV_PIX_FMT_YUV420P}
    };
    Frame *frame[5] = {NULL, NULL, NULL, NULL, NULL};

    bool flag = false;
    float pts = 0;
    cout << "begin" << endl;

    for (int i = 0; i < 5; ++i) {
        video[i].run();
    }
    for (int i = 0; i < 5; ++i) {
        while ((frame[i] = video[i].getFrame()) == NULL);
        pts = std::max(pts, frame[i]->pts);
    }

    while (!flag) {
        flag = true;
        for (int i = 0; i < 5; ++i) {
            if (pts - frame[i]->pts > 0.01) {
                delete frame[i];
                while ((frame[i] = video[i].getFrame()) == NULL);
                pts = std::max(pts, frame[i]->pts);
            }

            if (pts - frame[i]->pts > 0.01) {
                flag = false;
            }
        }
    }

    int cnt = 0;
    flag = true;
    time_t begin = time(NULL);
    time_t end;
    float pts_begin = frame[0]->pts;
    int remain;

//    int fd = open("/home/zhd/pipe", O_WRONLY);
    int fd = open("/dev/null", O_WRONLY);

    while (true) {
        if (cnt % 50 == 0) {
            end = time(NULL);
            cout << end - begin << " " << frame[0]->pts - pts_begin << " "
                 << (frame[0]->pts - pts_begin) / (end - begin) << endl;
            for (int i = 0; i < 5; ++i) {
                printf("\t%d %.3f %d\n", i, frame[i]->pts, video[i].getBufferSize());
            }
        }
        remain = sizeof(float);
        while (remain > 0) {
            int r = write(fd, (char *) (&frame[0]->pts) + (sizeof(float) - remain), remain);
            if (r < 0) {
                return 0;
            }
            remain -= r;
        }
        for (int i = 0; i < 5; ++i) {
            remain = frame[i]->size;
            while (remain > 0) {
                int r = write(fd, frame[i]->data + (frame[i]->size - remain), remain);
                if (r < 0) {
                    return 0;
                }
                remain -= r;
            }
            delete frame[i];
        }
        pts = 0;
        for (int i = 0; i < 5; ++i) {
            while ((frame[i] = video[i].getFrame()) == NULL);
            pts = std::max(pts, frame[i]->pts);
        }
        bool sync = true;
        for (int i = 0; i < 5; ++i) {
            if (pts - frame[i]->pts > 0.01)
                sync = false;
        }
        if (!sync) {
            for (int i = 0; i < 5; ++i) {
                printf("%d %.3f\n", i, frame[i]->pts);
            }
            cout << "not sync" << endl;
            flag = false;
            while (!flag) {
                flag = true;
                for (int i = 0; i < 5; ++i) {
                    if (pts - frame[i]->pts > 0.01) {
                        delete frame[i];
                        while ((frame[i] = video[i].getFrame()) == NULL);
                        pts = std::max(pts, frame[i]->pts);
                    }

                    if (pts - frame[i]->pts > 0.01) {
                        flag = false;
                    }
                }
            }
            for (int i = 0; i < 5; ++i) {
                printf("%d %.3f\n", i, frame[i]->pts);
            }
//            return 0;
        } else {
//            for (int i = 0; i < 5; ++i) {
//                printf("%d %.3f\n", i, frame[i]->pts);
//            }
        }
        for (int i = 0; i < 5; ++i) {
            cv::Mat rgbImg;
            rgbImg.create(height, width, CV_8UC3);
            char filename[256];
            memcpy(rgbImg.data, frame[i]->data, height * width * 3 * sizeof(unsigned char));
            sprintf(filename, "/home/zhd/CLionProjects/ffmpeg_test/png/%.3f-%d.png", frame[i]->pts, i);
//            cv::imwrite(filename, rgbImg);
        }
        ++cnt;
    }

//
//    while (!sync) {
//        for (int i = 0; i < 5; ++i) {
//            if (pts >)
//        }
//    }
    //    while (video1.getBufferSize() != -1) {
//        Frame *frame;
//        while ((frame = video1.getFrame()) == NULL)
//        {
//            if (video1.getBufferSize() == -1)
//                break;
//        }
//        if (frame == NULL)
//            break;
//        cout << frame->pts << " " << frame->size << endl;
//        delete frame;
//    }

    return 0;
    //    char video_url[] = "/data/buffer/cam1/010.mp4";
    //    char video_url[] = "rtmp://172.31.203.194:1935/live/1";
    //    char video_url[] = "/Users/zhanghaodan/Desktop/dancing.mp4";

    int ret;
    AVFormatContext *fmt = NULL;
    AVPacket *pkt = NULL;

    pkt = av_packet_alloc();
    fmt = avformat_alloc_context();

    ret = avformat_open_input(&fmt, video_url, NULL, NULL);
    ret = avformat_find_stream_info(fmt, NULL);

//    av_dump_format(fmt, 0, video_url, 0);

    for (int i = 0; i < fmt->nb_streams; ++i) {
        cout << fmt->streams[i]->codecpar->codec_type << endl;
        cout << fmt->streams[i]->codecpar->codec_id << endl;
    }

    AVCodecParameters *codec_para = fmt->streams[0]->codecpar;
    AVCodec *codec = avcodec_find_decoder(codec_para->codec_id);
    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codec_ctx, codec_para);
    avcodec_open2(codec_ctx, codec, NULL);

    AVFrame *frame_yuv = av_frame_alloc();
    AVFrame *frame_raw = av_frame_alloc();

    int buffer_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
                                               width,
                                               height,
                                               1);
    uint8_t *buffer = (uint8_t *) av_malloc(buffer_size);
    av_image_fill_arrays(frame_yuv->data,
                         frame_yuv->linesize,
                         buffer,
                         AV_PIX_FMT_YUV420P,
                         width,
                         height,
                         1);

    SwsContext *sws_ctx = sws_getContext(codec_ctx->width,
                                         codec_ctx->height,
                                         codec_ctx->pix_fmt,
                                         width,
                                         height,
                                         AV_PIX_FMT_YUV420P,
                                         SWS_BICUBIC, NULL, NULL, NULL);

    while (1) {
        ret = av_read_frame(fmt, pkt);
        if (ret != 0)
            break;
        if (pkt->stream_index != 0)
            continue;
        float base = (float) fmt->streams[0]->time_base.num / fmt->streams[0]->time_base.den;
        cout << "packet: " << ret << " "
             << pkt->stream_index << " "
             << pkt->pts << " "
             << (float) pkt->pts * base << " "
             << pkt->duration << " "
             << fmt->streams[0]->time_base.num << " " << fmt->streams[0]->time_base.den
             << endl;

        ret = avcodec_send_packet(codec_ctx, pkt);

        while ((ret = avcodec_receive_frame(codec_ctx, frame_raw)) != AVERROR(EAGAIN)) {
            cout << cnt << " "
                 << frame_raw->pts << " "
                 << frame_raw->pkt_duration << " "
                 << endl;

            sws_scale(sws_ctx,
                      (const uint8_t *const *) frame_raw->data,
                      frame_raw->linesize,
                      0,
                      codec_ctx->height,
                      frame_yuv->data,
                      frame_yuv->linesize
            );

            cout << frame_raw->width << " "
                 << frame_raw->height << " "
                 << frame_yuv->width << " "
                 << frame_yuv->height << " "
                 << endl;
            ++cnt;

//            cout << frame_raw->format << " " << frame_yuv->format << endl;
//            cout << frame_raw->data[0][0] << endl;
//            cv::Mat yuvImg;
//            cv::Mat rgbImg;
//            yuvImg.create(height * 3 / 2, width, CV_8UC1);
//            memcpy(yuvImg.data, frame_yuv->data[0], height * width * 3 / 2 * sizeof(unsigned char));
//            cv::cvtColor(yuvImg, rgbImg, CV_YUV420p2RGBA);
//
//            cv::imshow("yuv", rgbImg);
//            cv::waitKey();
            return 0;
        }
        if (cnt > 30)
            break;
    }
}
