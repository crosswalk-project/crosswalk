// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Environment;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

import dalvik.system.DexClassLoader;

/**
 * Context wrapper for running xwalk from expansion file.
 */
public class XWalkExpansionContext extends android.content.ContextWrapper {

    /**
     * AndroidManifest.xml metadata name, needs to be child of <application>
     */
    public final String MANIFEST_METADATA_NAME = "org.xwalk.expansionFile";

    private static final String LIBXWALKCORE = "libxwalkcore.so";
    private static final String TAG = "XWalkExpansion";

    private Context mBase = null;
    private ClassLoader mClassLoader = null;

    /**
     * Extract versionCode from expansion file
     * @param path Absolute path to expansion file
     * @return versionCode, or 0 on failure.
     */
    public static int getExpansionFileVersion(String path) {

        if (path == null)
            return 0;

        // See https://developer.android.com/google/play/expansion-files.html
        // for expansion file naming convention.
        // We extract the 2nd part of the name.
        File file = new File(path);
        String name = file.getName();
        String[] parts = name.split("\\.");

        if (parts.length > 1)
            return Integer.parseInt(parts[1]);

        return 0;
    }

    /**
     * @param base Context to wrap
     */
    public XWalkExpansionContext(Context base) {
        super(base);
    }

    /**
     * Get ClassLoader to the expansion file based on AndroidManifest.xml metadata.
     * @return DexClassLoader, or null on failure.
     */
    @Override
    public ClassLoader getClassLoader() {

        // Check for cached classloader
        if (mClassLoader != null) {
            return mClassLoader;
        }

        // Create classloader.
        String expType = getManifestExpansionType();
        if (expType == null) {
            return null;
        }

        File expFile = findExpansionFile(expType);
        if (expFile == null) {
            return null;
        }

        if (expFile.getAbsolutePath().endsWith(".obb")) {
            expFile = prepareExpansionContent(expFile);
            if (expFile == null) {
                return null;
            }
        }

        mClassLoader = createClassLoader(expFile);
        if (mClassLoader == null) {
            return null;
        }

        Log.d(TAG, "Created class loader");
        return mClassLoader;
    }

    /**
     * Get expansion type specified in AndroidManifest.xml ()
     * @return "main" or "patch" or null. Null is ok when not configured for expansion file.
     */
    public String getManifestExpansionType() {

        // Look up expansion type from manifest.
        String expType = null;
        try {
            PackageManager pm = getPackageManager();
            ApplicationInfo ai = pm.getApplicationInfo(getPackageName(), PackageManager.GET_META_DATA);
            if (ai.metaData != null) {
                expType = ai.metaData.getString(MANIFEST_METADATA_NAME);
            }
        } catch (PackageManager.NameNotFoundException e) {
            Log.e(TAG, "Package " + getPackageName() + " not found");
        }

        // Validate expansion type.
        if (expType != null && !expType.equals("main") && !expType.equals("patch")) {
            Log.e(TAG, "Invalid expansion type " + expType);
        }

        Log.d(TAG, "Expansion type " + expType);
        return expType;
    }

    /**
     * Find expansion file.
     * @param expType Metadata value from AndroidManifest.xml
     * @return Expansion file.
     */
    private File findExpansionFile(String expType) {

        // Assemble expansion file name and find it on the file system.
        // See https://developer.android.com/google/play/expansion-files.html#GettingFilenames
        final String EXP_PATH = "/Android/obb/";
        File root = Environment.getExternalStorageDirectory();
        File expDir = new File(root.toString() + EXP_PATH + getPackageName());
        if (!expDir.isDirectory()) {
            Log.e(TAG, "Missing expansion dir " + expDir.getAbsolutePath());
            return null;
        }

        // Look what we have. We need to rename the expansion file
        // to .apk for being able to load it with the DexClassLoader
        String expPathApk = null;
        String expPathObb = null;
        for (String expName : expDir.list()) {
            if (expName.startsWith(expType) && expName.endsWith(".apk")) {
                expPathApk = expDir + "/" + expName;
            } else if (expName.startsWith(expType) && expName.endsWith(".obb")) {
                expPathObb = expDir + "/" + expName;
            }
        }

        if (expPathApk == null && expPathObb == null) {
            Log.e(TAG, "No expansion file found in " + expDir.getAbsolutePath());
            return null;
        }

        File expFile = null;
        if (getExpansionFileVersion(expPathObb) > getExpansionFileVersion(expPathApk)) {
            expFile = new File(expPathObb);
        } else if (expPathApk != null) {
            expFile = new File(expPathApk);
        }

        if (expFile != null) {
            Log.d(TAG, "Found expansion file " + expFile.getAbsolutePath());
        } else {
            Log.e(TAG, "No expansion file found in " + expDir.getAbsolutePath());
        }

        return expFile;
    }

