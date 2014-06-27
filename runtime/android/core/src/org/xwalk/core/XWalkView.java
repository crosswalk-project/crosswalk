// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.AttributeSet;
import android.webkit.ValueCallback;

import org.xwalk.core.internal.XWalkNavigationHistoryInternal;
import org.xwalk.core.internal.XWalkPreferencesInternal;
import org.xwalk.core.internal.XWalkResourceClientInternal;
import org.xwalk.core.internal.XWalkUIClientInternal;
import org.xwalk.core.internal.XWalkViewInternal;

/**
 * <p>XWalkView represents an Android view for web apps/pages. Thus most of attributes
 * for Android view are valid for this class. Since it internally uses
 * <a href="http://developer.android.com/reference/android/view/SurfaceView.html">
 * android.view.SurfaceView</a> for rendering web pages by default, it can't be resized,
 * rotated, transformed and animated due to the limitations of SurfaceView.
 * Alternatively, if the preference key {@link XWalkPreferences#ANIMATABLE_XWALK_VIEW}
 * is set to True, XWalkView can be transformed and animated because
 * <a href="http://developer.android.com/reference/android/view/TextureView.html">
 * TextureView</a> is intentionally used to render web pages for animation support.
 * Besides, XWalkView won't be rendered if it's invisible.</p>
 *
 * <p>XWalkView needs hardware acceleration to render web pages. As a result, the
 * AndroidManifest.xml of the caller's app must be appended with the attribute
 * "android:hardwareAccelerated" and its value must be set as "true".</p>
 * <pre>
 * &lt;application android:name="android.app.Application" android:label="XWalkUsers"
 *     android:hardwareAccelerated="true"&gt;
 * </pre>
 *
 * <p>Crosswalk provides 2 major callback classes, namely {@link XWalkResourceClient} and
 * {@link XWalkUIClient} for listening to the events related to resource loading and UI.
 * By default, Crosswalk has a default implementation. Callers can override them if needed.</p>
 *
 * <p>Unlike other Android views, this class has to listen to system events like application life
 * cycle, intents, and activity result. The web engine inside this view need to get and handle
 * them. And the onDestroy() method of XWalkView MUST be called explicitly when an XWalkView
 * won't be used anymore, otherwise it will cause the memory leak from the native side of the web
 * engine. It's similar to the 
 * <a href="http://developer.android.com/reference/android/webkit/WebView.html#destroy()">
 * destroy()</a> method of Android WebView. For example:</p>
 *
 * <pre>
 *   import android.app.Activity;
 *   import android.os.Bundle;
 *
 *   import org.xwalk.core.XWalkResourceClient;
 *   import org.xwalk.core.XWalkUIClient;
 *   import org.xwalk.core.XWalkView;
 *
 *   public class MyActivity extends Activity {
 *       XWalkView mXwalkView;
 *
 *       class MyResourceClient extends XWalkResourceClient {
 *           MyResourceClient(XWalkView view) {
 *               super(view);
 *           }
 *
 *           &#64;Override
 *           WebResourceResponse shouldInterceptLoadRequest(XWalkView view, String url) {
 *               // Handle it here.
 *               ...
 *           }
 *       }
 *
 *       class MyUIClient extends XWalkUIClient {
 *           MyUIClient(XWalkView view) {
 *               super(view);
 *           }
 *
 *           &#64;Override
 *           void onFullscreenToggled(XWalkView view, String url) {
 *               // Handle it here.
 *               ...
 *           }
 *       }
 *
 *       &#64;Override
 *       protected void onCreate(Bundle savedInstanceState) {
 *           mXwalkView = new XWalkView(this, null);
 *           setContentView(mXwalkView);
 *           mXwalkView.setResourceClient(new MyResourceClient(mXwalkView));
 *           mXwalkView.setUIClient(new MyUIClient(mXwalkView));
 *           mXwalkView.load("http://www.crosswalk-project.org", null);
 *       }
 *
 *       &#64;Override
 *       protected void onPause() {
 *           super.onPause();
 *           if (mXwalkView != null) {
 *               mXwalkView.pauseTimers();
 *               mXwalkView.onHide();
 *           }
 *       }
 *
 *       &#64;Override
 *       protected void onResume() {
 *           super.onResume();
 *           if (mXwalkView != null) {
 *               mXwalkView.resumeTimers();
 *               mXwalkView.onShow();
 *           }
 *       }
 *
 *       &#64;Override
 *       protected void onDestroy() {
 *           super.onDestroy();
 *           if (mXwalkView != null) {
 *               mXwalkView.onDestroy();
 *           }
 *       }
 *
 *       &#64;Override
 *       protected void onActivityResult(int requestCode, int resultCode, Intent data) {
 *           if (mXwalkView != null) {
 *               mXwalkView.onActivityResult(requestCode, resultCode, data);
 *           }
 *       }
 *
 *       &#64;Override
 *       protected void onNewIntent(Intent intent) {
 *           if (mXwalkView != null) {
 *               mXwalkView.onNewIntent(intent);
 *           }
 *       }
 *   }
 * </pre>
 */
