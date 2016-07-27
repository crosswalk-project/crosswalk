/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import android.annotation.TargetApi;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.ApplicationErrorReport;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.Paint;
import android.graphics.Rect;
import android.net.http.SslCertificate;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.util.AttributeSet;
import android.util.Log;
import android.view.accessibility.AccessibilityNodeProvider;
import android.view.KeyEvent;
import android.view.SurfaceView;
import android.view.ViewGroup;
import android.view.ViewStructure;
import android.view.View.OnTouchListener;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.MotionEvent;
import android.view.View;
import android.webkit.ValueCallback;
import android.widget.FrameLayout;

import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.lang.ref.WeakReference;
import java.util.Map;

import org.chromium.base.CommandLine;
import org.chromium.content.browser.ContentViewClient;
import org.chromium.content.browser.ContentViewCore;
import org.chromium.content.browser.ContentViewRenderView;
import org.chromium.content.browser.SmartClipProvider;

/**
 * <p>XWalkViewInternal represents an Android view for web apps/pages. Thus most of attributes
 * for Android view are valid for this class. Since it internally uses
 * <a href="http://developer.android.com/reference/android/view/SurfaceView.html">
 * android.view.SurfaceView</a> for rendering web pages by default, it can't be resized,
 * rotated, transformed and animated due to the limitations of SurfaceView.
 * Alternatively, XWalkViewInternal can be transformed and animated by using
 * <a href="http://developer.android.com/reference/android/view/TextureView.html">
 * TextureView</a>, which is intentionally used to render web pages for animation support.
 * Besides, XWalkViewInternal won't be rendered if it's invisible.</p>
 *
 * <p>Crosswalk provides two ways to choose TextureView or SurfaceView:</p>
 * <ol><li>[To Be Deprecated] Set preference key
 * {@link XWalkPreferencesInternal#ANIMATABLE_XWALK_VIEW} to true to use TextureView,
 * and vice versa. Notice that all XWalkViews share the same preference value.</li>
 * <li>Application developer can set this attribute for a single XWalkView by XML without
 * impact on other XWalkViews, notice that in this case the value of
 * XWalkPreferencesInternal#ANIMATABLE_XWALK_VIEW is invaild for this XWalkView.
 * See below steps for detail:
 *  <ul type="disc">
 *   <li> Create an attrs.xml under res/values/ as below, the attrs name must be "animatable":
 *    <pre>
 *    &lt;?xml version="1.0" encoding="utf-8"?&gt;
 *    &lt;resources&gt;
 *      &lt;declare-styleable name="AnimatableView"&gt;
 *        &lt;attr name="animatable" format="boolean" /&gt;
 *      &lt;/declare-styleable&gt;
 *    &lt;/resources&gt;</pre>
 *   </li>
 *   <li>Add xwalk namespace into activity layout file, such as layout/activity_main.xml.
 *    <pre>
 *    &lt;LinearLayout xmlns:android="http://schemas.android.com/apk/res/android"
 *        xmlns:xwalk="http://schemas.android.com/apk/res-auto"
 *        ......</pre>
 *   </li>
 *   <li>Set xwalk attribute to true or false in the same xml as above. True for TextureView,
 *         false for SurfaceView, and SurfaceView is the default.
 *    <pre>
 *    &lt;org.xwalk.core.XWalkView
 *        android:id="@+id/xwalkview"
 *        android:layout_width="match_parent"
 *        android:layout_height="match_parent"
 *        xwalk:animatable="true" &gt;
 *    &lt;/org.xwalk.core.XWalkView&gt;</pre>
 *   </li>
 *   <li>Use XWalkView in MainActivity.java.
 *    <pre>mXWalkView = (XWalkView) findViewById(R.id.xwalkview);</pre>
 *   </li></ul>
 *   There is debug message on logcat according to your "animatable" values:
 *    "XWalkContent: CompositingSurfaceType is TextureView"
 *   </li></ol>
 *
 * <p>XWalkViewInternal needs hardware acceleration to render web pages. As a result, the
 * AndroidManifest.xml of the caller's app must be appended with the attribute
 * "android:hardwareAccelerated" and its value must be set as "true".</p>
 * <pre>
 * &lt;application android:name="android.app.Application" android:label="XWalkUsers"
 *     android:hardwareAccelerated="true"&gt;
 * </pre>
 *
 * <p>Crosswalk provides 2 major callback classes, namely {@link XWalkResourceClientInternal} and
 * {@link XWalkUIClientInternal} for listening to the events related to resource loading and UI.
 * By default, Crosswalk has a default implementation. Callers can override them if needed.</p>
 *
 * <p><strong>Unlike WebView, you shouldn't use XWalkView directly. It must be accompanied with
 * {@link XWalkActivity} or {@link XWalkInitializer}. </strong></p>
 *
 * <p>For example:</p>
 *
 * <pre>
 * import android.content.Intent;
 * import android.os.Bundle;
 * import android.webkit.ValueCallback;
 *
 * import org.xwalk.core.XWalkActivity;
 * import org.xwalk.core.XWalkResourceClientInternal;
 * import org.xwalk.core.XWalkUIClientInternal;
 * import org.xwalk.core.XWalkViewInternal;
 * import org.xwalk.core.XWalkWebResourceRequestInternal;
 * import org.xwalk.core.XWalkWebResourceResponseInternal;
 *
 * public class MainActivity extends XWalkActivity {
 *     private XWalkViewInternal mXWalkView;
 *
 *     private class MyResourceClient extends XWalkResourceClientInternal {
 *         public MyResourceClient(XWalkViewInternal view) {
 *             super(view);
 *         }
 *
 *         &#64;Override
 *         public XWalkWebResourceResponseInternal shouldInterceptLoadRequest(XWalkViewInternal view,
 *                 XWalkWebResourceRequestInternal request) {
 *             // Handle it here.
 *             // Use createXWalkWebResourceResponse instead of "new XWalkWebResourceResponse"
 *             // to create the response.
 *             // Similar with before, there are two function to use:
 *             // 1) createXWalkWebResourceResponse(String mimeType, String encoding, InputStream data)
 *             // 2) createXWalkWebResourceResponse(String mimeType, String encoding, InputStream data,
 *             //             int statusCode, String reasonPhrase, Map&lt;String, String&gt; responseHeaders)
 *
 *             return createXWalkWebResourceResponse("text/html", "UTF-8", null);
 *         }
 *     }
 *
 *     private class MyUIClient extends XWalkUIClientInternal {
 *         public MyUIClient(XWalkViewInternal view) {
 *             super(view);
 *         }
 *
 *         &#64;Override
 *         public boolean onCreateWindowRequested(XWalkView view, InitiateBy initiator,
 *                 ValueCallback&lt;XWalkViewInternal&gt; callback) {
 *             XWalkViewInternal newView = new XWalkViewInternal(MainActivity.this);
 *             callback.onReceiveValue(newView);
 *             return true;
 *         }
 *     }
 *
 *     &#64;Override
 *     protected void onCreate(Bundle savedInstanceState) {
 *         super.onCreate(savedInstanceState);
 *
 *         // Until onXWalkReady() is invoked, you should do nothing with the
 *         // embedding API except the following:
 *         // 1. Instantiate the XWalkView object
 *         // 2. Call XWalkPreferences.setValue()
 *         // 3. Call mXWalkView.setXXClient(), e.g., setUIClient
 *         // 4. Call mXWalkView.setXXListener(), e.g., setDownloadListener
 *         // 5. Call mXWalkView.addJavascriptInterface()
 *
 *         setContentView(R.layout.activity_main);
 *         mXWalkView = (XWalkViewInternal) findViewById(R.id.xwalkview);
 *         mXWalkView.setResourceClient(new MyResourceClient(mXWalkView));
 *         mXWalkView.setUIClient(new MyUIClient(mXWalkView));
 *     }
 *
 *     &#64;Override
 *     public void onXWalkReady() {
 *         // Do anyting with the embedding API
 *
 *         mXWalkView.load("https://crosswalk-project.org/", null);
 *     }
 *
 *     &#64;Override
 *     protected void onNewIntent(Intent intent) {
 *         if (mXWalkView != null) {
 *             mXWalkView.onNewIntent(intent);
 *         }
 *     }
 * }
 * </pre>
 */
