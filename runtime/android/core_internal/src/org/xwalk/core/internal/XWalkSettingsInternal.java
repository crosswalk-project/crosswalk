// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.provider.Settings;
import android.webkit.WebSettings;

import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;
import org.chromium.base.ThreadUtils;
import org.chromium.content_public.browser.WebContents;

/**
 * Settings for single XWalkView object
 */
@JNINamespace("xwalk")
@XWalkAPI(createInternally = true)
public class XWalkSettingsInternal {

    private static final String TAG = "XWalkSettings";

    // This class must be created on the UI thread. Afterwards, it can be
    // used from any thread. Internally, the class uses a message queue
    // to call native code on the UI thread only.

    // Lock to protect all settings.
    private final Object mXWalkSettingsLock = new Object();

    private final Context mContext;

    private boolean mAllowScriptsToCloseWindows = true;
    private boolean mLoadsImagesAutomatically = true;
    private boolean mImagesEnabled = true;
    private boolean mJavaScriptEnabled = true;
    private boolean mAllowUniversalAccessFromFileURLs = false;
    private boolean mAllowFileAccessFromFileURLs = false;
    private boolean mJavaScriptCanOpenWindowsAutomatically = true;
    private int mCacheMode = WebSettings.LOAD_DEFAULT;
    private boolean mSupportMultipleWindows = false;
    private boolean mAppCacheEnabled = true;
    private boolean mDomStorageEnabled = true;
    private boolean mDatabaseEnabled = true;
    private boolean mUseWideViewport = false;
    private boolean mMediaPlaybackRequiresUserGesture = false;
    private String mDefaultVideoPosterURL;
    private final boolean mPasswordEchoEnabled;

    // Not accessed by the native side.
    private boolean mBlockNetworkLoads;  // Default depends on permission of embedding APK.
    private boolean mAllowContentUrlAccess = true;
    private boolean mAllowFileUrlAccess = true;
    private boolean mShouldFocusFirstNode = true;
    private boolean mGeolocationEnabled = true;
    private String mUserAgent;
    private String mAcceptLanguages;

    // Protects access to settings global fields.
    private static final Object sGlobalContentSettingsLock = new Object();
    // For compatibility with the legacy WebView, we can only enable AppCache when the path is
    // provided. However, we don't use the path, so we just check if we have received it from the
    // client.
    private static boolean sAppCachePathIsSet = false;

    // The native side of this object.
    private long mNativeXWalkSettings = 0;

    // A flag to avoid sending superfluous synchronization messages.
    private boolean mIsUpdateWebkitPrefsMessagePending = false;
    // Custom handler that queues messages to call native code on the UI thread.
    private final EventHandler mEventHandler;

    private static final int MINIMUM_FONT_SIZE = 1;
    private static final int MAXIMUM_FONT_SIZE = 72;
    private int mDefaultFontSize = 16;
    private int mDefaultFixedFontSize = 13;

    private boolean mAutoCompleteEnabled = true;

    private float mInitialPageScalePercent = 0;
    private double mDIPScale = 1.0;
    private int mTextSizePercent = 100;
    private ZoomSupportChangeListener mZoomChangeListener;
    private boolean mSupportZoom = true;
    private boolean mBuiltInZoomControls = false;
    private boolean mDisplayZoomControls = true;

    private boolean mSpatialNavigationEnabled = true;
    private boolean mQuirksModeEnabled = false;

    static class LazyDefaultUserAgent{
        private static final String sInstance = nativeGetDefaultUserAgent();
    }

    // Class to handle messages to be processed on the UI thread.
    private class EventHandler {
        // Message id for updating Webkit preferences
        private static final int UPDATE_WEBKIT_PREFERENCES = 0;
        // Actual UI thread handler
        private Handler mHandler;

        EventHandler() {
        }

        void bindUiThread() {
            if (mHandler != null) return;
            mHandler = new Handler(ThreadUtils.getUiThreadLooper()) {
                    @Override
                    public void handleMessage(Message msg) {
                        switch (msg.what) {
                            case UPDATE_WEBKIT_PREFERENCES:
                                synchronized (mXWalkSettingsLock) {
                                    updateWebkitPreferencesOnUiThread();
                                    mIsUpdateWebkitPrefsMessagePending = false;
                                    mXWalkSettingsLock.notifyAll();
                                }
                                break;
                        }
                    }
                };
        }

