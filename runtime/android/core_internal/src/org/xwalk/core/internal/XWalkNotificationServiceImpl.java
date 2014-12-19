// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import java.util.HashMap;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.os.Build.VERSION;
import android.os.Build.VERSION_CODES;
import android.util.AndroidRuntimeException;
import android.util.Log;

import org.xwalk.core.internal.XWalkContentsClientBridge;
import org.xwalk.core.internal.XWalkNotificationService;
import org.xwalk.core.internal.XWalkViewInternal;

/**
 * @hide
 */
public class XWalkNotificationServiceImpl implements XWalkNotificationService {
    private static final String TAG = "XWalkNotificationServiceImpl";

    private static final String XWALK_ACTION_CLICK_NOTIFICATION_SUFFIX = ".notification.click";
    private static final String XWALK_ACTION_CLOSE_NOTIFICATION_SUFFIX = ".notification.close";
    private static final String XWALK_INTENT_EXTRA_KEY_NOTIFICATION_ID = "xwalk.NOTIFICATION_ID";
    private static final String XWALK_INTENT_CATEGORY_NOTIFICATION_PREFIX = "notification_";

    private class WebNotification {
        WebNotification() {
            mMessageNum = 1;
        }

        public Integer mNotificationId;
        public String  mReplaceId;
        public Notification.Builder mBuilder;
        public Integer mMessageNum;
    }

    private Context mContext;
    private XWalkContentsClientBridge mBridge;
    private XWalkViewInternal mView;
    private NotificationManager mNotificationManager;
    private BroadcastReceiver mNotificationCloseReceiver;
    private HashMap<Integer, WebNotification> mExistNotificationIds;
    private HashMap<String, WebNotification>  mExistReplaceIds;

