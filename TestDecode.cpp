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
            {"rtmp://172.31.204.119:1935/live/1", width, height, AV_PIX_FMT_BGR24},
            {"rtmp://172.31.204.119:1935/live/2", width, height, AV_PIX_FMT_BGR24},
            {"rtmp://172.31.204.119:1935/live/3", width, height, AV_PIX_FMT_BGR24},
            {"rtmp://172.31.204.119:1935/live/4", width, height, AV_PIX_FMT_BGR24},
            {"rtmp://172.31.204.119:1935/live/5", width, height, AV_PIX_FMT_BGR24}

    };
    Frame *frame[5] = {NULL, NULL, NULL, NULL, NULL};
    int64_t offset[5] = {40, 160, 0, 40, 360};

    bool flag = false;
    float pts = 0;
    int64_t raw_pts = 0;

    for (int i = 0; i < 5; ++i) {
        video[i].run();
    }
    for (int i = 0; i < 5; ++i) {
        cout << video[i].getBufferSize() << " ";
    }
    cout << endl;
    for (int i = 0; i < 5; ++i) {
        while ((frame[i] = video[i].getFrame()) == NULL);
        pts = std::max(pts, frame[i]->pts);
        raw_pts = std::max(raw_pts, frame[i]->raw_pts + offset[i]);
    }
    for (int i = 0; i < 5; ++i) {
        cout << frame[i]->raw_pts << " ";
    }
    cout << endl;


    for (int i = 0; i < 5; ++i) {
        cout << video[i].getBufferSize() << " ";
    }
    cout << endl;

    while (!flag) {
        flag = true;
        for (int i = 0; i < 5; ++i) {
            if (raw_pts - frame[i]->raw_pts - offset[i] > 20) {
                cout << "delete " << i + 1 << " " << pts << " " << frame[i]->pts << " " << pts - frame[i]->pts << " "
                     << frame[i]->raw_pts << endl;
                delete frame[i];
                while ((frame[i] = video[i].getFrame()) == NULL);
                pts = std::max(pts, frame[i]->pts);
                raw_pts = std::max(raw_pts, frame[i]->raw_pts + offset[i]);
            }

            if (raw_pts - frame[i]->raw_pts - offset[i] > 20) {
                flag = false;
            }
        }

        for (int i = 0; i < 5; ++i) {
            cout << frame[i]->raw_pts << " ";
        }
        cout << endl;
        for (int i = 0; i < 5; ++i) {
            cout << video[i].getBufferSize() << " ";
        }
        cout << endl;
    }

    int cnt = 0;
    flag = true;
    auto start_all = chrono::high_resolution_clock::now();
    float pts_begin = frame[0]->pts;
    int remain;

    ImageSender sender;
    sender.open("/home/zhd/CLionProjects/pipe_transmission/pipe_dir/pipe1");
//    int fd = open("/dev/null", O_WRONLY);
//    fcntl(fd, F_SETPIPE_SZ, 1048576);
    while (true) {
        if (cnt % 25 == 0) {
            auto end_all = chrono::high_resolution_clock::now();

            cout << chrono::duration<double, milli>(end_all - start_all).count() / 1000 << " "
                 << frame[0]->pts - pts_begin << " "
                 << (frame[0]->pts - pts_begin) / chrono::duration<double, milli>(end_all - start_all).count() * 1000
                 << endl;
            for (int i = 0; i < 5; ++i) {
                printf("\t%d %d %3d %3d\n",
                       i, frame[i]->raw_pts + offset[i], video[i].getBufferSize(), video[i].getPacketBufferSize());
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
        if ((raw_pts / 40) % 8 == 0)
            sender.sendData(data);
        delete[] data->imgs;
        delete[] data->w;
        delete[] data->h;
        delete data;

        pts = 0;
        for (int i = 0; i < 5; ++i) {
            while ((frame[i] = video[i].getFrame()) == NULL);
            raw_pts = std::max(raw_pts, frame[i]->raw_pts + offset[i]);
            pts = std::max(pts, frame[i]->pts);
        }
        bool sync = true;
        for (int i = 0; i < 5; ++i) {
            if (raw_pts - frame[i]->raw_pts - offset[i] > 20)
                sync = false;
        }
        if (!sync) {
            cout << "not sync" << endl;
            cout << pts << endl;
            for (int i = 0; i < 5; ++i) {
                printf("%d %d\n", i, frame[i]->raw_pts);
            }
//            return 0;
        }
        ++cnt;
    }
    cout << "return" << endl;
    return 0;
}
