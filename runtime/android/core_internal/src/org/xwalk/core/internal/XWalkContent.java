// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Rect;
import android.os.Build.VERSION;
import android.os.Build.VERSION_CODES;
import android.os.Bundle;
import android.view.View;
import android.view.WindowManager;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.util.Log;
import android.view.ViewGroup;
import android.webkit.ValueCallback;
import android.webkit.WebResourceResponse;
import android.widget.FrameLayout;

import java.io.IOException;
import java.io.InputStream;
import java.lang.annotation.Annotation;

import org.chromium.base.CalledByNative;
import org.chromium.base.JNINamespace;
import org.chromium.base.ThreadUtils;
import org.chromium.components.navigation_interception.InterceptNavigationDelegate;
import org.chromium.content.browser.ContentView;
import org.chromium.content.browser.ContentViewCore;
import org.chromium.content.browser.ContentViewRenderView;
import org.chromium.content.browser.ContentViewRenderView.CompositingSurfaceType;
import org.chromium.content.browser.ContentViewStatics;
import org.chromium.content.common.CleanupReference;
import org.chromium.content_public.browser.JavaScriptCallback;
import org.chromium.content_public.browser.LoadUrlParams;
import org.chromium.content_public.browser.NavigationHistory;
import org.chromium.content_public.browser.NavigationController;
import org.chromium.content_public.browser.WebContents;
import org.chromium.media.MediaPlayerBridge;
import org.chromium.ui.base.ActivityWindowAndroid;
import org.chromium.ui.gfx.DeviceDisplayInfo;

@JNINamespace("xwalk")
/**
 * This class is the implementation class for XWalkViewInternal by calling internal
 * various classes.
 */
class XWalkContent extends FrameLayout implements XWalkPreferencesInternal.KeyValueChangeListener {
    private static String TAG = "XWalkContent";
    private static Class<? extends Annotation> javascriptInterfaceClass = null;

    private ContentViewCore mContentViewCore;
    private ContentView mContentView;
    private ContentViewRenderView mContentViewRenderView;
    private ActivityWindowAndroid mWindow;
    private XWalkDevToolsServer mDevToolsServer;
    private XWalkViewInternal mXWalkView;
    private XWalkContentsClientBridge mContentsClientBridge;
    private XWalkContentsIoThreadClient mIoThreadClient;
    private XWalkWebContentsDelegateAdapter mXWalkContentsDelegateAdapter;
    private XWalkSettings mSettings;
    private XWalkGeolocationPermissions mGeolocationPermissions;
    private XWalkLaunchScreenManager mLaunchScreenManager;
    private NavigationController mNavigationController;
    private WebContents mWebContents;

    long mNativeContent;
    long mNativeWebContents;

    static void setJavascriptInterfaceClass(Class<? extends Annotation> clazz) {
      assert(javascriptInterfaceClass == null);
      javascriptInterfaceClass = clazz;
    }

    private static final class DestroyRunnable implements Runnable {
        private final long mNativeContent;
        private DestroyRunnable(long nativeXWalkContent) {
            mNativeContent = nativeXWalkContent;
        }

        @Override
        public void run() {
            nativeDestroy(mNativeContent);
        }
    }

    // Reference to the active mNativeContent pointer while it is active use
    // (ie before it is destroyed).
    private CleanupReference mCleanupReference;

    public XWalkContent(Context context, AttributeSet attrs, XWalkViewInternal xwView) {
        super(context, attrs);

        // Initialize the WebContensDelegate.
        mXWalkView = xwView;
        mContentsClientBridge = new XWalkContentsClientBridge(mXWalkView);
        mXWalkContentsDelegateAdapter = new XWalkWebContentsDelegateAdapter(
            mContentsClientBridge);
        mIoThreadClient = new XWalkIoThreadClientImpl();

        // Initialize mWindow which is needed by content
        mWindow = new ActivityWindowAndroid(xwView.getActivity());

        SharedPreferences sharedPreferences = new InMemorySharedPreferences();
        mGeolocationPermissions = new XWalkGeolocationPermissions(sharedPreferences);

        MediaPlayerBridge.setResourceLoadingFilter(
                new XWalkMediaPlayerResourceLoadingFilter());

        setNativeContent(nativeInit());

        XWalkPreferencesInternal.load(this);
    }