public class XWalkView extends XWalkViewInternal {

    /**
     * Normal reload mode as default.
     * @since 1.0
     */
    public static final int RELOAD_NORMAL = 0;
    /**
     * Reload mode with bypassing the cache.
     * @since 1.0
     */
    public static final int RELOAD_IGNORE_CACHE = 1;

    /**
     * Constructor for inflating via XML.
     * @param context  a Context object used to access application assets.
     * @param attrs    an AttributeSet passed to our parent.
     * @since 1.0
     */
    public XWalkView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    /**
     * Constructor for Crosswalk runtime. In shared mode, context isi
     * different from activity. In embedded mode, they're same.
     * @param context  a Context object used to access application assets
     * @param activity the activity for this XWalkView.
     * @since 1.0
     */
    public XWalkView(Context context, Activity activity) {
        super(context, activity);
    }

    /**
     * Load a web page/app from a given base URL or a content.
     * If url is null or empty and content is null or empty, then this function
     * will do nothing.
     * If content is not null, load the web page/app from the content.
     * If content is not null and the url is not set, return "about:blank" ifi
     * calling {@link XWalkView#getUrl()}.
     * If content is null, try to load the content from the url.
     *
     * It supports URL schemes like 'http:', 'https:' and 'file:'.
     * It can also load files from Android assets, e.g. 'file:///android_asset/'.
     * @param url the url for web page/app.
     * @param content the content for the web page/app. Could be empty.
     * @since 1.0
     */
    public void load(String url, String content) {
        super.load(url, content);
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
    public void loadAppFromManifest(String url, String content) {
        super.loadAppFromManifest(url, content);
    }

    /**
     * Reload a web app with a given mode.
     * @param mode the reload mode.
     * @since 1.0
     */
    public void reload(int mode) {
        super.reload(mode);
    }

    /**
     * Stop current loading progress.
     * @since 1.0
     */
    public void stopLoading() {
        super.stopLoading();
    }

    /**
     * Get the url of current web page/app. This may be different from what's passed
     * by caller.
     * @return the url for current web page/app.
     * @since 1.0
     */
    public String getUrl() {
        return super.getUrl();
    }

    /**
     * Get the title of current web page/app. This may be different from what's passed
     * by caller.
     * @return the title for current web page/app.
     * @since 1.0
     */
    public String getTitle() {
        return super.getTitle();
    }

    /**
     * Get the original url specified by caller.
     * @return the original url.
     * @since 1.0
     */
    public String getOriginalUrl() {
        return super.getOriginalUrl();
    }

    /**
     * Get the navigation history for current XWalkView. It's synchronized with
     * this XWalkView if any backward/forward and navigation operations.
     * @return the navigation history.
     * @since 1.0
     */
    public XWalkNavigationHistory getNavigationHistory() {
        XWalkNavigationHistoryInternal history = super.getNavigationHistory();
        if (history == null || history instanceof XWalkNavigationHistory) {
            return (XWalkNavigationHistory) history;
        }

        return new XWalkNavigationHistory(history);
    }

    /**
     * Injects the supplied Java object into this XWalkView.
     * Each method defined in the class of the object should be
     * marked with {@link JavascriptInterface} if it's called by JavaScript.
     * @param object the supplied Java object, called by JavaScript.
     * @param name the name injected in JavaScript.
     * @since 1.0
     */
    public void addJavascriptInterface(Object object, String name) {
        super.addJavascriptInterface(object, name);
    }

    /**
     * Evaluate a fragment of JavaScript code and get the result via callback.
     * @param script the JavaScript string.
     * @param callback the callback to handle the evaluated result.
     * @since 1.0
     */
    public void evaluateJavascript(String script, ValueCallback<String> callback) {
        super.evaluateJavascript(script, callback);
    }

    /**
     * Clear the resource cache. Note that the cache is per-application, so this
     * will clear the cache for all XWalkViews used.
     * @param includeDiskFiles indicate whether to clear disk files for cache.
     * @since 1.0
     */
    public void clearCache(boolean includeDiskFiles) {
        super.clearCache(includeDiskFiles);
    }

    /**
     * Indicate that a HTML element is occupying the whole screen.
     * @return true if any HTML element is occupying the whole screen.
     * @since 1.0
     */
    public boolean hasEnteredFullscreen() {
        return super.hasEnteredFullscreen();
    }

    /**
     * Leave fullscreen mode if it's. Do nothing if it's not
     * in fullscreen.
     * @since 1.0
     */
    public void leaveFullscreen() {
        super.leaveFullscreen();
    }

    /**
     * Pause all layout, parsing and JavaScript timers for all XWalkView instances.
     * Typically it should be called when the activity for this view is paused,
     * and accordingly {@link #resumeTimers} should be called when the activity
     * is resumed again.
     *
     * Note that it will globally impact all XWalkView instances, not limited to
     * just this XWalkView.
     *
     * @since 1.0
     */
    public void pauseTimers() {
        super.pauseTimers();
    }

    /**
     * Resume all layout, parsing and JavaScript timers for all XWalkView instances.
     * Typically it should be called when the activity for this view is resumed.
     *
     * Note that it will globally impact all XWalkView instances, not limited to
     * just this XWalkView.
     *
     * @since 1.0
     */
    public void resumeTimers() {
        super.resumeTimers();
    }

    /**
     * Pause many other things except JavaScript timers inside rendering engine,
     * like video player, modal dialogs, etc. See {@link #pauseTimers} about pausing
     * JavaScript timers.
     * Typically it should be called when the activity for this view is paused.
     * @since 1.0
     */
    public void onHide() {
        super.onHide();
    }

    /**
     * Resume video player, modal dialogs. Embedders are in charge of calling
     * this during resuming this activity if they call onHide.
     * Typically it should be called when the activity for this view is resumed.
     * @since 1.0
     */
    public void onShow() {
        super.onShow();
    }

    /**
     * Release internal resources occupied by this XWalkView.
     * @since 1.0
     */
    public void onDestroy() {
        super.onDestroy();
    }

    /**
     * Pass through activity result to XWalkView. Many internal facilities need this
     * to handle activity result like JavaScript dialog, Crosswalk extensions, etc.
     * See <a href="http://developer.android.com/reference/android/app/Activity.html">
     * android.app.Activity.onActivityResult()</a>.
     * @param requestCode passed from android.app.Activity.onActivityResult().
     * @param resultCode passed from android.app.Activity.onActivityResult().
     * @param data passed from android.app.Activity.onActivityResult().
     * @since 1.0
     */
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
    }

