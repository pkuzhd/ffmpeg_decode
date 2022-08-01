//
// Created by zhanghaodan on 2022/7/15.
//

#include <iostream>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <chrono>

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
#include "utils/ImageSender.h"

//#define F_LINUX_SPECIFIC_BASE 1024
//#define F_SETPIPE_SZ (F_LINUX_SPECIFIC_BASE + 7)
//#define F_GETPIPE_SZ (F_LINUX_SPECIFIC_BASE + 8)
using std::cout;
using std::endl;
using namespace std;

int main() {
    int width = 1920;
    int height = 1080;
    Video2frame video[5] = {
            {"../data/1.mp4", width, height, AV_PIX_FMT_BGR24},
            {"../data/2.mp4", width, height, AV_PIX_FMT_BGR24},
            {"../data/3.mp4", width, height, AV_PIX_FMT_BGR24},
            {"../data/4.mp4", width, height, AV_PIX_FMT_BGR24},
            {"../data/5.mp4", width, height, AV_PIX_FMT_BGR24}

    };
    Frame *frame[5] = {NULL, NULL, NULL, NULL, NULL};

    bool flag = false;
    float pts = 0;

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
                cout << "delete" << endl;
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
    auto start_all = chrono::high_resolution_clock::now();
    float pts_begin = frame[0]->pts;
    int remain;

    ImageSender sender;
    sender.open("../pipe_transmission/pipe_dir/pipe1");
//    int fd = open("/dev/null", O_WRONLY);
//    fcntl(fd, F_SETPIPE_SZ, 1048576);
    while (true) {
        if (cnt % 50 == 0) {
            auto end_all = chrono::high_resolution_clock::now();

            cout << chrono::duration<double, milli>(end_all - start_all).count() / 1000 << " "
                 << frame[0]->pts - pts_begin << " "
                 << (frame[0]->pts - pts_begin) / chrono::duration<double, milli>(end_all - start_all).count() * 1000
                 << endl;
            for (int i = 0; i < 5; ++i) {
                printf("\t%d %.3f %3d %3d\n",
                       i, frame[i]->pts, video[i].getBufferSize(), video[i].getPacketBufferSize());
            }
        }

        ImageData *data = new ImageData;

        data->n = 5;
        data->w = new int[data->n];
        data->h = new int[data->n];
        for (int i = 0; i < 5; ++i) {
            data->h[i] = height;
            data->w[i] = width;
        }
        data->imgs = new char[data->n * data->w[0] * data->h[0] * 3];
        for (int i = 0; i < 5; ++i) {
            memcpy(data->imgs + i * height * width * 3, frame[i]->data, height * width * 3);
            delete frame[i];
        }
        sender.sendData(data);
        delete[] data->imgs;
        delete[] data->w;
        delete[] data->h;
        delete data;
//        remain = sizeof(float);
//        while (remain > 0) {
//            int r = write(fd, (char *) (&frame[0]->pts) + (sizeof(float) - remain), remain);
//            if (r < 0) {
//                return 0;
//            }
//            remain -= r;
//        }
//
//        for (int i = 0; i < 5; ++i) {
//            remain = frame[i]->size;
//
//            while (remain > 0) {
//                int r = write(fd, frame[i]->data + (frame[i]->size - remain), remain);
//                if (r < 0) {
//                    return 0;
//                }
//                remain -= r;
//            }
//            delete frame[i];
//
//        }
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
            cout << pts << endl;
            for (int i = 0; i < 5; ++i) {
                printf("%d %.3f\n", i, frame[i]->pts);
            }
            return 0;
        }
        ++cnt;
    }
    return 0;
}