    private void setNativeContent(long newNativeContent) {
        if (mNativeContent != 0) {
            destroy();
            mContentViewCore = null;
        }

        assert mNativeContent == 0 && mCleanupReference == null && mContentViewCore == null;

        // Initialize ContentViewRenderView
        boolean animated = XWalkPreferencesInternal.getValue(
                XWalkPreferencesInternal.ANIMATABLE_XWALK_VIEW);
        CompositingSurfaceType surfaceType =
                animated ? CompositingSurfaceType.TEXTURE_VIEW : CompositingSurfaceType.SURFACE_VIEW;
        mContentViewRenderView = new ContentViewRenderView(getContext(), surfaceType) {
            protected void onReadyToRender() {
                // Anything depending on the underlying Surface readiness should
                // be placed here.
            }
        };
        mContentViewRenderView.onNativeLibraryLoaded(mWindow);
        mLaunchScreenManager = new XWalkLaunchScreenManager(getContext(), mXWalkView);
        mContentViewRenderView.registerFirstRenderedFrameListener(mLaunchScreenManager);
        addView(mContentViewRenderView, new FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.MATCH_PARENT));

        mNativeContent = newNativeContent;

        // The native side object has been bound to this java instance, so now is the time to
        // bind all the native->java relationships.
        mCleanupReference = new CleanupReference(this, new DestroyRunnable(mNativeContent));

        mNativeWebContents = nativeGetWebContents(mNativeContent);


