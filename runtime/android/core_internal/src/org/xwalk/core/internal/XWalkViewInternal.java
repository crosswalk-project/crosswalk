// Copyright (c) 2013-2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

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
import android.provider.MediaStore;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.ViewGroup;
import android.webkit.ValueCallback;
import android.widget.FrameLayout;

import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.lang.ref.WeakReference;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;

import org.chromium.base.ActivityState;
import org.chromium.base.ApplicationStatus;
import org.chromium.base.ApplicationStatus.ActivityStateListener;
import org.chromium.base.CommandLine;
import org.xwalk.core.internal.extension.BuiltinXWalkExtensions;

/**
 * <p>XWalkViewInternal represents an Android view for web apps/pages. Thus most of attributes
 * for Android view are valid for this class. Since it internally uses
 * <a href="http://developer.android.com/reference/android/view/SurfaceView.html">
 * android.view.SurfaceView</a> for rendering web pages by default, it can't be resized,
 * rotated, transformed and animated due to the limitations of SurfaceView.
 * Alternatively, if the preference key {@link XWalkPreferencesInternal#ANIMATABLE_XWALK_VIEW}
 * is set to True, XWalkViewInternal can be transformed and animated because
 * <a href="http://developer.android.com/reference/android/view/TextureView.html">
 * TextureView</a> is intentionally used to render web pages for animation support.
 * Besides, XWalkViewInternal won't be rendered if it's invisible.</p>
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
 * <p>Unlike other Android views, this class has to listen to system events like intents and activity result.
 * The web engine inside this view need to get and handle them.
 * With contianer activity's lifecycle change, XWalkViewInternal will pause all timers and other
 * components like videos when activity paused, resume back them when activity resumed.
 * When activity is about to destroy, XWalkViewInternal will destroy itself as well.
 * Embedders can also call onHide() and pauseTimers() to explicitly pause XWalkViewInternal.
 * Similarily with onShow(), resumeTimers() and onDestroy().
 *
 * For example:</p>
 *
 * <pre>
 *   import android.app.Activity;
 *   import android.os.Bundle;
 *
 *   import org.xwalk.core.internal.XWalkResourceClientInternal;
 *   import org.xwalk.core.internal.XWalkUIClientInternal;
 *   import org.xwalk.core.internal.XWalkViewInternal;
 *
 *   public class MyActivity extends Activity {
 *       XWalkViewInternal mXwalkView;
 *
 *       class MyResourceClient extends XWalkResourceClientInternal {
 *           MyResourceClient(XWalkViewInternal view) {
 *               super(view);
 *           }
 *
 *           &#64;Override
 *           WebResourceResponse shouldInterceptLoadRequest(XWalkViewInternal view, String url) {
 *               // Handle it here.
 *               ...
 *           }
 *       }
 *
 *       class MyUIClient extends XWalkUIClientInternal {
 *           MyUIClient(XWalkViewInternal view) {
 *               super(view);
 *           }
 *
 *           &#64;Override
 *           void onFullscreenToggled(XWalkViewInternal view, String url) {
 *               // Handle it here.
 *               ...
 *           }
 *       }
 *
 *       &#64;Override
 *       protected void onCreate(Bundle savedInstanceState) {
 *           mXwalkView = new XWalkViewInternal(this, null);
 *           setContentView(mXwalkView);
 *           mXwalkView.setResourceClient(new MyResourceClient(mXwalkView));
 *           mXwalkView.setUIClient(new MyUIClient(mXwalkView));
 *           mXwalkView.load("http://www.crosswalk-project.org", null);
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
@XWalkAPI(extendClass = FrameLayout.class, createExternally = true)
public class XWalkViewInternal extends android.widget.FrameLayout {

    private class XWalkActivityStateListener implements ActivityStateListener {
        WeakReference<XWalkViewInternal> mXWalkViewRef;

        XWalkActivityStateListener(XWalkViewInternal view) {
            mXWalkViewRef = new WeakReference<XWalkViewInternal>(view);
        }

        @Override
        public void onActivityStateChange(Activity activity, int newState) {
            XWalkViewInternal view = mXWalkViewRef.get();
            if (view == null) return;
            view.onActivityStateChange(activity, newState);
        }
    }

    static final String PLAYSTORE_DETAIL_URI = "market://details?id=";
    public static final int INPUT_FILE_REQUEST_CODE = 1;
    private static final String TAG = XWalkViewInternal.class.getSimpleName();

    private XWalkContent mContent;
    private Activity mActivity;
    private Context mContext;
    private boolean mIsHidden;
    private XWalkActivityStateListener mActivityStateListener;
    private ValueCallback<Uri> mFilePathCallback;
    private String mCameraPhotoPath;

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
     * Constructor for inflating via XML.
     * @param context  a Context object used to access application assets.
     * @param attrs    an AttributeSet passed to our parent.
     * @since 1.0
     */
    @XWalkAPI(preWrapperLines = {
                  "        super(${param1}, ${param2});"},
              postWrapperLines = {
                  "        if (bridge == null) return;",
                  "        addView((FrameLayout)bridge, new FrameLayout.LayoutParams(",
                  "                FrameLayout.LayoutParams.MATCH_PARENT,",
                  "                FrameLayout.LayoutParams.MATCH_PARENT));"})
    public XWalkViewInternal(Context context, AttributeSet attrs) {
        super(convertContext(context), attrs);

        checkThreadSafety();
        mActivity = (Activity) context;
        mContext = getContext();
        init(mContext, attrs);
    }

    /**
     * Constructor for Crosswalk runtime. In shared mode, context isi
     * different from activity. In embedded mode, they're same.
     * @param context  a Context object used to access application assets
     * @param activity the activity for this XWalkViewInternal.
     * @since 1.0
     */
    @XWalkAPI(preWrapperLines = {
                  "        super(${param1}, null);"},
              postWrapperLines = {
                  "        if (bridge == null) return;",
                  "        addView((FrameLayout)bridge, new FrameLayout.LayoutParams(",
                  "                FrameLayout.LayoutParams.MATCH_PARENT,",
                  "                FrameLayout.LayoutParams.MATCH_PARENT));"})
    public XWalkViewInternal(Context context, Activity activity) {
        super(convertContext(context), null);

        checkThreadSafety();
        // Make sure mActivity is initialized before calling 'init' method.
        mActivity = activity;
        mContext = getContext();
        init(mContext, null);
    }

    private static Context convertContext(Context context) {
        Context ret = context;
        Context bridgeContext = ReflectionHelper.getBridgeContext();
        if (bridgeContext == null || context == null ||
                bridgeContext.getPackageName().equals(context.getPackageName())) {
            // Not acrossing package
            ret = context;
        } else {
            ret = new MixedContext(bridgeContext, context);
        }
        return ret;
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

    public void completeWindowCreation(XWalkViewInternal newXWalkView) {
        mContent.supplyContentsForPopup(newXWalkView == null ? null : newXWalkView.mContent);
    }

    private void init(Context context, AttributeSet attrs) {
        // Initialize chromium resources. Assign them the correct ids in
        // xwalk core.
        XWalkInternalResources.resetIds(context);

        // Intialize library, paks and others.
        try {
            XWalkViewDelegate.init(this);
            mActivityStateListener = new XWalkActivityStateListener(this);
            ApplicationStatus.registerStateListenerForActivity(
                    mActivityStateListener, getActivity());
        } catch (Throwable e) {
            // Try to find if there is UnsatisfiedLinkError in the cause chain of the met Throwable.
            Throwable linkError = e;
            while (true) {
                if (linkError == null) throw new RuntimeException(e);
                if (linkError instanceof UnsatisfiedLinkError) break;
                if (linkError.getCause() == null ||
                        linkError.getCause().equals(linkError)) {
                    throw new RuntimeException(e);
                }
                linkError = linkError.getCause();
            }
            final UnsatisfiedLinkError err = (UnsatisfiedLinkError) linkError;
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
        setUIClient(new XWalkUIClientInternal(this));
        setResourceClient(new XWalkResourceClientInternal(this));

        setDownloadListener(new XWalkDownloadListenerImpl(context));
        setNavigationHandler(new XWalkNavigationHandlerImpl(context));
        setNotificationService(new XWalkNotificationServiceImpl(context, this));

        if (!CommandLine.getInstance().hasSwitch("disable-xwalk-extensions")) {
            BuiltinXWalkExtensions.load(context, getActivity());
        } else {
            XWalkPreferencesInternal.setValue(XWalkPreferencesInternal.ENABLE_EXTENSIONS, false);
        }

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
    @XWalkAPI
    public void addJavascriptInterface(Object object, String name) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.addJavascriptInterface(object, name);
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
     * Pass through activity result to XWalkViewInternal. Many internal facilities need this
     * to handle activity result like JavaScript dialog, Crosswalk extensions, etc.
     * See <a href="http://developer.android.com/reference/android/app/Activity.html">
     * android.app.Activity.onActivityResult()</a>.
     * @param requestCode passed from android.app.Activity.onActivityResult().
     * @param resultCode passed from android.app.Activity.onActivityResult().
     * @param data passed from android.app.Activity.onActivityResult().
     * @since 1.0
     */
    @XWalkAPI
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (mContent == null) return;
        if(requestCode == INPUT_FILE_REQUEST_CODE && mFilePathCallback != null) {
            Uri results = null;

            // Check that the response is a good one
            if(Activity.RESULT_OK == resultCode) {
                if(data == null) {
                    // If there is not data, then we may have taken a photo
                    if(mCameraPhotoPath != null) {
                        results = Uri.parse(mCameraPhotoPath);
                    }
                } else {
                    String dataString = data.getDataString();
                    if (dataString != null) {
                        results = Uri.parse(dataString);
                    }
                }
            }

            mFilePathCallback.onReceiveValue(results);
            mFilePathCallback = null;
            return;
        }
        mContent.onActivityResult(requestCode, resultCode, data);
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
    // TODO(yongsheng): make it static?
    @XWalkAPI
    public String getAPIVersion() {
        return "4.1";
    }

    /**
     * Get the Crosswalk version.
     * @return the string of Crosswalk.
     * @since 1.0
     */
    // TODO(yongsheng): make it static?
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
    @XWalkAPI
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
    @XWalkAPI
    public void setResourceClient(XWalkResourceClientInternal client) {
        if (mContent == null) return;
        checkThreadSafety();
        mContent.setResourceClient(client);
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
        ApplicationStatus.unregisterActivityStateListener(mActivityStateListener);
        mActivityStateListener = null;
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
        return super.dispatchKeyEvent(event);
    }

    private void onActivityStateChange(Activity activity, int newState) {
        assert(getActivity() == activity);
        switch (newState) {
            case ActivityState.STARTED:
                onShow();
                break;
            case ActivityState.PAUSED:
                pauseTimers();
                break;
            case ActivityState.RESUMED:
                resumeTimers();
                break;
            case ActivityState.DESTROYED:
                onDestroy();
                break;
            case ActivityState.STOPPED:
                onHide();
                break;
            default:
                break;
        }
    }

    /**
     * Tell the client to show a file chooser.
     * @param uploadFile the callback class to handle the result from caller. It MUST
     *        be invoked in all cases. Leave it not invoked will block all following
     *        requests to open file chooser.
     * @param acceptType value of the 'accept' attribute of the input tag associated
     *        with this file picker.
     * @param capture value of the 'capture' attribute of the input tag associated
     *        with this file picker
     */
    public boolean showFileChooser(ValueCallback<Uri> uploadFile, String acceptType,
            String capture) {
        mFilePathCallback = uploadFile;

        Intent takePictureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        if (takePictureIntent.resolveActivity(getActivity().getPackageManager()) != null) {
            // Create the File where the photo should go
            File photoFile = null;
            try {
                photoFile = createImageFile();
                takePictureIntent.putExtra("PhotoPath", mCameraPhotoPath);
            } catch (IOException ex) {
                // Error occurred while creating the File
                Log.e(TAG, "Unable to create Image File", ex);
            }

            // Continue only if the File was successfully created
            if (photoFile != null) {
                mCameraPhotoPath = "file:" + photoFile.getAbsolutePath();
                takePictureIntent.putExtra(MediaStore.EXTRA_OUTPUT,
                        Uri.fromFile(photoFile));
            } else {
                takePictureIntent = null;
            }
        }

        Intent contentSelectionIntent = new Intent(Intent.ACTION_GET_CONTENT);
        contentSelectionIntent.addCategory(Intent.CATEGORY_OPENABLE);
        contentSelectionIntent.setType("*/*");

        Intent camcorder = new Intent(MediaStore.ACTION_VIDEO_CAPTURE);
        Intent soundRecorder = new Intent(
                MediaStore.Audio.Media.RECORD_SOUND_ACTION);
        ArrayList<Intent> extraIntents = new ArrayList<Intent>();
        extraIntents.add(takePictureIntent);
        extraIntents.add(camcorder);
        extraIntents.add(soundRecorder);

        Intent chooserIntent = new Intent(Intent.ACTION_CHOOSER);
        chooserIntent.putExtra(Intent.EXTRA_INTENT, contentSelectionIntent);
        chooserIntent.putExtra(Intent.EXTRA_TITLE, "Choose an action");
        chooserIntent.putExtra(Intent.EXTRA_INITIAL_INTENTS,
                extraIntents.toArray(new Intent[] { }));
        getActivity().startActivityForResult(chooserIntent, INPUT_FILE_REQUEST_CODE);
        return true;
    }

    private File createImageFile() throws IOException {
        // Create an image file name
        String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
        String imageFileName = "JPEG_" + timeStamp + "_";
        File storageDir = Environment.getExternalStoragePublicDirectory(
                Environment.DIRECTORY_PICTURES);
        File imageFile = File.createTempFile(
                imageFileName,  /* prefix */
                ".jpg",         /* suffix */
                storageDir      /* directory */
        );
        return imageFile;
    }
}