        void maybeRunOnUiThreadBlocking(Runnable r) {
            if (mHandler != null) {
                ThreadUtils.runOnUiThreadBlocking(r);
            }
        }

        void maybePostOnUiThread(Runnable r) {
            if (mHandler != null) {
                mHandler.post(r);
            }
        }

        private void updateWebkitPreferencesLocked() {
            assert Thread.holdsLock(mXWalkSettingsLock);
            if (mNativeXWalkSettings == 0) return;
            if (mHandler == null) return;
            if (ThreadUtils.runningOnUiThread()) {
                updateWebkitPreferencesOnUiThread();
            } else {
                // We're being called on a background thread, so post a message.
                if (mIsUpdateWebkitPrefsMessagePending) {
                    return;
                }
                mIsUpdateWebkitPrefsMessagePending = true;
                mHandler.sendMessage(Message.obtain(null, UPDATE_WEBKIT_PREFERENCES));
                // We must block until the settings have been sync'd to native to
                // ensure that they have taken effect.
                try {
                    while (mIsUpdateWebkitPrefsMessagePending) {
                        mXWalkSettingsLock.wait();
                    }
                } catch (InterruptedException e) {}
            }
        }
    }

    interface ZoomSupportChangeListener {
        public void onGestureZoomSupportChanged(
                boolean supportsDoubleTapZoom, boolean supportsMultiTouchZoom);
    }

    // Never use this constructor.
    // It is only used in XWalkSettingsBridge.
    XWalkSettingsInternal() {
        mContext = null;
        mEventHandler = null;
        mPasswordEchoEnabled = false;
    }

    XWalkSettingsInternal(Context context, WebContents webContents,
            boolean isAccessFromFileURLsGrantedByDefault) {
        ThreadUtils.assertOnUiThread();
        mContext = context;
        mBlockNetworkLoads = mContext.checkPermission(
                android.Manifest.permission.INTERNET,
                Process.myPid(),
                Process.myUid()) != PackageManager.PERMISSION_GRANTED;

        if (isAccessFromFileURLsGrantedByDefault) {
            mAllowUniversalAccessFromFileURLs = true;
            mAllowFileAccessFromFileURLs = true;
        }

        mUserAgent = LazyDefaultUserAgent.sInstance;

        // Respect the system setting for password echoing.
        mPasswordEchoEnabled = Settings.System.getInt(context.getContentResolver(),
                Settings.System.TEXT_SHOW_PASSWORD, 1) == 1;

        mEventHandler = new EventHandler();

        setWebContents(webContents);
    }

    void setWebContents(WebContents webContents) {
        synchronized (mXWalkSettingsLock) {
            if (mNativeXWalkSettings != 0) {
                nativeDestroy(mNativeXWalkSettings);
                assert mNativeXWalkSettings == 0;
            }
            if (webContents != null) {
                mEventHandler.bindUiThread();
                mNativeXWalkSettings = nativeInit(webContents);
                nativeUpdateEverythingLocked(mNativeXWalkSettings);
            }
        }
    }

    @CalledByNative
    private void nativeXWalkSettingsGone(long nativeXWalkSettings) {
        assert mNativeXWalkSettings != 0 && mNativeXWalkSettings == nativeXWalkSettings;
        mNativeXWalkSettings = 0;
    }

    public void setAllowScriptsToCloseWindows(boolean allow) {
        synchronized (mXWalkSettingsLock) {
            if (mAllowScriptsToCloseWindows != allow) {
                mAllowScriptsToCloseWindows = allow;
            }
        }
    }

    public boolean getAllowScriptsToCloseWindows() {
        synchronized (mXWalkSettingsLock) {
            return mAllowScriptsToCloseWindows;
        }
    }

    /**
     * See {@link android.webkit.WebSettings#setCacheMode}.
     */
    public void setCacheMode(int mode) {
        synchronized (mXWalkSettingsLock) {
            if (mCacheMode != mode) {
                mCacheMode = mode;
            }
        }
    }

    /**
     * See {@link android.webkit.WebSettings#getCacheMode}.
     */
    public int getCacheMode() {
        synchronized (mXWalkSettingsLock) {
            return mCacheMode;
        }
    }

