// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension.api.contacts;

import org.xwalk.runtime.extension.XWalkExtension;
import org.xwalk.runtime.extension.XWalkExtensionContext;

import android.content.ContentProviderOperation;
import android.content.ContentResolver;
import android.content.OperationApplicationException;
import android.os.Handler;
import android.os.RemoteException;
import android.provider.ContactsContract;
import android.provider.ContactsContract.RawContacts;
import android.util.Log;

import java.util.ArrayList;

import org.json.JSONException;
import org.json.JSONObject;

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
        if (message.isEmpty()) {
            return;
        }
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
                jsonOutput.put("data", finder.find(jsonInput.getString("options")));
            } else if (cmd.equals("remove")) {
                ArrayList<ContentProviderOperation> ops = new ArrayList<ContentProviderOperation>();
                String[] args = new String[] { jsonInput.getString("contactId") };
                ops.add(ContentProviderOperation.newDelete(RawContacts.CONTENT_URI)
                        .withSelection(RawContacts.CONTACT_ID + "=?", args).build());
                try {
                    mResolver.applyBatch(ContactsContract.AUTHORITY, ops);
                } catch (RemoteException e) {
                    Log.e(TAG, "Failed to apply batch to delete contacts: " + e.toString());
                } catch (OperationApplicationException e) {
                    Log.e(TAG, "Failed to apply batch to delete contacts: " + e.toString());
                }
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
}