    /**
     * Pass through intents to XWalkView. Many internal facilities need this
     * to receive the intents like web notification. See
     * <a href="http://developer.android.com/reference/android/app/Activity.html">
     * android.app.Activity.onNewIntent()</a>.
     * @param intent passed from android.app.Activity.onNewIntent().
     * @since 1.0
     */
    public boolean onNewIntent(Intent intent) {
        return super.onNewIntent(intent);
    }

    /**
     * Save current internal state of this XWalkView. This can help restore this state
     * afterwards restoring.
     * @param outState the saved state for restoring.
     * @since 1.0
     */
    public boolean saveState(Bundle outState) {
        return super.saveState(outState);
    }

    /**
     * Restore the state from the saved bundle data.
     * @param inState the state saved from saveState().
     * @return true if it can restore the state.
     * @since 1.0
     */
    public boolean restoreState(Bundle inState) {
        return super.restoreState(inState);
    }

    /**
     * Get the API version of Crosswalk embedding API.
     * @return the string of API level.
     * @since 1.0
     */
    // TODO(yongsheng): make it static?
    public String getAPIVersion() {
        return super.getAPIVersion();
    }

    /**
     * Get the Crosswalk version.
     * @return the string of Crosswalk.
     * @since 1.0
     */
    // TODO(yongsheng): make it static?
    public String getXWalkVersion() {
        return super.getXWalkVersion();
    }

    /**
     * Embedders use this to customize their handlers to events/callbacks related
     * to UI.
     * @param client the XWalkUIClient defined by callers.
     * @since 1.0
     */
    public void setUIClient(XWalkUIClient client) {
        super.setUIClient(client);
    }

    /**
     * Embedders use this to customize their handlers to events/callbacks related
     * to resource loading.
     * @param client the XWalkResourceClient defined by callers.
     * @since 1.0
     */
    public void setResourceClient(XWalkResourceClient client) {
        super.setResourceClient(client);
    }
}
