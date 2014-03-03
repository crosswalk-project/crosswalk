// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.client;

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

import org.xwalk.core.XWalkContentsClientBridge;
import org.xwalk.core.XWalkNotificationService;
import org.xwalk.core.XWalkView;

public class XWalkDefaultNotificationService implements XWalkNotificationService {
    private static final String TAG = "XWalkDefaultNotificationService";

    private static final String XWALK_ACTION_CLICK_NOTIFICATION_SUFFIX = ".notification.click";
    private static final String XWALK_ACTION_CLOSE_NOTIFICATION_SUFFIX = ".notification.close";
    private static final String XWALK_INTENT_EXTRA_KEY_NOTIFICATION_ID = "xwalk.NOTIFICATION_ID";
    private static final String XWALK_INTENT_EXTRA_KEY_PROCESS_ID = "xwalk.PROCESS_ID";
    private static final String XWALK_INTENT_EXTRA_KEY_ROUTE_ID = "xwalk.ROUTE_ID";
    private static final String XWALK_INTENT_CATEGORY_NOTIFICATION_PREFIX = "notification_";

    private Context mContext;
    private XWalkContentsClientBridge mBridge;
    private XWalkView mView;
    private NotificationManager mNotificationManager;
    private BroadcastReceiver mNotificationCloseReceiver;
    private IntentFilter mNotificationCloseIntentFilter;
    private HashMap<Integer, Notification.Builder> mExistNotificationIds;

    public XWalkDefaultNotificationService(Context context, XWalkView view) {
        mContext = context;
        mView = view;
        mNotificationManager =
                (NotificationManager) mContext.getSystemService(Context.NOTIFICATION_SERVICE);
        // Cancel all exist notifications at startup time. To avoid receiving legacy pendingIntents.
        mNotificationManager.cancelAll();
        mExistNotificationIds = new HashMap<Integer, Notification.Builder>();
    }

    @Override
    public void setBridge(XWalkContentsClientBridge bridge) {
        mBridge = bridge;
    }

    private static String getCategoryFromNotificationId(int id) {
        return XWALK_INTENT_CATEGORY_NOTIFICATION_PREFIX + id;
    }

    private void notificationChanged() {
        unregisterReceiver();
        if (mExistNotificationIds.isEmpty()) {
            Log.i(TAG, "notifications are all cleared," +
                    "unregister broadcast receiver for close pending intent");
        } else {
            registerReceiver();
        }
    }

