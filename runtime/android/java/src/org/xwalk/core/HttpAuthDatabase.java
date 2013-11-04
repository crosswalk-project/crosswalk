// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteException;
import android.util.Log;

/**
 * This database is used to support the methods of
 * setHttpAuthUsernamePassword() and getHttpAuthUsernamePassword()
 * clearHttpAuthUsernamePassword() and hasHttpAuthUsernamePassword().
 *
 * While this class is intended to be used as a singleton, this property is not enforced in this
 * layer, primarily for ease of testing. To line up with the classic implementation and behavior,
 * there is no specific handling and reporting when SQL errors occur.
 *
 * Note on thread-safety: As per the classic implementation, most API functions have thread safety
 * provided by the underlying SQLiteDatabase instance. The exception is database opening: this
 * is handled in the dedicated background thread, which also provides a performance gain
 * if triggered early on (e.g. as a side effect of CookieSyncManager.createInstance() call),
 * sufficiently in advance of the first blocking usage of the API.
 */
public class HttpAuthDatabase {

    private static final String LOGTAG = HttpAuthDatabase.class.getName();

    private static final int DATABASE_VERSION = 1;

    private SQLiteDatabase mDatabase = null;

    private static final String ID_COL = "_id";

    private static final String[] ID_PROJECTION = new String[] {
        ID_COL
    };

    // column id strings for "httpauth" table
    private static final String HTTPAUTH_TABLE_NAME = "httpauth";
    private static final String HTTPAUTH_HOST_COL = "host";
    private static final String HTTPAUTH_REALM_COL = "realm";
    private static final String HTTPAUTH_USERNAME_COL = "username";
    private static final String HTTPAUTH_PASSWORD_COL = "password";

    /**
     * Initially false until the background thread completes.
     */
    private boolean mInitialized = false;

    /**
     * Create an instance of HttpAuthDatabase for the named file, and kick-off background
     * initialization of that database.
     *
     * @param context the Context to use for opening the database
     * @param databaseFile Name of the file to be initialized.
     */
    public HttpAuthDatabase(final Context context, final String databaseFile) {
        new Thread() {
            @Override
            public void run() {
                initOnBackgroundThread(context, databaseFile);
            }
        }.start();
    }

    /**
     * Initializes the databases and notifies any callers waiting on waitForInit.
     *
     * @param context the Context to use for opening the database
     * @param databaseFile Name of the file to be initialized.
     */
    private synchronized void initOnBackgroundThread(Context context, String databaseFile) {
        if (mInitialized) {
            return;
        }

        initDatabase(context, databaseFile);

        // Thread done, notify.
        mInitialized = true;
        notifyAll();
    }

    /**
     * Opens the database, and upgrades it if necessary.
     *
     * @param context the Context to use for opening the database
     * @param databaseFile Name of the file to be initialized.
     */
    private void initDatabase(Context context, String databaseFile) {
        try {
            mDatabase = context.openOrCreateDatabase(databaseFile, 0, null);
        } catch (SQLiteException e) {
            // try again by deleting the old db and create a new one
            if (context.deleteDatabase(databaseFile)) {
                mDatabase = context.openOrCreateDatabase(databaseFile, 0, null);
            }
        }

        if (mDatabase == null) {
            // Not much we can do to recover at this point
            Log.e(LOGTAG, "Unable to open or create " + databaseFile);
            return;
        }

        if (mDatabase.getVersion() != DATABASE_VERSION) {
            mDatabase.beginTransactionNonExclusive();
            try {
                createTable();
                mDatabase.setTransactionSuccessful();
            } finally {
                mDatabase.endTransaction();
            }
        }
    }

    private void createTable() {
        mDatabase.execSQL("CREATE TABLE " + HTTPAUTH_TABLE_NAME
                + " (" + ID_COL + " INTEGER PRIMARY KEY, "
                + HTTPAUTH_HOST_COL + " TEXT, " + HTTPAUTH_REALM_COL
                + " TEXT, " + HTTPAUTH_USERNAME_COL + " TEXT, "
                + HTTPAUTH_PASSWORD_COL + " TEXT," + " UNIQUE ("
                + HTTPAUTH_HOST_COL + ", " + HTTPAUTH_REALM_COL
                + ") ON CONFLICT REPLACE);");

        mDatabase.setVersion(DATABASE_VERSION);
    }

