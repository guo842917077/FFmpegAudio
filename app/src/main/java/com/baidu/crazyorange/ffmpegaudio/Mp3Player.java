package com.baidu.crazyorange.ffmpegaudio;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class Mp3Player implements SurfaceHolder.Callback {
    static {
        System.loadLibrary("player");
    }


    private SurfaceHolder mSurfaceHolder;
    private String mDataSource;

    private OnProgressListener mProgressListener;
    private OnErrorListener mErrorListener;
    private OnPrepareListener mPrepareListener;

    public void setSurfaceHolder(SurfaceView surfaceView) {
        if (null != this.mSurfaceHolder) {
            this.mSurfaceHolder.removeCallback(this);
        }
        this.mSurfaceHolder = surfaceView.getHolder();
        this.mSurfaceHolder.addCallback(this);
    }

    /**
     * @param input  mp3 path
     * @param output pcm path
     */
    public native void sound(String input, String output);


    private native void native_prepare(String dataSource);

    private native void native_start();

    private native void native_set_surface(Surface surface);

    public native void play(String input);


    public void start(){
        native_start();
    }

    public void onPrepare() {
        if (null != mPrepareListener) {
            this.mPrepareListener.onPrepare();
        }
    }

    public void onProgress(int progress) {
        if (null != mProgressListener) {
            this.mProgressListener.onProgress(progress);
        }
    }

    public void onError(int errorCode) {
        if (null != mErrorListener) {
            this.mErrorListener.onError(errorCode);
        }
    }


    public interface OnPrepareListener {
        void onPrepare();
    }

    public interface OnProgressListener {
        void onProgress(int progress);
    }

    public interface OnErrorListener {
        void onError(int errorCode);
    }

    public void setProgressListener(OnProgressListener listener) {
        this.mProgressListener = listener;
    }

    public void setPrepareListener(OnPrepareListener listener) {
        this.mPrepareListener = listener;
    }

    public void setErrorListener(OnErrorListener listener) {
        this.mErrorListener = listener;
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        native_set_surface(holder.getSurface());
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    public void prepare() {
        native_prepare(mDataSource);
    }

    public void setDataSource(String path) {
        this.mDataSource = path;
    }
}
