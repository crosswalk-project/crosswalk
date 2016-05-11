// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.net.Uri;
import android.provider.Browser;
import android.text.TextUtils;
import android.util.Log;
import android.webkit.MimeTypeMap;

import java.net.URI;
import java.util.List;

import org.chromium.components.navigation_interception.NavigationParams;
import org.chromium.ui.base.PageTransition;

import org.xwalk.core.internal.XWalkNavigationHandler;

/**
 * @hide
 */
public class XWalkNavigationHandlerImpl implements XWalkNavigationHandler {
    private static final String TAG = "XWalkNavigationHandlerImpl";

    // WTAI prefix.
    private static final String PROTOCOL_WTAI_PREFIX = "wtai://";
    private static final String PROTOCOL_WTAI_MC_PREFIX = "wtai://wp/mc;";

    // Android action uri prefix.
    private static final String ACTION_TEL_PREFIX = "tel:";
    private static final String ACTION_SMS_PREFIX = "sms:";
    private static final String ACTION_MAIL_PREFIX = "mailto:";
    private static final String ACTION_GEO_PREFIX = "geo:";
    private static final String ACTION_MARKET_PREFIX = "market:";
    private static final String ACTION_INTENT_PREFIX = "intent:";

    private Context mContext;

    public static final String EXTRA_BROWSER_FALLBACK_URL = "browser_fallback_url";
    private String mFallbackUrl;

    public XWalkNavigationHandlerImpl(Context context) {
        mContext = context;
    }

    @Override
    public boolean handleNavigation(NavigationParams params) {
        final String url = params.url;
        if (UrlUtilities.isAcceptedScheme(url)) return false;
        Intent intent = null;
        if (url.startsWith(PROTOCOL_WTAI_PREFIX)) {
            intent = createIntentForWTAI(url);
        } else {
            intent = createIntentForActionUri(url);
        }
        if (intent == null && shouldOverrideUrlLoadingInternal(params)) return true;
        if (intent != null && startActivity(intent)) return true;

        return handleUrlByMimeType(url);
    }

    protected boolean startActivity(Intent intent) {
        try {
            if (!(mContext instanceof Activity)) {
                intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            }
            mContext.startActivity(intent);
        } catch (ActivityNotFoundException exception) {
            Log.w(TAG, "Activity not found for Intent:");
            Log.w(TAG, intent.toUri(0));
            return false;
        }
        return true;
    }

    private Intent createIntentForWTAI(String url) {
        Intent intent = null;
        if (url.startsWith(PROTOCOL_WTAI_MC_PREFIX)) {
            String number = url.substring(PROTOCOL_WTAI_MC_PREFIX.length());
            String mcUrl = ACTION_TEL_PREFIX + number;
            intent = new Intent(Intent.ACTION_DIAL);
            intent.setData(Uri.parse(mcUrl));
        }
        return intent;
    }

    private Intent createIntentForActionUri(String url) {
        Intent intent = null;
        if (url.startsWith(ACTION_TEL_PREFIX)) {
            // If dialing phone (tel:5551212).
            intent = new Intent(Intent.ACTION_DIAL);
            intent.setData(Uri.parse(url));
        } else if (url.startsWith(ACTION_GEO_PREFIX)) {
            // If displaying map (geo:0,0?q=address).
            intent = new Intent(Intent.ACTION_VIEW);
            intent.setData(Uri.parse(url));
        } else if (url.startsWith(ACTION_MAIL_PREFIX)) {
            // If sending email (mailto:abc@corp.com).
            intent = new Intent(Intent.ACTION_VIEW);
            intent.setData(Uri.parse(url));
        } else if (url.startsWith(ACTION_SMS_PREFIX)) {
            // If sms:5551212?body=This is the message.
            intent = new Intent(Intent.ACTION_VIEW);

            // Get address.
            String address = null;
            int parmIndex = url.indexOf('?');
            if (parmIndex == -1) {
                address = url.substring(4);
            } else {
                address = url.substring(4, parmIndex);

                // If body, then set sms body.
                Uri uri = Uri.parse(url);
                String query = uri.getQuery();
                if (query != null) {
                    if (query.startsWith("body=")) {
                        intent.putExtra("sms_body", query.substring(5));
                    }
                }
            }
            intent.setData(Uri.parse(ACTION_SMS_PREFIX + address));
            intent.putExtra("address", address);
            intent.setType("vnd.android-dir/mms-sms");
        } else if (url.startsWith(ACTION_MARKET_PREFIX)) {
            // If Android Market.
            intent = new Intent(Intent.ACTION_VIEW);
            intent.setData(Uri.parse(url));
        }
        return intent;
    }

