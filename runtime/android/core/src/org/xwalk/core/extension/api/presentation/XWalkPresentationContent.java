// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.extension.api.presentation;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.view.View;

import org.xwalk.core.XWalkView;
import org.xwalk.core.XWalkResourceClient;
import org.xwalk.core.XWalkUIClient;

/**
 * Represents the content to be presented on the secondary display.
 */
public class XWalkPresentationContent {
    public final int INVALID_PRESENTATION_ID = -1;

    private int mPresentationId = INVALID_PRESENTATION_ID;
    private XWalkView mContentView;
    private Context mContext;
    private Activity mActivity;
    private PresentationDelegate mDelegate;

    private final XWalkResourceClient mXWalkResourceClient = new XWalkResourceClient() {
        @Override
        public void onLoadFinished(XWalkView view, String url) {
            mPresentationId = mContentView.getContentID();
            onContentLoaded();
        }
    };

    private final XWalkUIClient mXWalkUIClient = new XWalkUIClient() {
        @Override
        public void onJavascriptCloseWindow(XWalkView view) {
            // The content was closed already. Web need to invalidate the
            // presentation id now.
            mPresentationId = INVALID_PRESENTATION_ID;
            onContentClosed();
        }
    };

    public XWalkPresentationContent(Context context, Activity activity, PresentationDelegate delegate) {
        mContext = context;
        mActivity = activity;
        mDelegate = delegate;
    }

    public void load(final String url) {
        if (mContentView == null) {
            mContentView = new XWalkView(mContext, mActivity);
            mContentView.setResourceClient(mXWalkResourceClient);
        }
        mContentView.load(url, null);
    }

    public int getPresentationId() {
        return mPresentationId;
    }

    public View getContentView() {
        return mContentView;
    }

    public void close() {
        mContentView.onDestroy();
        mPresentationId = INVALID_PRESENTATION_ID;
        mContentView = null;
    }

    public void onPause() {
        mContentView.pauseTimers();
        mContentView.onHide();
    }

    public void onResume() {
        mContentView.resumeTimers();
        mContentView.onShow();
    }

    private void onContentLoaded() {
        if (mDelegate != null) mDelegate.onContentLoaded(this);
    }

    private void onContentClosed() {
        if (mDelegate != null) mDelegate.onContentClosed(this);
    }

    /**
     * Interface to hook into XWalkPresentationContent instance.
     */
    public interface PresentationDelegate {
        /**
         * Called when the presentation content is loaded.
         */
        public void onContentLoaded(XWalkPresentationContent content);

        /**
         * Called when the presentation content is closed.
         */
        public void onContentClosed(XWalkPresentationContent content);
    }
}
