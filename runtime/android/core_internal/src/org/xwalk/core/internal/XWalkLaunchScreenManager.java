// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import java.lang.Runnable;
import java.util.ArrayList;

import android.app.Activity;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.Shader.TileMode;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.TypedValue;
import android.view.Display;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.OrientationEventListener;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.ImageView.ScaleType;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;

import org.chromium.content.browser.ContentViewRenderView.FirstRenderedFrameListener;

/**
 * Provisionally set it as public due to the use of launch screen extension.
 * @hide
 */
public class XWalkLaunchScreenManager
        implements FirstRenderedFrameListener, DialogInterface.OnShowListener,
	           DialogInterface.OnDismissListener, PageLoadListener {
    // This string will be initialized before extension initialized,
    // and used by LaunchScreenExtension.
    private static String mIntentFilterStr;

    private final static String BORDER_MODE_REPEAT = "repeat";
    private final static String BORDER_MODE_STRETCH = "stretch";
    private final static String BORDER_MODE_ROUND = "round";

    private XWalkViewInternal mXWalkView;
    private Activity mActivity;
    private Context mLibContext;
    private Dialog mLaunchScreenDialog;
    private boolean mPageLoadFinished;
    private ReadyWhenType mReadyWhen;
    private boolean mFirstFrameReceived;
    private BroadcastReceiver mLaunchScreenReadyWhenReceiver;
    private boolean mCustomHideLaunchScreen;
    private int mCurrentOrientation;
    private OrientationEventListener mOrientationListener;

    private enum ReadyWhenType {
        FIRST_PAINT,
        USER_INTERACTIVE,
        COMPLETE,
        CUSTOM
    }

    private enum BorderModeType {
        REPEAT,
        STRETCH,
        ROUND,
        NONE
    }

    public XWalkLaunchScreenManager(Context context, XWalkViewInternal xwView) {
        mXWalkView = xwView;
        mLibContext = context;
        mActivity = mXWalkView.getActivity();
        mIntentFilterStr = mActivity.getPackageName() + ".hideLaunchScreen";
    }

    public void displayLaunchScreen(String readyWhen, final String imageBorderList) {
        if (mXWalkView == null) return;
        setReadyWhen(readyWhen);

        Runnable runnable = new Runnable() {
           public void run() {
                int bgResId = mActivity.getResources().getIdentifier(
                        "launchscreen_bg", "drawable", mActivity.getPackageName());
                if (bgResId == 0) return;
                Drawable bgDrawable = mActivity.getResources().getDrawable(bgResId);
                if (bgDrawable == null) return;

                mLaunchScreenDialog = new Dialog(mLibContext,
                                                 android.R.style.Theme_Holo_Light_NoActionBar);
                if ((mActivity.getWindow().getAttributes().flags &
                     WindowManager.LayoutParams.FLAG_FULLSCREEN) != 0) {
                    mLaunchScreenDialog.getWindow().setFlags(
                            WindowManager.LayoutParams.FLAG_FULLSCREEN,
                            WindowManager.LayoutParams.FLAG_FULLSCREEN);
                }
                mLaunchScreenDialog.setOnKeyListener(new Dialog.OnKeyListener() {
                    @Override
                    public boolean onKey(DialogInterface arg0, int keyCode,
                            KeyEvent event) {
                        if (keyCode == KeyEvent.KEYCODE_BACK) {
                            performHideLaunchScreen();
                            mActivity.onBackPressed();
                        }
                        return true;
                    }
                });
                mLaunchScreenDialog.setOnShowListener(XWalkLaunchScreenManager.this);
                mLaunchScreenDialog.setOnDismissListener(XWalkLaunchScreenManager.this);
                // Set background
                mLaunchScreenDialog.getWindow().setBackgroundDrawable(bgDrawable);
                // Set foreground image
                RelativeLayout root = getLaunchScreenLayout(imageBorderList);
                // The root layout can be null when there is no 'image' provided in the manifest.
                // We can just display the background instead of no launch screen dialog displayed.
                if (root != null) mLaunchScreenDialog.setContentView(root);
                mLaunchScreenDialog.show();

                // Change the layout depends on the orientation change.
                mOrientationListener = new OrientationEventListener(mActivity,
                        SensorManager.SENSOR_DELAY_NORMAL) {
                    public void onOrientationChanged(int ori) {
                        if (mLaunchScreenDialog == null || !mLaunchScreenDialog.isShowing()) {
                            return;
                        }
		        int orientation = getScreenOrientation();
                        if (orientation != mCurrentOrientation) {
                            RelativeLayout root = getLaunchScreenLayout(imageBorderList);
                            if (root == null) return;
                            mLaunchScreenDialog.setContentView(root);
                        }
                    }
                };
                mOrientationListener.enable();
                if (mReadyWhen == ReadyWhenType.CUSTOM) registerBroadcastReceiver();
            }
        };
        mActivity.runOnUiThread(runnable);
    }

    @Override
    public void onFirstFrameReceived() {
        mFirstFrameReceived = true;
        hideLaunchScreenWhenReady();
    }

    @Override
    public void onShow(DialogInterface dialog) {
        mActivity.getWindow().setBackgroundDrawable(null);
        if (mFirstFrameReceived) hideLaunchScreenWhenReady();
    }

    @Override
    public void onDismiss(DialogInterface dialog) {
        mOrientationListener.disable();
        mOrientationListener = null;
    }

    @Override
    public void onPageFinished(String url) {
        mPageLoadFinished = true;
        hideLaunchScreenWhenReady();
    }

    public static String getHideLaunchScreenFilterStr() {
        return mIntentFilterStr;
    }

    public int getScreenOrientation() {
        // getResources().getConfiguration().orientation returns wrong value in some devices.
        // Below is another way to calculate screen orientation.
        Display display = mActivity.getWindowManager().getDefaultDisplay();
        Point size = new Point();
        display.getSize(size);
        int orientation;
        if (size.x < size.y) {
            orientation = Configuration.ORIENTATION_PORTRAIT;
        } else {
            orientation = Configuration.ORIENTATION_LANDSCAPE;
        }
        return orientation;
    }

    private RelativeLayout getLaunchScreenLayout(String imageBorderList) {
        // Parse the borders depends on orientation.
        // imageBorderList format:"[default];[landscape];[portrait]"
        String[] borders = imageBorderList.split(";");
        // When there is no borders defined, display with no borders.
        if (borders.length < 1) return parseImageBorder("");
        int orientation = getScreenOrientation();
        mCurrentOrientation = orientation;
        if (borders.length >= 2 && orientation == Configuration.ORIENTATION_LANDSCAPE) {
            if (borders[1].equals("empty")) {
                // Has launch_screen.landscape configured, but no image_border set.
                // Display the iamge with no borders.
                return parseImageBorder("");
            } else if (borders[1].isEmpty()) {
                // No launch_screen.landscape configured.
                // Use launch_screen.default.
                return parseImageBorder(borders[0]);
            } else {
                return parseImageBorder(borders[1]);
            }
        } else if (borders.length == 3 && orientation == Configuration.ORIENTATION_PORTRAIT) {
            if (borders[2].equals("empty")) {
                // Has launch_screen.portrait configured, but no image_border set.
                // Display the iamge with no borders.
                return parseImageBorder("");
            } else if (borders[2].isEmpty()) {
                // No launch_screen.portrait configured.
                // Use launch_screen.default.
                return parseImageBorder(borders[0]);
            } else {
                return parseImageBorder(borders[2]);
            }
        }

        return parseImageBorder(borders[0]);
    }

    private int getSuitableSize(int maxSize, int divider) {
        int finalSize = divider;
        float minMod = divider;
        for (; divider > 1; divider--) {
            int mod = maxSize % divider;
            // Found the suitable size.
            if (mod == 0) {
                finalSize = divider;
                break;
            }
            // Record the best suitable one.
            // If there is no mod==0 found, return the divider which min(mod).
            if (mod < minMod) {
                minMod = mod;
                finalSize = divider;
            }
        }
        return finalSize;
    }

    /**
     * Get each section from 9-piece format image depends on the spec defined
     * @param img The foreground image.
     * @param x The position where the section start.
     * @param y The position where the section start.
     * @param width The width of the section.
     * @param height The height of the section.
     * @param mode The border type for this section.
     * @param maxWidth When mode == ROUND, this will be used.
     * @param maxHeight When mode == ROUND, this will be used.
     * @return The ImageView for this section.
     */
    private ImageView getSubImageView(Bitmap img, int x, int y, int width, int height,
                                      BorderModeType mode, int maxWidth, int maxHeight) {
        if (img == null) return null;

        if (width <= 0 || height <= 0) return null;

        // Check whether the section is inside the foreground image.
        Rect imgRect = new Rect(0, 0, img.getWidth(), img.getHeight());
        Rect subRect = new Rect(x, y, x + width, y + height);
        if (!imgRect.contains(subRect)) return null;

        Bitmap subImage = Bitmap.createBitmap(img, x, y, width, height);
        ImageView subImageView = new ImageView(mActivity);
        BitmapDrawable drawable;
        if (mode == BorderModeType.ROUND) {
            int originW = subImage.getWidth();
            int originH = subImage.getHeight();
            int newW = originW;
            int newH = originH;
            // Scale down the sub image to let the last image not cropped when it's repeated.
            if (maxWidth > 0) newW = getSuitableSize(maxWidth, originW);
            if (maxHeight > 0) newH = getSuitableSize(maxHeight, originH);
            // recreate the new scaled bitmap.
            Bitmap resizedBitmap = Bitmap.createScaledBitmap(subImage, newW, newH, true);
            // Treat as repeat mode.
            subImage = resizedBitmap;
            mode = BorderModeType.REPEAT;
        }
        if (mode == BorderModeType.REPEAT) {
            drawable = new BitmapDrawable(mActivity.getResources(), subImage);
            drawable.setTileModeXY(TileMode.REPEAT, TileMode.REPEAT);
            subImageView.setImageDrawable(drawable);
            subImageView.setScaleType(ScaleType.FIT_XY);
        } else if (mode == BorderModeType.STRETCH) {
            subImageView.setImageBitmap(subImage);
            subImageView.setScaleType(ScaleType.FIT_XY);
        } else {
            subImageView.setImageBitmap(subImage);
        }

        return subImageView;
    }

    private int getStatusBarHeight() {
        int resourceId = mActivity.getResources().getIdentifier(
                "status_bar_height", "dimen", "android");
        if (resourceId > 0) {
            return mActivity.getResources().getDimensionPixelSize(resourceId);
        }
        // If not found, return default one.
        return 25;
    }

    private RelativeLayout parseImageBorder(String imageBorder) {
        int topBorder = 0;
        int rightBorder = 0;
        int leftBorder = 0;
        int bottomBorder = 0;
        BorderModeType horizontalMode = BorderModeType.STRETCH;
        BorderModeType verticalMode = BorderModeType.STRETCH;

        if (imageBorder.equals("empty")) imageBorder = "";

        // Parse the value of image_border.
        String[] items = imageBorder.split(" ");
        ArrayList<String> borders = new ArrayList<String>();
        ArrayList<BorderModeType> modes = new ArrayList<BorderModeType>();
        for (int i = 0; i < items.length; i++) {
            String item = items[i];
            if (item.endsWith("px")) {
                borders.add(item.replaceAll("px", ""));
            } else if (item.equals(BORDER_MODE_REPEAT)) {
                modes.add(BorderModeType.REPEAT);
            } else if (item.equals(BORDER_MODE_STRETCH)) {
                modes.add(BorderModeType.STRETCH);
            } else if (item.equals(BORDER_MODE_ROUND)) {
                modes.add(BorderModeType.ROUND);
            }
        }
        // Parse borders as defined by the spec.
        try {
            if (borders.size() == 1) {
                topBorder = rightBorder = leftBorder = bottomBorder =
                        Integer.valueOf(borders.get(0));
            } else if (borders.size() == 2) {
                topBorder = bottomBorder = Integer.valueOf(borders.get(0));
                rightBorder = leftBorder = Integer.valueOf(borders.get(1));
            } else if (borders.size() == 3) {
                rightBorder = leftBorder = Integer.valueOf(borders.get(1));
                topBorder = Integer.valueOf(borders.get(0));
                bottomBorder = Integer.valueOf(borders.get(2));
            } else if (borders.size() == 4) {
                topBorder = Integer.valueOf(borders.get(0));
                rightBorder = Integer.valueOf(borders.get(1));
                leftBorder = Integer.valueOf(borders.get(2));
                bottomBorder = Integer.valueOf(borders.get(3));
            }
        } catch (NumberFormatException e) {
            topBorder = rightBorder = leftBorder = bottomBorder = 0;
        }

        // The border values are dpi from manifest.json, need to translate to px.
        DisplayMetrics matrix = mActivity.getResources().getDisplayMetrics();
        topBorder = (int)TypedValue.applyDimension(
                TypedValue.COMPLEX_UNIT_DIP, topBorder, matrix);
        rightBorder = (int)TypedValue.applyDimension(
                TypedValue.COMPLEX_UNIT_DIP, rightBorder, matrix);
        leftBorder = (int)TypedValue.applyDimension(
                TypedValue.COMPLEX_UNIT_DIP, leftBorder, matrix);
        bottomBorder = (int)TypedValue.applyDimension(
                TypedValue.COMPLEX_UNIT_DIP, bottomBorder, matrix);

        // Parse border mode as spec defined.
        if (modes.size() == 1) {
            horizontalMode = verticalMode = modes.get(0);
        } else if (modes.size() == 2) {
            horizontalMode = modes.get(0);
            verticalMode = modes.get(1);
        }

        // Get foreground image
        int imgResId = mActivity.getResources().getIdentifier(
                       "launchscreen_img", "drawable", mActivity.getPackageName());
        if (imgResId == 0) return null;
        Bitmap img = BitmapFactory.decodeResource(mActivity.getResources(), imgResId);
        if (img == null) return null;

        RelativeLayout root = new RelativeLayout(mActivity);
        root.setLayoutParams(new RelativeLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT));
        RelativeLayout.LayoutParams params;
        ImageView subImageView;

        // If no border specified, display the foreground image centered horizontally and vertically.
        if (borders.size() == 0) {
            subImageView = new ImageView(mActivity);
            subImageView.setImageBitmap(img);
            params = new RelativeLayout.LayoutParams(
                    RelativeLayout.LayoutParams.WRAP_CONTENT,
                    RelativeLayout.LayoutParams.WRAP_CONTENT);
            params.addRule(RelativeLayout.CENTER_IN_PARENT, RelativeLayout.TRUE);
            root.addView(subImageView, params);
            return root;
        }

        // Create the 9-piece layout as spec defined.

        // Get Screen width and height.
        Display display = mActivity.getWindowManager().getDefaultDisplay();
        Point size = new Point();
        display.getSize(size);

        // For non fullscreen, the height should substract status bar height
        if ((mActivity.getWindow().getAttributes().flags &
             WindowManager.LayoutParams.FLAG_FULLSCREEN) == 0) {
            size.y -= getStatusBarHeight();
        }

        // Image section-1 top left
        subImageView = getSubImageView(img, 0, 0, leftBorder, topBorder, BorderModeType.NONE, 0, 0);
        if (subImageView != null) {
            params = new RelativeLayout.LayoutParams(
                    RelativeLayout.LayoutParams.WRAP_CONTENT,
                    RelativeLayout.LayoutParams.WRAP_CONTENT);
            params.addRule(RelativeLayout.ALIGN_PARENT_LEFT, RelativeLayout.TRUE);
            params.addRule(RelativeLayout.ALIGN_PARENT_TOP, RelativeLayout.TRUE);
            root.addView(subImageView, params);
        }

        // Image section-2 top
        subImageView = getSubImageView(img, leftBorder, 0, img.getWidth() - leftBorder
                - rightBorder, topBorder, horizontalMode, size.x - leftBorder - rightBorder, 0);
        if (subImageView != null) {
            params = new RelativeLayout.LayoutParams(
                    RelativeLayout.LayoutParams.MATCH_PARENT,
                    RelativeLayout.LayoutParams.WRAP_CONTENT);
            params.addRule(RelativeLayout.ALIGN_PARENT_TOP, RelativeLayout.TRUE);
            params.addRule(RelativeLayout.CENTER_HORIZONTAL, RelativeLayout.TRUE);
            params.leftMargin = leftBorder;
            params.rightMargin = rightBorder;
            root.addView(subImageView, params);
        }

        // Image section-3 top right
        subImageView = getSubImageView(img, img.getWidth() - rightBorder, 0,
                rightBorder, topBorder, BorderModeType.NONE, 0, 0);
        if (subImageView != null) {
            params = new RelativeLayout.LayoutParams(
                    RelativeLayout.LayoutParams.WRAP_CONTENT,
                    RelativeLayout.LayoutParams.WRAP_CONTENT);
            params.addRule(RelativeLayout.ALIGN_PARENT_RIGHT, RelativeLayout.TRUE);
            params.addRule(RelativeLayout.ALIGN_PARENT_TOP, RelativeLayout.TRUE);
            root.addView(subImageView, params);
        }

        // Image section-4 left
        subImageView = getSubImageView(img, 0, topBorder, leftBorder, img.getHeight()
                - topBorder - bottomBorder, verticalMode, 0, size.y - topBorder - bottomBorder);
        if (subImageView != null) {
            params = new RelativeLayout.LayoutParams(
                    RelativeLayout.LayoutParams.WRAP_CONTENT,
                    RelativeLayout.LayoutParams.MATCH_PARENT);
            params.addRule(RelativeLayout.ALIGN_PARENT_LEFT, RelativeLayout.TRUE);
            params.addRule(RelativeLayout.CENTER_IN_PARENT, RelativeLayout.TRUE);
            params.topMargin = topBorder;
            params.bottomMargin = bottomBorder;
            root.addView(subImageView, params);
        }

        // Image section-5 middle
        subImageView = getSubImageView(img, leftBorder, topBorder, img.getWidth() - leftBorder - rightBorder,
                img.getHeight() - topBorder - bottomBorder, BorderModeType.NONE, 0, 0);
        if (subImageView != null) {
            subImageView.setScaleType(ScaleType.FIT_XY);
            params = new RelativeLayout.LayoutParams(
                    RelativeLayout.LayoutParams.MATCH_PARENT,
                    RelativeLayout.LayoutParams.MATCH_PARENT);
            params.leftMargin = leftBorder;
            params.topMargin = topBorder;
            params.rightMargin = rightBorder;
            params.bottomMargin = bottomBorder;
            root.addView(subImageView, params);
        }

        // Image section-6 right
        subImageView = getSubImageView(img, img.getWidth() - rightBorder, topBorder, rightBorder,
                img.getHeight() - topBorder - bottomBorder, verticalMode, 0,
                size.y - topBorder - bottomBorder);
        if (subImageView != null) {
            params = new RelativeLayout.LayoutParams(
                    RelativeLayout.LayoutParams.WRAP_CONTENT,
                    RelativeLayout.LayoutParams.MATCH_PARENT);
            params.addRule(RelativeLayout.CENTER_IN_PARENT, RelativeLayout.TRUE);
            params.addRule(RelativeLayout.ALIGN_PARENT_RIGHT, RelativeLayout.TRUE);
            params.topMargin = topBorder;
            params.bottomMargin = bottomBorder;
            root.addView(subImageView, params);
        }

        // Image section-7 bottom left
        subImageView = getSubImageView(img, 0, img.getHeight() - bottomBorder,
                leftBorder, bottomBorder, BorderModeType.NONE, 0, 0);
        if (subImageView != null) {
            params = new RelativeLayout.LayoutParams(
                    RelativeLayout.LayoutParams.WRAP_CONTENT,
                    RelativeLayout.LayoutParams.WRAP_CONTENT);
            params.addRule(RelativeLayout.ALIGN_PARENT_LEFT, RelativeLayout.TRUE);
            params.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM, RelativeLayout.TRUE);
            root.addView(subImageView, params);
        }

        // Image section-8 bottom
        subImageView = getSubImageView(img, leftBorder, img.getHeight() - bottomBorder,
                img.getWidth() - leftBorder - rightBorder, bottomBorder, horizontalMode,
                size.x - leftBorder - rightBorder, 0);
        if (subImageView != null) {
            params = new RelativeLayout.LayoutParams(
                    RelativeLayout.LayoutParams.MATCH_PARENT,
                    RelativeLayout.LayoutParams.WRAP_CONTENT);
            params.addRule(RelativeLayout.CENTER_HORIZONTAL, RelativeLayout.TRUE);
            params.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM, RelativeLayout.TRUE);
            params.leftMargin = leftBorder;
            params.rightMargin = rightBorder;
            root.addView(subImageView, params);
        }

        // Image section-9 bottom right
        subImageView = getSubImageView(img, img.getWidth() - rightBorder,
                img.getHeight() - bottomBorder, rightBorder, bottomBorder,
	        BorderModeType.NONE, 0, 0);
        if (subImageView != null) {
            params = new RelativeLayout.LayoutParams(
                    RelativeLayout.LayoutParams.WRAP_CONTENT,
                    RelativeLayout.LayoutParams.WRAP_CONTENT);
            params.addRule(RelativeLayout.ALIGN_PARENT_RIGHT, RelativeLayout.TRUE);
            params.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM, RelativeLayout.TRUE);
            root.addView(subImageView, params);
        }
        return root;
    }

    private void registerBroadcastReceiver() {
        IntentFilter intentFilter = new IntentFilter(mIntentFilterStr);
        mLaunchScreenReadyWhenReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                mCustomHideLaunchScreen = true;
                hideLaunchScreenWhenReady();
            }
        };
        mActivity.registerReceiver(mLaunchScreenReadyWhenReceiver, intentFilter);
    }

    private void hideLaunchScreenWhenReady() {
        if (mLaunchScreenDialog == null || !mFirstFrameReceived) return;
        if (mReadyWhen == ReadyWhenType.FIRST_PAINT) {
            performHideLaunchScreen();
        } else if (mReadyWhen == ReadyWhenType.USER_INTERACTIVE) {
            // TODO: Need to listen js DOMContentLoaded event,
            // will be implemented in the next step.
            performHideLaunchScreen();
        } else if (mReadyWhen == ReadyWhenType.COMPLETE) {
            if (mPageLoadFinished) performHideLaunchScreen();
        } else if (mReadyWhen == ReadyWhenType.CUSTOM) {
            if (mCustomHideLaunchScreen) performHideLaunchScreen();
        }
    }

    private void performHideLaunchScreen() {
        mLaunchScreenDialog.dismiss();
        mLaunchScreenDialog = null;
        if (mReadyWhen == ReadyWhenType.CUSTOM) {
            mActivity.unregisterReceiver(mLaunchScreenReadyWhenReceiver);
        }
    }

    private void setReadyWhen(String readyWhen) {
        if (readyWhen.equals("first-paint")) {
            mReadyWhen = ReadyWhenType.FIRST_PAINT;
        } else if (readyWhen.equals("user-interactive")) {
            mReadyWhen = ReadyWhenType.USER_INTERACTIVE;
        } else if (readyWhen.equals("complete")) {
            mReadyWhen = ReadyWhenType.COMPLETE;
        } else if (readyWhen.equals("custom")) {
            mReadyWhen = ReadyWhenType.CUSTOM;
        } else {
            mReadyWhen = ReadyWhenType.FIRST_PAINT;
        }
    }
}
