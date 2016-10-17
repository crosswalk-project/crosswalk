// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.graphics.Rect;
import android.net.http.SslCertificate;
import android.os.Build.VERSION;
import android.os.Build.VERSION_CODES;
import android.os.Bundle;
import android.view.View;
import android.view.WindowManager;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.util.Base64;
import android.util.Log;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.MotionEvent;
import android.view.SurfaceView;
import android.view.ViewGroup;
import android.view.View.OnTouchListener;
import android.webkit.ValueCallback;
import android.webkit.WebResourceResponse;
import android.widget.FrameLayout;

import java.io.IOException;
import java.io.InputStream;
import java.lang.annotation.Annotation;
import java.util.Arrays;
import java.util.Locale;
import java.util.Map;

import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;
import org.chromium.base.ThreadUtils;
import org.chromium.components.navigation_interception.InterceptNavigationDelegate;
import org.chromium.content.browser.ContentView;
import org.chromium.content.browser.ContentViewCore;
import org.chromium.content.browser.ContentViewRenderView;
import org.chromium.content.browser.ContentViewRenderView.CompositingSurfaceType;
import org.chromium.content.browser.ContentViewStatics;
import org.chromium.content.common.CleanupReference;
import org.chromium.content_public.browser.ContentBitmapCallback;
import org.chromium.content_public.browser.JavaScriptCallback;
import org.chromium.content_public.browser.LoadUrlParams;
import org.chromium.content_public.browser.NavigationHistory;
import org.chromium.content_public.browser.NavigationController;
import org.chromium.content_public.browser.WebContents;
import org.chromium.content_public.browser.navigation_controller.UserAgentOverrideOption;
import org.chromium.media.MediaPlayerBridge;
import org.chromium.ui.base.ActivityWindowAndroid;
import org.chromium.ui.base.WindowAndroid;
import org.chromium.ui.gfx.DeviceDisplayInfo;
import org.json.JSONArray;

@JNINamespace("xwalk")
/**
 * This class is the implementation class for XWalkViewInternal by calling internal
 * various classes.
 */
class XWalkContent implements XWalkPreferencesInternal.KeyValueChangeListener {
    private static String TAG = "XWalkContent";
    private static Class<? extends Annotation> javascriptInterfaceClass = null;

    private ContentViewCore mContentViewCore;
    private Context mViewContext;
    private XWalkContentView mContentView;
    private ContentViewRenderView mContentViewRenderView;
    private WindowAndroid mWindow;
    private XWalkDevToolsServer mDevToolsServer;
    private XWalkViewInternal mXWalkView;
    private XWalkContentsClientBridge mContentsClientBridge;
    private XWalkContentsIoThreadClient mIoThreadClient;
    private XWalkWebContentsDelegateAdapter mXWalkContentsDelegateAdapter;
    private XWalkSettingsInternal mSettings;
    private XWalkGeolocationPermissions mGeolocationPermissions;
    private XWalkLaunchScreenManager mLaunchScreenManager;
    private NavigationController mNavigationController;
    private WebContents mWebContents;
    private boolean mIsLoaded = false;
    private boolean mAnimated = false;
    private XWalkAutofillClientAndroid mXWalkAutofillClient;
    private XWalkGetBitmapCallbackInternal mXWalkGetBitmapCallbackInternal;
    private ContentBitmapCallback mGetBitmapCallback;
    private final HitTestData mPossiblyStaleHitTestData = new HitTestData();

    long mNativeContent;

    public static class HitTestData {
        // Used in getHitTestResult
        public int hitTestResultType;
        public String hitTestResultExtraData;

        // Used in requestFocusNodeHref(all three) and requestImageRef(only imageSrc)
        public String href;
        public String anchorText;
        public String imgSrc;
    }

    // TODO(hengzhi.wu): This should be in a global context, not per XWalkView.
    private double mDIPScale;

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

