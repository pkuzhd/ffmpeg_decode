//
// Created by zhd on 2022/4/1.
//

#include "Encoder.h"

#include <utility>

namespace net_video {

    Encoder::Encoder(std::string format)
            : format(std::move(format)), context(nullptr) {
        AVCodec *codec = avcodec_find_encoder_by_name(this->format.c_str());

        if (codec == nullptr)
            throw std::exception();

        context = avcodec_alloc_context3(codec);
        if (context == nullptr)
            throw std::exception();
    }

    Encoder::~Encoder() {
        if (context)
            avcodec_free_context(&context);
    }

    Status Encoder::open() {
        int ret;
        ret = avcodec_open2(context, context->codec, NULL);
        if (ret < 0)
            return ERROR;

        return Status::OK;
    }

    Status Encoder::close() {

        return Status::OK;
    }

    Status Encoder::setOption(const std::string &key, const std::string &value) {
        if (!context)
            return ERROR;

        int ret = av_opt_set(context, key.c_str(), value.c_str(), 0);
        if (ret < 0)
            return ERROR;

        return OK;
    }
}