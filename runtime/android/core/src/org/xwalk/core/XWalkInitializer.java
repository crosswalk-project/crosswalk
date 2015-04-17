// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.Context;
import android.os.Handler;

import org.xwalk.core.XWalkLibraryInterface.DecompressionListener;
import org.xwalk.core.XWalkLibraryInterface.InitializationListener;

/**
 * <code>XWalkInitializer</code> is a helper to initialize the Crosswalk environment as
 * {@link XWalkActivity}. But unlike {@link XWalkActivity}, <code>XWalkInitializer</code>
 * neither provide UI interactions to help the end-user to download the appropriate
 * Crosswalk library, nor prompt messages to notify the specific error during initializing.
 * Due to this limitation, <code>XWalkInitializer</code> only works for embedded mode, or
 * for shared mode if the appropriate Crosswalk library has already been installed on the
 * device.
 */
public class XWalkInitializer {
    /**
     * Interface used to initialize the Crosswalk environment
     */
    public interface XWalkInitListener {
        /**
         * Runs on the UI thread to notify initialization is started
         */
        public void onXWalkInitStarted();
        /**
         * Runs on the UI thread to notify initialization is cancelled
         */
        public void onXWalkInitCancelled();
        /**
         * Runs on the UI thread to notify initialization is completed successfully
         */
        public void onXWalkInitCompleted();
        /**
         * Runs on the UI thread to notify initialization failed
         */
        public void onXWalkInitFailed();
    }

    private static final String TAG = "XWalkInitializer";

    private static XWalkInitializer sInstance;

    /**
     * Initializes the Crosswalk environment asynchronously
     *
     * <p>This method must be invoked on the UI thread.
     *
     * @param listener The {@link XWalkInitListener} to use
     * @param context The context that initialization is to run it
     *
     * @return False if initialization is running, true otherwise
     */
    public static boolean initAsync(XWalkInitListener listener, Context context) {
        if (sInstance != null) return false;
        new XWalkInitializer(listener, context).start();
        return true;
    }

    /**
     * Attempts to cancel initialization
     *
     * @return False if initialization is not running or could not be cancelled, true otherwise
     */
    public static boolean cancelInit() {
        if (sInstance == null) return false;
        return sInstance.cancel();
    }

    private XWalkLibraryListener mLibraryListener;
    private Handler mHandler;
    private XWalkInitListener mInitListener;
    private Context mContext;

    private static class XWalkLibraryListener
            implements DecompressionListener, InitializationListener {
        XWalkInitializer mInitializer;

        XWalkLibraryListener(XWalkInitializer initializer) {
            mInitializer = initializer;
        }

        @Override
        public void onDecompressionStarted() {
            sInstance = mInitializer;
            mInitializer.mInitListener.onXWalkInitStarted();
        }

        @Override
        public void onDecompressionCancelled() {
            sInstance = null;
            mInitializer.mInitListener.onXWalkInitCancelled();
        }

        @Override
        public void onDecompressionCompleted() {
            int status = XWalkLibraryLoader.initXWalkLibrary(mInitializer.mContext);
            if (status == XWalkLibraryInterface.STATUS_MATCH) {
                XWalkLibraryLoader.startInitialization(this);
            } else {
                mInitializer.mInitListener.onXWalkInitFailed();
            }
        }

        @Override
        public void onInitializationStarted() {
            if (sInstance == null) {
                sInstance = mInitializer;
                mInitializer.mInitListener.onXWalkInitStarted();
            }
        }

        @Override
        public void onInitializationCompleted() {
            sInstance = null;
            mInitializer.mInitListener.onXWalkInitCompleted();
        }
    }

    private XWalkInitializer(XWalkInitListener listener, Context context) {
        mLibraryListener = new XWalkLibraryListener(this);
        mHandler = new Handler();
        mInitListener = listener;
        mContext = context;
        XWalkLibraryLoader.prepareToInit();
    }

    private void start() {
        if (XWalkLibraryLoader.startDecompression(mLibraryListener, mContext)) return;

        int status = XWalkLibraryLoader.initXWalkLibrary(mContext);
        if (status == XWalkLibraryInterface.STATUS_MATCH) {
            XWalkLibraryLoader.startInitialization(mLibraryListener);
        } else {
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    mInitListener.onXWalkInitFailed();
                }
            });
        }
    }

    private boolean cancel() {
        return XWalkLibraryLoader.cancelDecompression();
    }
}
