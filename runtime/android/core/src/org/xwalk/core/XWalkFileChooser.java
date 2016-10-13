// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.Manifest;
import android.net.Uri;
import android.os.Environment;
import android.provider.MediaStore;
import android.util.Log;
import android.webkit.ValueCallback;

import java.io.File;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;

/**
 * <p><code>XWalkFileChooser</code> is the default implementation for choosing local file to updload.
 * This class should be used in <code>XWalkUIClient#openFileChooser</code> like below:</p>
 *
 * <pre>
 * import android.app.Activity;
 * import android.content.Intent;
 * import android.net.Uri;
 * import android.os.Bundle;
 * import android.webkit.ValueCallback;
 *
 * import org.xwalk.core.XWalkActivity;
 * import org.xwalk.core.XWalkFileChooser;
 * import org.xwalk.core.XWalkUIClient;
 * import org.xwalk.core.XWalkView;
 *
 * public class XWalkViewActivity extends XWalkActivity {
 *     private XWalkView mXWalkView;
 *     private XWalkFileChooser mXWalkFileChooser;
 *
 *     private class MyUIClient extends XWalkUIClient {
 *         public MyUIClient(XWalkView view) {
 *             super(view);
 *         }
 *
 *         &#64;Override
 *         public void openFileChooser(XWalkView view, ValueCallback&lt;Uri&gt; uploadFile,
 *                 String acceptType, String capture) {
 *             mXWalkFileChooser.showFileChooser(uploadFile, acceptType, capture);
 *         }
 *     }
 *
 *     &#64;Override
 *     protected void onCreate(Bundle savedInstanceState) {
 *         super.onCreate(savedInstanceState);
 *
 *         setContentView(R.layout.activity_main);
 *
 *         mXWalkView = (XWalkView) findViewById(R.id.xwalkview);
 *         mXWalkView.setUIClient(new MyUIClient(mXWalkView));
 *         mXWalkFileChooser = new XWalkFileChooser(this);
 *     }
 *
 *     &#64;Override
 *     protected void onXWalkReady() {
 *         mXWalkView.loadUrl("file:///android_asset/test.html");
 *     }
 *
 *     &#64;Override
 *     public void onActivityResult(int requestCode, int resultCode, Intent data) {
 *         mXWalkFileChooser.onActivityResult(requestCode, resultCode, data);
 *     }
 * }
 * </pre>
 */

public class XWalkFileChooser {
    private static final String IMAGE_TYPE = "image/";
    private static final String VIDEO_TYPE = "video/";
    private static final String AUDIO_TYPE = "audio/";
    private static final String ALL_IMAGE_TYPES = IMAGE_TYPE + "*";
    private static final String ALL_VIDEO_TYPES = VIDEO_TYPE + "*";
    private static final String ALL_AUDIO_TYPES = AUDIO_TYPE + "*";
    private static final String ANY_TYPES = "*/*";
    private static final String SPLIT_EXPRESSION = ",";
    private static final String PATH_PREFIX = "file:";
    private static final String WRITE_EXTERNAL_STORAGE= "android.permission.WRITE_EXTERNAL_STORAGE";

    public static final int INPUT_FILE_REQUEST_CODE = 1;

    private static final String TAG = "XWalkFileChooser";

    private Activity mActivity;
    private ValueCallback<Uri> mFilePathCallback;
    private String mCameraPhotoPath;

    /**
     * Create a file chooser using an activity.
     *
     * @param activity the activity to start file chooser
     */
    public XWalkFileChooser(Activity activity) {
        mActivity = activity;
    }

