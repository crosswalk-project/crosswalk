// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Rect;
import android.os.Bundle;
import android.os.Looper;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.ViewGroup;
import android.webkit.WebSettings;
import android.widget.FrameLayout;

import org.xwalk.core.client.XWalkDefaultClient;
import org.xwalk.core.client.XWalkDefaultDownloadListener;
import org.xwalk.core.client.XWalkDefaultNavigationHandler;
import org.xwalk.core.client.XWalkDefaultNotificationService;
import org.xwalk.core.client.XWalkDefaultWebChromeClient;

public class XWalkView extends FrameLayout {

    private XWalkContent mContent;
    private XWalkDevToolsServer mDevToolsServer;
    private Activity mActivity;
    private Context mContext;

    public XWalkView(Context context, Activity activity) {
        super(context, null);

        checkThreadSafety();
        // Make sure mActivity is initialized before calling 'init' method.
        mActivity = activity;
        mContext = context;
        init(context, null);
    }

    public Activity getActivity() {
        if (mActivity != null) {
            return mActivity;
        } else if (getContext() instanceof Activity) {
            return (Activity)getContext();
        }

        // Never achieve here.
        assert(false);
        return null;
    }

    public Context getViewContext() {
        return mContext;
    }

    /**
     * Constructors for inflating via XML.
     */
    public XWalkView(Context context, AttributeSet attrs) {
        super(context, attrs);

        checkThreadSafety();
        mContext = context;
        init(context, attrs);
    }

    private void init(Context context, AttributeSet attrs) {
        // Initialize chromium resources. Assign them the correct ids in
        // xwalk core.
        XWalkInternalResources.resetIds(context);

        // Intialize library, paks and others.
        XWalkViewDelegate.init(this);

        initXWalkContent(context, attrs);
    }

    private void initXWalkContent(Context context, AttributeSet attrs) {
        mContent = new XWalkContent(context, attrs, this);
        addView(mContent,
                new FrameLayout.LayoutParams(
                        FrameLayout.LayoutParams.MATCH_PARENT,
                        FrameLayout.LayoutParams.MATCH_PARENT));


        // Set default XWalkDefaultClient.
        setXWalkClient(new XWalkDefaultClient(context, this));
        // Set default XWalkWebChromeClient and DownloadListener. The default actions
        // are provided via the following clients if special actions are not needed.
        setXWalkWebChromeClient(new XWalkDefaultWebChromeClient(context, this));
        setDownloadListener(new XWalkDefaultDownloadListener(context));
        setNavigationHandler(new XWalkDefaultNavigationHandler(context));
        setNotificationService(new XWalkDefaultNotificationService(context, this));
    }

    public void loadUrl(String url) {
        checkThreadSafety();
        mContent.loadUrl(url);
    }

    public void loadAppFromManifest(String path, String manifest) {
        mContent.loadAppFromManifest(path, manifest);
    }

    public void reload() {
        checkThreadSafety();
        mContent.reload();
    }

    public void addJavascriptInterface(Object object, String name) {
        checkThreadSafety();
        mContent.addJavascriptInterface(object, name);
    }

    public String getUrl() {
        checkThreadSafety();
        return mContent.getUrl();
    }

    public String getTitle() {
        checkThreadSafety();
        return mContent.getTitle();
    }

    public void clearCache(boolean includeDiskFiles) {
        checkThreadSafety();
        mContent.clearCache(includeDiskFiles);
    }

    public void clearHistory() {
        checkThreadSafety();
        mContent.clearHistory();
    }

    public boolean canGoBack() {
        checkThreadSafety();
        return mContent.canGoBack();
    }

    public void goBack() {
        checkThreadSafety();
        mContent.goBack();
    }

    public boolean canGoForward() {
        checkThreadSafety();
        return mContent.canGoForward();
    }

    public void goForward() {
        checkThreadSafety();
        mContent.goForward();
    }

    public boolean isFullscreen() {
        checkThreadSafety();
        return mContent.isFullscreen();
    }

    public void exitFullscreen() {
        checkThreadSafety();
        mContent.exitFullscreen();
    }

    public boolean requestFocus(int direction, Rect previouslyFocusedRect) {
        checkThreadSafety();
        return false;
    }

    public void setLayoutParams(ViewGroup.LayoutParams params) {
        checkThreadSafety();
        super.setLayoutParams(params);
    }

    public XWalkSettings getSettings() {
        checkThreadSafety();
        return mContent.getSettings();
    }

    public String getOriginalUrl() {
        checkThreadSafety();
        return mContent.getOriginalUrl();
    }

