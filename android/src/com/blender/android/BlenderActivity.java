package com.blender.android;

import android.app.Activity;
import android.content.res.AssetManager;
import android.graphics.SurfaceTexture;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.FrameLayout;

import java.io.File;

public class BlenderActivity extends Activity {
    private static final String TAG = "BlenderAndroid";
    private SurfaceView mSurfaceView;
    private boolean mBlenderRunning = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mSurfaceView = new SurfaceView(this);
        mSurfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                startBlender(holder.getSurface());
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
                BlenderLib.initSurface(holder.getSurface(), w, h);
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                stopBlender();
            }
        });

        mSurfaceView.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                BlenderLib.touchEvent(
                        event.getAction(),
                        event.getX(),
                        event.getY()
                );
                return true;
            }
        });

        setContentView(mSurfaceView);
    }

    private void startBlender(Surface surface) {
        if (mBlenderRunning) return;

        String dataPath = getFilesDir().getAbsolutePath() + "/blender/";
        String tempPath = getCacheDir().getAbsolutePath();

        new Thread(() -> {
            int ret = BlenderLib.initBlender(dataPath, tempPath);
            if (ret == 0) {
                mBlenderRunning = true;
                BlenderLib.initSurface(surface,
                        mSurfaceView.getWidth(),
                        mSurfaceView.getHeight());
                Log.i(TAG, "Blender initialized successfully");
            } else {
                Log.e(TAG, "Blender init failed: " + ret);
            }
        }).start();
    }

    private void stopBlender() {
        if (mBlenderRunning) {
            BlenderLib.exitBlender();
            mBlenderRunning = false;
        }
    }

    @Override
    protected void onDestroy() {
        stopBlender();
        super.onDestroy();
    }
}
