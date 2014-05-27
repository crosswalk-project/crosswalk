// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Rect;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.util.Log;
import android.view.ViewGroup;
import android.webkit.ValueCallback;
import android.webkit.WebResourceResponse;
import android.widget.FrameLayout;

import java.io.IOException;
import java.io.InputStream;

import org.chromium.base.CalledByNative;
import org.chromium.base.JNINamespace;
import org.chromium.base.ThreadUtils;
import org.chromium.components.navigation_interception.InterceptNavigationDelegate;
import org.chromium.content.browser.ContentView;
import org.chromium.content.browser.ContentViewCore;
import org.chromium.content.browser.ContentViewRenderView;
import org.chromium.content.browser.ContentViewRenderView.CompositingSurfaceType;
import org.chromium.content.browser.ContentViewStatics;
import org.chromium.content.browser.LoadUrlParams;
import org.chromium.content.browser.NavigationHistory;
import org.chromium.media.MediaPlayerBridge;
import org.chromium.ui.base.ActivityWindowAndroid;

@JNINamespace("xwalk")
/**
 * This class is the implementation class for XWalkView by calling internal
 * various classes.
 */
class XWalkContent extends FrameLayout implements XWalkPreferences.KeyValueChangeListener {
    private static String TAG = "XWalkContent";
    private ContentViewCore mContentViewCore;
    private ContentView mContentView;
    private ContentViewRenderView mContentViewRenderView;
    private ActivityWindowAndroid mWindow;
    private XWalkDevToolsServer mDevToolsServer;
    private XWalkView mXWalkView;
    private XWalkContentsClientBridge mContentsClientBridge;
    private XWalkContentsIoThreadClient mIoThreadClient;
    private XWalkWebContentsDelegateAdapter mXWalkContentsDelegateAdapter;
    private XWalkSettings mSettings;
    private XWalkGeolocationPermissions mGeolocationPermissions;
    private XWalkLaunchScreenManager mLaunchScreenManager;

    long mXWalkContent;
    long mWebContents;
    boolean mReadyToLoad = false;
    String mPendingUrl = null;
    String mPendingData = null;

    public XWalkContent(Context context, AttributeSet attrs, XWalkView xwView) {
        super(context, attrs);

        // Initialize the WebContensDelegate.
        mXWalkView = xwView;
        mContentsClientBridge = new XWalkContentsClientBridge(mXWalkView);
        mXWalkContentsDelegateAdapter = new XWalkWebContentsDelegateAdapter(
            mContentsClientBridge);
        mIoThreadClient = new XWalkIoThreadClientImpl();

        // Initialize mWindow which is needed by content
        mWindow = new ActivityWindowAndroid(xwView.getActivity());

        // Initialize ContentViewRenderView
        boolean animated = XWalkPreferences.getValue(XWalkPreferences.ANIMATABLE_XWALK_VIEW);
        CompositingSurfaceType surfaceType =
                animated ? CompositingSurfaceType.TEXTURE_VIEW : CompositingSurfaceType.SURFACE_VIEW;
        mContentViewRenderView = new ContentViewRenderView(context, mWindow, surfaceType) {
            protected void onReadyToRender() {
                if (mPendingUrl != null) {
                    doLoadUrl(mPendingUrl, mPendingData);
                    mPendingUrl = null;
                    mPendingData = null;
                }

                mReadyToLoad = true;
            }
        };
        mLaunchScreenManager = new XWalkLaunchScreenManager(context, mXWalkView);
        mContentViewRenderView.registerFirstRenderedFrameListener(mLaunchScreenManager);
        addView(mContentViewRenderView,
                new FrameLayout.LayoutParams(
                        FrameLayout.LayoutParams.MATCH_PARENT,
                        FrameLayout.LayoutParams.MATCH_PARENT));

        mXWalkContent = nativeInit(mXWalkContentsDelegateAdapter, mContentsClientBridge);
        mWebContents = nativeGetWebContents(mXWalkContent, mIoThreadClient,
                mContentsClientBridge.getInterceptNavigationDelegate());

        // Initialize ContentView.
        mContentView = ContentView.newInstance(getContext(), mWebContents, mWindow);
        addView(mContentView,
                new FrameLayout.LayoutParams(
                        FrameLayout.LayoutParams.MATCH_PARENT,
                        FrameLayout.LayoutParams.MATCH_PARENT));
        mContentView.getContentViewCore().setContentViewClient(mContentsClientBridge);

        mContentViewCore = mContentView.getContentViewCore();
        mContentViewRenderView.setCurrentContentViewCore(mContentViewCore);
        // For addJavascriptInterface
        mContentsClientBridge.installWebContentsObserver(mContentViewCore);

        mContentViewCore.setDownloadDelegate(mContentsClientBridge);

        // Set the third argument isAccessFromFileURLsGrantedByDefault to false, so that
        // the members mAllowUniversalAccessFromFileURLs and mAllowFileAccessFromFileURLs
        // won't be changed from false to true at the same time in the constructor of
        // XWalkSettings class.
        mSettings = new XWalkSettings(getContext(), mWebContents, false);
        // Enable AllowFileAccessFromFileURLs, so that files under file:// path could be
        // loaded by XMLHttpRequest.
        mSettings.setAllowFileAccessFromFileURLs(true);

        SharedPreferences sharedPreferences = new InMemorySharedPreferences();
        mGeolocationPermissions = new XWalkGeolocationPermissions(sharedPreferences);

        MediaPlayerBridge.setResourceLoadingFilter(
                new XWalkMediaPlayerResourceLoadingFilter());

        XWalkPreferences.load(this);
    }

