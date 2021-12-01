package io.agora.advancedvideo.rawdata;

import android.app.Activity;
import android.graphics.Bitmap;
import android.util.Log;

public class SaCert2Example extends Activity implements MediaDataVideoObserver {

    MediaDataObserverPlugin mediaDataObserverPlugin;
    String SA_TAG = "SA_Cert_2";

    public SaCert2Example()
    {
        mediaDataObserverPlugin = MediaDataObserverPlugin.the();
        MediaPreProcessing.setCallback(mediaDataObserverPlugin);
        MediaPreProcessing.setVideoCaptureByteBuffer(mediaDataObserverPlugin.byteBufferCapture);
        mediaDataObserverPlugin.addVideoObserver(this);
    }

    @Override
    public void onCaptureVideoFrame(byte[] data, int frameType, int width, int height, int bufferLength, int yStride, int uStride, int vStride, int rotation, long renderTimeMs) {
        Log.i(SA_TAG, "captured video frameType: " + frameType);
        Log.i(SA_TAG, "captured video width: " + width);
        Log.i(SA_TAG, "captured video height: " + height);
        Log.i(SA_TAG, "captured video buffer length: " + bufferLength);
        Log.i(SA_TAG, "captured video yStride: " + yStride);
        Log.i(SA_TAG, "captured video uStride: " + uStride);
        Log.i(SA_TAG, "captured video vStride: " + vStride);
        Log.i(SA_TAG, "captured video rotation: " + rotation);
        Log.i(SA_TAG, "captured video rednerTime: " + renderTimeMs);

        Bitmap bitmap = YUVUtils.i420ToBitmap(width, height, rotation, bufferLength, data, yStride, uStride, vStride);
        Bitmap bmp = YUVUtils.blur(getApplicationContext(), bitmap, 4);
        System.arraycopy(YUVUtils.bitmapToI420(width, height, bmp), 0, data, 0, bufferLength);
    }

    @Override
    public void onRenderVideoFrame(int uid, byte[] data, int frameType, int width, int height, int bufferLength, int yStride, int uStride, int vStride, int rotation, long renderTimeMs) {
        Log.i(SA_TAG, "Render video frameType: " + frameType);
        Log.i(SA_TAG, "Render  video width: " + width);
        Log.i(SA_TAG, "Render video height: " + height);
        Log.i(SA_TAG, "Render video buffer length: " + bufferLength);
        Log.i(SA_TAG, "Render video yStride: " + yStride);
        Log.i(SA_TAG, "Render video uStride: " + uStride);
        Log.i(SA_TAG, "Render video vStride: " + vStride);
        Log.i(SA_TAG, "Render video rotation: " + rotation);
        Log.i(SA_TAG, "Render video rednerTime: " + renderTimeMs);

        Bitmap bmp = YUVUtils.blur(getApplicationContext(), YUVUtils.i420ToBitmap(width, height, rotation, bufferLength, data, yStride, uStride, vStride), 4);
    }

    @Override
    public void onPreEncodeVideoFrame(byte[] data, int frameType, int width, int height, int bufferLength, int yStride, int uStride, int vStride, int rotation, long renderTimeMs) {
        Log.i(SA_TAG, "Pre encoded video frameType: " + frameType);
        Log.i(SA_TAG, "Pre encoded video width: " + width);
        Log.i(SA_TAG, "Pre encoded video height: " + height);
        Log.i(SA_TAG, "Pre encoded video buffer length: " + bufferLength);
        Log.i(SA_TAG, "Pre encoded video yStride: " + yStride);
        Log.i(SA_TAG, "Pre encoded video uStride: " + uStride);
        Log.i(SA_TAG, "Pre encoded video vStride: " + vStride);
        Log.i(SA_TAG, "Pre encoded video rotation: " + rotation);
        Log.i(SA_TAG, "Pre encoded video rednerTime: " + renderTimeMs);
    }
}
