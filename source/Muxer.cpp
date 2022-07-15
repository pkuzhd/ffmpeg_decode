//
// Created by zhd on 2022/4/1.
//

#include "Muxer.h"

#include <utility>

namespace net_video {

    Muxer::Muxer(std::string url, std::string format)
            : url(std::move(url)), format(std::move(format)), context(nullptr) {
        int ret;
        const char *format_name;
        if (this->format.empty())
            format_name = nullptr;
        else
            format_name = this->format.c_str();

        ret = avformat_alloc_output_context2(&context, nullptr, format_name, this->url.c_str());
        if (ret < 0)
            throw std::exception();
    }

    Muxer::~Muxer() {
        if (context)
            avformat_free_context(context);
    }

    Status Muxer::open() {
        int ret;

        ret = avio_open(&context->pb, url.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0)
            return ERROR;

        ret = avformat_write_header(context, nullptr);
        if (ret < 0)
            return ERROR;

        return OK;
    }

    Status Muxer::close() {
        int ret = 0;

        ret = av_write_trailer(context);
        if (ret < 0)
            return Status::ERROR;

        if (context && !(context->oformat->flags & AVFMT_NOFILE)) {
            ret = avio_closep(&context->pb);
            if (ret < 0)
                return Status::ERROR;
        }

        return Status::OK;
    }

    Status Muxer::addStream(std::shared_ptr<Encoder> encoder) {
        int ret;

        AVStream *stream = avformat_new_stream(context, nullptr);
        if (stream == nullptr)
            return Status::ERROR;

        stream->time_base = encoder->context->time_base;

        ret = avcodec_parameters_from_context(stream->codecpar, encoder->context);
        if (ret < 0)
            return Status::ERROR;

        return OK;
    }

}
