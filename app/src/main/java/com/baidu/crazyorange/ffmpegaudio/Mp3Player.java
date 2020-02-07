package com.baidu.crazyorange.ffmpegaudio;

public class Mp3Player {
    static {
        System.loadLibrary("player");
    }

    /**
     *
     * @param input mp3 path
     * @param output pcm path
     */
    public native void sound(String input,String output);
}
