// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.Context;
import android.util.Log;

import org.xwalk.core.XWalkLibraryLoader.ActivateListener;
import org.xwalk.core.XWalkLibraryLoader.DecompressListener;

/**
 * <p><code>XWalkInitializer</code> is an alternative to {@link XWalkActivity} with the difference
 * that it provides a way of initializing Crosswalk Project runtime in background silently.
 * <code>XWalkInitializer</code> also allows for more flexibility, the developer doesn't have to
 * have their activity class extend {@link XWalkActivity} anymore, and the context for initializing
 * Crosswalk environment doesn't have to be an activity either, it could be a service now. However,
 * {@link XWalkActivity} is still recommended because it makes the code simpler.</p>
 *
 * <p>If the initialization failed, which means Crosswalk Project runtime doesn't exist or doesn't
 * match the application, you could use {@link XWalkUpdater} to download suitable Crosswalk Project
 * runtime.</p>
 *
 * <h3>Edit Activity</h3>
 *
 * <p>Here is the sample code for embedded mode and shared mode:</p>
 *
 * <pre>
 * import android.app.Activity;
 * import android.os.Bundle;
 *
 * import org.xwalk.core.XWalkInitializer;
 * import org.xwalk.core.XWalkUpdater;
 * import org.xwalk.core.XWalkView;
 *
 * public class MainActivity extends Activity implements
 *        XWalkInitializer.XWalkInitListener,
 *        XWalkUpdater.XWalkUpdateListener {
 *
 *     private XWalkInitializer mXWalkInitializer;
 *     private XWalkUpdater mXWalkUpdater;
 *     private XWalkView mXWalkView;
 *
 *     &#64;Override
 *     protected void onCreate(Bundle savedInstanceState) {
 *         super.onCreate(savedInstanceState);
 *
 *         // Must call initAsync() before anything that involves the embedding
 *         // API, including invoking setContentView() with the layout which
 *         // holds the XWalkView object.
 *
 *         mXWalkInitializer = new XWalkInitializer(this, this);
 *         mXWalkInitializer.initAsync();
 *
 *         // Until onXWalkInitCompleted() is invoked, you should do nothing with the
 *         // embedding API except the following:
 *         // 1. Instantiate the XWalkView object
 *         // 2. Call XWalkPreferences.setValue()
 *         // 3. Call mXWalkView.setXXClient(), e.g., setUIClient
 *         // 4. Call mXWalkView.setXXListener(), e.g., setDownloadListener
 *         // 5. Call mXWalkView.addJavascriptInterface()
 *
 *         setContentView(R.layout.activity_main);
 *         mXWalkView = (XWalkView) findViewById(R.id.xwalkview);
 *     }
 *
 *     &#64;Override
 *     protected void onResume() {
 *         super.onResume();
 *
 *         // Try to initialize again when the user completed updating and
 *         // returned to current activity. The initAsync() will do nothing if
 *         // the initialization is proceeding or has already been completed.
 *
 *         mXWalkInitializer.initAsync();
 *     }
 *
 *     &#64;Override
 *     public void onXWalkInitStarted() {
 *     }
 *
 *     &#64;Override
 *     public void onXWalkInitCancelled() {
 *         // Perform error handling here
 *
 *         finish();
 *     }
 *
 *     &#64;Override
 *     public void onXWalkInitFailed() {
 *         if (mXWalkUpdater == null) {
 *             mXWalkUpdater = new XWalkUpdater(this, this);
 *         }
 *         mXWalkUpdater.updateXWalkRuntime();
 *     }
 *
 *     &#64;Override
 *     public void onXWalkInitCompleted() {
 *         // Do anyting with the embedding API
 *
 *         mXWalkView.loadUrl("https://crosswalk-project.org/");
 *     }
 *
 *     &#64;Override
 *     public void onXWalkUpdateCancelled() {
 *         // Perform error handling here
 *
 *         finish();
 *     }
 * }
 * </pre>
 *
 * <h3>Edit Layout</h3>
 *
 * <p>When the application was generated, some default layout resources were added to the project.
 * Add a single XWalkView resource to a proper place in the main layout resource file,
 * res/layout/activity_main.xml, like this:</p>
 *
 * <pre>
 *   &lt;org.xwalk.core.XWalkView
 *       android:id="@+id/xwalkview"
 *       android:layout_width="match_parent"
 *       android:layout_height="match_parent" /&gt;
 * </pre>
 *
 */
