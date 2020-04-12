//
// Created by Apple on 2020-02-10.
//

#include "java_call_helper.h"
#include "macro.h"

JavaCallHelper::JavaCallHelper(JavaVM *_javaVM, JNIEnv *_env, jobject &_jobj) : javaVM(_javaVM),
                                                                                env(_env) {
    // 将 obj 变成一个全局的对象,因为 jobj目前是一个函数参数 不是全局变量
    jobj = env->NewGlobalRef(_jobj);
    jclass jclazz = env->GetObjectClass(jobj);
    // 反射得到方法的引用  实际上就是一个 ArtMethod 结构体
    jm_error = env->GetMethodID(jclazz, "onError", "(I)V");
    jm_prepare = env->GetMethodID(jclazz, "onPrepare", "()V");
    jm_progress = env->GetMethodID(jclazz, "onProgress", "(I)V");
}

JavaCallHelper::~JavaCallHelper() {

}

void JavaCallHelper::onError(int thread, int code) {
    if (thread == THREAD_CHILD) {
        // 如果是在子线程 需要先绑定 JVM
        JNIEnv *jniEnv;
        // 将绑定一个jniEnv 到 javaVM 上
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            return;
        }
        jniEnv->CallVoidMethod(jobj, jm_error, code);
        // 解绑线程
        javaVM->DetachCurrentThread();
    } else {
        // 如果是在主线程 可以直接回调 Java 层的函数
        env->CallVoidMethod(jobj, jm_error, code);
    }
}

void JavaCallHelper::onPrepare(int thread) {
    if (thread == THREAD_CHILD) {
        // 如果是在子线程 需要先绑定 JVM
        JNIEnv *jniEnv;
        // 将绑定一个jniEnv 到 javaVM 上
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            return;
        }
        jniEnv->CallVoidMethod(jobj, jm_prepare);
        // 解绑线程
        javaVM->DetachCurrentThread();
    } else {
        // 如果是在主线程 可以直接回调 Java 层的函数
        env->CallVoidMethod(jobj, jm_prepare);
    }
}

void JavaCallHelper::onProgress(int thread, int progress) {
    if (thread == THREAD_CHILD) {
        // 如果是在子线程 需要先绑定 JVM
        JNIEnv *jniEnv;
        // 将绑定一个jniEnv 到 javaVM 上
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            return;
        }
        jniEnv->CallVoidMethod(jobj, jm_progress, progress);
        // 解绑线程
        javaVM->DetachCurrentThread();
    } else {
        // 如果是在主线程 可以直接回调 Java 层的函数
        env->CallVoidMethod(jobj, jm_progress, progress);
    }
}




