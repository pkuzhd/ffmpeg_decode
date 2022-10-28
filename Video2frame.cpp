//
// Created by zhd on 2022/3/28.
//

#include "Video2frame.h"


void demux_decode_thread(Video2frame *arg) {
    bool term = false;
    int ret;

    AVFormatContext *format_ctx = arg->format_ctx;
    AVPacket *pkt;
    AVCodecContext *codec_ctx = arg->codec_ctx;
    AVFrame *frame_raw = arg->frame_raw;
    AVFrame *frame_out = arg->frame_out;
    SwsContext *sws_ctx = arg->sws_ctx;
    std::queue<Frame *> &buffer = arg->buffer;
    std::queue<AVPacket *> &pkt_buffer = arg->pkt_buffer;
    int frame_size = arg->frame_size;

    while (!term) {
        pkt = av_packet_alloc();
        ret = av_read_frame(format_ctx, pkt);
        if (ret != 0)
            continue;

        if (pkt->stream_index == arg->video_stream_idx) {
            int size;
            arg->m.lock();
            size = buffer.size();
            pkt_buffer.push(pkt);
            arg->m.unlock();

            while (size >= 500) {
                arg->m.lock();
                size = buffer.size();
                arg->m.unlock();
            }

            if (size < 500) {
                arg->m.lock();
                pkt = pkt_buffer.front();
                pkt_buffer.pop();
                arg->m.unlock();

                avcodec_send_packet(codec_ctx, pkt);

                av_packet_free(&pkt);

                using std::cout;
                using std::endl;
//            if (ret != 0)
//                cout << ret << endl;

//            printf("%s pkt %d %.3f\n", arg->url.c_str(), pkt->pts,
//                   pkt->pts * 1.0 * av_q2d(format_ctx->streams[arg->video_stream_idx]->time_base));

                while ((ret = avcodec_receive_frame(codec_ctx, frame_raw)) != AVERROR(EAGAIN)) {
                    sws_scale(sws_ctx,
                              (const uint8_t *const *) frame_raw->data,
                              frame_raw->linesize,
                              0,
                              codec_ctx->height,
                              frame_out->data,
                              frame_out->linesize
                    );
                    float pts = frame_raw->pts * 1.0 * av_q2d(format_ctx->streams[arg->video_stream_idx]->time_base);
                    AVFrame *f = av_frame_alloc();
//                    uint8_t *buff = (uint8_t *) av_malloc(frame_size);
//                    av_image_fill_arrays(f->data,
//                                         f->linesize,
//                                         buff,
//                                         arg->pix_fmt,
//                                         arg->width,
//                                         arg->height,
//                                         1);
                    av_frame_copy_props(f, frame_out);
                    f->format = frame_out->format = arg->pix_fmt;
                    f->width = frame_out->width = arg->width;
                    f->height = frame_out->height = arg->height;
                    av_frame_get_buffer(f, 1);

                    av_frame_copy(f, frame_out);
                    Frame *frame = new Frame(pts, f, frame_size);
                    frame->raw_pts = frame_raw->pts;
//                printf("%s frame %ld %.3f %ld\n", arg->url.c_str(), frame_raw->pts,
//                       frame_raw->pts * 1.0 * av_q2d(format_ctx->streams[arg->video_stream_idx]->time_base),
//                       frame_raw->pkt_duration);

//                cout << "frame " << frame_raw->pts << endl;
//                std::cout << arg->url << " "
//                          << frame_raw->pts << " "
//                          << pts << " "
//                          << std::endl;
//                std::cout << pkt->pts << " "
//                          << pkt->duration << " "
//                          << frame_raw->pkt_duration << " "
//                          << std::endl;

                    av_frame_unref(frame_raw);

                    arg->m.lock();
                    buffer.push(frame);
                    arg->m.unlock();
                }
            }
        } else {
            av_packet_free(&pkt);
        }
        arg->m.lock();
        term = arg->term;
        arg->m.unlock();
    }
    std::cout << "end" << std::endl;
    arg->m.lock();
    arg->is_finish = true;
    arg->m.unlock();

}

Video2frame::Video2frame(std::string url, int width, int height, AVPixelFormat pix_fmt)
        : url(url), width(width), height(height), pix_fmt(pix_fmt) {
    format_ctx = avformat_alloc_context();
    avformat_open_input(&format_ctx, url.c_str(), NULL, NULL);
    avformat_find_stream_info(format_ctx, NULL);

    video_stream_idx = -1;
    for (int i = 0; i < format_ctx->nb_streams; ++i) {
        if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
            break;
        }
    }

    AVCodecParameters *codec_para = format_ctx->streams[video_stream_idx]->codecpar;
    AVCodec *codec = avcodec_find_decoder(codec_para->codec_id);
    codec_ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codec_ctx, codec_para);
    avcodec_open2(codec_ctx, codec, NULL);

    frame_raw = av_frame_alloc();
    frame_out = av_frame_alloc();

    frame_size = av_image_get_buffer_size(pix_fmt,
                                          width,
                                          height,
                                          1);
    uint8_t *buffer = (uint8_t *) av_malloc(frame_size);
    av_image_fill_arrays(frame_out->data,
                         frame_out->linesize,
                         buffer,
                         pix_fmt,
                         width,
                         height,
                         1);

    sws_ctx = sws_getContext(codec_ctx->width,
                             codec_ctx->height,
                             codec_ctx->pix_fmt,
                             width,
                             height,
                             pix_fmt,
                             SWS_BICUBIC, NULL, NULL, NULL);


}

Video2frame::~Video2frame() {
    m.lock();
    term = true;
    m.unlock();
    thread->join();
}

Frame *Video2frame::getFrame() {
    m.lock();
    Frame *frame;
    if (buffer.empty()) {
        frame = NULL;
    } else {
        frame = buffer.front();
        buffer.pop();
    }
    m.unlock();
    return frame;
}

float Video2frame::getPTS() {
    m.lock();
    float pts;
    if (buffer.empty()) {
        pts = -1;
    } else {
        pts = buffer.front()->pts;
    }
    m.unlock();
    return pts;
}


int Video2frame::getBufferSize() {
    m.lock();
    int size = buffer.size();
    if (is_finish && size == 0)
        size = -1;
    m.unlock();
    return size;
}

void Video2frame::run() {
    thread = new std::thread(demux_decode_thread, this);
}

int Video2frame::getPacketBufferSize() {
    m.lock();
    int size = pkt_buffer.size();
    m.unlock();
    return size;
}

