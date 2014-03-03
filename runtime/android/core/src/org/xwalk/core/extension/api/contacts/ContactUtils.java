// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.extension.api.contacts;

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
        if (id == null) return false;
        Cursor c = null;
        try {
            c = mResolver.query(ContactsContract.Contacts.CONTENT_URI,
                                null, ContactsContract.Contacts._ID + " = ?",
                                new String[]{id}, null);
            return (c.getCount() != 0);
        } catch (SecurityException e) {
            Log.e(TAG, "hasID: " + e.toString());
            return false;
        } finally {
            if (c != null) c.close();
        }
    }

    public String getRawId(String id) {
        Cursor c = null;
        try {
            c = mResolver.query(RawContacts.CONTENT_URI, new String[]{RawContacts._ID},
                                RawContacts.CONTACT_ID + "=?", new String[]{id}, null);
            if (c.moveToFirst()) {
                // Actually it is possible that for one contact id there are multiple rawIds.
                return c.getString(0);
            } else {
                return null;
            }
        } catch (SecurityException e) {
            Log.e(TAG, "getRawId: " + e.toString());
            return null;
        } finally {
            if (c != null) c.close();
        }
    }

    public String getId(String rawId) {
        Cursor c = null;
        try {
            c = mResolver.query(RawContacts.CONTENT_URI, new String[]{RawContacts.CONTACT_ID},
                                RawContacts._ID + "=?", new String[]{rawId}, null);
            if (c.moveToFirst()) {
                return c.getString(0);
            } else {
                return null;
            }
        } catch (SecurityException e) {
            Log.e(TAG, "getId: " + e.toString());
            return null;
        } finally {
            if (c != null) c.close();
        }
    }

    public Set<String> getCurrentRawIds() {
        Cursor c = null;
        try {
            c = mResolver.query(RawContacts.CONTENT_URI,
                                new String[]{RawContacts._ID}, null, null, null);
            Set<String> rawIds = new HashSet<String>();
            while (c.moveToNext()) {
                rawIds.add(c.getString(0));
            }
            return rawIds;
        } catch (SecurityException e) {
            Log.e(TAG, "getCurrentRawIds: " + e.toString());
            return null;
        } finally {
            if (c != null) c.close();
        }
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
        } catch (Exception e) {
            if (e instanceof RemoteException ||
                e instanceof OperationApplicationException ||
                e instanceof SecurityException) {
                Log.e(TAG, "getDefaultAccountNameAndType - Failed to apply batch: " + e.toString());
                return null;
            } else {
                throw new RuntimeException(e);
            }
        }

        Uri rawContactUri = null;
        long rawContactId = 0;
        for (ContentProviderResult result : results) {
            rawContactUri = result.uri;
            rawContactId = ContentUris.parseId(rawContactUri);
        }

        Cursor c = null;
        String accountType = "";
        String accountName = "";
        try {
            c = mResolver.query(RawContacts.CONTENT_URI,
                                new String[] {RawContacts.ACCOUNT_TYPE, RawContacts.ACCOUNT_NAME},
                                RawContacts._ID + "=?",
                                new String[] {String.valueOf(rawContactId)}, null);
            if (c.moveToFirst()) {
                if (!c.isAfterLast()) {
                    accountType = c.getString(c.getColumnIndex(RawContacts.ACCOUNT_TYPE));
                    accountName = c.getString(c.getColumnIndex(RawContacts.ACCOUNT_NAME));
                }
            }
        } catch (SecurityException e) {
            Log.e(TAG, "getDefaultAccountNameAndType: " + e.toString());
            return null;
        } finally {
            if (c != null) c.close();
        }

        mResolver.delete(rawContactUri, null, null);

        return new String[] { accountName, accountType };
    }

    public String getGroupId(String groupTitle) {
        final String selection = Groups.DELETED + "=? and " + Groups.GROUP_VISIBLE + "=?";
        Cursor c = null;
        try {
            c = mResolver.query(Groups.CONTENT_URI, null, selection, new String[]{"0", "1"}, null);
            c.moveToFirst();
            for (int i = 0; i < c.getCount(); i++) {
                final String title = c.getString(c.getColumnIndex(Groups.TITLE));
                if (title.equals(groupTitle)) {
                    return c.getString(c.getColumnIndex(Groups._ID));
                }
                c.moveToNext();
            }
            return null;
        } catch (SecurityException e) {
            Log.e(TAG, "getGroupId: " + e.toString());
            return null;
        } finally {
            if (c != null) c.close();
        }
    }

    public String getGroupTitle(String groupId) {
        final String selection = Groups.DELETED + "=? and " + Groups.GROUP_VISIBLE + "=?";
        Cursor c = null;
        try {
            c = mResolver.query(Groups.CONTENT_URI, null, selection, new String[]{"0", "1"}, null);
            c.moveToFirst();
            for (int i = 0; i < c.getCount(); i++) {
                final String id = c.getString(c.getColumnIndex(Groups._ID));
                if (id.equals(groupId)) {
                    return c.getString(c.getColumnIndex(Groups.TITLE));
                }
                c.moveToNext();
            }
            return null;
        } catch (SecurityException e) {
            Log.e(TAG, "getGroupTitle: " + e.toString());
            return null;
        } finally {
            if (c != null) c.close();
        }
    }

    public String getEnsuredGroupId(String groupTitle) {
        String groupId = getGroupId(groupTitle);
        if (groupId == null) {
            newGroup(groupTitle);
            groupId = getGroupId(groupTitle);
            if (groupId == null) return null;
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
        } catch (Exception e) {
            if (e instanceof RemoteException ||
                e instanceof OperationApplicationException ||
                e instanceof SecurityException) {
                Log.e(TAG, "newGroup - Failed to create new contact group: " + e.toString());
            } else {
                throw new RuntimeException(e);
            }
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