    /**
     * Tell the client to show a file chooser.
     * @param uploadFile the callback class to handle the result from caller. It MUST
     *        be invoked in all cases. Leave it not invoked will block all following
     *        requests to open file chooser.
     * @param acceptType value of the 'accept' attribute of the input tag associated
     *        with this file picker.
     * @param capture value of the 'capture' attribute of the input tag associated
     *        with this file picker
     * @return true if file chooser is launched
     * @since 7.0
     */
    public boolean showFileChooser(ValueCallback<Uri> uploadFile, String acceptType,
            String capture) {
        mFilePathCallback = uploadFile;

        Intent takePictureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        if (takePictureIntent.resolveActivity(mActivity.getPackageManager()) != null) {
            // Create the File where the photo should go
            File photoFile = createImageFile();
            // Continue only if the File was successfully created
            if (photoFile != null) {
                mCameraPhotoPath = PATH_PREFIX + photoFile.getAbsolutePath();
                takePictureIntent.putExtra("PhotoPath", mCameraPhotoPath);
                takePictureIntent.putExtra(MediaStore.EXTRA_OUTPUT, Uri.fromFile(photoFile));
            } else {
                takePictureIntent = null;
            }
        }

        Intent camcorder = new Intent(MediaStore.ACTION_VIDEO_CAPTURE);
        Intent soundRecorder = new Intent(MediaStore.Audio.Media.RECORD_SOUND_ACTION);
        Intent contentSelectionIntent = new Intent(Intent.ACTION_GET_CONTENT);
        contentSelectionIntent.addCategory(Intent.CATEGORY_OPENABLE);
        ArrayList<Intent> extraIntents = new ArrayList<Intent>();

        // A single mime type.
        if (!(acceptType.contains(SPLIT_EXPRESSION) || acceptType.contains(ANY_TYPES))) {
            if (capture.equals("true")) {
                if (acceptType.startsWith(IMAGE_TYPE)) {
                    if (takePictureIntent != null) {
                        mActivity.startActivityForResult(takePictureIntent, INPUT_FILE_REQUEST_CODE);
                        Log.d(TAG, "Started taking picture");
                        return true;
                    }
                } else if (acceptType.startsWith(VIDEO_TYPE)) {
                    mActivity.startActivityForResult(camcorder, INPUT_FILE_REQUEST_CODE);
                    Log.d(TAG, "Started camcorder");
                    return true;
                } else if (acceptType.startsWith(AUDIO_TYPE)) {
                    mActivity.startActivityForResult(soundRecorder, INPUT_FILE_REQUEST_CODE);
                    Log.d(TAG, "Started sound recorder");
                    return true;
                }
            } else {
                if (acceptType.startsWith(IMAGE_TYPE)) {
                    if (takePictureIntent != null) {
                        extraIntents.add(takePictureIntent);
                    }
                    contentSelectionIntent.setType(ALL_IMAGE_TYPES);
                } else if (acceptType.startsWith(VIDEO_TYPE)) {
                    extraIntents.add(camcorder);
                    contentSelectionIntent.setType(ALL_VIDEO_TYPES);
                } else if (acceptType.startsWith(AUDIO_TYPE)) {
                    extraIntents.add(soundRecorder);
                    contentSelectionIntent.setType(ALL_AUDIO_TYPES);
                }
            }
        }

        // Couldn't resolve an accept type.
        if (extraIntents.isEmpty() && canWriteExternalStorage()) {
            if (takePictureIntent != null) {
                extraIntents.add(takePictureIntent);
            }
            extraIntents.add(camcorder);
            extraIntents.add(soundRecorder);
            contentSelectionIntent.setType(ANY_TYPES);
        }

        Intent chooserIntent = new Intent(Intent.ACTION_CHOOSER);
        chooserIntent.putExtra(Intent.EXTRA_INTENT, contentSelectionIntent);
        if (!extraIntents.isEmpty()) {
            chooserIntent.putExtra(Intent.EXTRA_INITIAL_INTENTS,
                    extraIntents.toArray(new Intent[] { }));
        }
        mActivity.startActivityForResult(chooserIntent, INPUT_FILE_REQUEST_CODE);
        Log.d(TAG, "Started chooser");
        return true;
    }

    /**
     * Pass the result of your activity's onActivityResult after invoked showFileChooser.
     * @param requestCode The integer request code originally supplied to startActivityForResult(),
     *                    allowing you to identify who this result came from.
     * @param resultCode The integer result code returned by the child activity through its
     *                   setResult().
     * @param data An Intent, which can return result data to the caller (various data can be
     *             attached to Intent "extras").
     * @since 7.0
     */
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if(requestCode == INPUT_FILE_REQUEST_CODE && mFilePathCallback != null) {
            Log.d(TAG, "Activity result: " + resultCode);
            Uri results = null;

            // Check that the response is a good one
            if(Activity.RESULT_OK == resultCode) {
                // In Android M, camera results return an empty Intent rather than null.
                if(data == null || (data.getAction() == null && data.getData() == null)) {
                    // If there is not data, then we may have taken a photo
                    if(mCameraPhotoPath != null) {
                        results = Uri.parse(mCameraPhotoPath);
                    }
                } else {
                    String dataString = data.getDataString();
                    if (dataString != null) {
                        results = Uri.parse(dataString);
                    }
                    deleteImageFile();
                }
            } else if (Activity.RESULT_CANCELED == resultCode) {
                deleteImageFile();
            }

            if (results != null) {
                Log.d(TAG, "Received file: " + results.toString());
            }
            mFilePathCallback.onReceiveValue(results);
            mFilePathCallback = null;
        }
    }

    private boolean canWriteExternalStorage() {
        try {
            PackageManager packageManager = mActivity.getPackageManager();
            PackageInfo packageInfo = packageManager.getPackageInfo(
                    mActivity.getPackageName(), PackageManager.GET_PERMISSIONS);
            return Arrays.asList(packageInfo.requestedPermissions).contains(WRITE_EXTERNAL_STORAGE);
        } catch (NameNotFoundException | NullPointerException e) {
            return false;
        }
    }

    private File createImageFile() {
        // FIXME: If the external storage state is not "MEDIA_MOUNTED", we need to get
        // other volume paths by "getVolumePaths()" when it was exposed.
        String state = Environment.getExternalStorageState();
        if (!state.equals(Environment.MEDIA_MOUNTED)) {
            Log.e(TAG, "External storage is not mounted.");
            return null;
        }

        // Create an image file name
        String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
        String imageFileName = "JPEG_" + timeStamp + "_";
        File storageDir = Environment.getExternalStoragePublicDirectory(
                Environment.DIRECTORY_PICTURES);
        if (!storageDir.exists()) {
            storageDir.mkdirs();
        }

        try {
            File file = File.createTempFile(imageFileName, ".jpg", storageDir);
            Log.d(TAG, "Created image file: " +  file.getAbsolutePath());
            return file;
        } catch (IOException e) {
            // Error occurred while creating the File
            Log.e(TAG, "Unable to create Image File, " +
                    "please make sure permission 'WRITE_EXTERNAL_STORAGE' was added.");
            return null;
        }
    }

    private boolean deleteImageFile() {
        if (mCameraPhotoPath == null || !mCameraPhotoPath.contains(PATH_PREFIX)) {
            return false;
        }
        String filePath = mCameraPhotoPath.split(PATH_PREFIX)[1];
        boolean result = new File(filePath).delete();
        Log.d(TAG, "Delete image file: " + filePath + " result: " + result);
        return result;
    }
}