@XWalkAPI(extendClass = FrameLayout.class, createExternally = true)
public class XWalkViewInternal extends android.widget.FrameLayout
        implements ContentViewCore.InternalAccessDelegate, SmartClipProvider {

    static final String PLAYSTORE_DETAIL_URI = "market://details?id=";
    private static final String TAG = XWalkViewInternal.class.getSimpleName();

    private XWalkContent mContent;
    private Context mContext;
    private boolean mIsHidden;
    private XWalkExternalExtensionManagerInternal mExternalExtensionManager;

    /**
     * Normal reload mode as default.
     * @since 1.0
     */
    @XWalkAPI
    public static final int RELOAD_NORMAL = 0;
    /**
     * Reload mode with bypassing the cache.
     * @since 1.0
     */
    @XWalkAPI
    public static final int RELOAD_IGNORE_CACHE = 1;
    /**
     * SurfaceView is the default compositing surface which has a bit performance advantage,
     * such as it has less latency and uses less memory.
     * @since 7.0
     */
    @XWalkAPI
    public static final String SURFACE_VIEW = "SurfaceView";
    /**
     * Use TextureView as compositing surface which supports animation on the View.
     * @since 7.0
     */
    @XWalkAPI
    public static final String TEXTURE_VIEW = "TextureView";

    // The moment when the XWalkViewBridge is added to the XWalkView, the screen flashes black. The
    // reason is when the SurfaceView appears in the window the fist time, it requests the window's
    // parameters changing by calling IWindowSession.relayout(). But if the window already has
    // appropriate parameters, it will not refresh all the window's stuff and the screen will not
    // blink. So we add a 0px SurfaceView at first. This will recreate the window before the
    // activity is shown on the screen, and when the actual XWalkViewBridge is added, it will just
    // continue to use the window with current parameters. The temporary SurfaceView can be removed
    // at last.
    /**
     * Constructs a new XWalkView with a Context object.
     * @param context a Context object used to access application assets.
     * @since 6.0
     */
    @XWalkAPI(preWrapperLines = {
                  "        super(${param1}, null);",
                  "        SurfaceView surfaceView = new SurfaceView(${param1});",
                  "        surfaceView.setLayoutParams(new ViewGroup.LayoutParams(0, 0));",
                  "        addView(surfaceView);"},
              postWrapperLines = {
                  "        ReflectMethod getContentViewRenderViewMethod = new ReflectMethod(null, \"getContentViewRenderView\");",
                  "        getContentViewRenderViewMethod.init(bridge, null, \"getContentViewRenderView\");",
                  "        addView((FrameLayout)getContentViewRenderViewMethod.invoke(), new FrameLayout.LayoutParams(",
                  "                FrameLayout.LayoutParams.MATCH_PARENT,",
                  "                FrameLayout.LayoutParams.MATCH_PARENT));",
                  "        addView((FrameLayout)bridge, new FrameLayout.LayoutParams(",
                  "                FrameLayout.LayoutParams.MATCH_PARENT,",
                  "                FrameLayout.LayoutParams.MATCH_PARENT));",
                  "        removeViewAt(0);",
                  "        new org.xwalk.core.extension.XWalkExternalExtensionManagerImpl(this);"})
    public XWalkViewInternal(Context context) {
        super(context, null);

        checkThreadSafety();
        mContext = getContext();

        if (getScrollBarStyle() == View.SCROLLBARS_INSIDE_OVERLAY) {
            setHorizontalScrollBarEnabled(false);
            setVerticalScrollBarEnabled(false);
        }

        setFocusable(true);
        setFocusableInTouchMode(true);

        initXWalkContent(null);
    }

    // A View is usually in edit mode when displayed within a developer tool, like Android Studio.
    // So isInEditMode() should be used inside the corresponding XWalkView constructor.
    /**
     * Constructor for inflating via XML.
     * @param context  a Context object used to access application assets.
     * @param attrs    an AttributeSet passed to our parent.
     * @since 1.0
     */
    @XWalkAPI(preWrapperLines = {
                  "        super(${param1}, ${param2});",
                  "        if (isInEditMode()) return;",
                  "        if (${param2} != null)",
                  "            mAnimatable = ${param2}.getAttributeValue(",
                  "                    XWALK_ATTRS_NAMESPACE, ANIMATABLE);",
                  "        SurfaceView surfaceView = new SurfaceView(${param1});",
                  "        surfaceView.setLayoutParams(new ViewGroup.LayoutParams(0, 0));",
                  "        addView(surfaceView);"},
              postWrapperLines = {
                  "        ReflectMethod getContentViewRenderViewMethod = new ReflectMethod(null, \"getContentViewRenderView\");",
                  "        getContentViewRenderViewMethod.init(bridge, null, \"getContentViewRenderView\");",
                  "        addView((FrameLayout)getContentViewRenderViewMethod.invoke(), new FrameLayout.LayoutParams(",
                  "                FrameLayout.LayoutParams.MATCH_PARENT,",
                  "                FrameLayout.LayoutParams.MATCH_PARENT));",
                  "        addView((FrameLayout)bridge, new FrameLayout.LayoutParams(",
                  "                FrameLayout.LayoutParams.MATCH_PARENT,",
                  "                FrameLayout.LayoutParams.MATCH_PARENT));",
                  "        removeViewAt(0);",
                  "        new org.xwalk.core.extension.XWalkExternalExtensionManagerImpl(this);"},
              postBridgeLines = {
                  "        String animatable = null;",
                  "        try {",
                  "            animatable = (String) new ReflectField(wrapper, \"mAnimatable\").get();",
                  "        } catch (RuntimeException e) {",
                  "        }",
                  "        initXWalkContent(animatable);"})
    public XWalkViewInternal(Context context, AttributeSet attrs) {
        super(context, attrs);

        checkThreadSafety();
        mContext = getContext();

        if (getScrollBarStyle() == View.SCROLLBARS_INSIDE_OVERLAY) {
            setHorizontalScrollBarEnabled(false);
            setVerticalScrollBarEnabled(false);
        }

        setFocusable(true);
        setFocusableInTouchMode(true);
    }

    /**
     * Constructor for Crosswalk runtime. In shared mode, context isi
     * different from activity. In embedded mode, they're same.
     * <strong>This method is no longer supported.</strong>
     * @param context  a Context object used to access application assets
     * @param activity the activity for this XWalkViewInternal.
     * @deprecated Not currently supported
     * @since 1.0
     */
    @Deprecated
    @XWalkAPI(preWrapperLines = {
                  "        super(${param1}, null);",
                  "        SurfaceView surfaceView = new SurfaceView(${param1});",
                  "        surfaceView.setLayoutParams(new ViewGroup.LayoutParams(0, 0));",
                  "        addView(surfaceView);"},
              postWrapperLines = {
                  "        ReflectMethod getContentViewRenderViewMethod = new ReflectMethod(null, \"getContentViewRenderView\");",
                  "        getContentViewRenderViewMethod.init(bridge, null, \"getContentViewRenderView\");",
                  "        addView((FrameLayout)getContentViewRenderViewMethod.invoke(), new FrameLayout.LayoutParams(",
                  "                FrameLayout.LayoutParams.MATCH_PARENT,",
                  "                FrameLayout.LayoutParams.MATCH_PARENT));",
                  "        addView((FrameLayout)bridge, new FrameLayout.LayoutParams(",
                  "                FrameLayout.LayoutParams.MATCH_PARENT,",
                  "                FrameLayout.LayoutParams.MATCH_PARENT));",
                  "        removeViewAt(0);",
                  "        new org.xwalk.core.extension.XWalkExternalExtensionManagerImpl(this);"})
    public XWalkViewInternal(Context context, Activity activity) {
        super(context, null);

        checkThreadSafety();
        mContext = getContext();

        if (getScrollBarStyle() == View.SCROLLBARS_INSIDE_OVERLAY) {
            setHorizontalScrollBarEnabled(false);
            setVerticalScrollBarEnabled(false);
        }

        setFocusable(true);
        setFocusableInTouchMode(true);

        initXWalkContent(null);
    }

    // TODO(yongsheng): we should remove this since we have getContext()?
    /**
     * @hide
     */
    public Context getViewContext() {
        return mContext;
    }

    public void completeWindowCreation(XWalkViewInternal newXWalkView) {
        mContent.supplyContentsForPopup(newXWalkView == null ? null : newXWalkView.mContent);
    }

    protected void initXWalkContent(String animatable) {
        XWalkViewDelegate.init(null, mContext);

        mIsHidden = false;
        mContent = new XWalkContent(mContext, animatable, this);

        // If XWalkView was created in onXWalkReady(), and the activity which owns
        // XWalkView was destroyed, pauseTimers() will be invoked. Reentry the activity,
        // resumeTimers() will not be invoked since onResume() was invoked before
        // XWalkView creation. So to invoke resumeTimers() explicitly here.
        mContent.resumeTimers();
        // Set default XWalkClientImpl.
        setXWalkClient(new XWalkClient(this));
        // Set default XWalkWebChromeClient and DownloadListener. The default actions
        // are provided via the following clients if special actions are not needed.
        setXWalkWebChromeClient(new XWalkWebChromeClient(this));

        // Set with internal implementation. Could be overwritten by embedders'
        // setting.
        setUIClient(new XWalkUIClientInternal(this));
        setResourceClient(new XWalkResourceClientInternal(this));

        setDownloadListener(new XWalkDownloadListenerImpl(mContext));
        setNavigationHandler(new XWalkNavigationHandlerImpl(mContext));
        setNotificationService(new XWalkNotificationServiceImpl(mContext, this));

        XWalkPathHelper.initialize();
        XWalkPathHelper.setCacheDirectory(
                mContext.getApplicationContext().getCacheDir().getPath());

        String state = Environment.getExternalStorageState();
        if (Environment.MEDIA_MOUNTED.equals(state) ||
                Environment.MEDIA_MOUNTED_READ_ONLY.equals(state)) {
            File extCacheDir =  mContext.getApplicationContext().getExternalCacheDir();
            if (null != extCacheDir) {
                XWalkPathHelper.setExternalCacheDirectory(extCacheDir.getPath());
            }
        }
    }

    /**
     * Load a web page/app from a given base URL or a content.
     * If url is null or empty and content is null or empty, then this function
     * will do nothing.
     * If content is not null, load the web page/app from the content.
     * If content is not null and the url is not set, return "about:blank" ifi
     * calling {@link XWalkViewInternal#getUrl()}.
     * If content is null, try to load the content from the url.
     *
     * It supports URL schemes like 'http:', 'https:' and 'file:'.
     * It can also load files from Android assets, e.g. 'file:///android_asset/'.
     * @param url the url for web page/app.
     * @param content the content for the web page/app. Could be empty.
     * @since 1.0
     */
    @XWalkAPI
    public void load(String url, String content) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.loadUrl(url, content, null);
    }

    /**
     * Load a web page/app from a given base URL or a content with specified HTTP headers
     * @param url the url for web page/app.
     * @param content the content for the web page/app. Could be empty.
     * @param headers the additional HTTP headers
     * @since 6.0
     */
    @XWalkAPI
    public void load(String url, String content, Map<String, String> headers) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.loadUrl(url, content, headers);
    }

    /**
     * Load a web app from a given manifest.json file. If content is not null,
     * load the manifest.json from the content. If content is null, try to load
     * the manifest.json from the url. Note that url should not be null if the
     * launched path defined in manifest.json is relative.
     *
     * It supports URL schemes like 'http:', 'https:' and 'file:'.
     * It can also load files from Android assets, e.g. 'file:///android_asset/'.
     * @param url the url for manifest.json.
     * @param content the content for manifest.json.
     * @since 1.0
     */
    @XWalkAPI
    public void loadAppFromManifest(String url, String content) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.loadAppFromManifest(url, content);
    }

    /**
     * Reload a web app with a given mode.
     * @param mode the reload mode.
     * @since 1.0
     */
    @XWalkAPI
    public void reload(int mode) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.reload(mode);
    }

    /**
     * Stop current loading progress.
     * @since 1.0
     */
    @XWalkAPI
    public void stopLoading() {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.stopLoading();
    }

    /**
     * Get the url of current web page/app. This may be different from what's passed
     * by caller.
     * @return the url for current web page/app.
     * @since 1.0
     */
    @XWalkAPI
    public String getUrl() {
        if (mContent == null) return null;
        checkThreadSafety();
        return mContent.getUrl();
    }

    /**
     * Get the content height of current web page/app.
     * NOTE: When page finished loading, it's content size info may not be updated.
     * Pls wait for a while for info updating.
     * @return the height of current web page/app in Css coordinate.
     * @since 7.0
     */
    @XWalkAPI
    public int getContentHeight() {
        return mContent.getContentHeight();
    }

    /**
     * Get the title of current web page/app. This may be different from what's passed
     * by caller.
     * @return the title for current web page/app.
     * @since 1.0
     */
    @XWalkAPI
    public String getTitle() {
        if (mContent == null) return null;
        checkThreadSafety();
        return mContent.getTitle();
    }

    /**
     * Get the original url specified by caller.
     * @return the original url.
     * @since 1.0
     */
    @XWalkAPI
    public String getOriginalUrl() {
        if (mContent == null) return null;
        checkThreadSafety();
        return mContent.getOriginalUrl();
    }

    /**
     * Get the navigation history for current XWalkViewInternal. It's synchronized with
     * this XWalkViewInternal if any backward/forward and navigation operations.
     * @return the navigation history.
     * @since 1.0
     */
    @XWalkAPI
    public XWalkNavigationHistoryInternal getNavigationHistory() {
        if (mContent == null) return null;
        checkThreadSafety();
        return mContent.getNavigationHistory();
    }

    /**
     * Injects the supplied Java object into this XWalkViewInternal.
     * Each method defined in the class of the object should be
     * marked with {@link JavascriptInterface} if it's called by JavaScript.
     * @param object the supplied Java object, called by JavaScript.
     * @param name the name injected in JavaScript.
     * @since 1.0
     */
    @XWalkAPI(reservable = true)
    public void addJavascriptInterface(Object object, String name) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.addJavascriptInterface(object, name);
    }

    /**
     * Removes a previously injected Java object from this XWalkView. Note that
     * the removal will not be reflected in JavaScript until the page is next
     * (re)loaded. See {@link #addJavascriptInterface}.
     * @param name the name used to expose the object in JavaScript
     * @since 7.0
     */
    @XWalkAPI(reservable = true)
    public void removeJavascriptInterface(String name) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.removeJavascriptInterface(name);
    }

    /**
     * Evaluate a fragment of JavaScript code and get the result via callback.
     * @param script the JavaScript string.
     * @param callback the callback to handle the evaluated result.
     * @since 1.0
     */
    @XWalkAPI
    public void evaluateJavascript(String script, ValueCallback<String> callback) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.evaluateJavascript(script, callback);
    }

    /**
     * Clear the resource cache. Note that the cache is per-application, so this
     * will clear the cache for all XWalkViews used.
     * @param includeDiskFiles indicate whether to clear disk files for cache.
     * @since 1.0
     */
    @XWalkAPI
    public void clearCache(boolean includeDiskFiles) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.clearCache(includeDiskFiles);
    }

    /**
     * Clear the resource cache. Note that it only clear the cache for the specified
     * url.
     * @param url indicate which cache will be cleared.
     * @since 6.0
     */
    @XWalkAPI
    public void clearCacheForSingleFile(String url) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.clearCacheForSingleFile(url);
    }

    /**
     * Indicate that a HTML element is occupying the whole screen.
     * @return true if any HTML element is occupying the whole screen.
     * @since 1.0
     */
    @XWalkAPI
    public boolean hasEnteredFullscreen() {
        if (mContent == null) return false;
        checkThreadSafety();
        return mContent.hasEnteredFullscreen();
    }

    /**
     * Leave fullscreen mode if it's. Do nothing if it's not
     * in fullscreen.
     * @since 1.0
     */
    @XWalkAPI
    public void leaveFullscreen() {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.exitFullscreen();
    }

    /**
     * Pause all layout, parsing and JavaScript timers for all XWalkViewInternal instances.
     * It will be called when the container Activity get paused. It can also be explicitly
     * called to pause timers.
     *
     * Note that it will globally impact all XWalkViewInternal instances, not limited to
     * just this XWalkViewInternal.
     *
     * @since 1.0
     */
    @XWalkAPI
    public void pauseTimers() {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.pauseTimers();
    }

    /**
     * Resume all layout, parsing and JavaScript timers for all XWalkViewInternal instances.
     * It will be called when the container Activity get resumed. It can also be explicitly
     * called to resume timers.
     *
     * Note that it will globally impact all XWalkViewInternal instances, not limited to
     * just this XWalkViewInternal.
     *
     * @since 1.0
     */
    @XWalkAPI
    public void resumeTimers() {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.resumeTimers();
    }

    /**
     * Pause many other things except JavaScript timers inside rendering engine,
     * like video player, modal dialogs, etc. See {@link #pauseTimers} about pausing
     * JavaScript timers.
     * It will be called when the container Activity get paused. It can also be explicitly
     * called to pause above things.
     * @since 1.0
     */
    @XWalkAPI
    public void onHide() {
        if (mContent == null || mIsHidden) return;
        mContent.onPause();
        mIsHidden = true;
    }

    /**
     * Resume video player, modal dialogs. Embedders are in charge of calling
     * this during resuming this activity if they call onHide.
     * Typically it should be called when the activity for this view is resumed.
     * It will be called when the container Activity get resumed. It can also be explicitly
     * called to resume above things.
     * @since 1.0
     */
    @XWalkAPI
    public void onShow() {
        if (mContent == null || !mIsHidden ) return;
        mContent.onResume();
        mIsHidden = false;
    }

    /**
     * Release internal resources occupied by this XWalkViewInternal.
     * It will be called when the container Activity get destroyed. It can also be explicitly
     * called to release resources.
     * @since 1.0
     */
    @XWalkAPI
    public void onDestroy() {
        destroy();
    }

    /**
     * Pass through intents to XWalkViewInternal. Many internal facilities need this
     * to receive the intents like web notification. See
     * <a href="http://developer.android.com/reference/android/app/Activity.html">
     * android.app.Activity.onNewIntent()</a>.
     * @param intent passed from android.app.Activity.onNewIntent().
     * @since 1.0
     */
    @XWalkAPI
    public boolean onNewIntent(Intent intent) {
        if (mContent == null) return false;

        if (mExternalExtensionManager != null) {
            mExternalExtensionManager.onNewIntent(intent);
        }

        return mContent.onNewIntent(intent);
    }

    /**
     * Save current internal state of this XWalkViewInternal. This can help restore this state
     * afterwards restoring.
     * @param outState the saved state for restoring.
     * @since 1.0
     */
    @XWalkAPI
    public boolean saveState(Bundle outState) {
        if (mContent == null) return false;
        mContent.saveState(outState);
        return true;
    }

    /**
     * Restore the state from the saved bundle data.
     * @param inState the state saved from saveState().
     * @return true if it can restore the state.
     * @since 1.0
     */
    @XWalkAPI
    public boolean restoreState(Bundle inState) {
        if (mContent == null) return false;
        if (mContent.restoreState(inState) != null) return true;
        return false;
    }

    /**
     * Get the API version of Crosswalk embedding API.
     * @return the string of API level.
     * @since 1.0
     */
    @XWalkAPI
    public String getAPIVersion() {
        return String.valueOf(XWalkCoreVersion.API_VERSION) + ".0";
    }

    /**
     * Get the Crosswalk version.
     * @return the string of Crosswalk.
     * @since 1.0
     */
    @XWalkAPI
    public String getXWalkVersion() {
        if (mContent == null) return null;
        return mContent.getXWalkVersion();
    }

    /**
     * Embedders use this to customize their handlers to events/callbacks related
     * to UI.
     * @param client the XWalkUIClientInternal defined by callers.
     * @since 1.0
     */
    @XWalkAPI(reservable = true)
    public void setUIClient(XWalkUIClientInternal client) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.setUIClient(client);
    }

    /**
     * Embedders use this to customize their handlers to events/callbacks related
     * to resource loading.
     * @param client the XWalkResourceClientInternal defined by callers.
     * @since 1.0
     */
    @XWalkAPI(reservable = true)
    public void setResourceClient(XWalkResourceClientInternal client) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.setResourceClient(client);
    }

    /**
     * Set Background color of the view
     */
    @Override
    @XWalkAPI
    public void setBackgroundColor(int color) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.setBackgroundColor(color);
    }

    /**
     * Set origin access whitelist.
     * @param url the url for accesssing whitelist.
     * @param patterns representing hosts which the application should be able to access.
     * @since 6.0
     */
    @XWalkAPI
    public void setOriginAccessWhitelist(String url, String[] patterns) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.setOriginAccessWhitelist(url, patterns);
    }

    // We can't let XWalkView's setLayerType call to this via reflection as this method
    // may be called in XWalkView constructor but the XWalkView is not ready yet and then
    // UnsupportedOperationException is thrown, see XWALK-5021/XWALK-5047.
    // We only support hardware acceleration in XWalkView, so setLayerType should be noop
    @Override
    @XWalkAPI(disableReflectMethod = true,
              preWrapperLines = {"        return;"})
    public void setLayerType(int layerType, Paint paint) {
    }

     /**
     * Set the user agent of web page/app.
     * @param userAgent the user agent string passed from client.
     * @since 5.0
     */
    @XWalkAPI
    public void setUserAgentString(String userAgent) {
        XWalkSettingsInternal settings = getSettings();
        if (settings == null) return;
        checkThreadSafety();
        settings.setUserAgentString(userAgent);
    }

      /**
      * Get the user agent of web page/app.
      * @return the XWalkView's user-agent string.
      * @since 6.0
      */
     @XWalkAPI
     public String getUserAgentString() {
         XWalkSettingsInternal settings = getSettings();
         if (settings == null) return null;
         checkThreadSafety();
         return settings.getUserAgentString();
     }

     /**
     * Set the accept languages of XWalkView.
     * @param acceptLanguages the accept languages string passed from client.
     * @since 5.0
     */
    @XWalkAPI
    public void setAcceptLanguages(final String acceptLanguages) {
        XWalkSettingsInternal settings = getSettings();
        if (settings == null) return;
        checkThreadSafety();
        settings.setAcceptLanguages(acceptLanguages);
    }

    /**
     * Capture a bitmap of visible content.
     * @param callback callback to call when the bitmap capture is done.
     * @since 6.0
     */
    @XWalkAPI
    public void captureBitmapAsync(XWalkGetBitmapCallbackInternal callback) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.captureBitmapAsync(callback);
    }

    /**
     * Get XWalkSettings
     * @return the XWalkSettings object.
     * @since 6.0
     */
    @XWalkAPI
    public XWalkSettingsInternal getSettings() {
        if (mContent == null) return null;
        checkThreadSafety();
        return mContent.getSettings();
    }

    /**
     * This method is used by Cordova for hacking.
     * TODO(yongsheng): remove this and related test cases?
     */
    @XWalkAPI
    public void setNetworkAvailable(boolean networkUp) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.setNetworkAvailable(networkUp);
    }

    /**
     * Enables remote debugging and returns the URL at which the dev tools
     * server is listening for commands.
     */
    public void enableRemoteDebugging() {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.enableRemoteDebugging();
    }

    /**
     * Get the websocket url for remote debugging.
     * @return the web socket url to remote debug this xwalk view.
     * null will be returned if remote debugging is not enabled.
     * @since 4.1
     */
    @XWalkAPI
    public Uri getRemoteDebuggingUrl() {
        if (mContent == null) return null;
        checkThreadSafety();
        String wsUrl = mContent.getRemoteDebuggingUrl();
        if (wsUrl == null || wsUrl.isEmpty()) return null;

        return Uri.parse(wsUrl);
    }

    /**
    * Performs zoom in in this XWalkView.
    * @return true if zoom in succeeds, false if no zoom changes
    * @since 5.0
    */
    @XWalkAPI
    public boolean zoomIn() {
        if (mContent == null) return false;
        checkThreadSafety();
        return mContent.zoomIn();
    }

    /**
    * Performs zoom out in this XWalkView.
    * @return true if zoom out succeeds, false if no zoom changes
    * @since 5.0
    */
    @XWalkAPI
    public boolean zoomOut() {
        if (mContent == null) return false;
        checkThreadSafety();
        return mContent.zoomOut();
    }

    /**
    * Performs a zoom operation in this XWalkView.
    * @param factor the zoom factor to apply.
    * The zoom factor will be clamped to the XWalkView's zoom limits.
    * This value must be in the range 0.01 to 100.0 inclusive.
    * @since 5.0
    */
    @XWalkAPI
    public void zoomBy(float factor) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.zoomBy(factor);
    }

    /**
    * Gets whether this XWalkView can be zoomed in.
    * @return true if this XWalkView can be zoomed in
    * @since 5.0
    */
    @XWalkAPI
    public boolean canZoomIn() {
        if (mContent == null) return false;
        checkThreadSafety();
        return mContent.canZoomIn();
    }

    /**
    * Gets whether this XWalkView can be zoomed out.
    * @return true if this XWalkView can be zoomed out
    * @since 5.0
    */
    @XWalkAPI
    public boolean canZoomOut() {
        if (mContent == null) return false;
        checkThreadSafety();
        return mContent.canZoomOut();
    }

    /**
     * Create a new InputConnection for and InputMethod to interact with the view.
     * The default implementation returns the InputConnection created by ContentView
     * @param outAttrs Fill in with attribute information about the connection
     * @return the new InputConnection
     * @since 5.0
     */
    @XWalkAPI
    public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        return mContent.onCreateInputConnection(outAttrs);
    }

    /**
     * Sets the initial scale for this XWalkView.
     * @param scaleInPercent the initial scale in percent.
     * @since 6.0
     */
    @XWalkAPI
    public void setInitialScale(int scaleInPercent) {
        checkThreadSafety();
        XWalkSettingsInternal settings = getSettings();
        if (settings == null) return;
        settings.setInitialPageScale(scaleInPercent);
    }

    /**
     * It's used for Presentation API.
     * @hide
     */
    public int getContentID() {
        if (mContent == null) return -1;
        return mContent.getRoutingID();
    }

    boolean canGoBack() {
        if (mContent == null) return false;
        checkThreadSafety();
        return mContent.canGoBack();
    }

    void goBack() {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.goBack();
    }

    boolean canGoForward() {
        if (mContent == null) return false;
        checkThreadSafety();
        return mContent.canGoForward();
    }

    void goForward() {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.goForward();
    }

    void clearHistory() {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.clearHistory();
    }

    void destroy() {
        if (mContent == null) return;
        mContent.destroy();
        disableRemoteDebugging();
    }

    void disableRemoteDebugging() {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.disableRemoteDebugging();
    }

    private static void checkThreadSafety() {
        if (Looper.myLooper() != Looper.getMainLooper()) {
            Throwable throwable = new Throwable(
                "Warning: A XWalkViewInternal method was called on thread '" +
                Thread.currentThread().getName() + "'. " +
                "All XWalkViewInternal methods must be called on the UI thread. ");
            throw new RuntimeException(throwable);
        }
    }

    void navigateTo(int offset) {
        if (mContent == null) return;
        mContent.navigateTo(offset);
    }

    void setOverlayVideoMode(boolean enabled) {
        mContent.setOverlayVideoMode(enabled);
    }

    /**
     * Return the icon which current page has.
     * @return the favicon of current web page/app.
     * @since 6.0
     */
    @XWalkAPI
    public Bitmap getFavicon() {
        if (mContent == null) return null;
        checkThreadSafety();
        return mContent.getFavicon();
    }

    /**
    * Control whether the XWalkView's surface is placed on top of its window.
    * Note this only works when XWalkPreferences.ANIMATABLE_XWALK_VIEW is false.
    * @param onTop true for on top.
    * @since 5.0
    */
    @XWalkAPI
    public void setZOrderOnTop(boolean onTop) {
        if (mContent == null) return;
        mContent.setZOrderOnTop(onTop);
    }

    /**
     * Removes the autocomplete popup from the currently focused form field, if present.
     * Note this only affects the display of the autocomplete popup, it does not remove
     * any saved form data from this WebView's store.
     * This is a poorly named method, but we keep it for historical reasons.
     * @since 6.0
     */
    @XWalkAPI
    public void clearFormData() {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.hideAutofillPopup();
    }

    /**
     * Set the enabled state of this view.
     * @param visibility One of VISIBLE, INVISIBLE, or GONE.
     * @since 6.0
     */
    @XWalkAPI(disableReflectMethod = true,
              preWrapperLines = {
                  "        if (visibility == View.INVISIBLE) visibility = View.GONE;",
                  "        super.setVisibility(visibility);",
                  "        setXWalkViewInternalVisibility(visibility);",
                  "        setSurfaceViewVisibility(visibility);"})
    public void setVisibility(int visibility) {
    }

    /**
     * Set the enabled state of SurfaceView.
     * @param visibility One of VISIBLE, INVISIBLE, or GONE.
     * @since 6.0
     */
    @XWalkAPI(reservable = true)
    public void setSurfaceViewVisibility(int visibility) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.setVisibility(visibility);
    }

    /**
     * Set the enabled state of XWalkViewInternal.
     * @param visibility One of VISIBLE, INVISIBLE, or GONE.
     * @since 6.0
     */
    @XWalkAPI(reservable = true)
    public void setXWalkViewInternalVisibility(int visibility) {
        if (mContent == null) return;
        checkThreadSafety();
        super.setVisibility(visibility);
    }

    // Below methods are for test shell and instrumentation tests.
    /**
     * @hide
     */
    public void setXWalkClient(XWalkClient client) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.setXWalkClient(client);
    }

    /**
     * @hide
     */
    public void setXWalkWebChromeClient(XWalkWebChromeClient client) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.setXWalkWebChromeClient(client);
    }

    /**
     * Registers the interface to be used when content can not be handled by
     * the rendering engine, and should be downloaded instead. This will replace
     * the current handler.
     * @param listener an implementation of XWalkDownloadListenerInternal
     * @since 5.0
     */
    @XWalkAPI(reservable = true)
    public void setDownloadListener(XWalkDownloadListenerInternal listener) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.setDownloadListener(listener);
    }

    /**
     * @hide
     */
    public void setNavigationHandler(XWalkNavigationHandler handler) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.setNavigationHandler(handler);
    }

    /**
     * @hide
     */
    public void setNotificationService(XWalkNotificationService service) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.setNotificationService(service);
    }

    /**
     * @hide
     */
    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        if (event.getAction() == KeyEvent.ACTION_UP &&
                event.getKeyCode() == KeyEvent.KEYCODE_BACK) {
            // If there's navigation happens when app is fullscreen,
            // the content will still be fullscreen after navigation.
            // In such case, the back key will exit fullscreen first.
            if (hasEnteredFullscreen()) {
                leaveFullscreen();
                return true;
            } else if (canGoBack()) {
                goBack();
                return true;
            }
        }
        if (isFocused() && mContent != null) {
            return mContent.dispatchKeyEvent(event);
        }
        return super.dispatchKeyEvent(event);
    }

    // For instrumentation test.
    public ContentViewCore getXWalkContentForTest() {
        if (mContent == null) return null;
        return mContent.getContentViewCoreForTest();
    }

    // This is used to call back to XWalkView's performLongClick() so that developer can
    // override performLongClick() or setOnLongClickListener to disable copy/paste
    // action bar.
    @XWalkAPI(delegate = true,
              preWrapperLines = {"return performLongClick();"})
    public boolean performLongClickDelegate() {
        return false;
    }

    @Override
    @XWalkAPI
    public boolean onTouchEvent(MotionEvent event) {
        if (mContent == null) return false;
        checkThreadSafety();

        return mContent.onTouchEvent(event);
    }

    // Use delegate to access XWalkView's protected method
    @XWalkAPI(delegate = true,
              preWrapperLines = {"onScrollChanged(l, t, oldl, oldt);"})
    public void onScrollChangedDelegate(int l, int t, int oldl, int oldt) {
    }

    @XWalkAPI(delegate = true,
              preWrapperLines = {"onFocusChanged(gainFocus, direction, previouslyFocusedRect);"})
    public void onFocusChangedDelegate(boolean gainFocus, int direction, Rect previouslyFocusedRect) {
    }

    @XWalkAPI(delegate = true,
              preWrapperLines = {"onOverScrolled(scrollX, scrollY, clampedX, clampedY);"})
    public void onOverScrolledDelegate(int scrollX, int scrollY, boolean clampedX, boolean clampedY) {
    }

    @Override
    @XWalkAPI
    public void setOnTouchListener(OnTouchListener l) {
        if (mContent == null) return;
        checkThreadSafety();
        super.setOnTouchListener(l);
    }

    @Override
    @XWalkAPI
    public void scrollTo(int x, int y) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.scrollTo(x, y);
    }

    @Override
    @XWalkAPI
    public void scrollBy(int x, int y) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.scrollBy(x, y);
    }

    /**
     * Scroll the view with standard behaviour for scrolling beyond the normal content boundaries.
     * @param deltaX scroll delta distance on the horizontal orientation.
     * @param deltaY scroll delta distance on the vertical orientation.
     * @param scrollX current view offset on the horizontal orientation.
     * @param scrollY current view offset on the vertical orientation.
     * @param scrollRangeX the right boundaries of the view.
     * @param scrollRangeY the bottom boundaries of the view.
     * @param maxOverScrollX the max distance the view could over scrolled on the horizontal orientation.
     * @param maxOverScrollY the max distance the view could over scrolled on the vertical orientation.
     * @param isTouchEvent whether there is touch event.
     * @return return whether the scroll reach boundaries and triage over scroll event.
     * @since 6.0
     */
    @Override
    @XWalkAPI
    // NOTE: This API implementation is almost the same as the parent, but our onOverScrolled couldn't
    //       invoke scrollTo (because the view scroll is async in XWalkView and it may cause crash).
    protected boolean overScrollBy(int deltaX, int deltaY, int scrollX, int scrollY,
                                int scrollRangeX, int scrollRangeY, int maxOverScrollX,
                                int maxOverScrollY, boolean isTouchEvent) {
        final int overScrollMode = super.getOverScrollMode();
        final boolean canScrollHorizontal =
                computeHorizontalScrollRange() > computeHorizontalScrollExtent();
        final boolean canScrollVertical =
                computeVerticalScrollRange() > computeVerticalScrollExtent();
        final boolean overScrollHorizontal = overScrollMode == OVER_SCROLL_ALWAYS ||
                (overScrollMode == OVER_SCROLL_IF_CONTENT_SCROLLS && canScrollHorizontal);
        final boolean overScrollVertical = overScrollMode == OVER_SCROLL_ALWAYS ||
                (overScrollMode == OVER_SCROLL_IF_CONTENT_SCROLLS && canScrollVertical);

        int newScrollX = scrollX + deltaX;
        if (!overScrollHorizontal) {
            maxOverScrollX = 0;
        }

        int newScrollY = scrollY + deltaY;
        if (!overScrollVertical) {
            maxOverScrollY = 0;
        }

        // Clamp values if at the limits and record
        final int left = -maxOverScrollX;
        final int right = maxOverScrollX + scrollRangeX;
        final int top = -maxOverScrollY;
        final int bottom = maxOverScrollY + scrollRangeY;

        boolean clampedX = false;
        if (newScrollX > right) {
            newScrollX = right;
            clampedX = true;
        } else if (newScrollX < left) {
            newScrollX = left;
            clampedX = true;
        }

        boolean clampedY = false;
        if (newScrollY > bottom) {
            newScrollY = bottom;
            clampedY = true;
        } else if (newScrollY < top) {
            newScrollY = top;
            clampedY = true;
        }

        scrollTo(newScrollX, newScrollY);

        return clampedX || clampedY;
    }

    /**
     * Compute the horizontal range that the horizontal scrollbar represents.
     * @return the range of horizontal scrollbar.
     * @since 6.0
     */
    @XWalkAPI
    public int computeHorizontalScrollRange() {
        if (mContent == null) return 0;
        checkThreadSafety();
        return mContent.computeHorizontalScrollRange();
    }

    /**
     * Compute the horizontal offset of the horizontal scrollbar's
     * thumb within the horizontal range.
     * @return the horizontal offset of the horizontal scrollbar's thumb.
     * @since 6.0
     */
    @XWalkAPI
    public int computeHorizontalScrollOffset() {
        if (mContent == null) return 0;
        checkThreadSafety();
        return mContent.computeHorizontalScrollOffset();
    }

    /**
     * Compute the vertical range that the vertical scrollbar represents.
     * @return the range of the vertical scrollbar.
     * @since 6.0
     */
    @XWalkAPI
    public int computeVerticalScrollRange() {
        if (mContent == null) return 0;
        checkThreadSafety();
        return mContent.computeVerticalScrollRange();
    }

    /**
     * Compute the vertical offset the vertical scrollbar's thumb
     * within the horizontal range.
     * @return the vertical offset of the vertical scrollbar's thumb.
     * @since 6.0
     */
    @XWalkAPI
    public int computeVerticalScrollOffset() {
        if (mContent == null) return 0;
        checkThreadSafety();
        return mContent.computeVerticalScrollOffset();
    }

    /**
     * Compute the vertical extent of the vertical scrollbar's thumb
     * within the vertical range.
     * @return vertical offset of the vertical scrollbar's thumb.
     * @since 6.0
     */
    @XWalkAPI
    public int computeVerticalScrollExtent() {
        if (mContent == null) return 0;
        checkThreadSafety();
        return mContent.computeVerticalScrollExtent();
    }

    /**
     * Get the external extension manager for current XWalkView.
     * Embedders could employ this manager to load their own external extensions.
     * @return the external extension manager.
     */
    @XWalkAPI
    public XWalkExternalExtensionManagerInternal getExtensionManager() {
        if (mContent == null) return null;
        checkThreadSafety();
        return mExternalExtensionManager;
    }

    /**
     * XWalkExternalExtensionManagerInternal will call this function after its construction.
     * @hide
     */
    public void setExternalExtensionManager(XWalkExternalExtensionManagerInternal manager) {
        if (mContent == null) return;
        checkThreadSafety();
        mExternalExtensionManager = manager;
    }

    /**
     * Clears the SSL preferences table stored in response to proceeding with
     * SSL certificate errors.
     * @since 6.0
     */
    @XWalkAPI
    public void clearSslPreferences() {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.clearSslPreferences();
    }

    /**
     * Clears the client certificate preferences stored in response to
     * proceeding/cancelling client cert requests.
     * @since 6.0
     */
    @XWalkAPI
    public void clearClientCertPreferences(Runnable callback) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.clearClientCertPreferences(callback);
    }

    /**
     * Gets the SSL certificate for the main top-level page or null
     * if there is no certificate (the site is not secure).
     * @return the SSL certificate for the main top-level page.
     * @since 6.0
     */
    @XWalkAPI
    public SslCertificate getCertificate() {
        if (mContent == null) return null;
        checkThreadSafety();
        return mContent.getCertificate();
    }

    /**
     * Registers the listener to be notified as find-on-page operations progress.
     *
     * @param listener an implementation of {@link XWalkFindListener}
     * @since 7.0
     */
    @XWalkAPI(reservable = true)
    public void setFindListener(XWalkFindListenerInternal listener) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.setFindListener(listener);
    }

    /**
     * Finds all instances of find on the page and highlights them asynchronously.
     * Notifies any registered {@link XWalkFindListener}.
     * Successive calls to this will cancel any pending searches.
     *
     * @param searchString the string to find.
     * @since 7.0
     */
    @XWalkAPI
    public void findAllAsync(String searchString) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.findAllAsync(searchString);
    }

    /**
     * Highlights and scrolls to the next match found by {@link #findAllAsync},
     * wrapping around page boundaries as necessary.
     * Notifies any registered {@link XWalkFindListener}.
     * If {@link #findAllAsync} has not been called yet, or if {@link #clearMatches} has been
     * called since the last find operation, this function does nothing.
     *
     * @param forward the direction to search
     * @since 7.0
     */
    @XWalkAPI
    public void findNext(boolean forward) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.findNext(forward);
    }

    /**
     * Clears the highlighting surrounding text matches created by {@link #findAllAsync}.
     *
     * @since 7.0
     */
    @XWalkAPI
    public void clearMatches() {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.clearMatches();
    }

    /**
     * Gets the compositing surface type of this XWalkView.
     * @return SurfaceView or TextureView
     * @since 7.0
     */
    @XWalkAPI
    public String getCompositingSurfaceType() {
        checkThreadSafety();
        if (mContent == null) return null;
        return mContent.getCompositingSurfaceType();
    }

    @Override
    protected void onAttachedToWindow() {
        if (mContent == null) return;
        checkThreadSafety();

        super.onAttachedToWindow();
        mContent.onAttachedToWindow();
    }

    @Override
    protected void onDetachedFromWindow() {
        if (mContent == null) return;
        checkThreadSafety();

        super.onDetachedFromWindow();
        mContent.onDetachedFromWindow();
    }

    @Override
    protected void onVisibilityChanged(View changedView, int visibility) {
        if (mContent == null) return;
        checkThreadSafety();

        super.onVisibilityChanged(changedView, visibility);
        mContent.onVisibilityChanged(changedView, visibility);
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        if (mContent == null) return;
        checkThreadSafety();

        ContentViewClient client = mContent.getContentViewClient();

        // Allow the ContentViewClient to override the ContentView's width.
        int desiredWidthMeasureSpec = client.getDesiredWidthMeasureSpec();
        if (MeasureSpec.getMode(desiredWidthMeasureSpec) != MeasureSpec.UNSPECIFIED) {
            widthMeasureSpec = desiredWidthMeasureSpec;
        }

        // Allow the ContentViewClient to override the ContentView's height.
        int desiredHeightMeasureSpec = client.getDesiredHeightMeasureSpec();
        if (MeasureSpec.getMode(desiredHeightMeasureSpec) != MeasureSpec.UNSPECIFIED) {
            heightMeasureSpec = desiredHeightMeasureSpec;
        }

        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
    }

    @Override
    protected void onSizeChanged(int w, int h, int ow, int oh) {
        if (mContent == null) return;
        checkThreadSafety();

        super.onSizeChanged(w, h, ow, oh);
        mContent.onSizeChanged(w, h, ow, oh);
    }

    @Override
    public void onScrollChanged(int l, int t, int oldl, int oldt) {
        if (mContent == null) return;
        checkThreadSafety();

        super.onScrollChanged(l, t, oldl, oldt);
        onScrollChangedDelegate(l, t, oldl, oldt);

        // To keep the same behaviour with WebView onOverScrolled API,
        // call onOverScrolledDelegate here.
        onOverScrolledDelegate(l, t, false, false);
    }

    @Override
    protected void onFocusChanged(boolean gainFocus, int direction, Rect previouslyFocusedRect) {
        if (mContent == null) return;
        checkThreadSafety();

        super.onFocusChanged(gainFocus, direction, previouslyFocusedRect);
        onFocusChangedDelegate(gainFocus, direction, previouslyFocusedRect);
        mContent.onFocusChanged(gainFocus);
    }

    @Override
    public void onWindowFocusChanged(boolean hasWindowFocus) {
        if (mContent == null) return;
        checkThreadSafety();

        super.onWindowFocusChanged(hasWindowFocus);
        mContent.onWindowFocusChanged(hasWindowFocus);
    }

    @Override
    public boolean performLongClick() {
        checkThreadSafety();

        return performLongClickDelegate();
    }

    @Override
    public boolean onCheckIsTextEditor() {
        if (mContent == null) return false;
        checkThreadSafety();

        return mContent.onCheckIsTextEditor();
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (mContent == null) return false;
        checkThreadSafety();

        return mContent.onKeyUp(keyCode, event);
    }

    @Override
    public boolean dispatchKeyEventPreIme(KeyEvent event) {
        if (mContent == null) return false;
        checkThreadSafety();

        return mContent.dispatchKeyEventPreIme(event);
    }

    /**
     * Mouse move events are sent on hover enter, hover move and hover exit.
     * They are sent on hover exit because sometimes it acts as both a hover
     * move and hover exit.
     */
    @Override
    public boolean onHoverEvent(MotionEvent event) {
        if (mContent == null) return false;
        checkThreadSafety();

        boolean consumed = mContent.onHoverEvent(event);
        if (!mContent.isTouchExplorationEnabled()) super.onHoverEvent(event);
        return consumed;
    }

    @Override
    public boolean onGenericMotionEvent(MotionEvent event) {
        if (mContent == null) return false;
        checkThreadSafety();

        return mContent.onGenericMotionEvent(event);
    }

    @Override
    protected void onConfigurationChanged(Configuration newConfig) {
        if (mContent == null) return;
        checkThreadSafety();

        mContent.onConfigurationChanged(newConfig);
    }

    /**
     * Compute the horizontal extent of the horizontal scrollbar's thumb within the horizontal
     * range. This value is used to compute the length of the thumb within the scrollbar's track.
     * @return the horizontal extent of the scrollbar's thumb.
     * @since 7.0
     */
    @Override
    @XWalkAPI
    public int computeHorizontalScrollExtent() {
        if (mContent == null) return 0;
        checkThreadSafety();

        return mContent.computeHorizontalScrollExtent();
    }

    @Override
    public boolean awakenScrollBars(int startDelay, boolean invalidate) {
        if (mContent == null) return false;
        checkThreadSafety();

        return mContent.awakenScrollBars(startDelay, invalidate);
    }

    @Override
    @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
    public boolean performAccessibilityAction(int action, Bundle arguments) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN) {
            return false;
        }
        if (mContent == null) return false;
        checkThreadSafety();

        if (mContent.supportsAccessibilityAction(action)) {
            return mContent.performAccessibilityAction(action, arguments);
        }

        return super.performAccessibilityAction(action, arguments);
    }

    @Override
    @TargetApi(Build.VERSION_CODES.JELLY_BEAN)
    public AccessibilityNodeProvider getAccessibilityNodeProvider() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN) {
            return null;
        }
        if (mContent == null) return null;
        checkThreadSafety();

        AccessibilityNodeProvider provider = mContent.getAccessibilityNodeProvider();
        if (provider != null) {
            return provider;
        } else {
            return super.getAccessibilityNodeProvider();
        }
    }

    @Override
    @TargetApi(Build.VERSION_CODES.M)
    public void onProvideVirtualStructure(final ViewStructure structure) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
            return;
        }
        if (mContent == null) return;
        checkThreadSafety();

        mContent.onProvideVirtualStructure(structure);
    }

    // Start: Needed by ContentViewCore.InternalAccessDelegate.
    @Override
    public boolean super_onKeyUp(int keyCode, KeyEvent event) {
        checkThreadSafety();

        return super.onKeyUp(keyCode, event);
    }

    @Override
    public boolean super_dispatchKeyEventPreIme(KeyEvent event) {
        checkThreadSafety();

        return super.dispatchKeyEventPreIme(event);
    }

    @Override
    public boolean super_dispatchKeyEvent(KeyEvent event) {
        checkThreadSafety();


        return super.dispatchKeyEvent(event);
    }

    @Override
    public boolean super_onGenericMotionEvent(MotionEvent event) {
        checkThreadSafety();

        return super.onGenericMotionEvent(event);
    }

    @Override
    public void super_onConfigurationChanged(Configuration newConfig) {
        checkThreadSafety();

        super.onConfigurationChanged(newConfig);
    }

    @Override
    public boolean awakenScrollBars() {
        checkThreadSafety();

        return super.awakenScrollBars();
    }

    @Override
    public boolean super_awakenScrollBars(int startDelay, boolean invalidate) {
        checkThreadSafety();

        return super.awakenScrollBars(startDelay, invalidate);
    }
    // End: Needed by ContentViewCore.InternalAccessDelegate.

    // Start: Needed by SmartClipProvider.
    @Override
    public void extractSmartClipData(int x, int y, int width, int height) {
        if (mContent == null) return;
        checkThreadSafety();

        mContent.extractSmartClipData(x, y, width, height);
    }

    @Override
    public void setSmartClipResultHandler(final Handler resultHandler) {
        if (mContent == null) return;
        checkThreadSafety();

        mContent.setSmartClipResultHandler(resultHandler);
    }
    // End: Needed by SmartClipProvider.

    public ContentViewRenderView getContentViewRenderView() {
        if (mContent == null) return null;
        checkThreadSafety();

        return mContent.getContentViewRenderView();
    }
}