    public XWalkNotificationServiceImpl(Context context, XWalkViewInternal view) {
        mContext = context;
        mView = view;
        mNotificationManager =
                (NotificationManager) mContext.getSystemService(Context.NOTIFICATION_SERVICE);
        // Cancel all exist notifications at startup time. To avoid receiving legacy pendingIntents.
        mNotificationManager.cancelAll();

        mNotificationCloseReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                mView.onNewIntent(intent);
            }
        };

        mExistNotificationIds = new HashMap<Integer, WebNotification>();
        mExistReplaceIds      = new HashMap<String, WebNotification>();
    }

    private static String getCategoryFromNotificationId(int id) {
        return XWALK_INTENT_CATEGORY_NOTIFICATION_PREFIX + id;
    }

    @Override
    public void setBridge(XWalkContentsClientBridge bridge) {
        mBridge = bridge;
    }

    @Override
    public void shutdown() {
        if (!mExistNotificationIds.isEmpty()) {
            unregisterReceiver();
        }
        mBridge = null;
    }

    @Override
    public boolean maybeHandleIntent(Intent intent) {
        if (intent.getAction() == null) return false;
        int notificationId = intent.getIntExtra(XWALK_INTENT_EXTRA_KEY_NOTIFICATION_ID, -1);
        if (notificationId <= 0) return false;
        if (intent.getAction().equals(
                mView.getActivity().getPackageName() + XWALK_ACTION_CLOSE_NOTIFICATION_SUFFIX)) {
            onNotificationClose(notificationId, true);
            return true;
        } else if (intent.getAction().equals(
                mView.getActivity().getPackageName() + XWALK_ACTION_CLICK_NOTIFICATION_SUFFIX)) {
            onNotificationClick(notificationId);
            return true;
        }
        return false;
    }

    public Bitmap getNotificationIcon(Bitmap icon) {
        if (icon == null) return null;
        int originalWidth  = icon.getWidth();
        int originalHeight = icon.getHeight();
        if (originalWidth == 0 || originalHeight == 0) {
            return icon;
        }

        int targetWidth = mContext.getResources().getDimensionPixelSize(
                android.R.dimen.notification_large_icon_width);
        int targetHeight = mContext.getResources().getDimensionPixelSize(
                android.R.dimen.notification_large_icon_height);

        if (originalWidth > targetWidth && originalHeight > targetHeight) {
            if (originalWidth * targetHeight > originalHeight * targetWidth) {
                targetHeight = originalHeight * targetWidth / originalWidth;
            } else {
                targetWidth = originalWidth * targetHeight / originalHeight;
            }
        }

        return Bitmap.createScaledBitmap(icon, targetWidth, targetHeight, true);
    }

    @Override
    @SuppressWarnings("deprecation")
    public void showNotification(String title, String message, String replaceId,
            Bitmap icon, int notificationId) {
        Notification.Builder builder;

        if (!replaceId.isEmpty() && mExistReplaceIds.containsKey(replaceId)) {
            WebNotification webNotification = mExistReplaceIds.get(replaceId);
            notificationId = webNotification.mNotificationId;
            builder = webNotification.mBuilder;
            builder.setNumber(++webNotification.mMessageNum);
        } else {
            builder = new Notification.Builder(mContext.getApplicationContext())
                    .setAutoCancel(true);

            WebNotification webNotification = new WebNotification();
            webNotification.mNotificationId = notificationId;
            webNotification.mReplaceId = replaceId;
            webNotification.mBuilder = builder;

            mExistNotificationIds.put(notificationId, webNotification);
            if (!replaceId.isEmpty()) {
                mExistReplaceIds.put(replaceId, webNotification);
            }
        }

        builder.setContentTitle(title);
        builder.setContentText(message);

        int iconRes = mContext.getApplicationInfo().icon;
        if (iconRes == 0) {
            iconRes = android.R.drawable.sym_def_app_icon;
        }
        builder.setSmallIcon(iconRes);
        Bitmap bigIcon = getNotificationIcon(icon);
        if (bigIcon != null) builder.setLargeIcon(bigIcon);

        Context activity = mView.getActivity();
        String category = getCategoryFromNotificationId(notificationId);

        Intent clickIntent = new Intent(activity, activity.getClass())
                .setAction(activity.getPackageName() + XWALK_ACTION_CLICK_NOTIFICATION_SUFFIX)
                .putExtra(XWALK_INTENT_EXTRA_KEY_NOTIFICATION_ID, notificationId)
                .setFlags(Intent.FLAG_ACTIVITY_LAUNCHED_FROM_HISTORY | Intent.FLAG_ACTIVITY_SINGLE_TOP)
                .addCategory(category);

        Intent closeIntent = new Intent(activity.getPackageName() + XWALK_ACTION_CLOSE_NOTIFICATION_SUFFIX)
                .putExtra(XWALK_INTENT_EXTRA_KEY_NOTIFICATION_ID, notificationId)
                .addCategory(category);

        builder.setContentIntent(PendingIntent.getActivity(
                activity, 0, clickIntent, PendingIntent.FLAG_UPDATE_CURRENT));
        builder.setDeleteIntent(PendingIntent.getBroadcast(
                activity, 0, closeIntent, PendingIntent.FLAG_UPDATE_CURRENT));

        doShowNotification(notificationId, 
                VERSION.SDK_INT >= VERSION_CODES.JELLY_BEAN ? builder.build() : builder.getNotification());
        notificationChanged();
        onNotificationShown(notificationId);
    }

    @Override
    public void cancelNotification(int notificationId) {
        mNotificationManager.cancel(notificationId);
        onNotificationClose(notificationId, false);
    }

    public void doShowNotification(int notificationId, Notification notification) {
        mNotificationManager.notify(notificationId, notification);
    }

    public void onNotificationShown(int notificationId) {
        WebNotification webNotification = mExistNotificationIds.get(notificationId);
        if (webNotification == null) {
            return;
        }

        if (mBridge != null) {
            mBridge.notificationDisplayed(notificationId);
        }
    }

    public void onNotificationClick(int notificationId) {
        WebNotification webNotification = mExistNotificationIds.get(notificationId);
        if (webNotification == null) {
            return;
        }

        mExistNotificationIds.remove(notificationId);
        mExistReplaceIds.remove(webNotification.mReplaceId);

        notificationChanged();
        if (mBridge != null) {
            mBridge.notificationClicked(notificationId);
        }
    }

    public void onNotificationClose(
            int notificationId, boolean byUser) {
        WebNotification webNotification = mExistNotificationIds.get(notificationId);
        if (webNotification == null) {
            return;
        }

        mExistNotificationIds.remove(notificationId);
        mExistReplaceIds.remove(webNotification.mReplaceId);

        notificationChanged();
        if (mBridge != null) {
            mBridge.notificationClosed(notificationId, byUser);
        }
    }

    private void notificationChanged() {
        if (mExistNotificationIds.isEmpty()) {
            Log.i(TAG, "notifications are all cleared," +
                    "unregister broadcast receiver for close pending intent");
            unregisterReceiver();
        } else {
            registerReceiver();
        }
    }

    private void registerReceiver() {
        IntentFilter filter = new IntentFilter(
                mView.getActivity().getPackageName() + XWALK_ACTION_CLOSE_NOTIFICATION_SUFFIX);

        for(Integer id : mExistNotificationIds.keySet()) {
            filter.addCategory(getCategoryFromNotificationId(id));
        }
        
        try {
            mView.getActivity().registerReceiver(mNotificationCloseReceiver, filter);
        } catch (AndroidRuntimeException e) {
            //FIXME(wang16): The exception will happen when there are multiple xwalkviews in one activity.
            //               Remove it after notification service supports multi-views.
            Log.w(TAG, e.getLocalizedMessage());
        }
    }

    private void unregisterReceiver() {
        mView.getActivity().unregisterReceiver(mNotificationCloseReceiver);
    }
}
