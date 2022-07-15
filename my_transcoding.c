#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>

#ifdef __cplusplus
};
#endif

int main() {
    int stream_idx;
    int video_stream_idx;
    int ret;

    // input
    char *input = "http://172.31.203.194:8765/hls/1.m3u8";
    AVFormatContext *input_ctx = NULL, *output_ctx = NULL;
    ret = avformat_open_input(&input_ctx, input, NULL, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
    ret = avformat_find_stream_info(input_ctx, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }
    for (int i = 0; i < input_ctx->nb_streams; ++i) {
        AVStream *stream = input_ctx->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
            break;
        }
    }
    AVStream *stream = input_ctx->streams[video_stream_idx];
    AVCodec *dec = avcodec_find_decoder(stream->codecpar->codec_id);
    AVCodecContext *dec_ctx;
    dec_ctx = avcodec_alloc_context3(dec);
    ret = avcodec_parameters_to_context(dec_ctx, stream->codecpar);
    dec_ctx->framerate = av_guess_frame_rate(input_ctx, stream, NULL);
    ret = avcodec_open2(dec_ctx, dec, NULL);

    // output init
    char *output = "rtmp://172.31.203.194:1935/hls/1.4";
    avformat_alloc_output_context2(&output_ctx, NULL, "flv", output);
    AVStream *out_stream = avformat_new_stream(output_ctx, NULL);
    AVCodec *encoder = avcodec_find_encoder_by_name("libx264");
    AVCodecContext *enc_ctx = avcodec_alloc_context3(encoder);
    enc_ctx->height = dec_ctx->height;
    enc_ctx->width = dec_ctx->width;
    enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;
    if (encoder->pix_fmts)
        enc_ctx->pix_fmt = encoder->pix_fmts[0];
    else
        enc_ctx->pix_fmt = dec_ctx->pix_fmt;
    enc_ctx->time_base = av_inv_q(dec_ctx->framerate);
    if (output_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    ret = avcodec_open2(enc_ctx, encoder, NULL);
    ret = avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);
    out_stream->time_base = enc_ctx->time_base;
    av_dump_format(output_ctx, 0, output, 1);
    ret = avio_open(&output_ctx->pb, output, AVIO_FLAG_WRITE);
    ret = avformat_write_header(output_ctx, NULL);

    AVFrame *dec_frame = av_frame_alloc();
    AVPacket *in_pkt = av_packet_alloc();
    AVPacket *enc_pkt = av_packet_alloc();
    AVFrame *enc_frame = av_frame_alloc();

    enc_frame->format = AV_PIX_FMT_YUV420P;
    enc_frame->width = enc_ctx->width;
    enc_frame->height = enc_ctx->height;
    uint8_t *buffer = (uint8_t *) av_malloc(enc_frame->width * enc_frame->height * 3 / 2);
    av_image_fill_arrays(enc_frame->data,
                         enc_frame->linesize,
                         buffer,
                         enc_frame->format,
                         enc_frame->width,
                         enc_frame->height,
                         1);
    while (1) {
        if ((ret = av_read_frame(input_ctx, in_pkt)) < 0)
            break;

        if (in_pkt->stream_index == video_stream_idx) {
            av_packet_rescale_ts(in_pkt,
                                 input_ctx->streams[video_stream_idx]->time_base,
                                 dec_ctx->time_base);
            ret = avcodec_send_packet(dec_ctx, in_pkt);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Decoding failed\n");
                break;
            }

            while (ret >= 0) {
                ret = avcodec_receive_frame(dec_ctx, dec_frame);
                if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                    break;
                else if (ret < 0)
                    return 1;


                memcpy(enc_frame->data[0], dec_frame->data[0], enc_frame->width * enc_frame->height);
                memcpy(enc_frame->data[1], dec_frame->data[1], enc_frame->width * enc_frame->height / 4);
                memcpy(enc_frame->data[2], dec_frame->data[2], enc_frame->width * enc_frame->height / 4);
                enc_frame->pts = dec_frame->pts;

                ret = avcodec_send_frame(enc_ctx, enc_frame);
                if (ret < 0)
                    break;
                while (ret >= 0) {
                    ret = avcodec_receive_packet(enc_ctx, enc_pkt);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                        break;
                    enc_pkt->stream_index = video_stream_idx;

                    av_packet_rescale_ts(enc_pkt,
                                         enc_ctx->time_base,
                                         output_ctx->streams[video_stream_idx]->time_base);
                    ret = av_interleaved_write_frame(output_ctx, enc_pkt);
                }
                av_frame_unref(dec_frame);
            }
        }

        av_packet_unref(in_pkt);
    }
    return 0;
}