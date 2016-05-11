// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.text.TextUtils;
import android.util.Log;

import java.io.UnsupportedEncodingException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URLDecoder;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.regex.Matcher;

import org.chromium.base.CollectionUtil;

public class UrlUtilities {
    private static final String TAG = "UrlUtilities";

    // Following codes were ported from
    // chrome/android/java/src/org/chromium/chrome/browser/util/UrlUtilities.java
    // Add Crosswalk specific schemes to "ACCEPTED_SCHEMES".
    /**
     * URI schemes that ContentView can handle.
     */
    private static final HashSet<String> ACCEPTED_SCHEMES = CollectionUtil.newHashSet(
            "about", "app", "content", "data", "file", "http", "https", "javascript");

    /**
     * URI schemes that can be handled in Intent fallback navigation.
     */
    private static final HashSet<String> FALLBACK_VALID_SCHEMES = CollectionUtil.newHashSet(
            "http", "https");

    /**
     * @param uri A URI.
     *
     * @return True if the URI is valid for Intent fallback navigation.
     */
    public static boolean isValidForIntentFallbackNavigation(String uri) {
        try {
            return isValidForIntentFallbackNavigation(new URI(uri));
        } catch (URISyntaxException e) {
            return false;
        }
    }

    /**
     * @param uri A URI.
     *
     * @return True if the URI is valid for Intent fallback navigation.
     */
    public static boolean isValidForIntentFallbackNavigation(URI uri) {
        return FALLBACK_VALID_SCHEMES.contains(uri.getScheme());
    }

    /**
     * @param uri A URI.
     *
     * @return True if the URI's scheme is one that ContentView can handle.
     */
    public static boolean isAcceptedScheme(URI uri) {
        return ACCEPTED_SCHEMES.contains(uri.getScheme());
    }

    /**
     * @param uri A URI.
     *
     * @return True if the URI's scheme is one that ContentView can handle.
     */
    public static boolean isAcceptedScheme(String uri) {
        try {
            return isAcceptedScheme(new URI(uri));
        } catch (URISyntaxException e) {
            return false;
        }
    }

    // Following codes were ported from
    // chrome/android/java/src/org/chromium/chrome/browser/util/IntentUtils.java
    /**
     * Just like {@link Intent#getStringExtra(String)} but doesn't throw exceptions.
     */
    public static String safeGetStringExtra(Intent intent, String name) {
        try {
            return intent.getStringExtra(name);
        } catch (Throwable t) {
            // Catches un-parceling exceptions.
            Log.e(TAG, "getStringExtra failed on intent: " + intent);
            return null;
        }
    }

    /**
     * Retrieves a list of components that would handle the given intent.
     * @param context The application context.
     * @param intent The intent which we are interested in.
     * @return The list of component names.
     */
    public static List<ComponentName> getIntentHandlers(Context context, Intent intent) {
        List<ResolveInfo> list = context.getPackageManager().queryIntentActivities(intent, 0);
        List<ComponentName> nameList = new ArrayList<ComponentName>();
        for (ResolveInfo r : list) {
            nameList.add(new ComponentName(r.activityInfo.packageName, r.activityInfo.name));
        }
        return nameList;
    }

    public static boolean isSpecializedHandlerAvailable(Context context, Intent intent) {
        return isPackageSpecializedHandler(context, null, intent);
    }

    // Following codes were ported from
    // chrome/android/java/src/org/chromium/chrome/browser/externalnav/ExternalNavigationDelegateImpl.java
    /**
     * Check whether the given package is a specialized handler for the given intent
     *
     * @param context {@link Context} to use for getting the {@link PackageManager}.
     * @param packageName Package name to check against. Can be null or empty.
     * @param intent The intent to resolve for.
     * @return Whether the given package is a specialized handler for the given intent. If there is
     *         no package name given checks whether there is any specialized handler.
     */
    public static boolean isPackageSpecializedHandler(
            Context context, String packageName, Intent intent) {
        try {
            List<ResolveInfo> handlers = context.getPackageManager().queryIntentActivities(
                    intent, PackageManager.GET_RESOLVED_FILTER);
            if (handlers == null || handlers.size() == 0) return false;
            for (ResolveInfo resolveInfo : handlers) {
                IntentFilter filter = resolveInfo.filter;
                if (filter == null) {
                    // No intent filter matches this intent?
                    // Error on the side of staying in the browser, ignore
                    continue;
                }
                if (filter.countDataAuthorities() == 0 || filter.countDataPaths() == 0) {
                    // Generic handler, skip
                    continue;
                }
                if (TextUtils.isEmpty(packageName)) return true;
                ActivityInfo activityInfo = resolveInfo.activityInfo;
                if (activityInfo == null) continue;
                if (!activityInfo.packageName.equals(packageName)) continue;
                return true;
            }
        } catch (RuntimeException e) {
            Log.e(TAG, "isPackageSpecializedHandler e=" + e);
        }
        return false;
    }
}
