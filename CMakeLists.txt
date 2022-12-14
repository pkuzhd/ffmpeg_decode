cmake_minimum_required(VERSION 3.20)
project(ffmpeg_test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

include_directories(/home/zhd/.local/include)
link_directories(/home/zhd/.local/lib)



include_directories(../pipe_transmission)
add_subdirectory(../pipe_transmission pipe_transmission)

find_library(opencv 3)
set(OPENCV_COMPONENTS
        opencv_core
        opencv_imgproc
        opencv_imgcodecs
        opencv_highgui
        opencv_objdetect
        opencv_video)

# add_executable(ffmpeg_test
#         main.cpp
#         Video2frame.cpp Video2frame.h
#         )

add_library(VideoLib
        source/Muxer.cpp source/Muxer.h
        source/Encoder.cpp source/Encoder.h
        Video2frame.cpp Video2frame.h
        Frame2video.cpp Frame2video.h
        )
target_link_libraries(
        VideoLib
        avutil
        avformat
        avcodec
        swscale
)

add_executable(TestDecode
        TestDecode.cpp
        )
add_executable(pull
        pull.cpp
        )

add_executable(filtering_video
        filtering_video.c
        )
add_executable(scaling_video
        scaling_video.c
        )
add_executable(transcoding
        transcoding.c
        )
add_executable(my_transcoding
        my_transcoding.c
        )

add_executable(test
        test.cpp
        )

add_executable(push
        push.cpp
        )

add_executable(show_frame_info
        show_frame_info.cpp
        )

# target_link_libraries(ffmpeg_test
#         avutil
#         avformat
#         avcodec
#         swscale
#         SDL2
#         ${OPENCV_COMPONENTS}
#         )
target_link_libraries(TestDecode
        avutil
        avformat
        avcodec
        swscale
        TransLib
        VideoLib
        pthread
        )
target_link_libraries(push
        avutil
        avformat
        avcodec
        swscale
        TransLib
        VideoLib
        pthread
        )
target_link_libraries(show_frame_info
        avutil
        avformat
        avcodec
        swscale
        SDL2
        ${OPENCV_COMPONENTS}
        VideoLib
        )
target_link_libraries(filtering_video
        avfilter
        avutil
        avformat
        avcodec
        swscale
        SDL2
        ${OPENCV_COMPONENTS}
        )
target_link_libraries(scaling_video
        avfilter
        avutil
        avformat
        avcodec
        swscale
        SDL2
        ${OPENCV_COMPONENTS}
        )
target_link_libraries(transcoding
        avfilter
        avutil
        avformat
        avcodec
        swscale
        SDL2
        ${OPENCV_COMPONENTS}
        )
target_link_libraries(my_transcoding
        avfilter
        avutil
        avformat
        avcodec
        swscale
        SDL2
        ${OPENCV_COMPONENTS}
        )
target_link_libraries(pull
        avfilter
        avutil
        avformat
        avcodec
        swscale
        VideoLib
        )

add_executable(demux demux.cpp)
target_link_libraries(demux
        avutil
        avformat
        avcodec
        swscale
        ${OPENCV_COMPONENTS}
        )
#add_executable(rtmp_push rtmp_push.cpp)
#target_link_libraries(demux
#        avutil
#        avformat
#        avcodec
#        swscale
#        )
add_executable(decode decode_video.cpp)
target_link_libraries(decode
        avutil
        avformat
        avcodec
        swscale
        )
add_executable(encode encode_video.cpp)
target_link_libraries(encode
        avutil
        avformat
        avcodec
        swscale
        )