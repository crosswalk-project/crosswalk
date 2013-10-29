// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.view.View;

import org.xwalk.runtime.extension.XWalkExtension;
import org.xwalk.runtime.extension.XWalkExtensionContext;
import org.xwalk.runtime.extension.XWalkExtensionContextImpl;
import org.xwalk.runtime.extension.XWalkExtensionManager;

/**
 * The interface to provide the bridge between XWalkRuntimeView and the
 * real implementation like runtime core.
 *
 * This class is also used by XWalkExtensionManager to get the capability to
 * build runtime extension system.
 */
public interface XWalkRuntimeViewProvider {
    // For the embedded mode, the two arguments are the same.
    public void init(Context context, Activity activity);
    public XWalkExtensionContext getExtensionContext();

    // For handling life cycle and activity result.
    public void onCreate();
    public void onResume();
    public void onPause();
    public void onDestroy();
    public void onActivityResult(int requestCode, int resultCode, Intent data);

    // For RuntimeView APIs.
    public String getVersion();
    public View getView();
    public void loadAppFromUrl(String url);
    public void loadAppFromManifest(String manifestUrl);
    public String enableRemoteDebugging(String frontEndUrl, String socketName);
    public void disableRemoteDebugging();

    /*
     * Once runtime extension is created, notify the internal mechanism so that
     * it can create/do something it wants.
     */
    public abstract Object onExtensionRegistered(XWalkExtension extension);
    public abstract void onExtensionUnregistered(XWalkExtension extension);

    /**
     * Pass messages from native extension system to runtime extension system.
     * Might be overrided to do customizations here.
     */
    public void onMessage(XWalkExtension extension, int instanceID, String message);

    /**
     * Pass synchronized messages.
     */
    public String onSyncMessage(XWalkExtension extension, int instanceID, String message);

    /**
     * Pass message from runtime extension system to native and then to JavaScript.
     */
    public abstract void postMessage(XWalkExtension extension, int instanceID, String message);

    /**
     * Broadcast message from runtime extension system to native and then to JavaScript.
     * It means Java side will post the message to all instances of the extension.
     */
    public abstract void broadcastMessage(XWalkExtension extension, String message);

    /**
     * Destroy the extension, including the native resources allocated for it.
     */
    public abstract void destroyExtension(XWalkExtension extension);

    // For instrumentation test.
    public String getTitleForTest();
    public void setCallbackForTest(Object callback);
    public void loadDataForTest(String data, String mimeType, boolean isBase64Encoded);
}