    private void registerReceiver() {
        mNotificationCloseReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                mView.onNewIntent(intent);
            }
        };
        mNotificationCloseIntentFilter = new IntentFilter(
                mView.getActivity().getPackageName() + XWALK_ACTION_CLOSE_NOTIFICATION_SUFFIX);
        for(Integer id : mExistNotificationIds.keySet()) {
            mNotificationCloseIntentFilter.addCategory(getCategoryFromNotificationId(id));
        }
        try {
            mView.getActivity().registerReceiver(mNotificationCloseReceiver, mNotificationCloseIntentFilter);
        } catch (AndroidRuntimeException e) {
            //FIXME(wang16): The exception will happen when there are multiple xwalkviews in one activity.
            //               Remove it after notification service supports multi-views.
            mNotificationCloseReceiver = null;
            Log.w(TAG, e.getLocalizedMessage());
        }
    }

    private void unregisterReceiver() {
        if (mNotificationCloseReceiver != null) {
            mView.getActivity().unregisterReceiver(mNotificationCloseReceiver);
            mNotificationCloseReceiver = null;
        }
    }

    @Override
    public void shutdown() {
        unregisterReceiver();
        mBridge = null;
    }

    @Override
    public boolean maybeHandleIntent(Intent intent) {
        if (intent.getAction() == null) return false;
        int notificationId = intent.getIntExtra(XWALK_INTENT_EXTRA_KEY_NOTIFICATION_ID, -1);
        int processId = intent.getIntExtra(XWALK_INTENT_EXTRA_KEY_PROCESS_ID, -1);
        int routeId = intent.getIntExtra(XWALK_INTENT_EXTRA_KEY_ROUTE_ID, -1);
        if (notificationId < 0) return false;
        if (intent.getAction().equals(
                mView.getActivity().getPackageName() + XWALK_ACTION_CLOSE_NOTIFICATION_SUFFIX)) {
            onNotificationClose(notificationId, true, processId, routeId);
            return true;
        } else if (intent.getAction().equals(
                mView.getActivity().getPackageName() + XWALK_ACTION_CLICK_NOTIFICATION_SUFFIX)) {
            onNotificationClick(notificationId, processId, routeId);
            return true;
        }
        return false;
    }

    @Override
    @SuppressWarnings("deprecation")
    public void updateNotificationIcon(int notificationId, Bitmap icon) {
        Notification.Builder builder = mExistNotificationIds.get(notificationId);
        if (builder != null) {
            int originalWidth = icon.getWidth();
            int originalHeight = icon.getHeight();
            if (originalWidth == 0 || originalHeight == 0)
                return;
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
            builder.setLargeIcon(
                    Bitmap.createScaledBitmap(icon, targetWidth, targetHeight, true));
            Notification notification;
            if (VERSION.SDK_INT >= VERSION_CODES.JELLY_BEAN) {
                notification = builder.build();
            } else {
                notification = builder.getNotification();
            }
            doShowNotification(notificationId, notification);
            mExistNotificationIds.put(notificationId, builder);
        }
    }

    @Override
    @SuppressWarnings("deprecation")
    public void showNotification(String title, String message,
            int notificationId, int processId, int routeId) {
        Context activity = mView.getActivity();
        String category = getCategoryFromNotificationId(notificationId);
        Intent clickIntent = new Intent(activity, activity.getClass());
        clickIntent.setAction(activity.getPackageName() + XWALK_ACTION_CLICK_NOTIFICATION_SUFFIX);
        clickIntent.putExtra(XWALK_INTENT_EXTRA_KEY_NOTIFICATION_ID, notificationId);
        clickIntent.putExtra(XWALK_INTENT_EXTRA_KEY_PROCESS_ID, processId);
        clickIntent.putExtra(XWALK_INTENT_EXTRA_KEY_ROUTE_ID, routeId);
        clickIntent.setFlags(
                Intent.FLAG_ACTIVITY_LAUNCHED_FROM_HISTORY | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        clickIntent.addCategory(category);
        PendingIntent pendingClickIntent = PendingIntent.getActivity(activity,
                0, clickIntent, PendingIntent.FLAG_UPDATE_CURRENT);
        Intent closeIntent =
                new Intent(activity.getPackageName() + XWALK_ACTION_CLOSE_NOTIFICATION_SUFFIX);
        closeIntent.putExtra(XWALK_INTENT_EXTRA_KEY_NOTIFICATION_ID, notificationId);
        closeIntent.putExtra(XWALK_INTENT_EXTRA_KEY_PROCESS_ID, processId);
        closeIntent.putExtra(XWALK_INTENT_EXTRA_KEY_ROUTE_ID, routeId);
        closeIntent.addCategory(category);
        PendingIntent pendingCloseIntent = PendingIntent.getBroadcast(activity,
                0, closeIntent, PendingIntent.FLAG_UPDATE_CURRENT);
        Notification.Builder builder = new Notification.Builder(mContext.getApplicationContext())
                .setContentIntent(pendingClickIntent)
                .setDeleteIntent(pendingCloseIntent);
        int iconRes = mContext.getApplicationInfo().icon;
        if (iconRes == 0) iconRes = android.R.drawable.sym_def_app_icon;
        builder = builder.setContentText(message)
                         .setContentTitle(title)
                         .setSmallIcon(iconRes)
                         .setAutoCancel(true);
        Notification notification;
        if (VERSION.SDK_INT >= VERSION_CODES.JELLY_BEAN) {
            notification = builder.build();
        } else {
            notification = builder.getNotification();
        }
        doShowNotification(notificationId, notification);
        mExistNotificationIds.put(notificationId, builder);
        notificationChanged();
        onNotificationShown(notificationId, processId, routeId);
    }

    @Override
    public void cancelNotification(int notificationId, int processId, int routeId) {
        mNotificationManager.cancel(notificationId);
        onNotificationClose(notificationId, false, processId, routeId);
    }

    public void doShowNotification(int id, Notification notification) {
        mNotificationManager.notify(id, notification);
    }

    public void onNotificationShown(int notificationId, int processId, int routeId) {
        if (mExistNotificationIds.containsKey(notificationId) && mBridge != null) {
            mBridge.notificationDisplayed(notificationId, processId, routeId);
        }
    }

    public void onNotificationClick(int notificationId, int processId, int routeId) {
        if (mExistNotificationIds.containsKey(notificationId)) {
            mExistNotificationIds.remove(notificationId);
            notificationChanged();
            if (mBridge != null) {
                mBridge.notificationClicked(notificationId, processId, routeId);
            }
        }
    }

    public void onNotificationClose(
            int notificationId, boolean byUser, int processId, int routeId) {
        if (mExistNotificationIds.containsKey(notificationId)) {
            mExistNotificationIds.remove(notificationId);
            notificationChanged();
            if (mBridge != null) {
                mBridge.notificationClosed(notificationId, byUser, processId, routeId);
            }
        }
    }
}
