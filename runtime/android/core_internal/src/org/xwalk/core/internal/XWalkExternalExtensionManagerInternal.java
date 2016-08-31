// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;

import org.chromium.base.ActivityState;

/**
 * <p>XWalkExternalExtensionManagerInternal represents an external extension manager.
 * Its instance would be created While constructing XWalkView.
 * XWalkView embedders could get the manager by calling XWalkView.getExtensionManager(),
 * and then employ the manager to load their own external extensions by path,
 * although XWalkView embedders must package their external extensions into the apk beforehand.</p>
 *
 * <pre>What would XWalkView embedders do:
 * 1, package extension MyExtension.js/json files into apk folder assets/xwalk-extensions/MyExtension.
 * 2, package MyExtension.jar into apk.
 * 3, add permissions required by MyExtension into AndroidManifest.xml.
 * 4, call XWalkView.getExtensionManager().loadExtension("xwalk-extensions/MyExtension") to load.
 * </pre>
 *
 * <p>In embedded mode, developers can call XWalkView.getExtensionManager() to load extensions
 * directly after the creation of XWalkView. But in shared mode and lite mode, the Crosswalk runtime
 * isn't loaded yet at the moment the activity is created, so can't load extensions immediately.
 * To make your code compatible with all modes, please refer to the examples in {@link XWalkActivity} or
 * {@link XWalkInitializer} to make sure to load extensions after XWalk runtime ready.</p>
 */
@XWalkAPI(createExternally = true)
public abstract class XWalkExternalExtensionManagerInternal {

    private XWalkViewInternal mXWalkView;

    /**
     * Constructs a new XWalkExternalExtensionManagerInternal for the XWalkViewInternal.
     * @param view the current XWalkViewInternal object.
     */
    @XWalkAPI
    public XWalkExternalExtensionManagerInternal(XWalkViewInternal view) {
        view.setExternalExtensionManager(this);
        mXWalkView = view;
    }

    /**
     * Get current Activity for XWalkViewInternal.
     * @return the current Activity.
     * @deprecated This method is no longer supported
     */
    @Deprecated
    @XWalkAPI
    public Activity getViewActivity() {
        Context context = getViewContext();
        if (context instanceof Activity) {
            return (Activity) context;
        }
        return null;
    }


    /**
     * Get current Context for XWalkViewInternal.
     * @return the current Context.
     */
    @XWalkAPI
    public Context getViewContext() {
        if (mXWalkView != null) {
            return mXWalkView.getViewContext();
        }
        return null;
    }

    /**
     * Load one single external extension by its path.
     * Do nothing here, just expose to be implemented by XWalkExternalExtensionManagerImpl.
     * @param extensionPath the extension folder containing extension js/json files.
     */
    @XWalkAPI
    public void loadExtension(String extensionPath) {
        return;
    }

    /**
     * Notify onStart().
     * Extension manager should propagate to all external extensions.
     */
    @XWalkAPI
    public void onStart() {}

    /**
     * Notify onResume().
     * Extension manager should propagate to all external extensions.
     */
    @XWalkAPI
    public void onResume() {}

    /**
     * Notify onPause().
     * Extension manager should propagate to all external extensions.
     */
    @XWalkAPI
    public void onPause() {}

    /**
     * Notify onStop().
     * Extension manager should propagate to all external extensions.
     */
    @XWalkAPI
    public void onStop() {}

    /**
     * Notify onDestroy().
     * <strong>Pleaes invoke super.onDestroy() first when you overriding this method.</strong>
     * Extension manager should propagate to all external extensions.
     */
    @XWalkAPI
    public void onDestroy() {
        mXWalkView.setExternalExtensionManager(null);
        mXWalkView = null;
    }

    /**
     * Notify onNewIntent().
     * Extension manager should propagate to all external extensions.
     * @param intent the Intent received.
     */
    @XWalkAPI
    public abstract void onNewIntent(Intent intent);

    /**
     * Notify onActivityResult().
     * Extension manager should propagate to all external extensions.
     * @param requestCode the request code.
     * @param resultCode the result code.
     * @param data the Intent data received.
     * @deprecated This method is no longer supported
     */
    @Deprecated
    @XWalkAPI
    public void onActivityResult(int requestCode, int resultCode, Intent data) {}
}