    /**
     * See {@link android.webkit.WebSettings#setBlockNetworkLoads}.
     */
    public void setBlockNetworkLoads(boolean flag) {
        synchronized (mXWalkSettingsLock) {
            if (!flag && mContext.checkPermission(
                    android.Manifest.permission.INTERNET,
                    Process.myPid(),
                    Process.myUid()) != PackageManager.PERMISSION_GRANTED) {
                throw new SecurityException("Permission denied - " +
                        "application missing INTERNET permission");
            }
            mBlockNetworkLoads = flag;
        }
    }

    /**
     * See {@link android.webkit.WebSettings#getBlockNetworkLoads}.
     */
    public boolean getBlockNetworkLoads() {
        synchronized (mXWalkSettingsLock) {
            return mBlockNetworkLoads;
        }
    }

    /**
     * See {@link android.webkit.WebSettings#setAllowFileAccess}.
     */
    public void setAllowFileAccess(boolean allow) {
        synchronized (mXWalkSettingsLock) {
            if (mAllowFileUrlAccess != allow) {
                mAllowFileUrlAccess = allow;
            }
        }
    }

    /**
     * See {@link android.webkit.WebSettings#getAllowFileAccess}.
     */
    public boolean getAllowFileAccess() {
        synchronized (mXWalkSettingsLock) {
            return mAllowFileUrlAccess;
        }
    }

    /**
     * See {@link android.webkit.WebSettings#setAllowContentAccess}.
     */
    public void setAllowContentAccess(boolean allow) {
        synchronized (mXWalkSettingsLock) {
            if (mAllowContentUrlAccess != allow) {
                mAllowContentUrlAccess = allow;
            }
        }
    }

    /**
     * See {@link android.webkit.WebSettings#getAllowContentAccess}.
     */
    public boolean getAllowContentAccess() {
        synchronized (mXWalkSettingsLock) {
            return mAllowContentUrlAccess;
        }
    }

    /**
     * See {@link android.webkit.WebSettings#setGeolocationEnabled}.
     */
    public void setGeolocationEnabled(boolean flag) {
        synchronized (mXWalkSettingsLock) {
            if (mGeolocationEnabled != flag) {
                mGeolocationEnabled = flag;
            }
        }
    }

    /**
     * @return Returns if geolocation is currently enabled.
     */
    boolean getGeolocationEnabled() {
        synchronized (mXWalkSettingsLock) {
            return mGeolocationEnabled;
        }
    }

    /**
     * See {@link android.webkit.WebSettings#setJavaScriptEnabled}.
     */
    public void setJavaScriptEnabled(boolean flag) {
        synchronized (mXWalkSettingsLock) {
            if (mJavaScriptEnabled != flag) {
                mJavaScriptEnabled = flag;
                mEventHandler.updateWebkitPreferencesLocked();
            }
        }
    }

    /**
     * See {@link android.webkit.WebSettings#setAllowUniversalAccessFromFileURLs}.
     */
    public void setAllowUniversalAccessFromFileURLs(boolean flag) {
        synchronized (mXWalkSettingsLock) {
            if (mAllowUniversalAccessFromFileURLs != flag) {
                mAllowUniversalAccessFromFileURLs = flag;
                mEventHandler.updateWebkitPreferencesLocked();
            }
        }
    }

    /**
     * See {@link android.webkit.WebSettings#setAllowFileAccessFromFileURLs}.
     */
    public void setAllowFileAccessFromFileURLs(boolean flag) {
        synchronized (mXWalkSettingsLock) {
            if (mAllowFileAccessFromFileURLs != flag) {
                mAllowFileAccessFromFileURLs = flag;
                mEventHandler.updateWebkitPreferencesLocked();
            }
        }
    }

    /**
     * See {@link android.webkit.WebSettings#setLoadsImagesAutomatically}.
     */
    public void setLoadsImagesAutomatically(boolean flag) {
        synchronized (mXWalkSettingsLock) {
            if (mLoadsImagesAutomatically != flag) {
                mLoadsImagesAutomatically = flag;
                mEventHandler.updateWebkitPreferencesLocked();
            }
        }
    }

    /**
     * See {@link android.webkit.WebSettings#getLoadsImagesAutomatically}.
     */
    public boolean getLoadsImagesAutomatically() {
        synchronized (mXWalkSettingsLock) {
            return mLoadsImagesAutomatically;
        }
    }

    /**
     * See {@link android.webkit.WebSettings#setImagesEnabled}.
     */
    public void setImagesEnabled(boolean flag) {
        synchronized (mXWalkSettingsLock) {
            if (mImagesEnabled != flag) {
                mImagesEnabled = flag;
                mEventHandler.updateWebkitPreferencesLocked();
            }
        }
    }

