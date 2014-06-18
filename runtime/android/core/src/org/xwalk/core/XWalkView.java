// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ApplicationErrorReport;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Rect;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Looper;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.ViewGroup;
import android.webkit.ValueCallback;
import android.widget.FrameLayout;

import java.io.PrintWriter;
import java.io.StringWriter;

import org.chromium.base.ActivityState;
import org.chromium.base.ApplicationStatus;

import org.xwalk.core.extension.XWalkExtensionManager;
import org.xwalk.core.extension.XWalkPathHelper;

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
public class XWalkView extends android.widget.FrameLayout {

    static final String PLAYSTORE_DETAIL_URI = "market://details?id=";

    private XWalkContent mContent;
    private Activity mActivity;
    private Context mContext;
    private XWalkExtensionManager mExtensionManager;
    private boolean mIsHidden;

    /** Normal reload mode as default. */
    public static final int RELOAD_NORMAL = 0;
    /** Reload mode with bypassing the cache. */
    public static final int RELOAD_IGNORE_CACHE = 1;

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
        try {
            XWalkViewDelegate.init(this);
        } catch (UnsatisfiedLinkError e) {
            final UnsatisfiedLinkError err = e;
            final Activity activity = getActivity();
            final String packageName = context.getPackageName();
            String missingArch = XWalkViewDelegate.isRunningOnIA() ? "Intel" : "ARM";
            final String message =
                    context.getString(R.string.cpu_arch_mismatch_message, missingArch);

            AlertDialog.Builder builder = new AlertDialog.Builder(activity);
            builder.setTitle(R.string.cpu_arch_mismatch_title)
                    .setMessage(message)
                    .setOnCancelListener(new DialogInterface.OnCancelListener() {
                        @Override
                        public void onCancel(DialogInterface dialog) {
                            activity.finish();
                        }
                    }).setPositiveButton(R.string.goto_store_button_label,
                            new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            activity.startActivity(new Intent(Intent.ACTION_VIEW,
                                    Uri.parse(PLAYSTORE_DETAIL_URI + packageName)));
                            activity.finish();
                        }
                    }).setNeutralButton(R.string.report_feedback_button_label,
                            new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            ApplicationErrorReport report = new ApplicationErrorReport();
                            report.type = ApplicationErrorReport.TYPE_CRASH;
                            report.packageName = report.processName = packageName;

                            ApplicationErrorReport.CrashInfo crash =
                                    new ApplicationErrorReport.CrashInfo();
                            crash.exceptionClassName = err.getClass().getSimpleName();
                            crash.exceptionMessage = "CPU architecture mismatch";
                            StringWriter writer = new StringWriter();
                            PrintWriter print = new PrintWriter(writer);
                            err.printStackTrace(print);
                            crash.stackTrace = writer.toString();
                            StackTraceElement stack = err.getStackTrace()[0];
                            crash.throwClassName = stack.getClassName();
                            crash.throwFileName = stack.getFileName();
                            crash.throwLineNumber = stack.getLineNumber();
                            crash.throwMethodName = stack.getMethodName();

                            report.crashInfo = crash;
                            report.systemApp = false;
                            report.time = System.currentTimeMillis();

                            Intent intent = new Intent(Intent.ACTION_APP_ERROR);
                            intent.putExtra(Intent.EXTRA_BUG_REPORT, report);
                            activity.startActivity(intent);
                            activity.finish();
                        }
                    });
            builder.create().show();
            return;
        }

        initXWalkContent(context, attrs);
    }

    private void initXWalkContent(Context context, AttributeSet attrs) {
        mIsHidden = false;
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

        XWalkPathHelper.initialize();
        XWalkPathHelper.setCacheDirectory(
                mContext.getApplicationContext().getCacheDir().getPath());

        String state = Environment.getExternalStorageState();
        if (Environment.MEDIA_MOUNTED.equals(state) ||
                Environment.MEDIA_MOUNTED_READ_ONLY.equals(state)) {
            XWalkPathHelper.setExternalCacheDirectory(
                    mContext.getApplicationContext().getExternalCacheDir().getPath());
        }
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
     */
    public void load(String url, String content) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.loadUrl(url, content);
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
     */
    public void loadAppFromManifest(String url, String content) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.loadAppFromManifest(url, content);
    }

    /**
     * Reload a web app with a given mode.
     * @param mode the reload mode.
     */
    public void reload(int mode) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.reload(mode);
    }

    /**
     * Stop current loading progress.
     */
    public void stopLoading() {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.stopLoading();
    }

    /**
     * Get the url of current web page/app. This may be different from what's passed
     * by caller.
     * @return the url for current web page/app.
     */
    public String getUrl() {
        if (mContent == null) return null;
        checkThreadSafety();
        return mContent.getUrl();
    }

    /**
     * Get the title of current web page/app. This may be different from what's passed
     * by caller.
     * @return the title for current web page/app.
     */
    public String getTitle() {
        if (mContent == null) return null;
        checkThreadSafety();
        return mContent.getTitle();
    }

    /**
     * Get the original url specified by caller.
     * @return the original url.
     */
    public String getOriginalUrl() {
        if (mContent == null) return null;
        checkThreadSafety();
        return mContent.getOriginalUrl();
    }

    /**
     * Get the navigation history for current XWalkView. It's synchronized with
     * this XWalkView if any backward/forward and navigation operations.
     * @return the navigation history.
     */
    public XWalkNavigationHistory getNavigationHistory() {
        if (mContent == null) return null;
        checkThreadSafety();
        return mContent.getNavigationHistory();
    }

    /**
     * Injects the supplied Java object into this XWalkView.
     * Each method defined in the class of the object should be
     * marked with {@link JavascriptInterface} if it's called by JavaScript.
     * @param object the supplied Java object, called by JavaScript.
     * @param name the name injected in JavaScript.
     */
    public void addJavascriptInterface(Object object, String name) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.addJavascriptInterface(object, name);
    }

    /**
     * Evaluate a fragment of JavaScript code and get the result via callback.
     * @param script the JavaScript string.
     * @param callback the callback to handle the evaluated result.
     */
    public void evaluateJavascript(String script, ValueCallback<String> callback) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.evaluateJavascript(script, callback);
    }

    /**
     * Clear the resource cache. Note that the cache is per-application, so this
     * will clear the cache for all XWalkViews used.
     * @param includeDiskFiles indicate whether to clear disk files for cache.
     */
    public void clearCache(boolean includeDiskFiles) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.clearCache(includeDiskFiles);
    }

    /**
     * Indicate that a HTML element is occupying the whole screen.
     * @return true if any HTML element is occupying the whole screen.
     */
    public boolean hasEnteredFullscreen() {
        if (mContent == null) return false;
        checkThreadSafety();
        return mContent.hasEnteredFullscreen();
    }

    /**
     * Leave fullscreen mode if it's. Do nothing if it's not
     * in fullscreen.
     */
    public void leaveFullscreen() {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.exitFullscreen();
    }

    /**
     * Pause all layout, parsing and JavaScript timers for all XWalkView instances.
     * Typically it should be called when the activity for this view is paused,
     * and accordingly {@link #resumeTimers} should be called when the activity
     * is resumed again.
     *
     * Note that it will globally impact all XWalkView instances, not limited to
     * just this XWalkView.
     */
    public void pauseTimers() {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.pauseTimers();
    }

    /**
     * Resume all layout, parsing and JavaScript timers for all XWalkView instances.
     * Typically it should be called when the activity for this view is resumed.
     *
     * Note that it will globally impact all XWalkView instances, not limited to
     * just this XWalkView.
     */
    public void resumeTimers() {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.resumeTimers();
    }

    /**
     * Pause many other things except JavaScript timers inside rendering engine,
     * like video player, modal dialogs, etc. See {@link #pauseTimers} about pausing
     * JavaScript timers.
     * Typically it should be called when the activity for this view is paused.
     */
    public void onHide() {
        if (mContent == null || mIsHidden) return;
        mExtensionManager.onPause();
        mContent.onPause();
        mIsHidden = true;
    }

    /**
     * Resume video player, modal dialogs. Embedders are in charge of calling
     * this during resuming this activity if they call onHide.
     * Typically it should be called when the activity for this view is resumed.
     */
    public void onShow() {
        if (mContent == null || !mIsHidden ) return;
        mExtensionManager.onResume();
        mContent.onResume();
        mIsHidden = false;
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
     * See <a href="http://developer.android.com/reference/android/app/Activity.html">
     * android.app.Activity.onActivityResult()</a>.
     * @param requestCode passed from android.app.Activity.onActivityResult().
     * @param resultCode passed from android.app.Activity.onActivityResult().
     * @param data passed from android.app.Activity.onActivityResult().
     */
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (mContent == null) return;
        mExtensionManager.onActivityResult(requestCode, resultCode, data);
        mContent.onActivityResult(requestCode, resultCode, data);
    }

    /**
     * Pass through intents to XWalkView. Many internal facilities need this
     * to receive the intents like web notification. See
     * <a href="http://developer.android.com/reference/android/app/Activity.html">
     * android.app.Activity.onNewIntent()</a>.
     * @param intent passed from android.app.Activity.onNewIntent().
     */
    public boolean onNewIntent(Intent intent) {
        if (mContent == null) return false;
        return mContent.onNewIntent(intent);
    }

    /**
     * Save current internal state of this XWalkView. This can help restore this state
     * afterwards restoring.
     * @param outState the saved state for restoring.
     */
    public boolean saveState(Bundle outState) {
        if (mContent == null) return false;
        mContent.saveState(outState);
        return true;
    }

    /**
     * Restore the state from the saved bundle data.
     * @param inState the state saved from saveState().
     * @return true if it can restore the state.
     */
    public boolean restoreState(Bundle inState) {
        if (mContent == null) return false;
        if (mContent.restoreState(inState) != null) return true;
        return false;
    }

    /**
     * Get the API version of Crosswalk embedding API.
     * @return the string of API level.
     */
    // TODO(yongsheng): make it static?
    public String getAPIVersion() {
        return "2.0";
    }

    /**
     * Get the Crosswalk version.
     * @return the string of Crosswalk.
     */
    // TODO(yongsheng): make it static?
    public String getXWalkVersion() {
        if (mContent == null) return null;
        return mContent.getXWalkVersion();
    }

    /**
     * Embedders use this to customize their handlers to events/callbacks related
     * to UI.
     * @param client the XWalkUIClient defined by callers.
     */
    public void setUIClient(XWalkUIClient client) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.setUIClient(client);
    }

    /**
     * Embedders use this to customize their handlers to events/callbacks related
     * to resource loading.
     * @param client the XWalkResourceClient defined by callers.
     */
    public void setResourceClient(XWalkResourceClient client) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.setResourceClient(client);
    }

    /**
     * Inherit from <a href="http://developer.android.com/reference/android/view/View.html">
     * android.view.View</a>. This class needs to handle some keys like
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
        if (mContent == null) return null;
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
        if (mContent == null) return;
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
        if (mContent == null) return null;
        checkThreadSafety();
        return mContent.enableRemoteDebugging(allowedUid);
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
        if (mContent == null) return;
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
        if (mContent == null) return;
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
     * @hide
     */
    public void setDownloadListener(DownloadListener listener) {
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
}