    public void setNetworkAvailable(boolean networkUp) {
        checkThreadSafety();
        mContent.setNetworkAvailable(networkUp);
    }

    public void setInitialScale(int scaleInPercent) {
        checkThreadSafety();
    }

    public void setXWalkWebChromeClient(XWalkWebChromeClient client) {
        checkThreadSafety();
        mContent.setXWalkWebChromeClient(client);
    }

    public void setXWalkClient(XWalkClient client) {
        checkThreadSafety();
        mContent.setXWalkClient(client);
    }

    public void stopLoading() {
        checkThreadSafety();
        mContent.stopLoading();
    }

    public void pauseTimers() {
        checkThreadSafety();
        mContent.pauseTimers();
    }

    public void resumeTimers() {
        checkThreadSafety();
        mContent.resumeTimers();
    }

    public void setDownloadListener(DownloadListener listener) {
        checkThreadSafety();
        mContent.setDownloadListener(listener);
    }

    public void setNavigationHandler(XWalkNavigationHandler handler) {
        checkThreadSafety();
        mContent.setNavigationHandler(handler);
    }

    public void setNotificationService(XWalkNotificationService service) {
        checkThreadSafety();
        mContent.setNotificationService(service);
    }

    // Enables remote debugging and returns the URL at which the dev tools server is listening
    // for commands. The allowedUid argument can be used to specify the uid of the process that is
    // permitted to connect.
    public String enableRemoteDebugging(int allowedUid) {
        checkThreadSafety();
        // Chrome looks for "devtools_remote" pattern in the name of a unix domain socket
        // to identify a debugging page
        final String socketName = getContext().getApplicationContext().getPackageName() + "_devtools_remote";
        if (mDevToolsServer == null) {
            mDevToolsServer = new XWalkDevToolsServer(socketName);
            mDevToolsServer.allowConnectionFromUid(allowedUid);
            mDevToolsServer.setRemoteDebuggingEnabled(true);
        }
        // devtools/page is hardcoded in devtools_http_handler_impl.cc (kPageUrlPrefix)
        return "ws://" + socketName + "/devtools/page/" + mContent.devToolsAgentId();
    }

    // Enables remote debugging and returns the URL at which the dev tools server is listening
    // for commands. Only the current process is allowed to connect to the server.
    public String enableRemoteDebugging() {
        return enableRemoteDebugging(mContext.getApplicationInfo().uid);
    }

    public void disableRemoteDebugging() {
        checkThreadSafety();
        if (mDevToolsServer ==  null) return;

        if (mDevToolsServer.isRemoteDebuggingEnabled()) {
            mDevToolsServer.setRemoteDebuggingEnabled(false);
        }
        mDevToolsServer.destroy();
        mDevToolsServer = null;
    }

    public void onPause() {
        mContent.onPause();
    }

    public void onResume() {
        mContent.onResume();
    }

    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            // If there's navigation happens when app is fullscreen,
            // the content will still be fullscreen after navigation.
            // In such case, the back key will exit fullscreen first.
            if (isFullscreen()) {
                exitFullscreen();
                return true;
            } else if (canGoBack()) {
                goBack();
                return true;
            }
        }
        return false;
    }

    public void onDestroy() {
        destroy();
    }

    public void destroy() {
        mContent.destroy();
        disableRemoteDebugging();
    }

    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        mContent.onActivityResult(requestCode, resultCode, data);
    }

    public boolean onNewIntent(Intent intent) {
        return mContent.onNewIntent(intent);
    }

    public String getVersion() {
        return mContent.getVersion();
    }

    public int getContentID() {
        return mContent.getRoutingID();
    }

    // TODO(shouqun): requestFocusFromTouch, setVerticalScrollBarEnabled are
    // from android.view.View;

    // For instrumentation test.
    public XWalkContent getXWalkViewContentForTest() {
        checkThreadSafety();
        return mContent;
    }

    public XWalkWebChromeClient getXWalkWebChromeClientForTest() {
        return mContent.getXWalkWebChromeClient();
    }

    public WebBackForwardList copyBackForwardList() {
        return mContent.copyBackForwardList();
    }

    public WebBackForwardList saveState(Bundle outState) {
        return mContent.saveState(outState);
    }

    public WebBackForwardList restoreState(Bundle inState) {
        return mContent.restoreState(inState);
    }

    private static void checkThreadSafety() {
        if (Looper.myLooper() != Looper.getMainLooper()) {
            Throwable throwable = new Throwable(
                "Warning: A XWalkView method was called on thread '" +
                Thread.currentThread().getName() + "'. " +
                "All XWalkView methods must be called on the UI thread. ");
            throw new RuntimeException(throwable);
        }
    }
}