    /**
     * See {@link android.webkit.WebSettings#getImagesEnabled}.
     */
    public boolean getImagesEnabled() {
        synchronized (mXWalkSettingsLock) {
            return mImagesEnabled;
        }
    }

    /**
     * See {@link android.webkit.WebSettings#getJavaScriptEnabled}.
     */
    public boolean getJavaScriptEnabled() {
        synchronized (mXWalkSettingsLock) {
            return mJavaScriptEnabled;
        }
    }

    /**
     * See {@link android.webkit.WebSettings#getAllowUniversalAccessFromFileURLs}.
     */
    public boolean getAllowUniversalAccessFromFileURLs() {
        synchronized (mXWalkSettingsLock) {
            return mAllowUniversalAccessFromFileURLs;
        }
    }

    /**
     * See {@link android.webkit.WebSettings#getAllowFileAccessFromFileURLs}.
     */
    public boolean getAllowFileAccessFromFileURLs() {
        synchronized (mXWalkSettingsLock) {
            return mAllowFileAccessFromFileURLs;
        }
    }

    /**
     * See {@link android.webkit.WebSettings#setJavaScriptCanOpenWindowsAutomatically}.
     */
    public void setJavaScriptCanOpenWindowsAutomatically(boolean flag) {
        synchronized (mXWalkSettingsLock) {
            if (mJavaScriptCanOpenWindowsAutomatically != flag) {
                mJavaScriptCanOpenWindowsAutomatically = flag;
                mEventHandler.updateWebkitPreferencesLocked();
            }
        }
    }

    /**
     * See {@link android.webkit.WebSettings#getJavaScriptCanOpenWindowsAutomatically}.
     */
    public boolean getJavaScriptCanOpenWindowsAutomatically() {
        synchronized (mXWalkSettingsLock) {
            return mJavaScriptCanOpenWindowsAutomatically;
        }
    }

    /**
     * See {@link android.webkit.WebSettings#setSupportMultipleWindows}.
     */
    public void setSupportMultipleWindows(boolean support) {
        synchronized (mXWalkSettingsLock) {
            if (mSupportMultipleWindows != support) {
                mSupportMultipleWindows = support;
                mEventHandler.updateWebkitPreferencesLocked();
            }
        }
    }

    /**
     * See {@link android.webkit.WebSettings#supportMultipleWindows}.
     */
    public boolean supportMultipleWindows() {
        synchronized (mXWalkSettingsLock) {
            return mSupportMultipleWindows;
        }
    }

    /**
     * Sets whether the XWalkView should enable support for the "viewport" HTML
     * meta tag or should use a wide viewport. When the value of the setting is
     * false, the layout width is always set to the width of the XWalkView control
     * in device-independent (CSS) pixels. When the value is true and the page
     * contains the viewport meta tag, the value of the width specified in the
     * tag is used. If the page does not contain the tag or does not provide a
     * width, then a wide viewport will be used.
     * @param use whether to enable support for the viewport meta tag.
     * @since 6.0
     */
    @XWalkAPI
    public void setUseWideViewPort(boolean use) {
        synchronized (mXWalkSettingsLock) {
            if (mUseWideViewport != use) {
                mUseWideViewport = use;
                mEventHandler.updateWebkitPreferencesLocked();
            }
        }
    }

    /**
      * Gets whether the XWalkView supports the "viewport" HTML meta tag or will
      * use a wide viewport.
      * @return true if the XWalkView supports the viewport meta tag.
      * @since 6.0
      */
    @XWalkAPI
    public boolean getUseWideViewPort() {
        synchronized (mXWalkSettingsLock) {
            return mUseWideViewport;
        }
    }

    /**
     * See {@link android.webkit.WebSettings#setAppCacheEnabled}.
     */
    public void setAppCacheEnabled(boolean flag) {
        synchronized (mXWalkSettingsLock) {
            if (mAppCacheEnabled != flag) {
                mAppCacheEnabled = flag;
                mEventHandler.updateWebkitPreferencesLocked();
            }
        }
    }

