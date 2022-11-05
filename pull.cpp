#include <iostream>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>

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

#define F_LINUX_SPECIFIC_BASE 1024
#define F_SETPIPE_SZ (F_LINUX_SPECIFIC_BASE + 7)
#define F_GETPIPE_SZ (F_LINUX_SPECIFIC_BASE + 8)
using std::cout;
using std::endl;

int main() {
    const int num_stream = 1;
    char video_url[] = "/home/zhd/010.2.mp4";
    int width = 1920;
    int height = 1080;
    Video2frame video[5] = {
            {"/home/zhd/webcam/5.mp4", width, height, AV_PIX_FMT_BGR24},
            {"/home/zhd/webcam/5.ts",             width, height, AV_PIX_FMT_BGR24},
            {"/home/zhd/webcam/5.ts",             width, height, AV_PIX_FMT_BGR24},
            {"/home/zhd/webcam/5.ts",             width, height, AV_PIX_FMT_BGR24},
            {"/home/zhd/webcam/5.ts",             width, height, AV_PIX_FMT_BGR24}

    };
    Frame *frame[5] = {NULL, NULL, NULL, NULL, NULL};

    bool flag = false;
    float pts = 0;

    for (int i = 0; i < num_stream; ++i) {
        video[i].run();
    }

    while (1) {
        for (int i = 0; i < num_stream; ++i) {
            while ((frame[i] = video[i].getFrame()) == nullptr);
            if (frame[i]) {
                printf("%d %ld %f\n", i, frame[i]->raw_pts, frame[i]->pts);
            } else
                cout << i << " null" << endl;
        }
    }

    return 0;

    for (int i = 0; i < num_stream; ++i) {
        while ((frame[i] = video[i].getFrame()) == NULL);
        pts = std::max(pts, frame[i]->pts);
    }

    while (!flag) {
        flag = true;
        for (int i = 0; i < num_stream; ++i) {
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

//    int fd = open("/home/pku/pipe_test/using_pipe", O_WRONLY);
//    int fd = open("/home/zhd/pipe", O_WRONLY);
    int fd = open("/dev/null", O_WRONLY);
    fcntl(fd, F_SETPIPE_SZ, 1048576);
    while (true) {
        if (cnt % 50 == 0) {
            end = time(NULL);
            cout << end - begin << " " << frame[0]->pts - pts_begin << " "
                 << (frame[0]->pts - pts_begin) / (end - begin) << endl;
            for (int i = 0; i < num_stream; ++i) {
                printf("\t%d %.3f %3d %3d\n",
                       i, frame[i]->pts, video[i].getBufferSize(), video[i].getPacketBufferSize());
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

        for (int i = 0; i < num_stream; ++i) {
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
        for (int i = 0; i < num_stream; ++i) {
            while ((frame[i] = video[i].getFrame()) == NULL);
            pts = std::max(pts, frame[i]->pts);
        }
        bool sync = true;
        for (int i = 0; i < num_stream; ++i) {
            if (pts - frame[i]->pts > 0.01)
                sync = false;
        }
        if (!sync) {
            cout << pts << endl;
            for (int i = 0; i < num_stream; ++i) {
                printf("%d %.3f\n", i, frame[i]->pts);
            }
            return 0;
        }
        ++cnt;
    }
    return 0;
}
