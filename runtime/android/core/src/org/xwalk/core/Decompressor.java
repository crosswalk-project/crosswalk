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
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import org.chromium.base.PathUtils;

import SevenZip.Compression.LZMA.Decoder;

public class Decompressor {
    private static final String TAG = "Decompressor";
    private static final String BRIDGE_PACKAGE = "org.xwalk.core.internal";
    private static final String XWALK_CORE_BRIDGE = "XWalkCoreBridge";
    private static final String XWALK_CORE_PACKAGE = "org.xwalk.core";
    private static final String[] MANDATORY_LIBRARIES = {"libxwalkcore.so"};

    public static boolean process(Context context) throws Exception {
        if (context == null) return false;
        if (!xwalkLibraryCompressed(context)) return true;

        String lib = PathUtils.getDataDirectory(context.getApplicationContext());
        long start = System.currentTimeMillis();
        boolean success = decompressXWalkLibrary(context, lib);
        long end = System.currentTimeMillis();
        Log.d(TAG, "decompress library cost: " + (end - start) + " milliseconds.");
        setLocalVersion(context);
        return success;
    }

    private static boolean decodeLzma(InputStream input, OutputStream output) throws IOException {
        final int propSize = 5;
        final int outSizeLength = 8;

        byte[] properties = new byte[propSize];
        if (input.read(properties, 0, propSize) != propSize) {
            Log.w(TAG, "Input .lzma file is too short");
            return false;
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

        return true;
    }

     private static boolean decompressXWalkLibrary(Context context, String libDir) throws Exception {
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
                if (!decodeLzma(input, output)) {
                    Log.d(TAG, "Decompress failed");
                    return false;
                }
                tmpfile.renameTo(outfile);
            } catch (Exception e) {
                Log.d(TAG, "Decompress failed: " + e.getMessage());
                throw e;
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

    private static int getCoreVersion(Context context) {
        int version = 0;
        ClassLoader loader = Decompressor.class.getClassLoader();

        if (loader == null) return version;

        try {
            Class<?> clazz = loader.loadClass(BRIDGE_PACKAGE + "." + XWALK_CORE_BRIDGE);
            version = (int) clazz.getField("XWALK_API_VERSION").get(null);
        } catch (ClassNotFoundException | NoSuchFieldException | IllegalAccessException e) {
            Log.e(TAG, e.getMessage());
        }

        return version;
    }   

    private static InputStream openRawResource(Context context, String library) {
        Resources res = context.getResources();
        String libraryName = library.split("\\.")[0];
        InputStream is = null;

        int id = res.getIdentifier(libraryName, "raw", context.getPackageName());
        if (id > 0) {
            try {
                is = res.openRawResource(id);
            } catch (Resources.NotFoundException e) {
                Log.d(TAG, "Could not find resource: " + e.getMessage());
            }
        }

        return is;
    }

    private static void setLocalVersion(Context context) {
        SharedPreferences sp = context.getSharedPreferences("libxwalkcore",
                Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = sp.edit();
        editor.putInt("version", getCoreVersion(context)).apply();
    }

    private static boolean xwalkLibraryCompressed(Context context) {
        if (context == null) return false;

        for (String library : MANDATORY_LIBRARIES) {
            InputStream is = openRawResource(context, library);
            if (is == null) return false;
        }

        return true;
    }
}