    /**
     * See {@link android.webkit.WebSettings#setAppCachePath}.
     */
    public void setAppCachePath(String path) {
        boolean needToSync = false;
        synchronized (sGlobalContentSettingsLock) {
            // AppCachePath can only be set once.
            if (!sAppCachePathIsSet && path != null && !path.isEmpty()) {
                sAppCachePathIsSet = true;
                needToSync = true;
            }
        }
        // The obvious problem here is that other WebViews will not be updated,
        // until they execute synchronization from Java to the native side.
        // But this is the same behaviour as it was in the legacy WebView.
        if (needToSync) {
            synchronized (mXWalkSettingsLock) {
                mEventHandler.updateWebkitPreferencesLocked();
            }
        }
    }

    /**
     * Gets whether Application Cache is enabled.
     *
     * @return true if Application Cache is enabled
     * @hide
     */
    @CalledByNative
    private boolean getAppCacheEnabled() {
        // When no app cache path is set, use chromium default cache path.
        return mAppCacheEnabled;
    }

    /**
     * See {@link android.webkit.WebSettings#setDomStorageEnabled}.
     */
    public void setDomStorageEnabled(boolean flag) {
        synchronized (mXWalkSettingsLock) {
            if (mDomStorageEnabled != flag) {
                mDomStorageEnabled = flag;
                mEventHandler.updateWebkitPreferencesLocked();
            }
        }
    }

    /**
     * See {@link android.webkit.WebSettings#getDomStorageEnabled}.
     */
    public boolean getDomStorageEnabled() {
       synchronized (mXWalkSettingsLock) {
           return mDomStorageEnabled;
       }
    }

    /**
     * See {@link android.webkit.WebSettings#setDatabaseEnabled}.
     */
    public void setDatabaseEnabled(boolean flag) {
        synchronized (mXWalkSettingsLock) {
            if (mDatabaseEnabled != flag) {
                mDatabaseEnabled = flag;
                mEventHandler.updateWebkitPreferencesLocked();
            }
        }
    }

    /**
     * See {@link android.webkit.WebSettings#getDatabaseEnabled}.
     */
    public boolean getDatabaseEnabled() {
       synchronized (mXWalkSettingsLock) {
           return mDatabaseEnabled;
       }
    }

    /**
     * See {@link android.webkit.WebSettings#setMediaPlaybackRequiresUserGesture}.
     */
    public void setMediaPlaybackRequiresUserGesture(boolean require) {
        synchronized (mXWalkSettingsLock) {
            if (mMediaPlaybackRequiresUserGesture != require) {
                mMediaPlaybackRequiresUserGesture = require;
                mEventHandler.updateWebkitPreferencesLocked();
            }
        }
    }

    /**
     * See {@link android.webkit.WebSettings#getMediaPlaybackRequiresUserGesture}.
     */
    public boolean getMediaPlaybackRequiresUserGesture() {
        synchronized (mXWalkSettingsLock) {
            return mMediaPlaybackRequiresUserGesture;
        }
    }

    /**
     * See {@link android.webkit.WebSettings#setDefaultVideoPosterURL}.
     */
    public void setDefaultVideoPosterURL(String url) {
        synchronized (mXWalkSettingsLock) {
            if (mDefaultVideoPosterURL != null && !mDefaultVideoPosterURL.equals(url) ||
                    mDefaultVideoPosterURL == null && url != null) {
                mDefaultVideoPosterURL = url;
                mEventHandler.updateWebkitPreferencesLocked();
            }
        }
    }

    /**
     * @return returns the default User-Agent used by each ContentViewCore instance, i.e. unless
     * overridden by {@link #setUserAgentString()}
     */
    public static String getDefaultUserAgent() {
        return LazyDefaultUserAgent.sInstance;
    }

    /**
     * Set the user agent of web page/app.
     * @param userAgent the user agent string passed from client.
     * @since 6.0
     */
    @XWalkAPI
    public void setUserAgentString(String userAgent) {
        synchronized (mXWalkSettingsLock) {
            final String oldUserAgent = mUserAgent;
            if (userAgent == null || userAgent.length() == 0) {
                mUserAgent = LazyDefaultUserAgent.sInstance;
            } else {
                mUserAgent = userAgent;
            }
            if (!oldUserAgent.equals(mUserAgent)) {
                mEventHandler.maybeRunOnUiThreadBlocking(new Runnable() {
                    @Override
                    public void run() {
                        if (mNativeXWalkSettings != 0) {
                            nativeUpdateUserAgent(mNativeXWalkSettings);
                        }
                    }
                });
            }
        }
    }

