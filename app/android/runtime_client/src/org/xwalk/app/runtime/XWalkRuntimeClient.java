// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.app.runtime;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.View;
import android.widget.FrameLayout;

import java.lang.reflect.Method;
import java.util.StringTokenizer;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * This class is to encapsulate the reflection detail of
 * invoking XWalkRuntimeView class in library APK.
 *
 * A web application APK should use this class in its Activity.
 */
public class XWalkRuntimeClient extends CrossPackageWrapper {
    private final static String RUNTIME_VIEW_CLASS_NAME = "org.xwalk.runtime.XWalkRuntimeView";
    private boolean mRuntimeLoaded = false;
    private Object mInstance;
    private Method mLoadAppFromUrl;
    private Method mLoadAppFromManifest;
    private Method mOnCreate;
    private Method mOnResume;
    private Method mOnPause;
    private Method mOnDestroy;
    private Method mOnActivityResult;
    private Method mEnableRemoteDebugging;
    private Method mDisableRemoteDebugging;
    private Method mOnKeyUp;

    // For instrumentation test.
    private Method mGetTitleForTest;
    private Method mSetCallbackForTest;
    private Method mLoadDataForTest;

    public XWalkRuntimeClient(Activity activity, AttributeSet attrs, CrossPackageWrapperExceptionHandler exceptionHandler) {
        super(activity, RUNTIME_VIEW_CLASS_NAME, exceptionHandler, Activity.class, Context.class, AttributeSet.class);
        Context libCtx = getLibraryContext();
        mInstance = this.createInstance(activity, libCtx, attrs);
        Method getVersion = lookupMethod("getVersion");
        String libVersion = (String) invokeMethod(getVersion, mInstance);
        if (libVersion == null || !compareVersion(libVersion, getVersion())) {
            handleException(new XWalkRuntimeLibraryException(
                    XWalkRuntimeLibraryException.XWALK_RUNTIME_LIBRARY_NOT_UP_TO_DATE_CRITICAL));
            return;
        }
        mRuntimeLoaded = true;
        mLoadAppFromUrl = lookupMethod("loadAppFromUrl", String.class);
        mLoadAppFromManifest = lookupMethod("loadAppFromManifest", String.class);
        mOnCreate = lookupMethod("onCreate");
        mOnResume = lookupMethod("onResume");
        mOnPause = lookupMethod("onPause");
        mOnDestroy = lookupMethod("onDestroy");
        mOnActivityResult = lookupMethod("onActivityResult", int.class, int.class, Intent.class);
        mEnableRemoteDebugging = lookupMethod("enableRemoteDebugging", String.class, String.class);
        mDisableRemoteDebugging = lookupMethod("disableRemoteDebugging");
        mOnKeyUp = lookupMethod("onKeyUp", int.class, KeyEvent.class);
    }

    /**
     * Compare the given versions.
     * @param libVersion version of library apk
     * @param clientVersion version of client
     * @return true if library is not older than client, false otherwise or either of the version string
     * is invalid. Valid string should be \d+[\.\d+]*
     */
    private static boolean compareVersion(String libVersion, String clientVersion) {
        if (libVersion.equals(clientVersion)) {
            return true;
        }
        Pattern version = Pattern.compile("\\d+(\\.\\d+)*");
        Matcher lib = version.matcher(libVersion);
        Matcher client = version.matcher(clientVersion);
        if (lib.matches() && client.matches()) {
            StringTokenizer libTokens = new StringTokenizer(libVersion, ".");
            StringTokenizer clientTokens = new StringTokenizer(clientVersion, ".");
            int libTokenCount = libTokens.countTokens();
            int clientTokenCount = clientTokens.countTokens();
            if (libTokenCount == clientTokenCount) {
                while (libTokens.hasMoreTokens()) {
                    int libValue = 0;
                    int clientValue = 0;
                    try {
                        libValue = Integer.parseInt(libTokens.nextToken());
                        clientValue = Integer.parseInt(clientTokens.nextToken());
                    } catch (NumberFormatException e) {
                        return false;
                    }
                    if (libValue == clientValue) continue;
                    return libValue > clientValue;
                }
                return true;
            } else {
                return libTokenCount > clientTokenCount;
            }
        }
        return false;
    }

