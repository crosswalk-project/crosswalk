// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.extension.api.contacts;

import android.content.ContentProviderOperation;
import android.content.ContentResolver;
import android.content.OperationApplicationException;
import android.database.Cursor;
import android.net.Uri;
import android.os.Handler;
import android.os.RemoteException;
import android.provider.ContactsContract;
import android.provider.ContactsContract.RawContacts;
import android.util.Log;

import java.util.ArrayList;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import org.xwalk.core.extension.XWalkExtension;
import org.xwalk.core.extension.XWalkExtensionContext;

public class Contacts extends XWalkExtension {
    public static final String NAME = "xwalk.experimental.contacts";
    public static final String JS_API_PATH = "jsapi/contacts_api.js";

    private static final String TAG = "Contacts";

    private final ContactEventListener mObserver;
    private final ContentResolver mResolver;

    public Contacts(String jsApiContent, XWalkExtensionContext context) {
        super(NAME, jsApiContent, context);
        mResolver = context.getContext().getContentResolver();
        mObserver = new ContactEventListener(new Handler(), this, mResolver);
        mResolver.registerContentObserver(ContactsContract.Contacts.CONTENT_URI, true, mObserver);
    }

    @Override
    public void onMessage(int instanceID, String message) {
        if (message.isEmpty()) return;
        try {
            JSONObject jsonInput = new JSONObject(message);
            String cmd = jsonInput.getString("cmd");
            if (cmd.equals("addEventListener")) {
                mObserver.startListening();
                return;
            }
            JSONObject jsonOutput = new JSONObject();
            jsonOutput.put("_promise_id", jsonInput.getString("_promise_id"));
            if (cmd.equals("save")) {
                ContactSaver saver = new ContactSaver(mResolver);
                jsonOutput.put("data", saver.save(jsonInput.getString("contact")));
            } else if (cmd.equals("find")) {
                ContactFinder finder = new ContactFinder(mResolver);
                String options = jsonInput.has("options") ? jsonInput.getString("options") : null;
                JSONArray results = finder.find(options);
                jsonOutput.put("data", results);
            } else if (cmd.equals("remove")) {
                ArrayList<ContentProviderOperation> ops = new ArrayList<ContentProviderOperation>();
                String[] args = new String[] { jsonInput.getString("contactId") };
                ops.add(ContentProviderOperation.newDelete(RawContacts.CONTENT_URI)
                        .withSelection(RawContacts.CONTACT_ID + "=?", args).build());
                try {
                    mResolver.applyBatch(ContactsContract.AUTHORITY, ops);
                } catch (Exception e) {
                    if (e instanceof RemoteException ||
                        e instanceof OperationApplicationException ||
                        e instanceof SecurityException) {
                        Log.e(TAG, "onMessage - Failed to apply batch: " + e.toString());
                        return;
                    } else {
                        throw new RuntimeException(e);
                    }
                }
            } else if (cmd.equals("clear")) {
                handleClear();
            } else {
                Log.e(TAG, "Unexpected message received: " + message);
                return;
            }
            this.postMessage(instanceID, jsonOutput.toString());
        } catch (JSONException e) {
            Log.e(TAG, e.toString());
        }
    }

    @Override
    public void onResume() {
        mObserver.onResume();
        mResolver.registerContentObserver(ContactsContract.Contacts.CONTENT_URI, true, mObserver);
    }

    @Override
    public void onPause() {
        mResolver.unregisterContentObserver(mObserver);
    }

    @Override
    public void onDestroy() {
        mResolver.unregisterContentObserver(mObserver);
    }

    // Remove all contacts.
    private void handleClear() {
        Cursor c = null;
        try {
            c = mResolver.query(ContactsContract.Contacts.CONTENT_URI, null, null, null, null);
            while (c.moveToNext()) {
                String key = c.getString(c.getColumnIndex(ContactsContract.Contacts.LOOKUP_KEY));
                Uri uri = Uri.withAppendedPath(ContactsContract.Contacts.CONTENT_LOOKUP_URI, key);
                mResolver.delete(uri, null, null);
            }
        } catch (SecurityException e) {
            Log.e(TAG, "handleClear - failed to query: " + e.toString());
        } finally {
            if (c != null) c.close();
        }
    }
}
