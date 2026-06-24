package com.blender.android;

import android.view.Surface;

/**
 * JNI interface to Blender 2.79b native library.
 * Loads libblender.so via System.loadLibrary().
 */
public class BlenderLib {
    static {
        System.loadLibrary("blender_jni");
    }

    /**
     * Initialize Blender with data and temp paths.
     */
    public static native int initBlender(String dataPath, String tempPath);

    /**
     * Set the rendering surface (from SurfaceView/TextureView).
     */
    public static native int initSurface(Surface surface, int width, int height);

    /**
     * Send touch event (action: 0=down, 1=up, 2=move).
     */
    public static native int touchEvent(int action, float x, float y);

    /**
     * Send key event.
     */
    public static native int keyEvent(int down, int keyCode, int metaState);

    /**
     * Exit Blender cleanly.
     */
    public static native void exitBlender();
}