    /**
     * Extract LIBXWALKCORE and create dex class loader
     * @param expFile Expansion file
     * @return Expansion file object on success, otherwise null.
     */
    File prepareExpansionContent(File expFile) {

        File expFileApk = new File(expFile.getAbsolutePath() + ".apk");
        if (!expFile.renameTo(expFileApk)) {
            Log.e(TAG, "Failed renaming expansion file to " + expFileApk.getAbsolutePath());
            return null;
        }

        boolean extracted = extractLib(expFileApk);
        if (!extracted) {
            return null;
        }

        Log.d(TAG, "Prepared expansion content");
        return expFileApk;
    }

    /**
     * Extract LIBXWALKCORE from expansion file, so it can be JNI'd.
     * @param expFile Expansion file
     * @return true on success.
     */
    private boolean extractLib(File expFile) {

        boolean haveLib = false;
        for (String abi : Build.SUPPORTED_ABIS) {
            String entryName = "lib/" + abi + "/" + LIBXWALKCORE;
            haveLib = extractEntry(expFile, entryName);
            if (haveLib)
                break;
        }

        if (!haveLib) {
            Log.e(TAG, "Failed to extract " + LIBXWALKCORE + " with supported ABI");
            return false;
        }

        Log.d(TAG, "Extracted " + LIBXWALKCORE);
        return true;
    }

    /**
     * Extract entry from the expansion file.
     * @param expFile Expansion file
     * @param entryName Name of entry
     * @return true on success.
     */
    private boolean extractEntry(File expFile, String entryName) {

        ZipFile zip;
        try {
            zip = new ZipFile(expFile);
        } catch (IOException e) {
            Log.e(TAG, "Failed to open expansion file");
            return false;
        }

        ZipEntry entry = new ZipEntry(entryName);
        InputStream is;
        try {
            is = zip.getInputStream(entry);
        } catch (IOException e) {
            Log.e(TAG, "Failed to open " + entry.getName() + " in expansion file");
            return false;
        }

        FileOutputStream os;
        try {
            os = openFileOutput(LIBXWALKCORE, Context.MODE_PRIVATE);
        } catch (IOException e) {
            Log.e(TAG, "Failed to open file output for extracting " + LIBXWALKCORE);
            return false;
        }

        byte[] buf = new byte[1024 * 1024];
        int len;
        try {
            while ((len = is.read(buf)) > -1) {
                os.write(buf, 0, len);
            }
            is.close();
            os.close();
        } catch (IOException e) {
            Log.e(TAG, "Failed to write " + LIBXWALKCORE);
            return false;
        }

        Log.d(TAG, "Extracted entry " + entryName);
        return true;
    }

    /**
     * Create dex class loader on the expansion file
     * @param expFile expansion file
     * @return ClassLoader or null on failure.
     */
    private ClassLoader createClassLoader(File expFile) {

        // Check libxwalkcore.so is newer than expFile
        File libFile = new File(getFilesDir() + "/" + LIBXWALKCORE);
        if (!libFile.isFile() ||
            (libFile.lastModified() < expFile.lastModified())) {
            boolean extracted = extractLib(expFile);
            if (!extracted) {
                return null;
            }
        }

        ClassLoader cl;
        try {
            cl = new DexClassLoader(expFile.getAbsolutePath(),
                    getCodeCacheDir().getAbsolutePath(),
                    getFilesDir().getAbsolutePath(),
                    ClassLoader.getSystemClassLoader());
        } catch (Exception e) {
            Log.e(TAG, "Could not create class loader on " + expFile.getAbsolutePath());
            return null;
        }

        return cl;
    }
}

