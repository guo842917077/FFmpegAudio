cmake_minimum_required(VERSION 3.4.1)

# 全局 source ，凡是含有 cpp 的后缀都编译进去
file(GLOB SOURCE src/main/cpp/*.cpp)
add_library( # Sets the name of the library.
        player
        SHARED
        ${SOURCE})
find_library( # Sets the name of the path variable.
        log-lib
        log)
include_directories(src/main/cpp/include)

set(my_lib_path  ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI})
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -L${my_lib_path}")
# z 库是一个压缩库  OpenSLES 是调用喇叭播放的库  都在 sdk 中的 ndk 中
target_link_libraries( # Specifies the target library.
        player
        avfilter avformat avcodec avutil swresample swscale
        ${log-lib}
        z
        OpenSLES
        android
        )