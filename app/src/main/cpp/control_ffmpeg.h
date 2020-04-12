//
// Created by Apple on 2020-02-10.
//

#ifndef FFMPEGAUDIO_CONTROL_FFMPEG_H
#define FFMPEGAUDIO_CONTROL_FFMPEG_H

#include <pthread.h>
#include <android/native_window.h>
#include "java_call_helper.h"
#include "VideoChannel.h"
#include "audio_channel.h"
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/time.h>
};

/**
 * 控制层
 */
class ControlFFmpeg {
public:
    ControlFFmpeg(JavaCallHelper *pHelper, const char *sourceData);

    ~ControlFFmpeg();

    void prepare();

    void prepareFFmpeg();

private:
    pthread_t mThread;
    AVFormatContext *mFormatContext;
    char *url;
    JavaCallHelper *javaCallHelper;

};


#endif //FFMPEGAUDIO_CONTROL_FFMPEG_H
