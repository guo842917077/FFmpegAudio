//
// Created by Apple on 2020-02-10.
//

#ifndef FFMPEGAUDIO_JAVA_CALL_HELPER_H
#define FFMPEGAUDIO_JAVA_CALL_HELPER_H

#include <jni.h>

/**
 * 反射 Java 方法
 */
class JavaCallHelper {
public:
    JavaCallHelper(JavaVM *_javaVM, JNIEnv *_env, jobject &_jobj);

    ~JavaCallHelper();

    void onError(int thread, int code);

    void onPrepare(int thread);

    void onProgress(int hread, int progress);

private:
    JavaVM *javaVM;
    JNIEnv *env;
    jobject jobj;
    jmethodID jm_prepare;
    jmethodID jm_error;
    jmethodID jm_progress;


};


#endif //FFMPEGAUDIO_JAVA_CALL_HELPER_H
