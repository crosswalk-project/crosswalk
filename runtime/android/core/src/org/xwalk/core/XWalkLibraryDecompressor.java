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
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import SevenZip.Compression.LZMA.Decoder;

class XWalkLibraryDecompressor {
    private static final String MANDATORY_LIBRARRY = "libxwalkcore.so";
    private static final String COMPRESSED_LIBRARY = "libxwalkcoreCompressed.so";
    private static final String TAG = "XWalkLib";

    public static boolean isCompressed(Context context) {
	// The libxwalkcoreCompressed.so exists in default directory if it's decompressed.
	String defaultLibDir = "/data/data/" + context.getPackageName() + "/lib/";
	File file = new File(defaultLibDir, COMPRESSED_LIBRARY);
	if(!file.exists())
	    return false;
        return true;
    }

    public static boolean isDecompressed(Context context) {
        int version = getLocalVersion(context);
        return version > 0 && version == XWalkAppVersion.API_VERSION;
    }

    public static boolean decompressLibrary(Context context) {
        String libDir = context.getDir(XWalkLibraryInterface.PRIVATE_DATA_DIRECTORY_SUFFIX,
                Context.MODE_PRIVATE).toString();

        long start = System.currentTimeMillis();
        boolean success = decompress(context, libDir);
        long end = System.currentTimeMillis();
        Log.d(TAG, "Decompress library cost: " + (end - start) + " milliseconds.");

        if (success) setLocalVersion(context, XWalkAppVersion.API_VERSION);
        return success;
    }

    private static boolean decompress(Context context, String libDir) {
        File f = new File(libDir);
        if (f.exists() && f.isFile()) f.delete();
        if (!f.exists() && !f.mkdirs()) return false;

        File tmpfile = null;
        InputStream input = null;
        OutputStream output = null;

        try {
            File outfile = new File(libDir, MANDATORY_LIBRARRY);
            tmpfile = new File(libDir, MANDATORY_LIBRARRY + ".tmp");
            input = new BufferedInputStream(openDecompressedFile(context, COMPRESSED_LIBRARY));
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

        return true;
    }

    static void decodeWithLzma(InputStream input, OutputStream output) throws IOException {
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

    private static InputStream openDecompressedFile(Context context, String library)
            throws FileNotFoundException {
        String defaultLibDir = "/data/data/" + context.getPackageName() + "/lib/";
        InputStream input = null;
        try {
            input = new FileInputStream(new File(defaultLibDir, library));
	} catch (FileNotFoundException e) {
            // TODO Auto-generated catch block
	    e.printStackTrace();
	}
        return input;
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