    /**
      * Get the user agent of web page/app.
      * @return the XWalkView's user-agent string.
      * @since 6.0
      */
    @XWalkAPI
    public String getUserAgentString() {
        synchronized (mXWalkSettingsLock) {
            return mUserAgent;
        }
    }

    @CalledByNative
    private String getUserAgentLocked() {
        return mUserAgent;
    }

    /**
     * See {@link android.webkit.WebSettings#getDefaultVideoPosterURL}.
     */
    public String getDefaultVideoPosterURL() {
        synchronized (mXWalkSettingsLock) {
            return mDefaultVideoPosterURL;
        }
    }

    @CalledByNative
    private void updateEverything() {
        synchronized (mXWalkSettingsLock) {
            nativeUpdateEverythingLocked(mNativeXWalkSettings);
        }
    }

    private void updateWebkitPreferencesOnUiThread() {
        if (mNativeXWalkSettings != 0) {
            ThreadUtils.assertOnUiThread();
            nativeUpdateWebkitPreferences(mNativeXWalkSettings);
        }
    }

     /**
     * Set the accept languages of XWalkView.
     * @param acceptLanguages the accept languages string passed from client.
     * @since 6.0
     */
    @XWalkAPI
    public void setAcceptLanguages(final String acceptLanguages) {
        synchronized (mXWalkSettingsLock) {
            if (mAcceptLanguages == acceptLanguages) return;
            mAcceptLanguages = acceptLanguages;
            mEventHandler.maybeRunOnUiThreadBlocking(new Runnable() {
                @Override
                public void run() {
                    if (mNativeXWalkSettings != 0) {
                        nativeUpdateAcceptLanguages(mNativeXWalkSettings);
                    }
                }
            });
        }
    }

    /**
     * Get the accept languages of XWalkView.
     * @return the accept languages
     * @since 6.0
     */
    @XWalkAPI
    public String getAcceptLanguages() {
        synchronized (mXWalkSettingsLock) {
            return mAcceptLanguages;
        }
    }

    /**
     * See {@link android.webkit.WebSettings#setSaveFormData}.
     */
    public void setSaveFormData(final boolean enable) {
        synchronized (mXWalkSettingsLock) {
            if (mAutoCompleteEnabled == enable) return;
            mAutoCompleteEnabled = enable;
            mEventHandler.maybeRunOnUiThreadBlocking(new Runnable() {
                @Override
                public void run() {
                    if (mNativeXWalkSettings != 0) {
                        nativeUpdateFormDataPreferences(mNativeXWalkSettings);
                    }
                }
            });
        }
    }

    /**
     * See {@link android.webkit.WebSettings#getSaveFormData}.
     */
    public boolean getSaveFormData() {
        synchronized (mXWalkSettingsLock) {
            return getSaveFormDataLocked();
        }
    }

    @CalledByNative
    private String getAcceptLanguagesLocked() {
        return mAcceptLanguages;
    }

    @CalledByNative
    private boolean getSaveFormDataLocked() {
        return mAutoCompleteEnabled;
    }

    void setDIPScale(double dipScale) {
        synchronized (mXWalkSettingsLock) {
            mDIPScale = dipScale;
            // TODO(hengzhi.wu): This should also be synced over to native side, but right now
            // the setDIPScale call is always followed by a setWebContents() which covers this.
        }
    }

    /**
     * Sets the initial scale for this XWalkView.
     * @param scaleInPercent the initial scale in percent.
     * @since 6.0
     */
    @XWalkAPI
    public void setInitialPageScale(final float scaleInPercent) {
        synchronized (mXWalkSettingsLock) {
            if (mInitialPageScalePercent == scaleInPercent) return;
            mInitialPageScalePercent = scaleInPercent;
            mEventHandler.maybeRunOnUiThreadBlocking(new Runnable() {
                @Override
                public void run() {
                    if (mNativeXWalkSettings != 0) {
                        nativeUpdateInitialPageScale(mNativeXWalkSettings);
                    }
                }
            });
        }
    }

    @CalledByNative
    private float getInitialPageScalePercentLocked() {
        assert Thread.holdsLock(mXWalkSettingsLock);
        return mInitialPageScalePercent;
    }

    @CalledByNative
    private double getDIPScaleLocked() {
        assert Thread.holdsLock(mXWalkSettingsLock);
        return mDIPScale;
    }

