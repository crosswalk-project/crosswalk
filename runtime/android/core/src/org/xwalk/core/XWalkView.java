// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
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
import android.webkit.ValueCallback;
import android.widget.FrameLayout;

import org.chromium.base.ActivityState;
import org.chromium.base.ApplicationStatus;

import org.xwalk.core.extension.XWalkExtensionManager;

/**
 * XWalkView represents an Android view for web apps/pages. Thus most of attributes
 * for Android view are true for this class. It includes an instance of
 * android.view.SurfaceView for rendering. Currently limitations for android.view.SurfaceView
 * also are applied for this class as well, like resizing, retation, transformation and
 * animation.
 *
 * It provides 2 major callback classes, namely XWalkResourceClient and XWalkUIClient for
 * listening to the events related resource loading and UI. By default, Crosswalk has an inner
 * implementation. Callers can override them if like.
 *
 * Unlike other Android views, this class has to listen to system events like application life
 * cycle, intents, and activity result. The web engine inside this view need to handle them.
 *
 * It already includes all newly created Web APIs from Crosswalk like Presentation,
 * DeviceCapabilities, etc..
 */
public class XWalkView extends android.widget.FrameLayout {

    private XWalkContent mContent;
    private Activity mActivity;
    private Context mContext;
    private XWalkExtensionManager mExtensionManager;

    /**
     * Constructor for inflating via XML.
     * @param context  a Context object used to access application assets.
     * @param attrs    an AttributeSet passed to our parent.
     */
    public XWalkView(Context context, AttributeSet attrs) {
        super(context, attrs);

        checkThreadSafety();
        mContext = context;
        init(context, attrs);
    }

    /**
     * Constructor for Crosswalk runtime. In shared mode, context isi
     * different from activity. In embedded mode, they're same.
     * @param context  a Context object used to access application assets
     * @param activity the activity for this XWalkView.
     */
    public XWalkView(Context context, Activity activity) {
        super(context, null);
        checkThreadSafety();

        // Make sure mActivity is initialized before calling 'init' method.
        mActivity = activity;
        mContext = context;
        init(context, null);
    }

    /**
     * Get the current activity passed from callers. It's never null.
     * @return the activity instance passed from callers.
     *
     * @hide
     */
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

    // TODO(yongsheng): we should remove this since we have getContext()?
    /**
     * @hide
     */
    public Context getViewContext() {
        return mContext;
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


        // Set default XWalkClientImpl.
        setXWalkClient(new XWalkClient(this));
        // Set default XWalkWebChromeClient and DownloadListener. The default actions
        // are provided via the following clients if special actions are not needed.
        setXWalkWebChromeClient(new XWalkWebChromeClient(this));

        // Set with internal implementation. Could be overwritten by embedders'
        // setting.
        setUIClient(new XWalkUIClient(this));
        setResourceClient(new XWalkResourceClient(this));

        setDownloadListener(new XWalkDownloadListenerImpl(context));
        setNavigationHandler(new XWalkNavigationHandlerImpl(context));
        setNotificationService(new XWalkNotificationServiceImpl(context, this));

        // Enable xwalk extension mechanism and start load extensions here.
        // Note that it has to be after above initialization.
        mExtensionManager = new XWalkExtensionManager(context, getActivity());
        mExtensionManager.loadExtensions();
    }

    /**
     * Load a web page/app from a given base URL or a content. If content is
     * specified, load the web page/app from the content. If it's null, try to
     * load the content from the baseUrl and return "about:blank" if calling
     * {@link XWalkView#getUrl()}.
     * @param baseUrl the base url for web page/app.
     * @param content the content for the web page/app. Could be empty.
     */
    public void load(String baseUrl, String content) {
        checkThreadSafety();
        mContent.loadUrl(baseUrl, content);
    }

    /**
     * Load a web app from a given manifest.json file. The content must not be
     * empty.
     * @param baseUrl the base url for manifest.json.
     * @param content the content for manifest.json.
     */
    public void loadAppFromManifest(String baseUrl, String content) {
        mContent.loadAppFromManifest(baseUrl, content);
    }

    /**
     * The reload mode.
     */
    public enum ReloadMode {
        /** Normal reload as default. */
        NORMAL,
        /** Reload bypassing the cache. */
        IGNORE_CACHE
    }

    /**
     * Reload a web app with a given mode.
     * @param the reload mode.
     */
    public void reload(ReloadMode mode) {
        checkThreadSafety();
        mContent.reload(mode);
    }

    /**
     * Stop current loading progress.
     */
    public void stopLoading() {
        checkThreadSafety();
        mContent.stopLoading();
    }

