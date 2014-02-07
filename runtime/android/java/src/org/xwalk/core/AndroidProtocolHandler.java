// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.Context;
import android.content.res.AssetManager;
import android.net.Uri;
import android.util.Log;
import android.util.TypedValue;

import java.io.InputStream;
import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URLConnection;
import java.util.List;

import org.chromium.base.CalledByNativeUnchecked;
import org.chromium.base.JNINamespace;

/**
 * Implements the Java side of Android URL protocol jobs.
 * See android_protocol_handler.cc.
 */
@JNINamespace("xwalk")
public class AndroidProtocolHandler {
    private static final String TAG = "AndroidProtocolHandler";

    // Supported URL schemes. This needs to be kept in sync with
    // clank/native/framework/chrome/url_request_android_job.cc.
    public static final String FILE_SCHEME = "file";
    private static final String CONTENT_SCHEME = "content";
    public static final String APP_SCHEME = "app";
    private static final String APP_SRC = "www";

    /**
     * Open an InputStream for an Android resource.
     * @param context The context manager.
     * @param url The url to load.
     * @return An InputStream to the Android resource.
     */
    // TODO(bulach): this should have either a throw clause, or
    // handle the exception in the java side rather than the native side.
    @CalledByNativeUnchecked
    public static InputStream open(Context context, String url) {
        Uri uri = verifyUrl(url);
        if (uri == null) {
            return null;
        }
        String path = uri.getPath();
        if (uri.getScheme().equals(FILE_SCHEME)) {
            if (path.startsWith(nativeGetAndroidAssetPath())) {
                return openAsset(context, uri);
            } else if (path.startsWith(nativeGetAndroidResourcePath())) {
                return openResource(context, uri);
            }
        } else if (uri.getScheme().equals(CONTENT_SCHEME)) {
            return openContent(context, uri);
        } else if (uri.getScheme().equals(APP_SCHEME)) {
            // The host should be the same as the lower case of the package
            // name, otherwise the resource request should be rejected.
            if (!uri.getHost().equals(context.getPackageName().toLowerCase())) return null;

            // path == "/" or path == ""
            if (path.length() <= 1) return null;

            return openAsset(context, appUriToFileUri(uri));
        }

        return null;
    }

    // Get the asset path of file:///android_asset/* url.
    public static String getAssetPath(Uri uri) {
        assert(uri.getScheme().equals(FILE_SCHEME));
        assert(uri.getPath() != null);
        assert(uri.getPath().startsWith(nativeGetAndroidAssetPath()));
        String path = uri.getPath();
        // Remove duplicate slashes and normalize the URL.
        path = (new java.io.File(path)).getAbsolutePath();
        return path.replaceFirst(nativeGetAndroidAssetPath(), "");
    }

    // Convert app uri to file uri to access the actual files in assets.
    public static Uri appUriToFileUri(Uri uri) {
        assert(uri.getScheme().equals(APP_SCHEME));
        assert(uri.getPath() != null);

        try {
            URI fileUri = new URI(FILE_SCHEME,
                nativeGetAndroidAssetPath() + APP_SRC + uri.getPath(), null);
            return Uri.parse(fileUri.normalize().toString());
        } catch (URISyntaxException e) {
            Log.e(TAG, "Unable to convert app URI to file URI: " + uri, e);
            return null;
        }
    }

    private static int getFieldId(Context context, String assetType, String assetName)
        throws ClassNotFoundException, NoSuchFieldException, IllegalAccessException {
        Class<?> d = context.getClassLoader()
            .loadClass(context.getPackageName() + ".R$" + assetType);
        java.lang.reflect.Field field = d.getField(assetName);
        int id = field.getInt(null);
        return id;
    }

    private static int getValueType(Context context, int field_id) {
      TypedValue value = new TypedValue();
      context.getResources().getValue(field_id, value, true);
      return value.type;
    }

