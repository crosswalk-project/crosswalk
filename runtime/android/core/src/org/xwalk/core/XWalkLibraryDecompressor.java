// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.Context;
import android.content.res.Resources;
import android.content.SharedPreferences;
import android.util.Log;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.EOFException;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import junit.framework.Assert;

import org.chromium.base.PathUtils;

import SevenZip.Compression.LZMA.Decoder;

class XWalkLibraryDecompressor {
    private static final String PRIVATE_DATA_DIRECTORY_SUFFIX = "xwalkcore";
    private static final String[] MANDATORY_LIBRARIES = { "libxwalkcore.so" };
    private static final String TAG = "XWalkLib";

    public static boolean isCompressed(Context context) {
        Assert.assertNotNull(context);

        for (String library : MANDATORY_LIBRARIES) {
            try {
                openRawResource(context, library);
            } catch (Resources.NotFoundException e) {
                return false;
            }
        }

        return true;
    }

    public static boolean isDecompressed(Context context) {
        Assert.assertNotNull(context);

        int version = getLocalVersion(context);
        return version > 0 && version == XWalkAppVersion.API_VERSION;
    }

    public static boolean decompressLibrary(Context context) {
        Assert.assertNotNull(context);

        PathUtils.setPrivateDataDirectorySuffix(PRIVATE_DATA_DIRECTORY_SUFFIX);
        String lib = PathUtils.getDataDirectory(context.getApplicationContext());

        long start = System.currentTimeMillis();
        boolean success = decompress(context, lib);
        long end = System.currentTimeMillis();
        Log.d(TAG, "Decompress library cost: " + (end - start) + " milliseconds.");

        if (success) setLocalVersion(context, XWalkAppVersion.API_VERSION);
        return success;
    }

    public static boolean loadDecompressedLibrary(Context context) {
        Assert.assertNotNull(context);

        PathUtils.setPrivateDataDirectorySuffix(PRIVATE_DATA_DIRECTORY_SUFFIX);
        String lib = PathUtils.getDataDirectory(context.getApplicationContext());

        try {
            for (String library : MANDATORY_LIBRARIES) {
                System.load(lib + "/" + library);
            }
        } catch (UnsatisfiedLinkError e) {
            Log.d(TAG, "Failed to load decompressed library");
            return false;
        }

        return true;
    }

    private static boolean decompress(Context context, String libDir) {
        if (context == null) return false;

        File f = new File(libDir);
        if (f.exists() && f.isFile()) f.delete();
        if (!f.exists() && !f.mkdirs()) return false;

        for (String library : MANDATORY_LIBRARIES) {
            File tmpfile = null;
            InputStream input = null;
            OutputStream output = null;

            try {
                File outfile = new File(libDir, library);
                tmpfile = new File(libDir, library + ".tmp");
                input = new BufferedInputStream(openRawResource(context, library));
                output = new BufferedOutputStream(new FileOutputStream(tmpfile));
                decodeWithLzma(input, output);
                tmpfile.renameTo(outfile);
            } catch (Resources.NotFoundException e) {
                Log.d(TAG, "Could not find resource: " + e.getMessage());
                return false;
            } catch (Exception e) {
                Log.d(TAG, "Decompress failed: " + e.getMessage());
                return false;
            } finally {
                if (output != null) {
                    try {
                        output.flush();
                    } catch (IOException e) {
                    }
                    try {
                        output.close();
                    } catch (IOException e) {
                    }
                }
                if (input != null) {
                    try {
                        input.close();
                    } catch (IOException e) {
                    }
                }
                tmpfile.delete();
            }
        }

        return true;
    }

    private static void decodeWithLzma(InputStream input, OutputStream output) throws IOException {
        final int propSize = 5;
        final int outSizeLength = 8;

        byte[] properties = new byte[propSize];
        if (input.read(properties, 0, propSize) != propSize) {
            throw new EOFException("Input .lzma file is too short");
        }

        Decoder decoder = new Decoder();
        if (!decoder.SetDecoderProperties(properties)) {
            Log.w(TAG, "Incorrect stream properties");
        }

        long outSize = 0;
        for (int i = 0; i < outSizeLength; i++) {
            int v = input.read();
            if (v < 0) {
                Log.w(TAG, "Can't read stream size");
            }
            outSize |= ((long)v) << (8 * i);
        }

        if (!decoder.Code(input, output, outSize)) {
            Log.w(TAG, "Error in data stream");
        }
    }

    private static InputStream openRawResource(Context context, String library)
            throws Resources.NotFoundException {
        Resources res = context.getResources();
        String libraryName = library.split("\\.")[0];
        int id = res.getIdentifier(libraryName, "raw", context.getPackageName());
        return res.openRawResource(id);
    }

    private static int getLocalVersion(Context context) {
        SharedPreferences sp = context.getSharedPreferences("libxwalkcore", Context.MODE_PRIVATE);
        return sp.getInt("version", 0);
    }

    private static void setLocalVersion(Context context, int version) {
        SharedPreferences sp = context.getSharedPreferences("libxwalkcore", Context.MODE_PRIVATE);
        sp.edit().putInt("version", version).apply();
    }
}