        // Initialize ContentView.
        mContentViewCore = new ContentViewCore(getContext());
        mContentView = ContentView.newInstance(getContext(), mContentViewCore);
        mContentViewCore.initialize(mContentView, mContentView, mNativeWebContents, mWindow);
        mWebContents = mContentViewCore.getWebContents();
        mNavigationController = mWebContents.getNavigationController();
        addView(mContentView, new FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.MATCH_PARENT));
        mContentViewCore.setContentViewClient(mContentsClientBridge);
        mContentViewRenderView.setCurrentContentViewCore(mContentViewCore);
        // For addJavascriptInterface
        mContentsClientBridge.installWebContentsObserver(mWebContents);

        // Set DIP scale.
        mContentsClientBridge.setDIPScale(DeviceDisplayInfo.create(getContext()).getDIPScale());

        mContentViewCore.setDownloadDelegate(mContentsClientBridge);

        // Set the third argument isAccessFromFileURLsGrantedByDefault to false, so that
        // the members mAllowUniversalAccessFromFileURLs and mAllowFileAccessFromFileURLs
        // won't be changed from false to true at the same time in the constructor of
        // XWalkSettings class.
        mSettings = new XWalkSettings(getContext(), mNativeWebContents, false);
        // Enable AllowFileAccessFromFileURLs, so that files under file:// path could be
        // loaded by XMLHttpRequest.
        mSettings.setAllowFileAccessFromFileURLs(true);
        // Enable this by default to suppport new window creation
        mSettings.setSupportMultipleWindows(true);

        nativeSetJavaPeers(mNativeContent, this, mXWalkContentsDelegateAdapter, mContentsClientBridge,
                mIoThreadClient, mContentsClientBridge.getInterceptNavigationDelegate());
    }

    public void supplyContentsForPopup(XWalkContent newContents) {
        if (mNativeContent == 0) return;

        long popupNativeXWalkContent = nativeReleasePopupXWalkContent(mNativeContent);
        if (popupNativeXWalkContent == 0) {
            Log.w(TAG, "Popup XWalkView bind failed: no pending content.");
            if (newContents != null) newContents.destroy();
            return;
        }
        if (newContents == null) {
            nativeDestroy(popupNativeXWalkContent);
            return;
        }

        newContents.receivePopupContents(popupNativeXWalkContent);
    }

    private void receivePopupContents(long popupNativeXWalkContents) {
        setNativeContent(popupNativeXWalkContents);

        mContentViewCore.onShow();
    }

    void doLoadUrl(String url, String content) {
        // Handle the same url loading by parameters.
        if (url != null && !url.isEmpty() &&
                TextUtils.equals(url, mWebContents.getUrl())) {
            mNavigationController.reload(true);
        } else {
            LoadUrlParams params = null;
            if (content == null || content.isEmpty()) {
                params = new LoadUrlParams(url);
            } else {
                params = LoadUrlParams.createLoadDataParamsWithBaseUrl(
                        content, "text/html", false, url, null);
            }
            params.setOverrideUserAgent(LoadUrlParams.UA_OVERRIDE_TRUE);
            mNavigationController.loadUrl(params);
        }

        mContentView.requestFocus();
    }

    public void loadUrl(String url, String data) {
        if (mNativeContent == 0) return;

        if ((url == null || url.isEmpty()) &&
                (data == null || data.isEmpty())) {
            return;
        }

        doLoadUrl(url, data);
    }

    public void reload(int mode) {
        if (mNativeContent == 0) return;

        switch (mode) {
            case XWalkViewInternal.RELOAD_IGNORE_CACHE:
                mNavigationController.reloadIgnoringCache(true);
                break;
            case XWalkViewInternal.RELOAD_NORMAL:
            default:
                mNavigationController.reload(true);
        }
    }

    public String getUrl() {
        if (mNativeContent == 0) return null;
        String url = mWebContents.getUrl();
        if (url == null || url.trim().isEmpty()) return null;
        return url;
    }

    public String getTitle() {
        if (mNativeContent == 0) return null;
        String title = mWebContents.getTitle().trim();
        if (title == null) title = "";
        return title;
    }

    public void addJavascriptInterface(Object object, String name) {
        if (mNativeContent == 0) return;
        mContentViewCore.addPossiblyUnsafeJavascriptInterface(object, name,
                javascriptInterfaceClass);
    }

    public void evaluateJavascript(String script, ValueCallback<String> callback) {
        if (mNativeContent == 0) return;
        final ValueCallback<String>  fCallback = callback;
        JavaScriptCallback coreCallback = null;
        if (fCallback != null) {
            coreCallback = new JavaScriptCallback() {
                @Override
                public void handleJavaScriptResult(String jsonResult) {
                    fCallback.onReceiveValue(jsonResult);
                }
            };
        }
        mContentViewCore.getWebContents().evaluateJavaScript(script, coreCallback);
    }

    public void setUIClient(XWalkUIClientInternal client) {
        if (mNativeContent == 0) return;
        mContentsClientBridge.setUIClient(client);
    }

    public void setResourceClient(XWalkResourceClientInternal client) {
        if (mNativeContent == 0) return;
        mContentsClientBridge.setResourceClient(client);
    }

    public void setXWalkWebChromeClient(XWalkWebChromeClient client) {
        if (mNativeContent == 0) return;
        mContentsClientBridge.setXWalkWebChromeClient(client);
    }

    public XWalkWebChromeClient getXWalkWebChromeClient() {
        if (mNativeContent == 0) return null;
        return mContentsClientBridge.getXWalkWebChromeClient();
    }

    public void setXWalkClient(XWalkClient client) {
        if (mNativeContent == 0) return;
        mContentsClientBridge.setXWalkClient(client);
    }

    public void setDownloadListener(DownloadListener listener) {
        if (mNativeContent == 0) return;
        mContentsClientBridge.setDownloadListener(listener);
    }

    public void setNavigationHandler(XWalkNavigationHandler handler) {
        if (mNativeContent == 0) return;
        mContentsClientBridge.setNavigationHandler(handler);
    }

    public void setNotificationService(XWalkNotificationService service) {
        if (mNativeContent == 0) return;
        mContentsClientBridge.setNotificationService(service);
    }

    public void onPause() {
        if (mNativeContent == 0) return;
        mContentViewCore.onHide();
    }

    public void onResume() {
        if (mNativeContent == 0) return;
        mContentViewCore.onShow();
    }

    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (mNativeContent == 0) return;
        mWindow.onActivityResult(requestCode, resultCode, data);
    }

    public boolean onNewIntent(Intent intent) {
        if (mNativeContent == 0) return false;
        return mContentsClientBridge.onNewIntent(intent);
    }

    public void clearCache(boolean includeDiskFiles) {
        if (mNativeContent == 0) return;
        nativeClearCache(mNativeContent, includeDiskFiles);
    }

    public void clearHistory() {
        if (mNativeContent == 0) return;
        mNavigationController.clearHistory();
    }

    public boolean canGoBack() {
        return (mNativeContent == 0) ? false : mNavigationController.canGoBack();
    }

    public void goBack() {
        if (mNativeContent == 0) return;
        mNavigationController.goBack();
    }

    public boolean canGoForward() {
        return (mNativeContent == 0) ? false : mNavigationController.canGoForward();
    }

    public void goForward() {
        if (mNativeContent == 0) return;
        mNavigationController.goForward();
    }

    void navigateTo(int offset)  {
        mNavigationController.goToOffset(offset);
    }

    public void stopLoading() {
        if (mNativeContent == 0) return;
        mWebContents.stop();
        mContentsClientBridge.onStopLoading();
    }

    // Currently, timer pause/resume is actually
    // a global setting. And multiple pause will fail the
    // DCHECK in content (content_view_statics.cc:57).
    // Here uses a static boolean to avoid this issue.
    private static boolean timerPaused = false;

    // TODO(Guangzhen): ContentViewStatics will be removed in upstream,
    // details in content_view_statics.cc.
    // We need follow up after upstream updates that.
    public void pauseTimers() {
        if (timerPaused || (mNativeContent == 0)) return;
        ContentViewStatics.setWebKitSharedTimersSuspended(true);
        timerPaused = true;
    }

    public void resumeTimers() {
        if (!timerPaused || (mNativeContent == 0)) return;
        ContentViewStatics.setWebKitSharedTimersSuspended(false);
        timerPaused = false;
    }

    public String getOriginalUrl() {
        if (mNativeContent == 0) return null;
        NavigationHistory history = mNavigationController.getNavigationHistory();
        int currentIndex = history.getCurrentEntryIndex();
        if (currentIndex >= 0 && currentIndex < history.getEntryCount()) {
            return history.getEntryAtIndex(currentIndex).getOriginalUrl();
        }
        return null;
    }

    public String getXWalkVersion() {
        if (mNativeContent == 0) return "";
        return nativeGetVersion(mNativeContent);
    }

    public void setBackgroundColor(int color) {
        if (mNativeContent == 0) return;
        nativeSetBackgroundColor(mNativeContent, color);
    }

    public void setNetworkAvailable(boolean networkUp) {
        if (mNativeContent == 0) return;
        nativeSetJsOnlineProperty(mNativeContent, networkUp);
    }

    // For instrumentation test.
    public ContentViewCore getContentViewCoreForTest() {
        return mContentViewCore;
    }

    // For instrumentation test.
    public void installWebContentsObserverForTest(XWalkContentsClient contentClient) {
        if (mNativeContent == 0) return;
        contentClient.installWebContentsObserver(mContentViewCore.getWebContents());
    }

    public String devToolsAgentId() {
        if (mNativeContent == 0) return "";
        return nativeDevToolsAgentId(mNativeContent);
    }

    public XWalkSettings getSettings() {
        return mSettings;
    }

    public void loadAppFromManifest(String url, String data) {
        if (mNativeContent == 0 ||
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

        if (!nativeSetManifest(mNativeContent, baseUrl, content)) {
            throw new RuntimeException("Failed to parse the manifest file: " + url);
        }
    }

    public XWalkNavigationHistoryInternal getNavigationHistory() {
        if (mNativeContent == 0) return null;

        return new XWalkNavigationHistoryInternal(mXWalkView, mNavigationController.getNavigationHistory());
    }

    public static final String SAVE_RESTORE_STATE_KEY = "XWALKVIEW_STATE";

    public XWalkNavigationHistoryInternal saveState(Bundle outState) {
        if (mNativeContent == 0 || outState == null) return null;

        byte[] state = nativeGetState(mNativeContent);
        if (state == null) return null;

        outState.putByteArray(SAVE_RESTORE_STATE_KEY, state);
        return getNavigationHistory();
    }

    public XWalkNavigationHistoryInternal restoreState(Bundle inState) {
        if (mNativeContent == 0 || inState == null) return null;

        byte[] state = inState.getByteArray(SAVE_RESTORE_STATE_KEY);
        if (state == null) return null;

        boolean result = nativeSetState(mNativeContent, state);

        // The onUpdateTitle callback normally happens when a page is loaded,
        // but is optimized out in the restoreState case because the title is
        // already restored. See WebContentsImpl::UpdateTitleForEntry. So we
        // call the callback explicitly here.
        if (result) {
            mContentsClientBridge.onUpdateTitle(mWebContents.getTitle());
        }

        return result ? getNavigationHistory() : null;
    }

    boolean hasEnteredFullscreen() {
        return mContentsClientBridge.hasEnteredFullscreen();
    }

    void exitFullscreen() {
        if (hasEnteredFullscreen()) {
            mContentsClientBridge.exitFullscreen(mNativeWebContents);
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
        if (enterFullscreen) {
            if (VERSION.SDK_INT >= VERSION_CODES.KITKAT) {
                View decorView = mXWalkView.getActivity().getWindow().getDecorView();
                decorView.setSystemUiVisibility(
                        View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
                        View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
                        View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
                        View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
                        View.SYSTEM_UI_FLAG_FULLSCREEN |
                        View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
            } else {
                mXWalkView.getActivity().getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
            }
        }
    }

    public void destroy() {
        if (mNativeContent == 0) return;

        XWalkPreferencesInternal.unload(this);
        // Reset existing notification service in order to destruct it.
        setNotificationService(null);
        // Remove its children used for page rendering from view hierarchy.
        removeView(mContentView);
        removeView(mContentViewRenderView);
        mContentViewRenderView.setCurrentContentViewCore(null);

        // Destroy the native resources.
        mContentViewRenderView.destroy();
        mContentViewCore.destroy();

        mCleanupReference.cleanupNow();
        mCleanupReference = null;
        mNativeContent = 0;
    }

    public int getRoutingID() {
        return nativeGetRoutingID(mNativeContent);
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
                            XWalkResourceClientInternal.ERROR_UNKNOWN, null, url);
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
                    nativeInvokeGeolocationCallback(mNativeContent, allow, origin);
                }
            });
        }
    }

    @CalledByNative
    private void onGeolocationPermissionsShowPrompt(String origin) {
        if (mNativeContent == 0) return;
        // Reject if geolocation is disabled, or the origin has a retained deny.
        if (!mSettings.getGeolocationEnabled()) {
            nativeInvokeGeolocationCallback(mNativeContent, false, origin);
            return;
        }
        // Allow if the origin has a retained allow.
        if (mGeolocationPermissions.hasOrigin(origin)) {
            nativeInvokeGeolocationCallback(mNativeContent,
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

    public void enableRemoteDebugging() {
        // Chrome looks for "devtools_remote" pattern in the name of a unix domain socket
        // to identify a debugging page
        final String socketName = getContext().getApplicationContext().getPackageName() + "_devtools_remote";
        if (mDevToolsServer == null) {
            mDevToolsServer = new XWalkDevToolsServer(socketName);
            mDevToolsServer.setRemoteDebuggingEnabled(
                    true, XWalkDevToolsServer.Security.ALLOW_SOCKET_ACCESS);
        }
    }

    void disableRemoteDebugging() {
        if (mDevToolsServer ==  null) return;

        if (mDevToolsServer.isRemoteDebuggingEnabled()) {
            mDevToolsServer.setRemoteDebuggingEnabled(false);
        }
        mDevToolsServer.destroy();
        mDevToolsServer = null;
    }

    public String getRemoteDebuggingUrl() {
        if (mDevToolsServer == null) return "";
        // devtools/page is hardcoded in devtools_http_handler_impl.cc (kPageUrlPrefix)
        return "ws://" + mDevToolsServer.getSocketName() + "/devtools/page/" + devToolsAgentId();
    }

    @Override
    public void onKeyValueChanged(String key, XWalkPreferencesInternal.PreferenceValue value) {
        if (key == null) return;
        if (key.equals(XWalkPreferencesInternal.REMOTE_DEBUGGING)) {
            if (value.getBooleanValue()) enableRemoteDebugging();
            else disableRemoteDebugging();
        } else if (key.equals(XWalkPreferencesInternal.ENABLE_JAVASCRIPT)) {
            if (mSettings != null) {
                mSettings.setJavaScriptEnabled(value.getBooleanValue());
            }
        } else if (key.equals(XWalkPreferencesInternal.JAVASCRIPT_CAN_OPEN_WINDOW)) {
            if (mSettings != null) {
                mSettings.setJavaScriptCanOpenWindowsAutomatically(value.getBooleanValue());
            }
        } else if (key.equals(XWalkPreferencesInternal.ALLOW_UNIVERSAL_ACCESS_FROM_FILE)) {
            if (mSettings != null) {
                mSettings.setAllowUniversalAccessFromFileURLs(value.getBooleanValue());
            }
        } else if (key.equals(XWalkPreferencesInternal.SUPPORT_MULTIPLE_WINDOWS)) {
            if (mSettings != null) {
                mSettings.setSupportMultipleWindows(value.getBooleanValue());
            }
        }
    }

    public void setOverlayVideoMode(boolean enabled) {
        if (mContentViewRenderView != null) {
            mContentViewRenderView.setOverlayVideoMode(enabled);
        }
    }

    private native long nativeInit();
    private static native void nativeDestroy(long nativeXWalkContent);
    private native long nativeGetWebContents(long nativeXWalkContent);
    private native long nativeReleasePopupXWalkContent(long nativeXWalkContent);
    private native void nativeSetJavaPeers(
            long nativeXWalkContent,
            XWalkContent xwalkContent,
            XWalkWebContentsDelegateAdapter xwalkContentsDelegate,
            XWalkContentsClientBridge contentsClientBridge,
            XWalkContentsIoThreadClient ioThreadClient,
            InterceptNavigationDelegate navigationInterceptionDelegate);
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
    private native void nativeSetBackgroundColor(long nativeXWalkContent, int color);
}
