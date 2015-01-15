// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.Context;
import android.content.res.Resources;
import android.content.res.TypedArray;
import android.content.res.XmlResourceParser;
import android.graphics.drawable.Drawable;
import android.util.TypedValue;

/**
 * XWalkMixedResources is used to combine the resources
 * from two different packages.
 *
 * Chromium uses resources through:
 *   R.layout; R.id; R.string; R.dimen; R.drawable;
 *   R.attr; R.style; R.menu ; R.color
 * R.layout and R.menu is covered by Resources.getLayout()
 * R.string is covered by Resources.getText()
 * R.dimen, R.drawable and R.color is covered by getValue()
 *
 * For R.id, if it's used like findViewById(R.id.xxx), R.id.xxx
 * is const at compile time within library context. It works
 * if the view in hierachy has the same id, which needs inflate
 * with layout resource from library context. Layout is covered
 * by getLayout(), so R.id is OK.
 *
 * TODO(wang16):
 * For R.attr and R.style, I have no confidence that it's covered.
 * But the only place use this R.attr and R.style is "select" tag
 * which is verified working well with this MixedResources.
 */
public class XWalkMixedResources extends Resources {
    public static String MARKET_PROGRESS_MESSAGE = "Please wait until library installed";
    public static String DOWNLOAD_PROGRESS_MESSAGE = "Downloading Crosswalk Runtime Library";
    public static String DOWNLOAD_FAILED_TITLE = "Crosswalk Runtime Library download failed";
    public static String DOWNLOAD_FAILED_MESSAGE = "Please retry or cancel";
    public static String DOWNLOAD_FAILED_DEVICE_NOT_FOUND = "Storage device Not Found";
    public static String DOWNLOAD_FAILED_INSUFFICIENT_SPACE = "Insufficient storage space";
    public static String DOWNLOAD_FAILED_TIME_OUT = "Download time out";
    public static String STARTUP_NOT_FOUND_TITLE = "Crosswalk Runtime Library not found";
    public static String STARTUP_NOT_FOUND_MESSAGE = "Please get the library";
    public static String STARTUP_OLDER_VERSION_TITLE = "Crosswalk Runtime Library is out of date";
    public static String STARTUP_OLDER_VERSION_MESSAGE = "Please update the library or continue";
    public static String STARTUP_NEWER_VERSION_TITLE = "Crosswalk Runtime Library is too new";
    public static String STARTUP_NEWER_VERSION_MESSAGE = "Please update your app or continue";
    public static String RUNTIME_OLDER_VERSION_TITLE = "Crosswalk Runtime Library is out of date";
    public static String RUNTIME_OLDER_VERSION_MESSAGE = "Next operation may not be supported";
    public static String RUNTIME_NEWER_VERSION_TITLE = "Crosswalk Runtime Library is too new";
    public static String RUNTIME_NEWER_VERSION_MESSAGE = "Next operation may not be supported";
    public static String DECOMPRESS_LIBRARY_MESSAGE = "Runtime is preparing...";

    public static String CANCEL = "Cancel";
    public static String CONTINUE = "Continue";
    public static String TERMINATE = "Terminate";
    public static String RETRY = "Retry";
    public static String GET_LIBRARY = "Get Library";

    private Resources mLibraryResource;

    private boolean isCalledInLibrary() {
        StackTraceElement[] stacks = Thread.currentThread().getStackTrace();
        for (StackTraceElement stack : stacks) {
            String className = stack.getClassName();
            if (className.startsWith("org.chromium") ||
                    className.startsWith("org.xwalk.core.internal")) {
                return true;
            } else if (className.startsWith("org.xwalk.core") &&
                    !className.endsWith("XWalkMixedResources")) {
                return false;
            }
        }
        return false;
    }

    XWalkMixedResources(Resources base, Resources libraryResources) {
        super(base.getAssets(), base.getDisplayMetrics(),
                base.getConfiguration());
        mLibraryResource = libraryResources;
    }

    @Override
    public CharSequence getText(int id) throws NotFoundException {
        boolean calledInLibrary = isCalledInLibrary();
        try {
            if (calledInLibrary) return mLibraryResource.getText(id);
            else return super.getText(id);
        } catch (NotFoundException e) {
            if (calledInLibrary) return super.getText(id);
            else return mLibraryResource.getText(id);
        }
    }

    @Override
    public XmlResourceParser getLayout(int id) throws NotFoundException {
        boolean calledInLibrary = isCalledInLibrary();
        try {
            if (calledInLibrary) return mLibraryResource.getLayout(id);
            else return super.getLayout(id);
        } catch (NotFoundException e) {
            if (calledInLibrary) return super.getLayout(id);
            else return mLibraryResource.getLayout(id);
        }
    }

    @Override
    public void getValue(int id, TypedValue outValue, boolean resolveRefs) {
        boolean calledInLibrary = isCalledInLibrary();
        try {
            if (calledInLibrary) mLibraryResource.getValue(id, outValue, resolveRefs);
            else super.getValue(id, outValue, resolveRefs);
        } catch (NotFoundException e) {
            if (calledInLibrary) super.getValue(id, outValue, resolveRefs);
            else mLibraryResource.getValue(id, outValue, resolveRefs);
        }
    }

    @Override
    public void getValueForDensity(int id, int density, TypedValue outValue, boolean resolveRefs) {
        boolean calledInLibrary = isCalledInLibrary();
        try {
            if (calledInLibrary) mLibraryResource.getValueForDensity(id, density, outValue, resolveRefs);
            else super.getValueForDensity(id, density, outValue, resolveRefs);
        } catch (NotFoundException e) {
            if (calledInLibrary) super.getValueForDensity(id, density, outValue, resolveRefs);
            else mLibraryResource.getValueForDensity(id, density, outValue, resolveRefs);
        }
    }

    @Override
    public int getIdentifier(String name, String defType, String defPackage) {
        boolean calledInLibrary = isCalledInLibrary();
        if (calledInLibrary) {
            int id = mLibraryResource.getIdentifier(name, defType, defPackage);
            return id != 0 ? id : super.getIdentifier(name, defType, defPackage);
        } else {
            int id = super.getIdentifier(name, defType, defPackage);
            return id != 0 ? id : mLibraryResource.getIdentifier(name, defType, defPackage);
        }
    }

    @Override
    public Drawable getDrawable(int id) {
        boolean calledInLibrary = isCalledInLibrary();
        try {
            if (calledInLibrary) return mLibraryResource.getDrawable(id);
            else return super.getDrawable(id);
        } catch (NotFoundException e) {
            if (calledInLibrary) return super.getDrawable(id);
            else return mLibraryResource.getDrawable(id);
        }
    }
}