    public XWalkContent(Context context, String animatable, XWalkViewInternal xwView) {
        // Initialize the WebContensDelegate.
        mXWalkView = xwView;
        mViewContext = mXWalkView.getContext();
        mContentsClientBridge = new XWalkContentsClientBridge(mXWalkView);
        mXWalkContentsDelegateAdapter = new XWalkWebContentsDelegateAdapter(
            mContentsClientBridge);
        mIoThreadClient = new XWalkIoThreadClientImpl();

        // Initialize mWindow which is needed by content.
        mWindow = WindowAndroid.activityFromContext(context) != null ?
                new ActivityWindowAndroid(context) : new WindowAndroid(context);

        SharedPreferences sharedPreferences = new InMemorySharedPreferences();
        mGeolocationPermissions = new XWalkGeolocationPermissions(sharedPreferences);

        MediaPlayerBridge.setResourceLoadingFilter(
                new XWalkMediaPlayerResourceLoadingFilter());

        setNativeContent(nativeInit(), animatable);

        XWalkPreferencesInternal.load(this);
        initCaptureBitmapAsync();
    }

    private void initCaptureBitmapAsync() {
        mGetBitmapCallback = new ContentBitmapCallback() {
            @Override
            public void onFinishGetBitmap(Bitmap bitmap, int response) {
                if (mXWalkGetBitmapCallbackInternal == null) return;
                mXWalkGetBitmapCallbackInternal.onFinishGetBitmap(bitmap, response);
            }
        };
    }

    public void captureBitmapAsync(XWalkGetBitmapCallbackInternal callback) {
        if (mNativeContent == 0) return;
        mXWalkGetBitmapCallbackInternal = callback;
        mWebContents.getContentBitmapAsync(Bitmap.Config.ARGB_8888, 1.0f,
            new Rect(), mGetBitmapCallback);
    }

    private void setNativeContent(long newNativeContent, String animatable) {
        if (mNativeContent != 0) {
            destroy();
            mContentViewCore = null;
        }

        assert mNativeContent == 0 && mCleanupReference == null && mContentViewCore == null;

        // Initialize ContentViewRenderView
        if(animatable == null)
            mAnimated = XWalkPreferencesInternal.getValue(
                    XWalkPreferencesInternal.ANIMATABLE_XWALK_VIEW);
        else
            mAnimated = animatable.equalsIgnoreCase("true");
        CompositingSurfaceType surfaceType =
                mAnimated ? CompositingSurfaceType.TEXTURE_VIEW : CompositingSurfaceType.SURFACE_VIEW;
        Log.d(TAG, "CompositingSurfaceType is " + (mAnimated ? "TextureView" : "SurfaceView"));
        mContentViewRenderView = new ContentViewRenderView(mViewContext, surfaceType) {
            protected void onReadyToRender() {
                // Anything depending on the underlying Surface readiness should
                // be placed here.
            }
        };
        mContentViewRenderView.onNativeLibraryLoaded(mWindow);
        mLaunchScreenManager = new XWalkLaunchScreenManager(mViewContext, mXWalkView);
        mContentViewRenderView.registerFirstRenderedFrameListener(mLaunchScreenManager);
        mXWalkView.addView(mContentViewRenderView, new FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.MATCH_PARENT));

        mNativeContent = newNativeContent;

        // The native side object has been bound to this java instance, so now is the time to
        // bind all the native->java relationships.
        mCleanupReference = new CleanupReference(this, new DestroyRunnable(mNativeContent));

        mWebContents = nativeGetWebContents(mNativeContent);

