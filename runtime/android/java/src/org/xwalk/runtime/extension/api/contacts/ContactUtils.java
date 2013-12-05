// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension.api.contacts;

import android.content.ContentProviderOperation;
import android.content.ContentProviderResult;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.OperationApplicationException;
import android.database.Cursor;
import android.net.Uri;
import android.os.RemoteException;
import android.provider.ContactsContract;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.Groups;
import android.provider.ContactsContract.RawContacts;
import android.util.Log;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public class ContactUtils {
    private static final String TAG = "ContactUtils";
    public ContentResolver mResolver;
    public ContactUtils(ContentResolver resolver) {
        mResolver = resolver;
    }

    public static <K, V> K getKeyFromValue(Map<K, V> map, V value) {
        K key = null;
        for (Map.Entry<K, V> entry : map.entrySet()) {
            if (value != null && value.equals(entry.getValue())) {
                key = entry.getKey();
                break;
            }
        }
        return key;
    }

    /**
     * @param strings e.g. ["apple", "orange", "banana"]
     * @return A list of question marks to be used in SQL clause, e.g. "?,?,?"
     */
    public static String makeQuestionMarkList(Set<String> strings) {
        String ret = "";
        for (int i = 0; i < strings.size(); ++i) {
            ret += "?,";
        }
        return ret.substring(0, ret.length()-1);
    }

    public boolean hasID(String id) {
        if (id == null) {
            return false;
        }

        final Cursor c = mResolver.query(
                ContactsContract.Contacts.CONTENT_URI,
                null, ContactsContract.Contacts._ID + " = ?", new String[]{id}, null);
        try {
            return (c.getCount() != 0);
        } finally {
            c.close();
        }
    }

    public String getRawId(String id) {
        String rawContactId = null;
        Cursor c = mResolver.query(
                RawContacts.CONTENT_URI, new String[]{RawContacts._ID},
                RawContacts.CONTACT_ID + "=?", new String[]{id}, null);
        try {
            if (c.moveToFirst()) {
                // Actually it is possible that for one contact id there are multiple rawIds.
                rawContactId = c.getString(0);
            }
            return rawContactId;
        } finally {
            c.close();
        }
    }

    public String getId(String rawId) {
        String contactId = null;
        Cursor c = mResolver.query(
                RawContacts.CONTENT_URI, new String[]{RawContacts.CONTACT_ID},
                RawContacts._ID + "=?", new String[]{rawId}, null);
        try {
            if (c.moveToFirst()) {
                contactId = c.getString(0);
            }
            return contactId;
        } finally {
            c.close();
        }
    }

    public Set<String> getCurrentRawIds() {
        Set<String> rawIds = new HashSet<String>();
        Cursor c = mResolver.query(
                RawContacts.CONTENT_URI, new String[]{RawContacts._ID}, null, null, null);
        try {
            while (c.moveToNext()) {
                rawIds.add(c.getString(0));
            }
        } finally {
            c.close();
        }
        return rawIds;
    }

    public String[] getDefaultAccountNameAndType() {
        ArrayList<ContentProviderOperation> ops = new ArrayList<ContentProviderOperation>();
        ops.add(ContentProviderOperation.newInsert(RawContacts.CONTENT_URI)
                .withValue(RawContacts.ACCOUNT_NAME, null)
                .withValue(RawContacts.ACCOUNT_TYPE, null)
                .build());

        ContentProviderResult[] results = null;
        try {
            results = mResolver.applyBatch(ContactsContract.AUTHORITY, ops);
        } catch (RemoteException e) {
            Log.e(TAG, "getDefaultAccountNameAndType - Failed to apply batch: " + e.toString());
        } catch (OperationApplicationException e) {
            Log.e(TAG, "getDefaultAccountNameAndType - Failed to apply batch: " + e.toString());
        }

        Uri rawContactUri = null;
        long rawContactId = 0;
        for (ContentProviderResult result : results) {
            rawContactUri = result.uri;
            rawContactId = ContentUris.parseId(rawContactUri);
        }

        Cursor c = mResolver.query(
                RawContacts.CONTENT_URI
                , new String[] {RawContacts.ACCOUNT_TYPE, RawContacts.ACCOUNT_NAME}
                , RawContacts._ID+"=?"
                , new String[] {String.valueOf(rawContactId)}
                , null);

        String accountType = "";
        String accountName = "";
        try {
            if (c.moveToFirst()) {
                if (!c.isAfterLast()) {
                    accountType = c.getString(c.getColumnIndex(RawContacts.ACCOUNT_TYPE));
                    accountName = c.getString(c.getColumnIndex(RawContacts.ACCOUNT_NAME));
                }
            }
        } finally {
            c.close();
        }

        mResolver.delete(rawContactUri, null, null);

        return new String[] { accountName, accountType };
    }

    public String getGroupId(String groupTitle) {
        final String selection = Groups.DELETED + "=? and " + Groups.GROUP_VISIBLE + "=?";
        Cursor cursor = mResolver.query(
                Groups.CONTENT_URI, null, selection, new String[]{"0", "1"}, null);
        try {
            cursor.moveToFirst();
            for (int i = 0; i < cursor.getCount(); i++) {
                final String title = cursor.getString(cursor.getColumnIndex(Groups.TITLE));
                if (title.equals(groupTitle)) {
                    return cursor.getString(cursor.getColumnIndex(Groups._ID));
                }
                cursor.moveToNext();
            }
            return null;
        } finally {
            cursor.close();
        }
    }

    public String getGroupTitle(String groupId) {
        final String selection = Groups.DELETED + "=? and " + Groups.GROUP_VISIBLE + "=?";
        Cursor cursor = mResolver.query(
                Groups.CONTENT_URI, null, selection, new String[]{"0", "1"}, null);
        try {
            cursor.moveToFirst();
            for (int i = 0; i < cursor.getCount(); i++) {
                final String id = cursor.getString(cursor.getColumnIndex(Groups._ID));
                if (id.equals(groupId)) {
                    return cursor.getString(cursor.getColumnIndex(Groups.TITLE));
                }
                cursor.moveToNext();
            }
            return null;
        } finally {
            cursor.close();
        }
    }

    public String getEnsuredGroupId(String groupTitle) {
        String groupId = getGroupId(groupTitle);
        if (groupId == null) {
            newGroup(groupTitle);
            groupId = getGroupId(groupTitle);
            if (groupId == null) {
                return null;
            }
        }
        return groupId;
    }

    public void newGroup(String groupTitle) {
        final String accountNameType[] = getDefaultAccountNameAndType();
        ArrayList<ContentProviderOperation> o = new ArrayList<ContentProviderOperation>();
        o.add(ContentProviderOperation.newInsert(Groups.CONTENT_URI)
                .withValue(Groups.TITLE, groupTitle)
                .withValue(Groups.GROUP_VISIBLE, true)
                .withValue(Groups.ACCOUNT_NAME, accountNameType[0])
                .withValue(Groups.ACCOUNT_TYPE, accountNameType[1])
                .build());
        try {
            mResolver.applyBatch(ContactsContract.AUTHORITY, o);
        } catch (RemoteException e) {
            Log.e(TAG, "newGroup - Failed to create new contact group: " + e.toString());
        } catch (OperationApplicationException e) {
            Log.e(TAG, "newGroup - Failed to create new contact group: " + e.toString());
        }
    }

    public void cleanByMimeType(String id, String mimeType) {
        mResolver.delete(Data.CONTENT_URI, 
                         String.format("%s = ? AND %s = ?", Data.CONTACT_ID, Data.MIMETYPE),
                         new String[] {id, mimeType});
    }

    /**
     * Get date only from a JSON date string
     * @param string e.g. "1969-12-31T16:00:20.012-0800"
     * @return string e.g. "1969-12-31"
     */
    public String dateTrim(String string) {
        String date = null;
        try {
            final SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd", java.util.Locale.getDefault());
            date = df.format(df.parse(string));
        } catch (ParseException e) {
            Log.e(TAG, "dateFormat - parse failed: " + e.toString());
        }
        return date;
    }
}