    /**
     * Waits for the background initialization thread to complete and check the database creation
     * status.
     *
     * @return true if the database was initialized, false otherwise
     */
    private boolean waitForInit() {
        synchronized (this) {
            while (!mInitialized) {
                try {
                    wait();
                } catch (InterruptedException e) {
                    Log.e(LOGTAG, "Caught exception while checking initialization", e);
                }
            }
        }
        return mDatabase != null;
    }

    /**
     * Sets the HTTP authentication password. Tuple (HTTPAUTH_HOST_COL, HTTPAUTH_REALM_COL,
     * HTTPAUTH_USERNAME_COL) is unique.
     *
     * @param host the host for the password
     * @param realm the realm for the password
     * @param username the username for the password.
     * @param password the password
     */
    public void setHttpAuthUsernamePassword(String host, String realm, String username,
            String password) {
        if (host == null || realm == null || !waitForInit()) {
            return;
        }

        final ContentValues c = new ContentValues();
        c.put(HTTPAUTH_HOST_COL, host);
        c.put(HTTPAUTH_REALM_COL, realm);
        c.put(HTTPAUTH_USERNAME_COL, username);
        c.put(HTTPAUTH_PASSWORD_COL, password);
        mDatabase.insert(HTTPAUTH_TABLE_NAME, HTTPAUTH_HOST_COL, c);
    }

    /**
     * Retrieves the HTTP authentication username and password for a given host and realm pair. If
     * there are multiple username/password combinations for a host/realm, only the first one will
     * be returned.
     *
     * @param host the host the password applies to
     * @param realm the realm the password applies to
     * @return a String[] if found where String[0] is username (which can be null) and
     *         String[1] is password.  Null is returned if it can't find anything.
     */
    public String[] getHttpAuthUsernamePassword(String host, String realm) {
        if (host == null || realm == null || !waitForInit()){
            return null;
        }

        final String[] columns = new String[] {
            HTTPAUTH_USERNAME_COL, HTTPAUTH_PASSWORD_COL
        };
        final String selection = "(" + HTTPAUTH_HOST_COL + " == ?) AND " +
                "(" + HTTPAUTH_REALM_COL + " == ?)";

        String[] ret = null;
        Cursor cursor = null;
        try {
            cursor = mDatabase.query(HTTPAUTH_TABLE_NAME, columns, selection,
                    new String[] { host, realm }, null, null, null);
            if (cursor.moveToFirst()) {
                ret = new String[] {
                        cursor.getString(cursor.getColumnIndex(HTTPAUTH_USERNAME_COL)),
                        cursor.getString(cursor.getColumnIndex(HTTPAUTH_PASSWORD_COL)),
                };
            }
        } catch (IllegalStateException e) {
            Log.e(LOGTAG, "getHttpAuthUsernamePassword", e);
        } finally {
            if (cursor != null) cursor.close();
        }
        return ret;
    }

    /**
     * Determines if there are any HTTP authentication passwords saved.
     *
     * @return true if there are passwords saved
     */
    public boolean hasHttpAuthUsernamePassword() {
        if (!waitForInit()) {
            return false;
        }

        Cursor cursor = null;
        boolean ret = false;
        try {
            cursor = mDatabase.query(HTTPAUTH_TABLE_NAME, ID_PROJECTION, null, null, null, null,
                    null);
            ret = cursor.moveToFirst();
        } catch (IllegalStateException e) {
            Log.e(LOGTAG, "hasEntries", e);
        } finally {
            if (cursor != null) cursor.close();
        }
        return ret;
    }

    /**
     * Clears the HTTP authentication password database.
     */
    public void clearHttpAuthUsernamePassword() {
        if (!waitForInit()) {
            return;
        }
        mDatabase.delete(HTTPAUTH_TABLE_NAME, null, null);
    }
}
