// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
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
import android.view.ViewGroup;
import android.widget.FrameLayout;

import org.chromium.base.CalledByNative;
import org.chromium.base.JNINamespace;
import org.chromium.base.ThreadUtils;
import org.chromium.components.navigation_interception.InterceptNavigationDelegate;
import org.chromium.content.browser.ContentVideoView;
import org.chromium.content.browser.ContentView;
import org.chromium.content.browser.ContentViewCore;
import org.chromium.content.browser.ContentViewRenderView;
import org.chromium.content.browser.ContentViewStatics;
import org.chromium.content.browser.LoadUrlParams;
import org.chromium.content.browser.NavigationHistory;
import org.chromium.ui.WindowAndroid;

@JNINamespace("xwalk")
/**
 * This class is the implementation class for XWalkView by calling internal
 * various classes.
 */
public class XWalkContent extends FrameLayout {
    private ContentViewCore mContentViewCore;
    private ContentView mContentView;
    private ContentViewRenderView mContentViewRenderView;
    private WindowAndroid mWindow;
    private XWalkView mXWalkView;
    private XWalkContentsClientBridge mContentsClientBridge;
    private XWalkWebContentsDelegateAdapter mXWalkContentsDelegateAdapter;
    private XWalkSettings mSettings;
    private XWalkGeolocationPermissions mGeolocationPermissions;

    int mXWalkContent;
    int mWebContents;
    boolean mReadyToLoad = false;
    String mPendingUrl = null;

    public XWalkContent(Context context, AttributeSet attrs, XWalkView xwView) {
        super(context, attrs);

        // Initialize the WebContensDelegate.
        mXWalkView = xwView;
        mContentsClientBridge = new XWalkContentsClientBridge(mXWalkView);
        mXWalkContentsDelegateAdapter = new XWalkWebContentsDelegateAdapter(
            mContentsClientBridge);

        // Initialize ContentViewRenderView
        mContentViewRenderView = new ContentViewRenderView(context) {
            protected void onReadyToRender() {
                if (mPendingUrl != null) {
                    doLoadUrl(mPendingUrl);
                    mPendingUrl = null;
                }

                mReadyToLoad = true;
            }
        };
        addView(mContentViewRenderView,
                new FrameLayout.LayoutParams(
                        FrameLayout.LayoutParams.MATCH_PARENT,
                        FrameLayout.LayoutParams.MATCH_PARENT));

        mXWalkContent = nativeInit(mXWalkContentsDelegateAdapter, mContentsClientBridge);
        mWebContents = nativeGetWebContents(mXWalkContent,
                mContentsClientBridge.getInterceptNavigationDelegate());

        // Initialize mWindow which is needed by content
        mWindow = new WindowAndroid(xwView.getActivity());

        // Initialize ContentView.
        mContentView = ContentView.newInstance(getContext(), mWebContents, mWindow);
        addView(mContentView,
                new FrameLayout.LayoutParams(
                        FrameLayout.LayoutParams.MATCH_PARENT,
                        FrameLayout.LayoutParams.MATCH_PARENT));
        mContentView.setContentViewClient(mContentsClientBridge);

        mContentViewRenderView.setCurrentContentView(mContentView);

        // For addJavascriptInterface
        mContentViewCore = mContentView.getContentViewCore();
        mContentsClientBridge.installWebContentsObserver(mContentViewCore);

        mContentView.setDownloadDelegate(mContentsClientBridge);

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
    }

    void doLoadUrl(String url) {
        //TODO(Xingnan): Configure appropriate parameters here.
        // Handle the same url loading by parameters.
        if (TextUtils.equals(url, mContentView.getUrl())) {
            mContentView.reload();
        } else {
            mContentView.loadUrl(new LoadUrlParams(url));
        }

        mContentView.requestFocus();
    }

    public void loadUrl(String url) {
        if (url == null)
            return;

        if (mReadyToLoad)
            doLoadUrl(url);
        else
            mPendingUrl = url;
    }