    void doLoadUrl(String url, String content) {
        // Handle the same url loading by parameters.
        if (url != null && !url.isEmpty() &&
                TextUtils.equals(url, mContentViewCore.getUrl())) {
            mContentViewCore.reload(true);
        } else {
            LoadUrlParams params = null;
            if (content == null || content.isEmpty()) {
                params = new LoadUrlParams(url);
            } else {
                params = LoadUrlParams.createLoadDataParamsWithBaseUrl(
                        content, "text/html", false, url, null);
            }
            params.setOverrideUserAgent(LoadUrlParams.UA_OVERRIDE_TRUE);
            mContentViewCore.loadUrl(params);
        }

        mContentView.requestFocus();
    }

    public void loadUrl(String url, String data) {
        if ((url == null || url.isEmpty()) &&
                (data == null || data.isEmpty())) {
            return;
        }

        if (mReadyToLoad) {
            doLoadUrl(url, data);
        } else {
            mPendingUrl = url;
            mPendingData = data;
        }
    }

    public void reload(int mode) {
        if (mReadyToLoad) {
            switch (mode) {
                case XWalkView.RELOAD_IGNORE_CACHE:
                    mContentViewCore.reloadIgnoringCache(true);
                    break;
                case XWalkView.RELOAD_NORMAL:
                default:
                    mContentViewCore.reload(true);

            }
        }
    }

    public String getUrl() {
        String url = mContentViewCore.getUrl();
        if (url == null || url.trim().isEmpty()) return null;
        return url;
    }

    public String getTitle() {
        String title = mContentViewCore.getTitle().trim();
        if (title == null) title = "";
        return title;
    }

    public void addJavascriptInterface(Object object, String name) {
        mContentViewCore.addPossiblyUnsafeJavascriptInterface(object, name,
                JavascriptInterface.class);
    }

    public void evaluateJavascript(String script, ValueCallback<String> callback) {
        final ValueCallback<String>  fCallback = callback;
        ContentViewCore.JavaScriptCallback coreCallback = new ContentViewCore.JavaScriptCallback() {
            @Override
            public void handleJavaScriptResult(String jsonResult) {
                fCallback.onReceiveValue(jsonResult);
            }
        };
        mContentViewCore.evaluateJavaScript(script, coreCallback);
    }

    public void setUIClient(XWalkUIClient client) {
        mContentsClientBridge.setUIClient(client);
    }

    public void setResourceClient(XWalkResourceClient client) {
        mContentsClientBridge.setResourceClient(client);
    }

    public void setXWalkWebChromeClient(XWalkWebChromeClient client) {
        mContentsClientBridge.setXWalkWebChromeClient(client);
    }

    public XWalkWebChromeClient getXWalkWebChromeClient() {
        return mContentsClientBridge.getXWalkWebChromeClient();
    }

    public void setXWalkClient(XWalkClient client) {
        mContentsClientBridge.setXWalkClient(client);
    }

    public void setDownloadListener(DownloadListener listener) {
        mContentsClientBridge.setDownloadListener(listener);
    }

    public void setNavigationHandler(XWalkNavigationHandler handler) {
        mContentsClientBridge.setNavigationHandler(handler);
    }

