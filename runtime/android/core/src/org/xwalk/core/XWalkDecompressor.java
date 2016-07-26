// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.Context;
import android.content.res.Resources;
import android.os.SystemClock;
import android.util.Log;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;
import java.util.concurrent.RejectedExecutionException;
import java.util.List;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

import SevenZip.Compression.LZMA.Decoder;

class XWalkDecompressor {
    private static final String[] MANDATORY_LIBRARIES = {
        "libxwalkcore.so"
    };

    private static final String[] MANDATORY_RESOURCES = {
        "libxwalkcore.so",
        "classes.dex",
        "icudtl.dat",
        "xwalk.pak",
        "xwalk_100_percent.pak",
    };

    private static final String TAG = "XWalkLib";

    private static final int STREAM_BUFFER_SIZE = 0x1000;
    private static final int LZMA_PROP_SIZE = 5;
    private static final int LZMA_OUTSIZE = 8;

    public static boolean isLibraryCompressed() {
        for (String library : MANDATORY_LIBRARIES) {
            try {
                InputStream input = openRawResource(library);
                try {
                    input.close();
                } catch (IOException e) {
                }
            } catch (Resources.NotFoundException e) {
                return false;
            }
        }
        return true;
    }

    public static boolean decompressLibrary() {
        String libDir = XWalkEnvironment.getPrivateDataDir();
        File f = new File(libDir);
        if (f.exists() && f.isFile()) f.delete();
        if (!f.exists() && !f.mkdirs()) return false;

        long start = SystemClock.uptimeMillis();
        for (String library : MANDATORY_LIBRARIES) {
            try {
                Log.d(TAG, "Decompressing " + library);
                InputStream input = openRawResource(library);
                extractLzmaToFile(input, new File(libDir, library));
            } catch (Resources.NotFoundException e) {
                Log.d(TAG, library + " not found");
                return false;
            } catch (IOException e) {
                Log.e(TAG, e.getLocalizedMessage());
                return false;
            }
        }
        Log.d(TAG, "Time to decompress : " + (SystemClock.uptimeMillis() - start) + " ms");
        return true;
    }

    public static boolean isResourceCompressed(String libFile) {
        ZipFile zipFile = null;
        try {
            zipFile = new ZipFile(libFile);

            for (String resource : MANDATORY_RESOURCES) {
                ZipEntry entry = zipFile.getEntry("assets" + File.separator + resource + ".lzma");
                if (entry == null) return false;
            }
        } catch (IOException e) {
            return false;
        } finally {
            try {
                zipFile.close();
            } catch (IOException | NullPointerException e) {
            }
        }
        return true;
    }

    public static boolean extractResource(String libFile, String destDir) {
        Log.d(TAG, "Extract resource from Apk " + libFile);
        long start = SystemClock.uptimeMillis();

        ZipFile zipFile = null;
        try {
            zipFile = new ZipFile(libFile);

            for (String resource : MANDATORY_RESOURCES) {
                ZipEntry entry = null;
                if (isNativeLibrary(resource)) {
                    String abi = XWalkEnvironment.getDeviceAbi();
                    String path = "lib" + File.separator + abi + File.separator + resource;
                    entry = zipFile.getEntry(path);
                    if (entry == null && XWalkEnvironment.is64bitDevice()) {
                        if (abi.equals("arm64-v8a")) {
                            abi = "armeabi-v7a";
                        } else if (abi.equals("x86_64")) {
                            abi = "x86";
                        }
                        path = "lib" + File.separator + abi + File.separator + resource;
                        entry = zipFile.getEntry(path);
                    }
                } else if (isAsset(resource)) {
                    String path = "assets" + File.separator + resource;
                    entry = zipFile.getEntry(path);
                } else {
                    entry = zipFile.getEntry(resource);
                }

                if (entry == null) {
                    Log.e(TAG, resource + " not found");
                    return false;
                }
                Log.d(TAG, "Extracting " + resource);
                extractStreamToFile(zipFile.getInputStream(entry), new File(destDir, resource));
            }
        } catch (IOException e) {
            Log.d(TAG, e.getLocalizedMessage());
            return false;
        } finally {
            try {
                zipFile.close();
            } catch (IOException | NullPointerException e) {
            }
        }
        Log.d(TAG, "Time to extract : " + (SystemClock.uptimeMillis() - start) + " ms");
        return true;
    }