    public void reload() {
        if (mReadyToLoad) {
            mContentView.reload();
        }
    }

    public String getUrl() {
        String url = mContentView.getUrl();
        if (url == null || url.trim().isEmpty()) return null;
        return url;
    }

    public String getTitle() {
        String title = mContentView.getTitle().trim();
        if (title == null) title = "";
        return title;
    }

    public void addJavascriptInterface(Object object, String name) {
        mContentViewCore.addJavascriptInterface(object, name);
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

    public void onPause() {
        mContentView.onHide();
    }

    public void onResume() {
        mContentView.onShow();
    }

    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        mWindow.onActivityResult(requestCode, resultCode, data);
    }

    public void clearCache(boolean includeDiskFiles) {
        if (mXWalkContent == 0) return;
        nativeClearCache(mXWalkContent, includeDiskFiles);
    }

    public void clearHistory() {
        mContentView.clearHistory();
    }

    public boolean canGoBack() {
        return mContentView.canGoBack();
    }

    public void goBack() {
        mContentView.goBack();
    }

    public boolean canGoForward() {
        return mContentView.canGoForward();
    }

    public void goForward() {
        mContentView.goForward();
    }

    public void stopLoading() {
        mContentView.stopLoading();
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

    public String getVersion() {
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

    public void loadAppFromManifest(String path, String manifest) {
        if (path == null || manifest == null || mXWalkContent == 0) {
            return;
        }

        if (!nativeSetManifest(mXWalkContent, path, manifest)) {
            throw new RuntimeException("Failed to parse the manifest file.");
        }
    }

    public WebBackForwardList copyBackForwardList() {
        return new WebBackForwardList(mContentViewCore.getNavigationHistory());
    }

    public static final String SAVE_RESTORE_STATE_KEY = "XWALKVIEW_STATE";

    public WebBackForwardList saveState(Bundle outState) {
        if (outState == null) return null;

        byte[] state = nativeGetState(mXWalkContent);
        if (state == null) return null;

        outState.putByteArray(SAVE_RESTORE_STATE_KEY, state);
        return copyBackForwardList();
    }

    public WebBackForwardList restoreState(Bundle inState) {
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

        return result ? copyBackForwardList() : null;
    }

    @CalledByNative
    public void onGetUrlFromManifest(String url) {
        if (url != null && !url.isEmpty()) {
            loadUrl(url);
        }
    }

    public void destroy() {
        if (mXWalkContent == 0) return;

        // Remove its children used for page rendering from view hierarchy.
        removeView(mContentView);
        removeView(mContentViewRenderView);
        mContentViewRenderView.setCurrentContentView(null);

        // Destroy the native resources.
        mContentViewRenderView.destroy();
        mContentView.destroy();

        nativeDestroy(mXWalkContent);
        mXWalkContent = 0;
    }

    public int getRoutingID() {
        return nativeGetRoutingID(mXWalkContent);
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

    private native int nativeInit(XWalkWebContentsDelegate webViewContentsDelegate,
            XWalkContentsClientBridge bridge);
    private static native void nativeDestroy(int nativeXWalkContent);
    private native int nativeGetWebContents(int nativeXWalkContent,
            InterceptNavigationDelegate delegate);
    private native void nativeClearCache(int nativeXWalkContent, boolean includeDiskFiles);
    private native String nativeDevToolsAgentId(int nativeXWalkContent);
    private native String nativeGetVersion(int nativeXWalkContent);
    private native void nativeSetJsOnlineProperty(int nativeXWalkContent, boolean networkUp);
    private native boolean nativeSetManifest(int nativeXWalkContent, String path, String manifest);
    private native int nativeGetRoutingID(int nativeXWalkContent);
    private native void nativeInvokeGeolocationCallback(
            int nativeXWalkContent, boolean value, String requestingFrame);
    private native byte[] nativeGetState(int nativeXWalkContent);
    private native boolean nativeSetState(int nativeXWalkContent, byte[] state);
}