    @CalledByNative
    private boolean getPasswordEchoEnabledLocked() {
        assert Thread.holdsLock(mXWalkSettingsLock);
        return mPasswordEchoEnabled;
    }

    /**
     * Sets the text zoom of the page in percent. The default is 100.
     * @param textZoom the text zoom in percent.
     * @since 6.0
     */
    @XWalkAPI
    public void setTextZoom(final int textZoom) {
        synchronized (mXWalkSettingsLock) {
            if (mTextSizePercent == textZoom) return;
            mTextSizePercent = textZoom;
            mEventHandler.updateWebkitPreferencesLocked();
        }
    }

    /**
     * Gets the text zoom of the page in percent.
     * @return the text zoom of the page in percent.
     * @since 6.0
     */
    @XWalkAPI
    public int getTextZoom() {
        synchronized (mXWalkSettingsLock) {
            return mTextSizePercent;
        }
    }

    private int clipFontSize(int size) {
        if (size < MINIMUM_FONT_SIZE) {
            return MINIMUM_FONT_SIZE;
        } else if (size > MAXIMUM_FONT_SIZE) {
            return MAXIMUM_FONT_SIZE;
        }
        return size;
    }

    /**
     * Sets the default font size. The default is 16.
     * @param size non-negative integer between 1 and 72.
     *             Any number outside the specified range will be pinned.
     * @since 6.0
     */
    @XWalkAPI
    public void setDefaultFontSize(int size) {
        synchronized (mXWalkSettingsLock) {
            size = clipFontSize(size);
            if (mDefaultFontSize == size) return;
            mDefaultFontSize = size;
            mEventHandler.updateWebkitPreferencesLocked();
        }
    }

    /**
     * Gets the default font size.
     * @return a non-negative integer between 1 and 72.
     * @since 6.0
     */
    @XWalkAPI
    public int getDefaultFontSize() {
        synchronized (mXWalkSettingsLock) {
            return mDefaultFontSize;
        }
    }

    /**
     * Sets the default fixed font size. The default is 16.
     * @param size a non-negative integer between 1 and 72.
     *             Any number outside the specified range will be pinned.
     * @since 6.0
     */
    @XWalkAPI
    public void setDefaultFixedFontSize(int size) {
        synchronized (mXWalkSettingsLock) {
            size = clipFontSize(size);
            if (mDefaultFixedFontSize == size) return;
            mDefaultFixedFontSize = size;
            mEventHandler.updateWebkitPreferencesLocked();
        }
    }

    /**
     * Gets the default fixed font size.
     * @return a non-negative integer between 1 and 72.
     * @since 6.0
     */
    @XWalkAPI
    public int getDefaultFixedFontSize() {
        synchronized (mXWalkSettingsLock) {
            return mDefaultFixedFontSize;
        }
    }

    void setZoomListener(ZoomSupportChangeListener zoomChangeListener) {
        synchronized (mXWalkSettingsLock) {
            mZoomChangeListener = zoomChangeListener;
        }
    }

    private void onGestureZoomSupportChanged(
            final boolean supportsDoubleTapZoom, final boolean supportsMultiTouchZoom) {
        // Always post asynchronously here, to avoid doubling back onto the caller.
        mEventHandler.maybePostOnUiThread(new Runnable() {
            @Override
            public void run() {
                synchronized (mXWalkSettingsLock) {
                    if (mZoomChangeListener == null) return;
                    mZoomChangeListener.onGestureZoomSupportChanged(
                            supportsDoubleTapZoom, supportsMultiTouchZoom);
                }
            }
        });
    }

    @CalledByNative
    private boolean supportsDoubleTapZoomLocked() {
        assert Thread.holdsLock(mXWalkSettingsLock);
        return mSupportZoom && mBuiltInZoomControls && mUseWideViewport;
    }

    private boolean supportsMultiTouchZoomLocked() {
        assert Thread.holdsLock(mXWalkSettingsLock);
        return mSupportZoom && mBuiltInZoomControls;
    }

    /**
     * Sets whether the XWalkView should support zooming using its on-screen zoom controls
     * and gestures. The particular zoom mechanisms that should be used can be set with
     * setBuiltInZoomControls(boolean). This setting does not affect zooming performed
     * using the zoomIn() and zoomOut() methods. The default is true.
     * @param support whether the XWalkView should support zoom.
     * @since 6.0
     */
    @XWalkAPI
    public void setSupportZoom(boolean support) {
        synchronized (mXWalkSettingsLock) {
            if (mSupportZoom == support) return;
            mSupportZoom = support;
            onGestureZoomSupportChanged(
                    supportsDoubleTapZoomLocked(), supportsMultiTouchZoomLocked());
        }
    }