        // Initialize ContentView.
        mContentViewCore = new ContentViewCore(mViewContext);
        mContentView = XWalkContentView.createContentView(
                mViewContext, mContentViewCore, mXWalkView);
        mContentViewCore.initialize(mContentView, mContentView, mWebContents, mWindow);
        mNavigationController = mWebContents.getNavigationController();
        mXWalkView.addView(mContentView, new FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.MATCH_PARENT));
        mContentViewCore.setContentViewClient(mContentsClientBridge);
        mContentViewRenderView.setCurrentContentViewCore(mContentViewCore);
        // For addJavascriptInterface
        mContentsClientBridge.installWebContentsObserver(mWebContents);

        // Set the third argument isAccessFromFileURLsGrantedByDefault to false, so that
        // the members mAllowUniversalAccessFromFileURLs and mAllowFileAccessFromFileURLs
        // won't be changed from false to true at the same time in the constructor of
        // XWalkSettings class.
        mSettings = new XWalkSettingsInternal(mViewContext, mWebContents, false);
        // Enable AllowFileAccessFromFileURLs, so that files under file:// path could be
        // loaded by XMLHttpRequest.
        mSettings.setAllowFileAccessFromFileURLs(true);

        // Set DIP scale.
        mDIPScale = DeviceDisplayInfo.create(mViewContext).getDIPScale();
        mContentsClientBridge.setDIPScale(mDIPScale);
        mSettings.setDIPScale(mDIPScale);

        String language = Locale.getDefault().toString().replaceAll("_", "-").toLowerCase();
        if (language.isEmpty()) language = "en";
        mSettings.setAcceptLanguages(language);

        XWalkSettingsInternal.ZoomSupportChangeListener zoomListener =
                new XWalkSettingsInternal.ZoomSupportChangeListener() {
                    @Override
                    public void onGestureZoomSupportChanged(
                            boolean supportsDoubleTapZoom, boolean supportsMultiTouchZoom) {
                        mContentViewCore.updateDoubleTapSupport(supportsDoubleTapZoom);
                        mContentViewCore.updateMultiTouchZoomSupport(supportsMultiTouchZoom);
                    }
                };
        mSettings.setZoomListener(zoomListener);

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
        setNativeContent(popupNativeXWalkContents, null);

        mContentViewCore.onShow();
    }

    private void doLoadUrl(LoadUrlParams params) {
        params.setOverrideUserAgent(UserAgentOverrideOption.TRUE);
        mNavigationController.loadUrl(params);
        mContentView.requestFocus();
        mIsLoaded = true;
    }

    private static String fixupBase(String url) {
        return TextUtils.isEmpty(url) ? "about:blank" : url;
    }

    private static String fixupData(String data) {
        return TextUtils.isEmpty(data) ? "" : data;
    }

    private static String fixupHistory(String url) {
        return TextUtils.isEmpty(url) ? "about:blank" : url;
    }

    private static String fixupMimeType(String mimeType) {
        return TextUtils.isEmpty(mimeType) ? "text/html" : mimeType;
    }

    private static boolean isBase64Encoded(String encoding) {
        return "base64".equals(encoding);
    }

    public void loadData(String data, String mimeType, String encoding) {
        if (mNativeContent == 0) return;

        data = TextUtils.isEmpty(data) ? "" : data;
        mimeType = TextUtils.isEmpty(mimeType) ? "text/html" : mimeType;
        doLoadUrl(LoadUrlParams.createLoadDataParams(
                  fixupData(data), fixupMimeType(mimeType),isBase64Encoded(encoding)));
    }

    public void loadDataWithBaseURL(
            String baseUrl, String data, String mimeType, String encoding, String historyUrl) {
        if (mNativeContent == 0) return;

        data = fixupData(data);
        mimeType = fixupMimeType(mimeType);
        LoadUrlParams loadUrlParams;
        baseUrl = fixupBase(baseUrl);
        historyUrl = fixupHistory(historyUrl);

        // When loading data with a non-data: base URL, the classic WebView would effectively
        // "dump" that string of data into the WebView without going through regular URL
        // loading steps such as decoding URL-encoded entities. We achieve this same behavior by
        // base64 encoding the data that is passed here and then loading that as a data: URL.
        try {
            loadUrlParams = LoadUrlParams.createLoadDataParamsWithBaseUrl(
                    Base64.encodeToString(data.getBytes("utf-8"), Base64.DEFAULT), mimeType,
                    true, baseUrl, historyUrl, "utf-8");
        } catch (java.io.UnsupportedEncodingException e) {
            Log.w(TAG, "Unable to load data string " + data, e);
            return;
        }
        doLoadUrl(loadUrlParams);
    }

    public void loadUrl(String url) {
        // Early out to match the old classic Android WebView's implementation.
        if (url == null) {
            return;
        }
        loadUrl(url, null);
    }

    public void loadUrl(String url, Map<String, String> additionalHttpHeaders) {
        if (mNativeContent == 0) return;
        LoadUrlParams params = new LoadUrlParams(url);
        if (additionalHttpHeaders != null) params.setExtraHeaders(additionalHttpHeaders);
        doLoadUrl(params);
    }

    public void reload(int mode) {
        if (mNativeContent == 0) return;

        switch (mode) {
            case XWalkViewInternal.RELOAD_IGNORE_CACHE:
                mNavigationController.reloadBypassingCache(true);
                break;
            case XWalkViewInternal.RELOAD_NORMAL:
            default:
                mNavigationController.reload(true);
        }
        mIsLoaded = true;
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

    public void removeJavascriptInterface(String name) {
        if (mNativeContent == 0) return;
        mContentViewCore.removeJavascriptInterface(name);
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

    public int getContentHeight() {
        return (int)Math.ceil(mContentViewCore.getContentHeightCss());
    }

    public void setXWalkClient(XWalkClient client) {
        if (mNativeContent == 0) return;
        mContentsClientBridge.setXWalkClient(client);
    }

    public void setDownloadListener(XWalkDownloadListenerInternal listener) {
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

    public boolean onNewIntent(Intent intent) {
        if (mNativeContent == 0) return false;
        return mContentsClientBridge.onNewIntent(intent);
    }

    public void clearCache(boolean includeDiskFiles) {
        if (mNativeContent == 0) return;
        nativeClearCache(mNativeContent, includeDiskFiles);
    }

    public void clearCacheForSingleFile(final String url) {
        if (mNativeContent == 0) return;
        if (mIsLoaded == false) {
            mXWalkView.post(new Runnable() {
                @Override
                public void run() {
                    clearCacheForSingleFile(url);
                }
            });
            return;
        }
        nativeClearCacheForSingleFile(mNativeContent, url);
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

    public Bitmap getFavicon() {
        if (mNativeContent == 0) return null;
        return mContentsClientBridge.getFavicon();
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

    public HitTestData getLastHitTestResult() {
        if (mNativeContent == 0) return null;
        nativeUpdateLastHitTestData(mNativeContent);
        return mPossiblyStaleHitTestData;
    }

    public String getXWalkVersion() {
        if (mNativeContent == 0) return "";
        return nativeGetVersion(mNativeContent);
    }

    private boolean isOpaque(int color) {
        return ((color >> 24) & 0xFF) == 0xFF;
    }

    @CalledByNative
    public void setBackgroundColor(final int color) {
        if (mNativeContent == 0) return;
        if (!mIsLoaded) {
            mXWalkView.post(new Runnable() {
                @Override
                public void run() {
                    setBackgroundColor(color);
                }
            });
            return;
        }
        if (isOpaque(color)) {
            setOverlayVideoMode(false);
            mContentViewCore.setBackgroundOpaque(true);
        } else {
            setOverlayVideoMode(true);
            mContentViewCore.setBackgroundOpaque(false);
        }
        mContentViewCore.setBackgroundColor(color);
        mContentViewRenderView.setSurfaceViewBackgroundColor(color);
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

    public XWalkSettingsInternal getSettings() {
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
                content = AndroidProtocolHandler.getUrlContent(mXWalkView.getContext(), url);
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
        mIsLoaded = true;
    }

    public void setOriginAccessWhitelist(String url, String[] patterns) {
        if (mNativeContent == 0 || TextUtils.isEmpty(url)) return;

        // Reset origin access whitelists if pattern is null.
        String matchPatterns = "";
        if (patterns != null) {
            JSONArray arrays = new JSONArray(Arrays.asList(patterns));
            matchPatterns = arrays.toString();
        }

        nativeSetOriginAccessWhitelist(mNativeContent, url, matchPatterns);
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
        if (result) mContentsClientBridge.onUpdateTitle(mWebContents.getTitle());


        return result ? getNavigationHistory() : null;
    }

    boolean hasEnteredFullscreen() {
        return mContentsClientBridge.hasEnteredFullscreen();
    }

    void exitFullscreen() {
        if (hasEnteredFullscreen()) {
            mWebContents.exitFullscreen();
        }
    }

    @CalledByNative
    public void onGetUrlFromManifest(String url) {
        if (url != null && !url.isEmpty()) {
            loadUrl(url);
        }
    }

    @CalledByNative
    public void onGetUrlAndLaunchScreenFromManifest(String url, String readyWhen, String imageBorder) {
        if (url == null || url.isEmpty()) return;
        mLaunchScreenManager.displayLaunchScreen(readyWhen, imageBorder);
        mContentsClientBridge.registerPageLoadListener(mLaunchScreenManager);
        loadUrl(url);
    }

    @CalledByNative
    public void onGetFullscreenFlagFromManifest(boolean enterFullscreen) {
        if (!(mXWalkView.getContext() instanceof Activity)) return;

        Activity activity = (Activity) mXWalkView.getContext();
        if (enterFullscreen) {
            if (VERSION.SDK_INT >= VERSION_CODES.KITKAT) {
                View decorView = activity.getWindow().getDecorView();
                decorView.setSystemUiVisibility(
                        View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
                        View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
                        View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
                        View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
                        View.SYSTEM_UI_FLAG_FULLSCREEN |
                        View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
            } else {
                activity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
            }
        }
    }

    public void destroy() {
        if (mNativeContent == 0) return;

        XWalkPreferencesInternal.unload(this);
        // Reset existing notification service in order to destruct it.
        setNotificationService(null);
        // Remove its children used for page rendering from view hierarchy.
        mXWalkView.removeView(mContentView);
        mXWalkView.removeView(mContentViewRenderView);
        mContentViewRenderView.setCurrentContentViewCore(null);

        // Destroy the native resources.
        mCleanupReference.cleanupNow();
        mContentViewRenderView.destroy();
        mContentViewCore.destroy();

        mCleanupReference = null;
        mNativeContent = 0;
    }

    public int getRoutingID() {
        return nativeGetRoutingID(mNativeContent);
    }

    public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        return mContentView.onCreateInputConnectionSuper(outAttrs);
    }

    public boolean onTouchEvent(MotionEvent event) {
        if (event.getActionMasked() == MotionEvent.ACTION_DOWN) {
            // Note this will trigger IPC back to browser even if nothing is
            // hit.
            nativeRequestNewHitTestDataAt(mNativeContent,
                    event.getX() / (float) mDIPScale,
                    event.getY() / (float) mDIPScale,
                    event.getTouchMajor() / (float) mDIPScale);
        }
        return mContentViewCore.onTouchEvent(event);
    }

    public void setOnTouchListener(OnTouchListener l) {
        mContentView.setOnTouchListener(l);
    }

    public void scrollTo(int x, int y) {
        mContentView.scrollTo(x, y);
    }

    public void scrollBy(int x, int y) {
        mContentView.scrollBy(x, y);
    }

    public int computeHorizontalScrollRange() {
        return mContentView.computeHorizontalScrollRangeDelegate();
    }

    public int computeHorizontalScrollOffset() {
        return mContentView.computeHorizontalScrollOffsetDelegate();
    }

    public int computeVerticalScrollRange() {
        return mContentView.computeVerticalScrollRangeDelegate();
    }

    public int computeVerticalScrollOffset() {
        return mContentView.computeVerticalScrollOffsetDelegate();
    }

    public int computeVerticalScrollExtent() {
        return mContentView.computeVerticalScrollExtentDelegate();
    }

    //--------------------------------------------------------------------------------------------
    private class XWalkIoThreadClientImpl extends XWalkContentsIoThreadClient {
        // All methods are called on the IO thread.

        @Override
        public int getCacheMode() {
            return mSettings.getCacheMode();
        }

        @Override
        public XWalkWebResourceResponseInternal shouldInterceptRequest(
                XWalkContentsClient.WebResourceRequestInner request) {

            // Notify a resource load is started. This is not the best place to start the callback
            // but it's a workable way.
            mContentsClientBridge.getCallbackHelper().postOnResourceLoadStarted(request.url);

            XWalkWebResourceResponseInternal xwalkWebResourceResponse =
                    mContentsClientBridge.shouldInterceptRequest(request);

            if (xwalkWebResourceResponse == null) {
                mContentsClientBridge.getCallbackHelper().postOnLoadResource(request.url);
            } else {
                if (request.isMainFrame && xwalkWebResourceResponse.getData() == null) {
                    mContentsClientBridge.getCallbackHelper().postOnReceivedError(
                            XWalkResourceClientInternal.ERROR_UNKNOWN, null, request.url);
                }
            }
            return xwalkWebResourceResponse;
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

        @Override
        public void onReceivedResponseHeaders(XWalkContentsClient.WebResourceRequestInner request,
                XWalkWebResourceResponseInternal response) {
            mContentsClientBridge.getCallbackHelper().postOnReceivedResponseHeaders(request, response);
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

    @CalledByNative
    private void updateHitTestData(
            int type, String extra, String href, String anchorText, String imgSrc) {
        mPossiblyStaleHitTestData.hitTestResultType = type;
        mPossiblyStaleHitTestData.hitTestResultExtraData = extra;
        mPossiblyStaleHitTestData.href = href;
        mPossiblyStaleHitTestData.anchorText = anchorText;
        mPossiblyStaleHitTestData.imgSrc = imgSrc;
    }

    public void enableRemoteDebugging() {
        // Chrome looks for "devtools_remote" pattern in the name of a unix domain socket
        // to identify a debugging page
        final String socketName = mViewContext.getApplicationContext().getPackageName() + "_devtools_remote";
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
        } else if (key.equals(XWalkPreferencesInternal.SPATIAL_NAVIGATION)) {
            if (mSettings != null) {
                mSettings.setSupportSpatialNavigation(value.getBooleanValue());
            }
        }
    }

    public void setOverlayVideoMode(boolean enabled) {
        if (mContentViewRenderView != null) {
            mContentViewRenderView.setOverlayVideoMode(enabled);
        }
    }

    public void setZOrderOnTop(boolean onTop) {
        if (mContentViewRenderView == null) return;
        mContentViewRenderView.setZOrderOnTop(onTop);
    }

    public boolean zoomIn() {
        if (mNativeContent == 0) return false;
        return mContentViewCore.zoomIn();
    }

    public boolean zoomOut() {
        if (mNativeContent == 0) return false;
        return mContentViewCore.zoomOut();
    }

    public void zoomBy(float delta) {
        if (mNativeContent == 0) return;
        if (delta < 0.01f || delta > 100.0f) {
            throw new IllegalStateException("zoom delta value outside [0.01, 100] range.");
        }
        mContentViewCore.pinchByDelta(delta);
    }

    public boolean canZoomIn() {
        if (mNativeContent == 0) return false;
        return mContentViewCore.canZoomIn();
    }

    public boolean canZoomOut() {
        if (mNativeContent == 0) return false;
        return mContentViewCore.canZoomOut();
    }

    /**
     * @see android.webkit.WebView#clearFormData()
     */
    public void hideAutofillPopup() {
        if (mNativeContent == 0) return;
        if (mIsLoaded == false) {
            mXWalkView.post(new Runnable() {
                @Override
                public void run() {
                    hideAutofillPopup();
                }
            });
            return;
        }

        if (mXWalkAutofillClient != null) {
            mXWalkAutofillClient.hideAutofillPopup();
        }
    }

    // It is only used for SurfaceView.
    public void setVisibility(int visibility) {
        SurfaceView surfaceView = mContentViewRenderView.getSurfaceView();
        if (surfaceView == null) return;
        surfaceView.setVisibility(visibility);
    }

    @CalledByNative
    private void setXWalkAutofillClient(XWalkAutofillClientAndroid client) {
        mXWalkAutofillClient = client;
        client.init(mContentViewCore);
    }

    public void clearSslPreferences() {
        if (mNativeContent == 0) return;
        mNavigationController.clearSslPreferences();
    }

    public void clearClientCertPreferences(Runnable callback) {
        if (mNativeContent == 0) return;
        mContentsClientBridge.clearClientCertPreferences(callback);
    }

    public SslCertificate getCertificate() {
        if (mNativeContent == 0) return null;
        return SslUtil.getCertificateFromDerBytes(nativeGetCertificate(mNativeContent));
    }

    public boolean hasPermission(final String permission) {
        if (mNativeContent == 0) return false;
        return mWindow.hasPermission(permission);
    }

    public void setFindListener(XWalkFindListenerInternal listener) {
        if (mNativeContent == 0) return;
        mContentsClientBridge.setFindListener(listener);
    }

    public void findAllAsync(String searchString) {
        if (mNativeContent == 0) return;
        nativeFindAllAsync(mNativeContent, searchString);
    }

    public void findNext(boolean forward) {
        if (mNativeContent == 0) return;
        nativeFindNext(mNativeContent, forward);
    }

    public void clearMatches() {
        if (mNativeContent == 0) return;
        nativeClearMatches(mNativeContent);
    }

    public String getCompositingSurfaceType() {
        if (mNativeContent == 0) return null;
        return mAnimated ? "TextureView" : "SurfaceView";
    }

    @CalledByNative
    public void onFindResultReceived(int activeMatchOrdinal, int numberOfMatches,
            boolean isDoneCounting) {
        mContentsClientBridge.onFindResultReceived(activeMatchOrdinal, numberOfMatches,
                isDoneCounting);
    }

    private native long nativeInit();
    private static native void nativeDestroy(long nativeXWalkContent);
    private native WebContents nativeGetWebContents(long nativeXWalkContent);
    private native long nativeReleasePopupXWalkContent(long nativeXWalkContent);
    private native void nativeSetJavaPeers(
            long nativeXWalkContent,
            XWalkContent xwalkContent,
            XWalkWebContentsDelegateAdapter xwalkContentsDelegate,
            XWalkContentsClientBridge contentsClientBridge,
            XWalkContentsIoThreadClient ioThreadClient,
            InterceptNavigationDelegate navigationInterceptionDelegate);
    private native void nativeClearCache(long nativeXWalkContent, boolean includeDiskFiles);
    private native void nativeClearCacheForSingleFile(long nativeXWalkContent, String url);
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
    private native void nativeSetOriginAccessWhitelist(
            long nativeXWalkContent, String url, String patterns);
    private native void nativeRequestNewHitTestDataAt(long nativeXWalkContent, float x, float y,
            float touchMajor);
    private native void nativeUpdateLastHitTestData(long nativeXWalkContent);
    private native byte[] nativeGetCertificate(long nativeXWalkContent);
    private native void nativeFindAllAsync(long nativeXWalkContent, String searchString);
    private native void nativeFindNext(long nativeXWalkContent, boolean forward);
    private native void nativeClearMatches(long nativeXWalkContent);
}