    public static boolean decompressResource(String libFile, String destDir) {
        Log.d(TAG, "Decompress resource from Apk " + libFile);
        long start = SystemClock.uptimeMillis();

        List<Callable<Boolean>> taskList =
                new ArrayList<Callable<Boolean>>(MANDATORY_RESOURCES.length);
        ExecutorService pool = Executors.newFixedThreadPool(MANDATORY_RESOURCES.length);
        ZipFile zipFile = null;
        boolean success = true;

        try {
            zipFile = new ZipFile(libFile);

            for (String resource: MANDATORY_RESOURCES) {
                ZipEntry entry = zipFile.getEntry("assets" + File.separator + resource + ".lzma");
                if (entry == null) {
                    Log.e(TAG, resource + " not found");
                    return false;
                }

                File destFile = new File(destDir, resource);
                taskList.add(new DecompressResourceTask(zipFile, entry, destFile));
            }

            try {
                List<Future<Boolean>> futureList = pool.invokeAll(taskList);
                for (Future<Boolean> f : futureList) {
                    success &= f.get();
                }
            } catch (InterruptedException | RejectedExecutionException | ExecutionException e) {
                Log.d(TAG, "Failed to execute decompression");
                return false;
            } finally {
                pool.shutdown();
            }
        } catch (IOException e) {
            Log.d(TAG, e.getLocalizedMessage());
            return false;
        } finally {
            try {
                zipFile.close();
            } catch (IOException | NullPointerException e) {
            }
        }
        Log.d(TAG, "Time to decompress : " + (SystemClock.uptimeMillis() - start) + " ms");
        return success;
    }

    private static class DecompressResourceTask implements Callable<Boolean> {
        ZipFile mZipFile;
        ZipEntry mZipEntry;
        File mDestFile;

        DecompressResourceTask(ZipFile zipFile, ZipEntry zipEntry, File destFile) {
            mZipFile = zipFile;
            mZipEntry = zipEntry;
            mDestFile = destFile;
        }

        @Override
        public Boolean call() {
            try {
                Log.d(TAG, "Decompressing " + mZipEntry.getName());
                extractLzmaToFile(mZipFile.getInputStream(mZipEntry), mDestFile);
            } catch (IOException e) {
                Log.e(TAG, e.getLocalizedMessage());
                return false;
            }
            return true;
        }
    }

    private static boolean isNativeLibrary(String resource) {
        return resource.endsWith(".so");
    }

    private static boolean isAsset(String resource) {
        return resource.endsWith(".dat") || resource.endsWith(".pak");
    }

    private static InputStream openRawResource(String library)
            throws Resources.NotFoundException {
        Context context = XWalkEnvironment.getApplicationContext();
        Resources res = context.getResources();
        String libraryName = library.split("\\.")[0];
        int id = res.getIdentifier(libraryName, "raw", context.getPackageName());
        return res.openRawResource(id);
    }

    private static void extractLzmaToFile(InputStream srcStream, File destFile) throws IOException {
        InputStream input = null;
        OutputStream output = null;

        try {
            input = new BufferedInputStream(srcStream);
            output = new BufferedOutputStream(new FileOutputStream(destFile));

            byte[] properties = new byte[LZMA_PROP_SIZE];
            if (input.read(properties, 0, LZMA_PROP_SIZE) != LZMA_PROP_SIZE) {
                throw new IOException("Input lzma file is too short");
            }

            Decoder decoder = new Decoder();
            if (!decoder.SetDecoderProperties(properties)) {
                throw new IOException("Incorrect lzma properties");
            }

            long outSize = 0;
            for (int i = 0; i < LZMA_OUTSIZE; i++) {
                int v = input.read();
                if (v < 0) {
                    Log.w(TAG, "Can't read stream size");
                }
                outSize |= ((long)v) << (8 * i);
            }

            if (!decoder.Code(input, output, outSize)) {
                throw new IOException("Error in data stream");
            }
        } catch (IOException e) {
            if (destFile.isFile()) destFile.delete();
            throw e;
        } finally {
            try {
                output.flush();
            } catch (IOException | NullPointerException e) {
            }
            try {
                output.close();
            } catch (IOException | NullPointerException e) {
            }
            try {
                input.close();
            } catch (IOException | NullPointerException e) {
            }
        }
    }

    private static void extractStreamToFile(InputStream input, File file) throws IOException {
        OutputStream output = null;

        try {
            // If the input stream is already closed, this method will throw an IOException so that
            // we don't create an unused output stream.
            input.available();
            output = new FileOutputStream(file);

            byte[] buffer = new byte[STREAM_BUFFER_SIZE];
            for (int len = 0; (len = input.read(buffer)) >= 0;) {
                output.write(buffer, 0, len);
            }
        } catch (IOException e) {
            if (file.isFile()) file.delete();
            throw e;
        } finally {
            try {
                output.flush();
            } catch (IOException | NullPointerException e) {
            }
            try {
                output.close();
            } catch (IOException | NullPointerException e) {
            }
            try {
                input.close();
            } catch (IOException | NullPointerException e) {
            }
        }
    }
}