    public void setNotificationService(XWalkNotificationService service) {
        mContentsClientBridge.setNotificationService(service);
    }

    public void onPause() {
        mContentViewCore.onHide();
    }

    public void onResume() {
        mContentViewCore.onShow();
    }

    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        mWindow.onActivityResult(requestCode, resultCode, data);
    }

    public boolean onNewIntent(Intent intent) {
        return mContentsClientBridge.onNewIntent(intent);
    }

    public void clearCache(boolean includeDiskFiles) {
        if (mXWalkContent == 0) return;
        nativeClearCache(mXWalkContent, includeDiskFiles);
    }

    public void clearHistory() {
        mContentViewCore.clearHistory();
    }

    public boolean canGoBack() {
        return mContentViewCore.canGoBack();
    }

    public void goBack() {
        mContentViewCore.goBack();
    }

    public boolean canGoForward() {
        return mContentViewCore.canGoForward();
    }

    public void goForward() {
        mContentViewCore.goForward();
    }

    void navigateTo(int offset)  {
        mContentViewCore.goToOffset(offset);
    }

    public void stopLoading() {
        mContentViewCore.stopLoading();
    }

    // TODO(Guangzhen): ContentViewStatics will be removed in upstream,
    // details in content_view_statics.cc.
    // We need follow up after upstream updates that.
    public void pauseTimers() {
        ContentViewStatics.setWebKitSharedTimersSuspended(true);
    }

    public void resumeTimers() {
        ContentViewStatics.setWebKitSharedTimersSuspended(false);
    }

    public String getOriginalUrl() {
        NavigationHistory history = mContentViewCore.getNavigationHistory();
        int currentIndex = history.getCurrentEntryIndex();
        if (currentIndex >= 0 && currentIndex < history.getEntryCount()) {
            return history.getEntryAtIndex(currentIndex).getOriginalUrl();
        }
        return null;
    }

    public String getXWalkVersion() {
        if (mXWalkContent == 0) return "";
        return nativeGetVersion(mXWalkContent);
    }

    public void setNetworkAvailable(boolean networkUp) {
        if (mXWalkContent == 0) return;
        nativeSetJsOnlineProperty(mXWalkContent, networkUp);
    }

    // For instrumentation test.
    public ContentViewCore getContentViewCoreForTest() {
        return mContentViewCore;
    }

    // For instrumentation test.
    public void installWebContentsObserverForTest(XWalkContentsClient contentClient) {
        contentClient.installWebContentsObserver(mContentViewCore);
    }

    public String devToolsAgentId() {
        if (mXWalkContent == 0) return "";
        return nativeDevToolsAgentId(mXWalkContent);
    }

    public XWalkSettings getSettings() {
        return mSettings;
    }

    public void loadAppFromManifest(String url, String data) {
        if (mXWalkContent == 0 ||
                ((url == null || url.isEmpty()) &&
                        (data == null || data.isEmpty()))) {
            return;
        }

        String content = data;
        // If the data of manifest.json is not set, try to load it.
        if (data == null || data.isEmpty()) {
            try {
                content = AndroidProtocolHandler.getUrlContent(mXWalkView.getActivity(), url);
            } catch (IOException e) {
                throw new RuntimeException("Failed to read the manifest: " + url);
            }
        }

        // Calculate the base url of manifestUrl. Used by native side.
        // TODO(yongsheng): It's from runtime side. Need to find a better way
        // to get base url.
        String baseUrl = url;
        int position = url.lastIndexOf("/");
        if (position != -1) {
            baseUrl = url.substring(0, position + 1);
        } else {
            Log.w(TAG, "The url of manifest.json is probably not set correctly.");
        }

        if (!nativeSetManifest(mXWalkContent, baseUrl, content)) {
            throw new RuntimeException("Failed to parse the manifest file: " + url);
        }
    }

    public XWalkNavigationHistory getNavigationHistory() {
        return new XWalkNavigationHistory(mXWalkView, mContentViewCore.getNavigationHistory());
    }

    public static final String SAVE_RESTORE_STATE_KEY = "XWALKVIEW_STATE";

    public XWalkNavigationHistory saveState(Bundle outState) {
        if (outState == null) return null;

        byte[] state = nativeGetState(mXWalkContent);
        if (state == null) return null;

        outState.putByteArray(SAVE_RESTORE_STATE_KEY, state);
        return getNavigationHistory();
    }

    public XWalkNavigationHistory restoreState(Bundle inState) {
        if (inState == null) return null;

        byte[] state = inState.getByteArray(SAVE_RESTORE_STATE_KEY);
        if (state == null) return null;

        boolean result = nativeSetState(mXWalkContent, state);

        // The onUpdateTitle callback normally happens when a page is loaded,
        // but is optimized out in the restoreState case because the title is
        // already restored. See WebContentsImpl::UpdateTitleForEntry. So we
        // call the callback explicitly here.
        if (result) {
            mContentsClientBridge.onUpdateTitle(mContentViewCore.getTitle());
        }

        return result ? getNavigationHistory() : null;
    }

    boolean hasEnteredFullscreen() {
        return mContentsClientBridge.hasEnteredFullscreen();
    }

    void exitFullscreen() {
        if (hasEnteredFullscreen()) {
            mContentsClientBridge.exitFullscreen(mWebContents);
        }
    }

    @CalledByNative
    public void onGetUrlFromManifest(String url) {
        if (url != null && !url.isEmpty()) {
            loadUrl(url, null);
        }
    }

    @CalledByNative
    public void onGetUrlAndLaunchScreenFromManifest(String url, String readyWhen, String imageBorder) {
        if (url == null || url.isEmpty()) return;
        mLaunchScreenManager.displayLaunchScreen(readyWhen, imageBorder);
        mContentsClientBridge.registerPageLoadListener(mLaunchScreenManager);
        loadUrl(url, null);
    }

    @CalledByNative
    public void onGetFullscreenFlagFromManifest(boolean enterFullscreen) {
        if (enterFullscreen) mContentsClientBridge.onToggleFullscreen(true);
    }

    public void destroy() {
        if (mXWalkContent == 0) return;

        XWalkPreferences.unload(this);
        // Reset existing notification service in order to destruct it.
        setNotificationService(null);
        // Remove its children used for page rendering from view hierarchy.
        removeView(mContentView);
        removeView(mContentViewRenderView);
        mContentViewRenderView.setCurrentContentViewCore(null);

        // Destroy the native resources.
        mContentViewRenderView.destroy();
        mContentViewCore.destroy();

        nativeDestroy(mXWalkContent);
        mXWalkContent = 0;
    }

    public int getRoutingID() {
        return nativeGetRoutingID(mXWalkContent);
    }

    //--------------------------------------------------------------------------------------------
    private class XWalkIoThreadClientImpl implements XWalkContentsIoThreadClient {
        // All methods are called on the IO thread.

        @Override
        public int getCacheMode() {
            return mSettings.getCacheMode();
        }

        @Override
        public InterceptedRequestData shouldInterceptRequest(final String url,
                boolean isMainFrame) {

            // Notify a resource load is started. This is not the best place to start the callback
            // but it's a workable way.
            mContentsClientBridge.getCallbackHelper().postOnResourceLoadStarted(url);

            WebResourceResponse webResourceResponse = mContentsClientBridge.shouldInterceptRequest(url);
            InterceptedRequestData interceptedRequestData = null;

            if (webResourceResponse == null) {
                mContentsClientBridge.getCallbackHelper().postOnLoadResource(url);
            } else {
                if (isMainFrame && webResourceResponse.getData() == null) {
                    mContentsClientBridge.getCallbackHelper().postOnReceivedError(
                            XWalkResourceClient.ERROR_UNKNOWN, null, url);
                }
                interceptedRequestData = new InterceptedRequestData(webResourceResponse.getMimeType(),
                                                                    webResourceResponse.getEncoding(),
                                                                    webResourceResponse.getData());
            }
            return interceptedRequestData;
        }

        @Override
        public boolean shouldBlockContentUrls() {
            return !mSettings.getAllowContentAccess();
        }

        @Override
        public boolean shouldBlockFileUrls() {
            return !mSettings.getAllowFileAccess();
        }

        @Override
        public boolean shouldBlockNetworkLoads() {
            return mSettings.getBlockNetworkLoads();
        }

        @Override
        public void onDownloadStart(String url,
                                    String userAgent,
                                    String contentDisposition,
                                    String mimeType,
                                    long contentLength) {
            mContentsClientBridge.getCallbackHelper().postOnDownloadStart(url, userAgent,
                    contentDisposition, mimeType, contentLength);
        }

        @Override
        public void newLoginRequest(String realm, String account, String args) {
            mContentsClientBridge.getCallbackHelper().postOnReceivedLoginRequest(realm, account, args);
        }
    }

    private class XWalkGeolocationCallback implements XWalkGeolocationPermissions.Callback {
        @Override
        public void invoke(final String origin, final boolean allow, final boolean retain) {
            ThreadUtils.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if (retain) {
                        if (allow) {
                            mGeolocationPermissions.allow(origin);
                        } else {
                            mGeolocationPermissions.deny(origin);
                        }
                    }
                    nativeInvokeGeolocationCallback(mXWalkContent, allow, origin);
                }
            });
        }
    }

    @CalledByNative
    private void onGeolocationPermissionsShowPrompt(String origin) {
        // Reject if geolocation is disabled, or the origin has a retained deny.
        if (!mSettings.getGeolocationEnabled()) {
            nativeInvokeGeolocationCallback(mXWalkContent, false, origin);
            return;
        }
        // Allow if the origin has a retained allow.
        if (mGeolocationPermissions.hasOrigin(origin)) {
            nativeInvokeGeolocationCallback(mXWalkContent,
                    mGeolocationPermissions.isOriginAllowed(origin),
                    origin);
            return;
        }
        mContentsClientBridge.onGeolocationPermissionsShowPrompt(
                origin, new XWalkGeolocationCallback());
    }

    @CalledByNative
    public void onGeolocationPermissionsHidePrompt() {
        mContentsClientBridge.onGeolocationPermissionsHidePrompt();
    }

    public String enableRemoteDebugging(int allowedUid) {
        // Chrome looks for "devtools_remote" pattern in the name of a unix domain socket
        // to identify a debugging page
        final String socketName = getContext().getApplicationContext().getPackageName() + "_devtools_remote";
        if (mDevToolsServer == null) {
            mDevToolsServer = new XWalkDevToolsServer(socketName);
            mDevToolsServer.allowConnectionFromUid(allowedUid);
            mDevToolsServer.setRemoteDebuggingEnabled(true);
        }
        // devtools/page is hardcoded in devtools_http_handler_impl.cc (kPageUrlPrefix)
        return "ws://" + socketName + "/devtools/page/" + devToolsAgentId();
    }

    // Enables remote debugging and returns the URL at which the dev tools server is listening
    // for commands. Only the current process is allowed to connect to the server.
    String enableRemoteDebugging() {
        return enableRemoteDebugging(getContext().getApplicationInfo().uid);
    }

    void disableRemoteDebugging() {
        if (mDevToolsServer ==  null) return;

        if (mDevToolsServer.isRemoteDebuggingEnabled()) {
            mDevToolsServer.setRemoteDebuggingEnabled(false);
        }
        mDevToolsServer.destroy();
        mDevToolsServer = null;
    }

    @Override
    public void onKeyValueChanged(String key, boolean value) {
        if (key == XWalkPreferences.REMOTE_DEBUGGING) {
            if (value) enableRemoteDebugging();
            else disableRemoteDebugging();
        }
    }

    public void setOverlayVideoMode(boolean enabled) {
        if (mContentViewRenderView != null) {
            mContentViewRenderView.setOverlayVideoMode(enabled);
        }
    }

    private native long nativeInit(XWalkWebContentsDelegate webViewContentsDelegate,
            XWalkContentsClientBridge bridge);
    private static native void nativeDestroy(long nativeXWalkContent);
    private native long nativeGetWebContents(long nativeXWalkContent,
            XWalkContentsIoThreadClient ioThreadClient,
            InterceptNavigationDelegate delegate);
    private native void nativeClearCache(long nativeXWalkContent, boolean includeDiskFiles);
    private native String nativeDevToolsAgentId(long nativeXWalkContent);
    private native String nativeGetVersion(long nativeXWalkContent);
    private native void nativeSetJsOnlineProperty(long nativeXWalkContent, boolean networkUp);
    private native boolean nativeSetManifest(long nativeXWalkContent, String path, String manifest);
    private native int nativeGetRoutingID(long nativeXWalkContent);
    private native void nativeInvokeGeolocationCallback(
            long nativeXWalkContent, boolean value, String requestingFrame);
    private native byte[] nativeGetState(long nativeXWalkContent);
    private native boolean nativeSetState(long nativeXWalkContent, byte[] state);
}