    /**
     * Get the url of current web page/app. This may be different from what's passed
     * by caller.
     * @return the url for current web page/app.
     */
    public String getUrl() {
        checkThreadSafety();
        return mContent.getUrl();
    }

    /**
     * Get the title of current web page/app. This may be different from what's passed
     * by caller.
     * @return the title for current web page/app.
     */
    public String getTitle() {
        checkThreadSafety();
        return mContent.getTitle();
    }

    /**
     * Get the original url specified by caller.
     * @return the original url.
     */
    public String getOriginalUrl() {
        checkThreadSafety();
        return mContent.getOriginalUrl();
    }

    /**
     * Get the navigation history for current XWalkView. It's synchronized with
     * this XWalkView if any backward/forward and navigation operations.
     * @return the navigation history.
     */
    public XWalkNavigationHistory getNavigationHistory() {
        return mContent.getNavigationHistory();
    }

    /**
     * Injects the supplied Java object into this XWalkView.
     * @param object the supplied Java object, called by JavaScript.
     * @param name the name injected in JavaScript.
     */
    public void addJavascriptInterface(Object object, String name) {
        checkThreadSafety();
        mContent.addJavascriptInterface(object, name);
    }

    /**
     * Evaluate a fragment of JavaScript code and get the result via callback.
     * @param script the JavaScript string.
     * @param callback the callback to handle the evaluated result.
     */
    public void evaluateJavascript(String script, ValueCallback<String> callback) {
        checkThreadSafety();
        mContent.evaluateJavascript(script, callback);
    }

    /**
     * Clear the resource cache. Note that the cache is per-application, so this
     * will clear the cache for all XWalkViews used.
     * @param includeDiskFiles indicate whether to clear disk files for cache.
     */
    public void clearCache(boolean includeDiskFiles) {
        checkThreadSafety();
        mContent.clearCache(includeDiskFiles);
    }

    /**
     * Indicate that a HTML element is occupying the whole screen.
     * @return true if any HTML element is occupying the whole screen.
     */
    public boolean hasEnteredFullscreen() {
        checkThreadSafety();
        return mContent.hasEnteredFullscreen();
    }

    /**
     * Leave fullscreen mode if it's. Do nothing if it's not
     * in fullscreen.
     */
    public void leaveFullscreen() {
        checkThreadSafety();
        mContent.exitFullscreen();
    }

    /**
     * Pause timers of rendering engine. Typically it should be called
     * when the activity for this view is paused.
     */
    public void pauseTimers() {
        checkThreadSafety();
        mContent.pauseTimers();
    }

    /**
     * Resume timers of rendering engine. Typically it should be called
     * when the activyt for this view is resumed.
     */
    public void resumeTimers() {
        checkThreadSafety();
        mContent.resumeTimers();
    }

    /**
     * Aside from timers, this method can pause many other things inside
     * rendering engine, like video player, modal dialogs, etc.
     * Typically it should be called when the activity for this view is paused.
     */
    public void onHide() {
        mExtensionManager.onPause();
        mContent.onPause();
    }

    /**
     * Resume video player, modal dialogs. Embedders are in charge of calling
     * this during resuming this activity if they call onHide.
     * Typically it should be called when the activity for this view is resumed.
     */
    public void onShow() {
        mExtensionManager.onResume();
        mContent.onResume();
    }

    /**
     * Release internal resources occupied by this XWalkView.
     */
    public void onDestroy() {
        destroy();
    }