    private static InputStream openResource(Context context, Uri uri) {
        assert(uri.getScheme().equals(FILE_SCHEME));
        assert(uri.getPath() != null);
        assert(uri.getPath().startsWith(nativeGetAndroidResourcePath()));
        // The path must be of the form "/android_res/asset_type/asset_name.ext".
        List<String> pathSegments = uri.getPathSegments();
        if (pathSegments.size() != 3) {
            Log.e(TAG, "Incorrect resource path: " + uri);
            return null;
        }
        String assetPath = pathSegments.get(0);
        String assetType = pathSegments.get(1);
        String assetName = pathSegments.get(2);
        if (!("/" + assetPath + "/").equals(nativeGetAndroidResourcePath())) {
            Log.e(TAG, "Resource path does not start with " + nativeGetAndroidResourcePath() +
                  ": " + uri);
            return null;
        }
        // Drop the file extension.
        assetName = assetName.split("\\.")[0];
        try {
            // Use the application context for resolving the resource package name so that we do
            // not use the browser's own resources. Note that if 'context' here belongs to the
            // test suite, it does not have a separate application context. In that case we use
            // the original context object directly.
            if (context.getApplicationContext() != null) {
                context = context.getApplicationContext();
            }
            int field_id = getFieldId(context, assetType, assetName);
            int value_type = getValueType(context, field_id);
            if (value_type == TypedValue.TYPE_STRING) {
                return context.getResources().openRawResource(field_id);
            } else {
                Log.e(TAG, "Asset not of type string: " + uri);
                return null;
            }
        } catch (ClassNotFoundException e) {
            Log.e(TAG, "Unable to open resource URL: " + uri, e);
            return null;
        } catch (NoSuchFieldException e) {
            Log.e(TAG, "Unable to open resource URL: " + uri, e);
            return null;
        } catch (IllegalAccessException e) {
            Log.e(TAG, "Unable to open resource URL: " + uri, e);
            return null;
        }
    }

    private static InputStream openAsset(Context context, Uri uri) {
        try {
            AssetManager assets = context.getAssets();
            return assets.open(getAssetPath(uri), AssetManager.ACCESS_STREAMING);
        } catch (IOException e) {
            Log.e(TAG, "Unable to open asset URL: " + uri);
            return null;
        }
    }

    private static InputStream openContent(Context context, Uri uri) {
        assert(uri.getScheme().equals(CONTENT_SCHEME));
        try {
            // We strip the query parameters before opening the stream to
            // ensure that the URL we try to load exactly matches the URL
            // we have permission to read.
            Uri baseUri = stripQueryParameters(uri);
            return context.getContentResolver().openInputStream(baseUri);
        } catch (Exception e) {
            Log.e(TAG, "Unable to open content URL: " + uri);
            return null;
        }
    }

    /**
     * Determine the mime type for an Android resource.
     * @param context The context manager.
     * @param stream The opened input stream which to examine.
     * @param url The url from which the stream was opened.
     * @return The mime type or null if the type is unknown.
     */
    // TODO(bulach): this should have either a throw clause, or
    // handle the exception in the java side rather than the native side.
    @CalledByNativeUnchecked
    public static String getMimeType(Context context, InputStream stream, String url) {
        Uri uri = verifyUrl(url);
        if (uri == null) {
            return null;
        }
        String path = uri.getPath();
        // The content URL type can be queried directly.
        if (uri.getScheme().equals(CONTENT_SCHEME)) {
            return context.getContentResolver().getType(uri);
        // Asset files may have a known extension.
        } else if (uri.getScheme().equals(APP_SCHEME) ||
                   uri.getScheme().equals(FILE_SCHEME) &&
                   path.startsWith(nativeGetAndroidAssetPath())) {
            String mimeType = URLConnection.guessContentTypeFromName(path);
            if (mimeType != null) {
                return mimeType;
            }
        }
        // Fall back to sniffing the type from the stream.
        try {
            return URLConnection.guessContentTypeFromStream(stream);
        } catch (IOException e) {
            return null;
        }
    }

    /**
     * Get the package name of the current Activity.
     * @param context The context manager.
     * @return Package name.
     */
    @CalledByNativeUnchecked
    public static String getPackageName(Context context) {
        // Make sure the context is the application context.
        // Or it will get the wrong package name in shared mode.
        return context.getPackageName();
    }

    /**
     * Make sure the given string URL is correctly formed and parse it into a Uri.
     * @return a Uri instance, or null if the URL was invalid.
     */
    private static Uri verifyUrl(String url) {
        if (url == null) {
            return null;
        }
        Uri uri = Uri.parse(url);
        if (uri == null) {
            Log.e(TAG, "Malformed URL: " + url);
            return null;
        }
        String path = uri.getPath();
        if (path == null || path.length() == 0) {
            Log.e(TAG, "URL does not have a path: " + url);
            return null;
        }
        return uri;
    }

    /**
     * Remove query parameters from a Uri.
     * @param uri The input uri.
     * @return The given uri without query parameters.
     */
    private static Uri stripQueryParameters(Uri uri) {
        assert(uri.getAuthority() != null);
        assert(uri.getPath() != null);
        Uri.Builder builder = new Uri.Builder();
        builder.scheme(uri.getScheme());
        builder.encodedAuthority(uri.getAuthority());
        builder.encodedPath(uri.getPath());
        return builder.build();
    }

    /**
     * Set the context to be used for resolving resource queries.
     * @param context Context to be used, or null for the default application
     *                context.
     */
    public static void setResourceContextForTesting(Context context) {
        nativeSetResourceContextForTesting(context);
    }

    private static native void nativeSetResourceContextForTesting(Context context);
    private static native String nativeGetAndroidAssetPath();
    private static native String nativeGetAndroidResourcePath();
}
