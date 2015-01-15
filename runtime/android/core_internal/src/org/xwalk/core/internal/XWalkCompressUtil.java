package org.xwalk.core.internal;

import SevenZip.Compression.LZMA.Decoder;

import android.content.Context;
import android.content.res.Resources;
import android.util.Log;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class XWalkCompressUtil {
    private static final String TAG = "XWalkCompressUtil";

    public static boolean XWalkLibraryCompressed(Context context, String[] libraries) {
        if (context == null) return false;

        for (String library : libraries) {
            InputStream is = openRawResource(context, library);
            if (is == null) return false;
        }

        return true;
    }

    public static boolean decompressXWalkLibrary(Context context, String[] libraries, String libDir) throws Exception {
        if (context == null) return false;

        File f = new File(libDir);
        if (f.exists() && f.isFile()) f.delete();
        if (!f.exists() && !f.mkdirs()) return false;

        for (String library : libraries) {
            File tmpfile = null;
            InputStream input = null;
            OutputStream output = null;

            try {
                File outfile = new File(libDir, library);
                // FIXME: perhaps need to check validation.
                if (outfile.isFile()) {
                    continue;
                }
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
}