    /**
     * Pass through activity result to XWalkView. Many internal facilities need this
     * to handle activity result like JavaScript dialog, Crosswalk extensions, etc.
     * See android.app.Activity.onActivityResult().
     * @param requestCode passed from android.app.Activity.onActivityResult().
     * @param resultCode passed from android.app.Activity.onActivityResult().
     * @param data passed from android.app.Activity.onActivityResult().
     */
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        mExtensionManager.onActivityResult(requestCode, resultCode, data);
        mContent.onActivityResult(requestCode, resultCode, data);
    }

    /**
     * Pass through intents to XWalkView. Many internal facilities need this
     * to receive the intents like web notification. See
     * android.app.Activity.onNewIntent().
     * @param intent passed from android.app.Activity.onNewIntent().
     */
    public boolean onNewIntent(Intent intent) {
        return mContent.onNewIntent(intent);
    }

    /**
     * Save current internal state of this XWalkView. This can help restore this state
     * afterwards restoring.
     * @param outState the saved state for restoring.
     */
    public boolean saveState(Bundle outState) {
        mContent.saveState(outState);
        return true;
    }

    /**
     * Restore the state from the saved bundle data.
     * @param inState the state saved from saveState().
     * @return true if it can restore the state.
     */
    public boolean restoreState(Bundle inState) {
        if (mContent.restoreState(inState) != null) return true;
        return false;
    }

    /**
     * Get the API version of Crosswalk embedding API.
     * @return the string of API level.
     */
    // TODO(yongsheng): make it static?
    public String getAPIVersion() {
        return "1.0";
    }

    /**
     * Get the Crosswalk version.
     * @return the string of Crosswalk.
     */
    // TODO(yongsheng): make it static?
    public String getXWalkVersion() {
        return mContent.getXWalkVersion();
    }

    /**
     * Embedders use this to customize their handlers to events/callbacks related
     * to UI.
     * @param client the XWalkUIClient defined by callers.
     */
    public void setUIClient(XWalkUIClient client) {
        checkThreadSafety();
        mContent.setUIClient(client);
    }

    /**
     * Embedders use this to customize their handlers to events/callbacks related
     * to resource loading.
     * @param client the XWalkResourceClient defined by callers.
     */
    public void setResourceClient(XWalkResourceClient client) {
        checkThreadSafety();
        mContent.setResourceClient(client);
    }

    /**
     * Inherit from android.view.View. This class needs to handle some keys like
     * 'BACK'.
     * @param keyCode passed from android.view.View.onKeyUp().
     * @param event passed from android.view.View.onKeyUp().
     */
    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
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
        return false;
    }

    // TODO(yongsheng): this is not public.
    /**
     * @hide
     */
    public XWalkSettings getSettings() {
        checkThreadSafety();
        return mContent.getSettings();
    }

    /**
     * This method is used by Cordova for hacking.
     * TODO(yongsheng): remove this and related test cases?
     *
     * @hide
     */
    public void setNetworkAvailable(boolean networkUp) {
        checkThreadSafety();
        mContent.setNetworkAvailable(networkUp);
    }

    /**
     * Enables remote debugging and returns the URL at which the dev tools server is listening
     * for commands. The allowedUid argument can be used to specify the uid of the process that is
     * permitted to connect.
     * TODO(yongsheng): how to enable this in XWalkPreferences?
     *
     * @hide
     */
    public String enableRemoteDebugging(int allowedUid) {
        checkThreadSafety();
        return mContent.enableRemoteDebugging(allowedUid);
    }

    /**
     * It's used for Presentation API.
     * @hide
     */
    public int getContentID() {
        return mContent.getRoutingID();
    }

    boolean canGoBack() {
        checkThreadSafety();
        return mContent.canGoBack();
    }

    void goBack() {
        checkThreadSafety();
        mContent.goBack();
    }

    boolean canGoForward() {
        checkThreadSafety();
        return mContent.canGoForward();
    }

    void goForward() {
        checkThreadSafety();
        mContent.goForward();
    }

    void clearHistory() {
        checkThreadSafety();
        mContent.clearHistory();
    }

    void destroy() {
        mExtensionManager.onDestroy();
        mContent.destroy();
        disableRemoteDebugging();
    }

    // Enables remote debugging and returns the URL at which the dev tools server is listening
    // for commands. Only the current process is allowed to connect to the server.
    String enableRemoteDebugging() {
        return enableRemoteDebugging(mContext.getApplicationInfo().uid);
    }

    void disableRemoteDebugging() {
        checkThreadSafety();
        mContent.disableRemoteDebugging();
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

    boolean isOwnerActivityRunning() {
        int status = ApplicationStatus.getStateForActivity(getActivity());
        if (status == ActivityState.DESTROYED) return false;
        return true;
    }

    void navigateTo(int offset) {
        mContent.navigateTo(offset);
    }

    void setOverlayVideoMode(boolean enabled) {
        mContent.setOverlayVideoMode(enabled);
    }

    // Below methods are for test shell and instrumentation tests.
    /**
     * @hide
     */
    public void setXWalkClient(XWalkClient client) {
        checkThreadSafety();
        mContent.setXWalkClient(client);
    }

    /**
     * @hide
     */
    public void setXWalkWebChromeClient(XWalkWebChromeClient client) {
        checkThreadSafety();
        mContent.setXWalkWebChromeClient(client);
    }

    /**
     * @hide
     */
    public void setDownloadListener(DownloadListener listener) {
        checkThreadSafety();
        mContent.setDownloadListener(listener);
    }

    /**
     * @hide
     */
    public void setNavigationHandler(XWalkNavigationHandler handler) {
        checkThreadSafety();
        mContent.setNavigationHandler(handler);
    }

    /**
     * @hide
     */
    public void setNotificationService(XWalkNotificationService service) {
        checkThreadSafety();
        mContent.setNotificationService(service);
    }
}