    private boolean handleUrlByMimeType(String url) {
        MimeTypeMap map = MimeTypeMap.getSingleton();
        String extenstion = MimeTypeMap.getFileExtensionFromUrl(url);
        String mimeType = map.getMimeTypeFromExtension(extenstion);

        if (shouldHandleMimeType(mimeType)) {
            Intent sendIntent = new Intent();
            sendIntent.setAction(Intent.ACTION_VIEW);
            sendIntent.setDataAndType(Uri.parse(url), mimeType);

            if (sendIntent.resolveActivity(mContext.getPackageManager()) != null) {
                startActivity(sendIntent);
                return true;
            }
        }
        return false;
    }

    private boolean shouldHandleMimeType(String mimeType) {
        // Currently only "application/*" MIME type should be handled.
        // Other types (text/*, video/*, image/*, audio/*) are not handled
        // by intent actions.
        if (mimeType != null && mimeType.startsWith("application/")) {
            // "application/xhtml+xml" should not be handled by intent actions. See XWALK-2912.
            if (mimeType == "application/xhtml+xml" || mimeType == "application/xml")
                return false;
            return true;
        }
        return false;
    }

    @Override
    public String getFallbackUrl() {
        return mFallbackUrl;
    }

    @Override
    public void resetFallbackUrl() {
        mFallbackUrl = null;
    }

    // Following codes were ported from
    // chrome/android/java/src/org/chromium/chrome/browser/externalnav/ExternalNavigationHandler.java
    // Removed the Chrome specific codes.
    private boolean shouldOverrideUrlLoadingInternal(NavigationParams params) {
        // Perform generic parsing of the URI to turn it into an Intent.
        Intent intent;
        final String url = params.url;
        try {
            intent = Intent.parseUri(url, Intent.URI_INTENT_SCHEME);
        } catch (Exception ex) {
            Log.w(TAG, "Bad URI=" + url + " ex=" + ex);
            return false;
        }
        // pageTransition is a combination of an enumeration (core value) and bitmask.
        int pageTransitionCore = params.pageTransitionType & PageTransition.CORE_MASK;
        boolean isLink = pageTransitionCore == PageTransition.LINK;
        boolean isFormSubmit = pageTransitionCore == PageTransition.FORM_SUBMIT;
        boolean isFromIntent = (params.pageTransitionType & PageTransition.FROM_API) != 0;
        boolean isForwardBackNavigation =
                (params.pageTransitionType & PageTransition.FORWARD_BACK) != 0;
        boolean isExternalProtocol = !UrlUtilities.isAcceptedScheme(url);

        // http://crbug.com/169549 : If you type in a URL that then redirects in server side to an
        // link that cannot be rendered by the browser, we want to show the intent picker.
        boolean isTyped = pageTransitionCore == PageTransition.TYPED;
        boolean typedRedirectToExternalProtocol = isTyped && params.isRedirect
                && isExternalProtocol;

        boolean hasBrowserFallbackUrl = false;
        String browserFallbackUrl =
                UrlUtilities.safeGetStringExtra(intent, EXTRA_BROWSER_FALLBACK_URL);
        if (browserFallbackUrl != null
                && UrlUtilities.isValidForIntentFallbackNavigation(browserFallbackUrl)) {
            hasBrowserFallbackUrl = true;
        } else {
            browserFallbackUrl = null;
        }

        // We do not want to show the intent picker for core types typed, bookmarks, auto toplevel,
        // generated, keyword, keyword generated. See below for exception to typed URL and
        // redirects:
        // - http://crbug.com/143118 : URL intercepting should not be invoked on navigations
        //   initiated by the user in the omnibox / NTP.
        // - http://crbug.com/159153 : Don't override http or https URLs from the NTP or bookmarks.
        // - http://crbug.com/162106: Intent picker should not be presented on returning to a page.
        //   This should be covered by not showing the picker if the core type is reload.

        // http://crbug.com/164194 . A navigation forwards or backwards should never trigger
        // the intent picker.
        if (isForwardBackNavigation) {
            return false;
        }

        // http://crbug.com/149218: We want to show the intent picker for ordinary links, providing
        // the link is not an incoming intent from another application, unless it's a redirect (see
        // below).
        boolean linkNotFromIntent = isLink && !isFromIntent;

        // http://crbug.com/170925: We need to show the intent picker when we receive an intent from
        // another app that 30x redirects to a YouTube/Google Maps/Play Store/Google+ URL etc.
        boolean incomingIntentRedirect = isLink && isFromIntent && params.isRedirect;

        // http://crbug.com/181186: We need to show the intent picker when we receive a redirect
        // following a form submit.
        boolean isRedirectFromFormSubmit = isFormSubmit && params.isRedirect;

        if (!typedRedirectToExternalProtocol) {
            if (!linkNotFromIntent && !incomingIntentRedirect && !isRedirectFromFormSubmit) {
                return false;
            }
        }

        // Special case - It makes no sense to use an external application for a YouTube
        // pairing code URL, since these match the current tab with a device (Chromecast
        // or similar) it is supposed to be controlling. Using a different application
        // that isn't expecting this (in particular YouTube) doesn't work.
        if (url.matches(".*youtube\\.com.*[?&]pairingCode=.*")) {
            return false;
        }

        List<ComponentName> resolvingComponentNames =
                UrlUtilities.getIntentHandlers(mContext, intent);
        boolean canResolveActivity = resolvingComponentNames.size() > 0;
        // check whether the intent can be resolved. If not, we will see
        // whether we can download it from the Market.
        if (!canResolveActivity) {
            if (hasBrowserFallbackUrl) {
                mFallbackUrl = browserFallbackUrl;
                return false;
            }
            String packagename = intent.getPackage();
            if (packagename != null) {
                try {
                    intent = new Intent(Intent.ACTION_VIEW, Uri.parse(
                            "market://details?id=" + packagename
                            + "&referrer=" + mContext.getPackageName()));
                    intent.addCategory(Intent.CATEGORY_BROWSABLE);
                    intent.setPackage("com.android.vending");
                    intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    mContext.startActivity(intent);
                    return true;
                } catch (ActivityNotFoundException ex) {
                    // ignore the error on devices that does not have
                    // play market installed.
                    return false;
                }
            } else {
                return false;
            }
        }

        if (hasBrowserFallbackUrl) {
            intent.removeExtra(EXTRA_BROWSER_FALLBACK_URL);
        }

        // Sanitize the Intent, ensuring web pages can not bypass browser
        // security (only access to BROWSABLE activities).
        intent.addCategory(Intent.CATEGORY_BROWSABLE);
        intent.setComponent(null);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH_MR1) {
            Intent selector = intent.getSelector();
            if (selector != null) {
                selector.addCategory(Intent.CATEGORY_BROWSABLE);
                selector.setComponent(null);
            }
        }