    @Override
    public void handleException(Exception e) {
        if (mRuntimeLoaded) {
            super.handleException(new XWalkRuntimeLibraryException(
                    XWalkRuntimeLibraryException.XWALK_RUNTIME_LIBRARY_INVOKE_FAILED, e));
        } else {
            super.handleException(new XWalkRuntimeLibraryException(
                    XWalkRuntimeLibraryException.XWALK_RUNTIME_LIBRARY_LOAD_FAILED, e));
        }
    }

    public FrameLayout get() {
        return (FrameLayout) mInstance;
    }

    /**
     * Get the version information of current runtime client.
     *
     * @return the string containing the version information.
     */
    public static String getVersion() {
        return XWalkRuntimeClientVersion.XWALK_RUNTIME_CLIENT_VERSION;
    }

    /**
     * Load a web application through the entry url. It may be
     * a file from assets or a url from network.
     *
     * @param url the url of loaded html resource.
     */
    public void loadAppFromUrl(String url) {
        invokeMethod(mLoadAppFromUrl, mInstance, url);
    }

    /**
     * Load a web application through the url of the manifest file.
     * The manifest file typically is placed in android assets. Now it is
     * compliant to W3C SysApps spec.
     *
     * @param manifestUrl the url of the manifest file
     */
    public void loadAppFromManifest(String manifestUrl) {
        invokeMethod(mLoadAppFromManifest, mInstance, manifestUrl);
    }

    /**
     * Tell runtime that the application is on creating. This can make runtime
     * be aware of application life cycle.
     */
    public void onCreate() {
        invokeMethod(mOnCreate, mInstance);
    }

    /**
     * Tell runtime that the application is on resuming. This can make runtime
     * be aware of application life cycle.
     */
    public void onResume() {
        invokeMethod(mOnResume, mInstance);
    }

    /**
     * Tell runtime that the application is on pausing. This can make runtime
     * be aware of application life cycle.
     */
    public void onPause() {
        invokeMethod(mOnPause, mInstance);
    }

    /**
     * Tell runtime that the application is on destroying. This can make runtime
     * be aware of application life cycle.
     */
    public void onDestroy() {
        invokeMethod(mOnDestroy, mInstance);
    }

    /**
     * Tell runtime that one activity exists so that it can know the result code
     * of the exit code.
     *
     * @param requestCode the request code to identify where the result is from
     * @param resultCode the result code of the activity
     * @param data the data to contain the result data
     */
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        invokeMethod(mOnActivityResult, mInstance, requestCode, resultCode, data);
    }

    /**
     * Enable remote debugging for the loaded web application. The caller
     * can set the url of debugging url. Besides, the socket name for remote
     * debugging has to be unique so typically the string can be appended
     * with the package name of the application.
     *
     * @param frontEndUrl the url of debugging url. If it's empty, then a
     *                    default url will be used.
     * @param socketName the unique socket name for setting up socket for
     *                   remote debugging.
     * @return the url of web socket for remote debugging.
     */
    public String enableRemoteDebugging(String frontEndUrl, String socketName) {
        return (String) invokeMethod(mEnableRemoteDebugging, mInstance, frontEndUrl, socketName);
    }

    /**
     * Disable remote debugging so runtime can close related stuff for
     * this feature.
     */
    public void disableRemoteDebugging() {
        invokeMethod(mDisableRemoteDebugging, mInstance);
    }

    /**
     * Passdown key-up event to the runtime view.
     * Usually meet the case of clicking on the back key.
     */
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        return (Boolean) invokeMethod(mOnKeyUp, mInstance, keyCode, event);
    }

    // The following functions just for instrumentation test.
    public View getViewForTest() {
        return (View)mInstance;
    }

    public String getTitleForTest() {
        if (mGetTitleForTest == null) {
            mGetTitleForTest = lookupMethod("getTitleForTest");
        }

        return (String) invokeMethod(mGetTitleForTest, mInstance);
    }

    public void setCallbackForTest(Object callback) {
        if (mSetCallbackForTest == null) {
            mSetCallbackForTest = lookupMethod("setCallbackForTest", Object.class);
        }

        invokeMethod(mSetCallbackForTest, mInstance, callback);
    }

    public void loadDataForTest(String data, String mimeType, boolean isBase64Encoded) {
        if (mLoadDataForTest == null) {
            mLoadDataForTest = lookupMethod("loadDataForTest", String.class, String.class, boolean.class);
        }

        invokeMethod(mLoadDataForTest, mInstance, data, mimeType, isBase64Encoded);
    }
}
