// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.Context;

import junit.framework.Assert;

import org.xwalk.core.XWalkLibraryLoader.ActivateListener;
import org.xwalk.core.XWalkLibraryLoader.DecompressListener;
import org.xwalk.core.XWalkLibraryLoader.DockListener;

/**
 * <p><code>XWalkInitializer</code> is an alternative to {@link XWalkActivity} with the difference
 * that it provides a way to initialize the Crosswalk runtime in background silently. Another
 * advantage is that the developer can use their own activity class directly rather than having it
 * extend {@link XWalkActivity}. However, {@link XWalkActivity} is still recommended because it
 * makes the code simpler.</p>
 *
 * <p>If the initialization failed, which means the Crosswalk runtime doesn't exist or doesn't match
 * the application, you could use {@link XWalkUpdater} to prompt the user to download the Crosswalk
 * runtime just like {@link XWalkActivity}.
 *
 * <p>For example:</p>
 *
 * <pre>
 * public class MyActivity extends Activity implements XWalkInitializer.XWalkInitListener {
 *     XWalkView mXWalkView;
 *
 *     &#64;Override
 *     protected void onCreate(Bundle savedInstanceState) {
 *         super.onCreate(savedInstanceState);
 *
 *         // Must call initAsync() before anything that involes the embedding API,
 *         // including calling setContentView() with the layout which holds the XWalkView object.
 *
 *         XWalkInitializer.initAsync(this, this);
 *
 *         // Before onXWalkInitCompleted() is invoked, you can do nothing with the embedding API
 *         // except the following:
 *         // 1. Create the instance of XWalkView
 *         // 2. Call setUIClient()
 *         // 3. Call setResourceClient()
 *
 *         setContentView(R.layout.activity_xwalkview);
 *         mXWalkView = (XWalkView) findViewById(R.id.xwalkview);
 *         mXWalkView.setUIClient(new MyXWalkUIClient(mXWalkView));
 *         mXWalkView.setResourceClient(new MyXWalkResourceClient(mXWalkView));
 *     }
 *
 *     &#64;Override
 *     public void onXWalkInitStarted() {
 *     }
 *
 *     &#64;Override
 *     public void onXWalkInitCompleted() {
 *         // Do anyting with the embedding API
 *
 *         XWalkPreferences.setValue(XWalkPreferences.SUPPORT_MULTIPLE_WINDOWS, true);
 *
 *         mXWalkView.load("http://crosswalk-project.org/", null);
 *     }
 *
 *     &#64;Override
 *     public void onXWalkInitFailed() {
 *         // Perform error handling here, or launch the XWalkUpdater
 *     }
 *
 *     &#64;Override
 *     public void onXWalkInitCancelled() {
 *         // Perform error handling here
 *     }
 * }
 * </pre>
 */
public class XWalkInitializer {
    /**
     * Interface used to initialize the Crosswalk runtime
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

    private static XWalkInitializer sInstance;

    /**
     * Initializes the Crosswalk runtime asynchronously.
     *
     * <p>This method must be invoked on the UI thread.
     *
     * @param listener The {@link XWalkInitListener} to use
     * @param context The context that the initialization is to run it
     *
     * @return True if the initialization is started, false if another initialization is proceeding
     */
    public static boolean initAsync(XWalkInitListener listener, Context context) {
        if (sInstance != null) return false;
        new XWalkInitializer(listener, context).initialize();
        return true;
    }

    /**
     * Attempts to cancel the initialization.
     *
     * @return False if the initialization is not proceeding or can't be cancelled, true otherwise.
     */
    public static boolean cancelInit() {
        return sInstance != null && sInstance.cancel();
    }

    /**
     * Check whether the initialization is procedding.
     */
    public static boolean isInProgress() {
        return sInstance != null;
    }

    private XWalkInitListener mInitListener;
    private Context mContext;

    private XWalkInitializer(XWalkInitListener listener, Context context) {
        mInitListener = listener;
        mContext = context;

        XWalkLibraryLoader.prepareToInit();
    }

    private void initialize() {
        sInstance = this;
        mInitListener.onXWalkInitStarted();

        XWalkLibraryLoader.startDecompress(new XWalkLibraryListener(), mContext);
    }

    private boolean cancel() {
        return XWalkLibraryLoader.cancelDecompress();
    }

    private class XWalkLibraryListener
            implements DecompressListener, DockListener, ActivateListener {
        @Override
        public void onDecompressStarted() {
        }

        @Override
        public void onDecompressCancelled() {
            sInstance = null;
            mInitListener.onXWalkInitCancelled();
        }

        @Override
        public void onDecompressCompleted() {
            XWalkLibraryLoader.startDock(this, mContext);
        }

        @Override
        public void onDockStarted() {
        }

        @Override
        public void onDockFailed() {
            sInstance = null;
            mInitListener.onXWalkInitFailed();
        }

        @Override
        public void onDockCompleted() {
            XWalkLibraryLoader.startActivate(this);
        }

        @Override
        public void onActivateStarted() {
        }

        @Override
        public void onActivateCompleted() {
            sInstance = null;
            mInitListener.onXWalkInitCompleted();
        }
    }
}
