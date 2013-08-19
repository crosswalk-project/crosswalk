// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime;

import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.content.ServiceConnection;

/**
 * MixContext provides ApplicationContext for the contextImpl object
 * created by Context.CreatePackageContext().
 *
 * For cross package usage, the library part need the possibility to
 * get both the application's context and the library itself's context.
 *
 */
public class MixContext extends ContextWrapper {
    private Context mAppCtx;

    public MixContext(Context base, Context app) {
        super(base);
        mAppCtx = app.getApplicationContext();
    }

    @Override
    public Context getApplicationContext() {
        return mAppCtx;
    }

    @Override
    public boolean bindService(Intent in, ServiceConnection conn, int flags) {
        return mAppCtx.bindService(in, conn, flags);
    }

    @Override
    public void unbindService(ServiceConnection conn) {
        mAppCtx.unbindService(conn);
    }
}
