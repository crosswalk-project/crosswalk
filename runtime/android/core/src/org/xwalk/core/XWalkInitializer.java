// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.util.Log;

import org.xwalk.core.XWalkLibraryLoader.ActivateListener;
import org.xwalk.core.XWalkLibraryLoader.DecompressListener;

/**
 * <p><code>XWalkInitializer</code> is an alternative to {@link XWalkActivity} with the difference
 * that it provides a way to initialize the Crosswalk runtime in background silently. Another
 * advantage is that the developer can use their own activity class directly rather than having it
 * extend {@link XWalkActivity}. However, {@link XWalkActivity} is still recommended because it
 * makes the code simpler.</p>
 *
 * <p>If the initialization failed, which means the Crosswalk runtime doesn't exist or doesn't match
 * the app, you could use {@link XWalkUpdater} to prompt the user to download suiteble Crosswalk
 * runtime.
 *
 * <p>For example:</p>
 *
 * <pre>
 * public class MyActivity extends Activity implements XWalkInitializer.XWalkInitListener {
 *     XWalkView mXWalkView;
 *     XWalkInitializer mXWalkInitializer;
 *
 *     &#64;Override
 *     protected void onCreate(Bundle savedInstanceState) {
 *         super.onCreate(savedInstanceState);
 *
 *         // Must call initAsync() before anything that involes the embedding API, including
 *         // invoking setContentView() with the layout which holds the XWalkView object.
 *
 *         mXWalkInitializer = new XWalkInitializer(this, this);
 *         mXWalkInitializer.initAsync();
 *
 *         // Until onXWalkInitCompleted() is invoked, you should do nothing with the embedding API
 *         // except the following:
 *         // 1. Instanciate the XWalkView object
 *         // 2. Call XWalkPreferences.setValue()
 *         // 3. Call XWalkView.setUIClient()
 *         // 4. Call XWalkView.setResourceClient()
 *
 *         XWalkPreferences.setValue(XWalkPreferences.ANIMATABLE_XWALK_VIEW, true);
 *
 *         setContentView(R.layout.activity_xwalkview);
 *         mXWalkView = (XWalkView) findViewById(R.id.xwalkview);
 *         mXWalkView.setUIClient(new MyXWalkUIClient(mXWalkView));
 *         mXWalkView.setResourceClient(new MyXWalkResourceClient(mXWalkView));
 *     }
 *
 *     &#64;Override
 *     public void onXWalkInitCompleted() {
 *         // Do anyting with the embedding API
 *         mXWalkView.load("http://crosswalk-project.org/", null);
 *     }
 *
 *     &#64;Override
 *     public void onXWalkInitStarted() {
 *     }
 *
 *     &#64;Override
 *     public void onXWalkInitCancelled() {
 *         // Perform error handling here
 *     }
 *
 *     &#64;Override
 *     public void onXWalkInitFailed() {
 *         // Perform error handling here, or launch the XWalkUpdater
 *     }
 * }
 * </pre>
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

    private static final String TAG = "XWalkActivity";

    private XWalkInitListener mInitListener;
    private Activity mActivity;

    private boolean mIsInitializing;
    private boolean mIsXWalkReady;

    /**
     * Create an initializer for single activity.
     *
     * <p>This method must be invoked on the UI thread.
     *
     * @param listener The {@link XWalkInitListener} to use.
     * @param activity The activity which initiate the initialization
     */
    public XWalkInitializer(XWalkInitListener listener, Activity activity) {
        mInitListener = listener;
        mActivity = activity;

        XWalkLibraryLoader.prepareToInit(mActivity);
    }

    /**
     * Initialize the Crosswalk runtime in background asynchronously.
     *
     * <p>This method must be invoked on the UI thread.
     *
     * @return Return true if initialization started, false if another initialization is proceeding
     * or Crosswalk initialization is already completed successfully.
     */
    public boolean initAsync() {
        if (mIsInitializing || mIsXWalkReady) return false;

        mIsInitializing = true;
        mInitListener.onXWalkInitStarted();
        if (XWalkLibraryLoader.isLibraryReady()) {
            Log.d(TAG, "Activate by XWalkInitializer");
            XWalkLibraryLoader.startActivate(new XWalkLibraryListener(), mActivity);
        } else {
            Log.d(TAG, "Initialize by XWalkInitializer");
            XWalkLibraryLoader.startDecompress(new XWalkLibraryListener(), mActivity);
        }
        return true;
    }

    /**
     * Attempt to cancel the initialization.
     *
     * @return False if the initialization is not proceeding or can't be cancelled, true otherwise.
     */
    public boolean cancelInit() {
        Log.d(TAG, "Cancel by XWalkInitializer");
        return mIsInitializing && XWalkLibraryLoader.cancelDecompress();
    }

    private class XWalkLibraryListener implements DecompressListener, ActivateListener {
        @Override
        public void onDecompressStarted() {
        }

        @Override
        public void onDecompressCancelled() {
            mIsInitializing = false;
            mInitListener.onXWalkInitCancelled();
        }

        @Override
        public void onDecompressCompleted() {
            XWalkLibraryLoader.startActivate(this, mActivity);
        }

        @Override
        public void onActivateStarted() {
        }

        @Override
        public void onActivateFailed() {
            mIsInitializing = false;
            mInitListener.onXWalkInitFailed();
        }

        @Override
        public void onActivateCompleted() {
            mIsInitializing = false;
            mIsXWalkReady = true;
            mInitListener.onXWalkInitCompleted();
        }
    }
}