        // Set the Browser application ID to us in case the user chooses Chrome
        // as the app.  This will make sure the link is opened in the same tab
        // instead of making a new one.
        intent.putExtra(Browser.EXTRA_APPLICATION_ID, mContext.getPackageName());
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

        // Make sure webkit can handle it internally before checking for specialized
        // handlers. If webkit can't handle it internally, we need to call
        // startActivityIfNeeded or startActivity.
        if (!isExternalProtocol) {
            if (!UrlUtilities.isSpecializedHandlerAvailable(mContext, intent)) {
                return false;
            } else if (params.referrer != null && (isLink || isFormSubmit)) {
                // Current URL has at least one specialized handler available. For navigations
                // within the same host, keep the navigation inside the browser unless the set of
                // available apps to handle the new navigation is different. http://crbug.com/463138
                URI currentUri;
                URI previousUri;
                try {
                    currentUri = new URI(url);
                    previousUri = new URI(params.referrer);
                } catch (Exception e) {
                    currentUri = null;
                    previousUri = null;
                }

                if (currentUri != null && previousUri != null
                        && TextUtils.equals(currentUri.getHost(), previousUri.getHost())) {
                    Intent previousIntent;
                    try {
                        previousIntent = Intent.parseUri(
                                params.referrer, Intent.URI_INTENT_SCHEME);
                    } catch (Exception e) {
                        previousIntent = null;
                    }

                    if (previousIntent != null)  {
                        List<ComponentName> currentHandlers =
                                UrlUtilities.getIntentHandlers(mContext, intent);
                        List<ComponentName> previousHandlers =
                                UrlUtilities.getIntentHandlers(mContext, previousIntent);

                        if (previousHandlers.containsAll(currentHandlers)) {
                            return false;
                        }
                    }
                }
            }
        }

        if (intent != null && startActivity(intent)) return true;
        return false;
    }
}