public class XWalkInitializer {
    /**
     * Interface used to initialize the Crosswalk runtime.
     */
    public interface XWalkInitListener {
        /**
         * Run on the UI thread to notify Crosswalk initialization is started.
         */
        public void onXWalkInitStarted();

        /**
         * Run on the UI thread to notify Crosswalk initialization is cancelled.
         */
        public void onXWalkInitCancelled();

        /**
         * Run on the UI thread to notify Crosswalk initialization failed.
         */
        public void onXWalkInitFailed();

        /**
         * Run on the UI thread to notify Crosswalk initialization is completed successfully.
         */
        public void onXWalkInitCompleted();
    }

    private static final String TAG = "XWalkLib";

    private XWalkInitListener mInitListener;
    private Context mContext;
    private boolean mIsXWalkReady;

    /**
     * Create an initializer
     *
     * <p>This method must be invoked on the UI thread.
     *
     * @param listener The {@link XWalkInitListener} to use.
     * @param context The context which initiate the initialization
     */
    public XWalkInitializer(XWalkInitListener listener, Context context) {
        mInitListener = listener;
        mContext = context;

        XWalkLibraryLoader.prepareToInit(mContext);
    }

    /**
     * Initialize the Crosswalk runtime in background asynchronously.
     *
     * <p>This method must be invoked on the UI thread.
     *
     * @return Return true if initialization started, false if another initialization is proceeding
     * , or XWalkUpdater is downloading, or initialization is already completed successfully.
     */
    public boolean initAsync() {
        if (mIsXWalkReady) return false;

        if (XWalkLibraryLoader.isInitializing() || XWalkLibraryLoader.isDownloading()) {
            Log.d(TAG, "Other initialization or download is proceeding");
            return false;
        }

        Log.d(TAG, "Initialized by XWalkInitializer");
        XWalkLibraryLoader.startDecompress(new XWalkLibraryListener());
        mInitListener.onXWalkInitStarted();
        return true;
    }

    /**
     * Attempt to cancel the initialization.
     *
     * @return False if the initialization is not proceeding or can't be cancelled, true otherwise.
     */
    public boolean cancelInit() {
        Log.d(TAG, "Cancel by XWalkInitializer");
        return XWalkLibraryLoader.cancelDecompress();
    }

    /**
     * Return whether Crosswalk's APIs are ready to use.
     *
     * @return true when or after {@link XWalkInitListener#onXWalkInitCompleted()} is invoked,
     *         false otherwise
     */
    public boolean isXWalkReady() {
        return mIsXWalkReady;
    }
    /**
     * Return whether running in shared mode. This method has meaning only when the return value
     * of {@link #isXWalkReady} is true.
     *
     * @return true if running in shared mode, false otherwise
     */
    public boolean isSharedMode() {
        return mIsXWalkReady && XWalkLibraryLoader.isSharedLibrary();
    }

    /**
     * Return whether running in download  mode. This method has meaning only when the return value
     * of {@link #isXWalkReady} is true.
     *
     * @return true if running in download mode, false otherwise
     */
    public boolean isDownloadMode() {
        return mIsXWalkReady && XWalkEnvironment.isDownloadMode();
    }

    private class XWalkLibraryListener implements DecompressListener, ActivateListener {
        @Override
        public void onDecompressStarted() {
        }

        @Override
        public void onDecompressCancelled() {
            mInitListener.onXWalkInitCancelled();
        }

        @Override
        public void onDecompressCompleted() {
            XWalkLibraryLoader.startActivate(this);
        }

        @Override
        public void onActivateStarted() {
        }

        @Override
        public void onActivateFailed() {
            mInitListener.onXWalkInitFailed();
        }

        @Override
        public void onActivateCompleted() {
            mIsXWalkReady = true;
            XWalkLibraryLoader.finishInit(mContext);
            mInitListener.onXWalkInitCompleted();
        }
    }
}