    /**
     * Gets whether the XWalkView supports zoom.
     * @return true if the XWalkView supports zoom.
     * @since 6.0
     */
    @XWalkAPI
    public boolean supportZoom() {
        synchronized (mXWalkSettingsLock) {
            return mSupportZoom;
        }
    }

    /**
     * Sets whether the XWalkView should use its built-in zoom mechanisms.
     * The built-in zoom mechanisms comprise on-screen zoom controls, which are
     * displayed over the XWalkView's content, and the use of a pinch gesture to
     * control zooming. Whether or not these on-screen controls are displayed
     * can be set with setDisplayZoomControls(boolean). The default is false.
     * @param enabled whether the XWalkView should use its built-in zoom mechanisms.
     * @since 6.0
     */
    @XWalkAPI
    public void setBuiltInZoomControls(boolean enabled) {
        synchronized (mXWalkSettingsLock) {
            if (mBuiltInZoomControls == enabled) return;
            mBuiltInZoomControls = enabled;
            onGestureZoomSupportChanged(
                    supportsDoubleTapZoomLocked(), supportsMultiTouchZoomLocked());
        }
    }

    /**
     * Gets whether the zoom mechanisms built into XWalkView are being used.
     * @return true if the zoom mechanisms built into XWalkView are being used.
     * @since 6.0
     */
    @XWalkAPI
    public boolean getBuiltInZoomControls() {
        synchronized (mXWalkSettingsLock) {
            return mBuiltInZoomControls;
        }
    }

    /**
     * Note: Just for test case.
     * Gets whether the XWalkView supports multi touch zoom.
     * @return true if the XWalkView supports multi touch zoom.
     */
    @XWalkAPI
    public boolean supportsMultiTouchZoomForTest() {
        synchronized (mXWalkSettingsLock) {
            return supportsMultiTouchZoomLocked();
        }
    }

    /**
     * Sets whether the XWalkView should support the spatial navigation,
     * like a TV remote control.
     * @param enable whether the XWalkView should support the spatial navigation.
     * @since 6.0
     */
    @XWalkAPI
    public void setSupportSpatialNavigation(boolean enable) {
        synchronized (mXWalkSettingsLock) {
            if (mSpatialNavigationEnabled == enable) return;
            mSpatialNavigationEnabled = enable;
            mEventHandler.updateWebkitPreferencesLocked();
        }
    }

    /**
     * Gets whether the XWalkView should support the spatial navigation.
     * @return true if XWalkView support the spatial navigation.
     * @since 6.0
     */
    @XWalkAPI
    public boolean getSupportSpatialNavigation() {
        synchronized (mXWalkSettingsLock) {
            return mSpatialNavigationEnabled;
        }
    }

    /**
     * Sets whether the XWalkView should support the quirks mode.
     * @param enable whether the XWalkView should support the quirks mode.
     * @since 6.0
     */
    @XWalkAPI
    public void setSupportQuirksMode(boolean enable) {
        synchronized (mXWalkSettingsLock) {
            if (mQuirksModeEnabled == enable) return;
            mQuirksModeEnabled = enable;
            mEventHandler.updateWebkitPreferencesLocked();
        }
    }

    /**
     * Gets whether the XWalkView should support the quirks mode.
     * @return true if XWalkView supports the quirks mode.
     * @since 6.0
     */
    @XWalkAPI
    public boolean getSupportQuirksMode() {
        synchronized (mXWalkSettingsLock) {
            return mQuirksModeEnabled;
        }
    }

    private native long nativeInit(WebContents webContents);

    private native void nativeDestroy(long nativeXWalkSettings);

    private static native String nativeGetDefaultUserAgent();

    private native void nativeUpdateEverythingLocked(long nativeXWalkSettings);

    private native void nativeUpdateUserAgent(long nativeXWalkSettings);

    private native void nativeUpdateWebkitPreferences(long nativeXWalkSettings);

    private native void nativeUpdateAcceptLanguages(long nativeXWalkSettings);

    private native void nativeUpdateFormDataPreferences(long nativeXWalkSettings);

    private native void nativeUpdateInitialPageScale(long nativeXWalkSettings);
}
