#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/native_window_jni.h>
#include "control_ffmpeg.h"
#include "java_call_helper.h"

#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"wangyi",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"wangyi",FORMAT,##__VA_ARGS__);

#define MAX_AUDIO_FRME_SIZE 48000 * 4
extern "C" {
//封装格式
#include "libavformat/avformat.h"
//解码
#include "libavcodec/avcodec.h"
//缩放
#include "libswscale/swscale.h"
//重采样 在音频中需要对音频流重采样，有的是用的 44100 进行采样的，在播放时需要重采样
#include "libswresample/swresample.h"
};

extern "C"
JNIEXPORT void JNICALL
Java_com_baidu_crazyorange_ffmpegaudio_Mp3Player_sound(JNIEnv *env, jobject instance,
                                                       jstring input_,
                                                       jstring output_) {
    const char *input = env->GetStringUTFChars(input_, 0);
    const char *output = env->GetStringUTFChars(output_, 0);
    avformat_network_init();
    // step 1: init context
    AVFormatContext *formatContext = avformat_alloc_context();
    // step 2: open audio file
    if (avformat_open_input(&formatContext, input, NULL, NULL) != 0) {
        LOGI("%s", "can not open audio file");
        return;
    }
    // step 3：get audio info
    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        LOGI("$s", "can not find audio info");
    }
    // step 4:get audio stream index  获取音频流的索引
    int audio_stream_idx = -1;
    for (int i = 0; i < formatContext->nb_streams; ++i) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_idx = i;
            break;
        }
    }
    // step 5: init codec and decoder
    AVCodecParameters *codecpar = formatContext->streams[audio_stream_idx]->codecpar;
    // init decoder
    AVCodec *coder = avcodec_find_decoder(codecpar->codec_id);
    // step 6: init decoder context
    AVCodecContext *codecContext = avcodec_alloc_context3(coder);
    // step 7: copy codec parameters to context : 将之前获得的解码器参数复制到解码器上下文中
    avcodec_parameters_to_context(codecContext, codecpar);
    // step 8:
    // 声明一个转换器上下文 以及配置
    SwrContext *swrContext = swr_alloc();
    // amount of input parameter
    // 获取到输入的参数，每个输入的音频这些参数可能都不同，所以需要动态获取
    // 输入格式
    AVSampleFormat in_sample = codecContext->sample_fmt;
    // input sampling rate
    int in_sample_rate = codecContext->sample_rate;
    // input channel
    uint64_t in_ch_layout = codecContext->channel_layout;
    /**
     * 输出的参数
     */
    AVSampleFormat out_sample = AV_SAMPLE_FMT_S16;
    int out_sample_rate = 44100;
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;
    // 给转换上下文设置这些参数
    swr_alloc_set_opts(swrContext, out_ch_layout, out_sample, out_sample_rate, in_ch_layout,
                       in_sample, in_sample_rate, 0, NULL);
    // 设置好之后进行初始化
    swr_init(swrContext);

    // 定义一个缓冲区  channel count * sampling rate
    uint8_t *out_buffer = static_cast<uint8_t *>(av_malloc(2 * 44100));
    FILE *fp_pcm = fopen(output, "wb");

    // read audio packet
    AVPacket *packet = av_packet_alloc();

    int count = 0;
    while (av_read_frame(formatContext, packet) >= 0) {
        // 将解码好的 packet 取出来,packet 是压缩好的数据
        avcodec_send_packet(codecContext, packet);
        AVFrame *frame = av_frame_alloc();
        // 解码 packet 为 frame
        int ret = avcodec_receive_frame(codecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret < 0) {
            LOGE("decoder finish");
            break;
        }
        // If this frame is not a audio frame,drop it .
        if (packet->stream_index != audio_stream_idx) {
            continue;
        }
        // frame 数据放到创建的缓冲区中
        swr_convert(swrContext, &out_buffer, 2 * 44100, (const uint8_t **) frame->data,
                    frame->nb_samples);
        // 输出内存到文件中
        int channel_count = av_get_channel_layout_nb_channels(out_ch_layout);
        // 音频数据对齐
        int out_buffer_size = av_samples_get_buffer_size(NULL, channel_count, frame->nb_samples,
                                                         out_sample, 1);
        // 1 代表写入的每个元素的大小 这里代表 1 个字节
        fwrite(out_buffer, 1, out_buffer_size, fp_pcm);
    };

    fclose(fp_pcm);
    av_free(out_buffer);
    swr_free(&swrContext);
    avcodec_close(codecContext);
    avformat_close_input(&formatContext);
    env->ReleaseStringUTFChars(input_, input);
    env->ReleaseStringUTFChars(output_, output);
}


ANativeWindow *window = 0;
ControlFFmpeg *controlFFmpeg;
JavaCallHelper *javaCallHelper;

// 在子线程如果想要调用 java 函数，就必须先将 native 的线程绑定到 jvm 引擎的实例上
JavaVM *javaVM = NULL;
/**
 * 这个函数会被主动调用
 *
 * 这里是用来获取 JavaVM 对象
 * @param vm
 * @param reserved
 * @return
 */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    javaVM = vm;
    return JNI_VERSION_1_4;
}

/**
 * 音视频同步，加入了控制层的概念
 * OpenSLES 播放喇叭的库
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_baidu_crazyorange_ffmpegaudio_Mp3Player_play(JNIEnv *env, jobject instance,
                                                      jstring input_) {
    const char *input = env->GetStringUTFChars(input_, 0);

    // 播放音频

    env->ReleaseStringUTFChars(input_, input);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_baidu_crazyorange_ffmpegaudio_Mp3Player_native_1prepare(JNIEnv *env, jobject instance,
                                                                 jstring dataSource_) {
    const char *dataSource = env->GetStringUTFChars(dataSource_, 0);
    javaCallHelper = new JavaCallHelper(javaVM, env, instance);
    controlFFmpeg = new ControlFFmpeg(javaCallHelper, dataSource);
    controlFFmpeg->prepare();

    env->ReleaseStringUTFChars(dataSource_, dataSource);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_baidu_crazyorange_ffmpegaudio_Mp3Player_native_1start(JNIEnv *env, jobject instance) {

    // TODO

}
extern "C"
JNIEXPORT void JNICALL
Java_com_baidu_crazyorange_ffmpegaudio_Mp3Player_native_1set_1surface(JNIEnv *env, jobject instance,
                                                                      jobject surface) {

    // TODO
    window = ANativeWindow_fromSurface(env, surface);

}