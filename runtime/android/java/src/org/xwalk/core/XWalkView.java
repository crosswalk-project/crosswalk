// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.view.ViewGroup;
import android.webkit.WebSettings;
import android.widget.FrameLayout;

import org.xwalk.core.client.XWalkDefaultClient;
import org.xwalk.core.client.XWalkDefaultDownloadListener;
import org.xwalk.core.client.XWalkDefaultNavigationHandler;
import org.xwalk.core.client.XWalkDefaultWebChromeClient;
import org.xwalk.runtime.XWalkManifestReader;

public class XWalkView extends FrameLayout {

    private XWalkContent mContent;
    private XWalkDevToolsServer mDevToolsServer;
    private Activity mActivity;
    private Context mContext;

    public XWalkView(Context context, Activity activity) {
        super(context, null);

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

        mContext = context;
        init(context, attrs);
    }

    private void init(Context context, AttributeSet attrs) {
        // Intialize library, paks and others.
        XWalkViewDelegate.init(context);

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
    }

    public void loadUrl(String url) {
        mContent.loadUrl(url);
    }

    public void loadAppFromManifest(String manifestUrl) {
        XWalkManifestReader manifestReader = new XWalkManifestReader(mActivity);
        String manifest = manifestReader.read(manifestUrl);
        int position = manifestUrl.lastIndexOf("/");
        if (position == -1) {
            throw new RuntimeException("The URL of manifest file is invalid.");
        }

        String path = manifestUrl.substring(0, position + 1);
        mContent.loadAppFromManifest(path, manifest);
    }

    public void reload() {
        mContent.reload();
    }

    public void addJavascriptInterface(Object object, String name) {
        mContent.addJavascriptInterface(object, name);
    }

    public String getUrl() {
        return mContent.getUrl();
    }

    public String getTitle() {
        return mContent.getTitle();
    }

    public void clearCache(boolean includeDiskFiles) {
        mContent.clearCache(includeDiskFiles);
    }

    public void clearHistory() {
        mContent.clearHistory();
    }

    public boolean canGoBack() {
        return mContent.canGoBack();
    }

    public void goBack() {
        mContent.goBack();
    }

    public boolean canGoForward() {
        return mContent.canGoForward();
    }

    public void goForward() {
        mContent.goForward();
    }

    public boolean requestFocus(int direction, Rect previouslyFocusedRect) {
        return false;
    }

    public void setLayoutParams(ViewGroup.LayoutParams params) {
        super.setLayoutParams(params);
    }

    public XWalkSettings getSettings() {
        return mContent.getSettings();
    }

    public String getOriginalUrl() {
        return mContent.getOriginalUrl();
    }

    public void setNetworkAvailable(boolean networkUp) {
        mContent.setNetworkAvailable(networkUp);
    }

    public void setInitialScale(int scaleInPercent) {
    }

    public void setXWalkWebChromeClient(XWalkWebChromeClient client) {
        mContent.setXWalkWebChromeClient(client);
    }

    public void setXWalkClient(XWalkClient client) {
        mContent.setXWalkClient(client);
    }

    public void stopLoading() {
        mContent.stopLoading();
    }

    public void pauseTimers() {
        mContent.pauseTimers();
    }

    public void resumeTimers() {
        mContent.resumeTimers();
    }

    public void setDownloadListener(DownloadListener listener) {
        mContent.setDownloadListener(listener);
    }

    public void setNavigationHandler(XWalkNavigationHandler handler) {
        mContent.setNavigationHandler(handler);
    }

    // Enables remote debugging and returns the URL at which the dev tools server is listening
    // for commands.
    public String enableRemoteDebugging() {
        // Chrome looks for "devtools_remote" pattern in the name of a unix domain socket
        // to identify a debugging page
        final String socketName = getContext().getApplicationContext().getPackageName() + "_devtools_remote";
        if (mDevToolsServer == null) {
            mDevToolsServer = XWalkDevToolsServer.getInstance(socketName);
            mDevToolsServer.setRemoteDebuggingEnabled(true);
        }
        // devtools/page is hardcoded in devtools_http_handler_impl.cc (kPageUrlPrefix)
        return "ws://" + socketName + "/devtools/page/" + mContent.devToolsAgentId();
    }

    public void disableRemoteDebugging() {
        if (mDevToolsServer ==  null) return;

        if (mDevToolsServer.isRemoteDebuggingEnabled()) {
            mDevToolsServer.setRemoteDebuggingEnabled(false);
        }
        mDevToolsServer = null;
    }

    public void onPause() {
        mContent.onPause();
    }

    public void onResume() {
        mContent.onResume();
    }

    public void onDestroy() {
        disableRemoteDebugging();
    }

    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        mContent.onActivityResult(requestCode, resultCode, data);
    }

    public String getVersion() {
        return mContent.getVersion();
    }

    // TODO(shouqun): requestFocusFromTouch, setVerticalScrollBarEnabled are
    // from android.view.View;

    // For instrumentation test.
    public XWalkContent getXWalkViewContentForTest() {
        return mContent;
    }
}
