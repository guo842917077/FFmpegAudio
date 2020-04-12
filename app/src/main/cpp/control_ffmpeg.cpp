//
// Created by Apple on 2020-02-10.
//

#include "control_ffmpeg.h"
#include "macro.h"

void *callbackFFmpeg(void *args) {
    ControlFFmpeg *controlFFmpeg = static_cast<ControlFFmpeg *>(args);
    controlFFmpeg->prepareFFmpeg();
}


ControlFFmpeg::ControlFFmpeg(JavaCallHelper *pHelper, const char *sourceData) {
    /**
     * 将路径拷贝到 url 数组中
     */
    url = new char[strlen(sourceData) + 1];
    strcpy(url, sourceData);
    this->javaCallHelper = pHelper;
}

ControlFFmpeg::~ControlFFmpeg() {

}

void ControlFFmpeg::prepare() {
    /**
     * 线程 ID
     * prepareFFmpeg 是一个函数
     */
    pthread_create(&mThread, NULL, callbackFFmpeg, NULL);
}

void ControlFFmpeg::prepareFFmpeg() {
    /**
     * 子线程中调用了 prepareFFmpeg 函数
     */
    avformat_network_init();
    mFormatContext = avformat_alloc_context();
    AVDictionary *opts = NULL;
    av_dict_set(&opts, "timeout", "30000000", 0);
    int ret = avformat_open_input(&mFormatContext, url, NULL, &opts);
    // 如果打开失败
    if (ret != 0) {
        javaCallHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_URL);
        return;
    }

    // 查找流
    if (avformat_find_stream_info(mFormatContext, NULL) < 0) {
        if (javaCallHelper) {
            javaCallHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_FIND_STREAMS);
        }
        return;
    }
    // 遍历流
    for (int i = 0; i < mFormatContext->nb_streams; ++i) {
        AVCodecParameters *codecpar = mFormatContext->streams[i]->codecpar;
        AVCodec *dec = avcodec_find_decoder(codecpar->codec_id);
        if (!dec) {
            if (javaCallHelper) {
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_FIND_DECODER_FAIL);
            }
            return;
        }
        //创建上下文
        AVCodecContext *codecContext = avcodec_alloc_context3(dec);
        if (!codecContext) {
            if (javaCallHelper)
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            return;
        }
        //复制参数到解码器
        if (avcodec_parameters_to_context(codecContext, codecpar) < 0) {
            if (javaCallHelper)
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            return;
        }
        //打开解码器
        if (avcodec_open2(codecContext, dec, 0) != 0) {
            if (javaCallHelper)
                javaCallHelper->onError(THREAD_CHILD, FFMPEG_OPEN_DECODER_FAIL);
            return;
        }
        // 音频流处理
        if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {

        } else if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {

        }
    }



    //
}
