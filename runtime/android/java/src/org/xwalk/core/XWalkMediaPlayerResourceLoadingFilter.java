// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.media.MediaPlayer;
import android.net.Uri;

import org.chromium.media.MediaPlayerBridge;

import java.io.File;
import java.util.HashMap;
import java.util.List;

/**
 * This class inherits from MediaPlayerBridge.ResourceLoadingFilter to
 * customize the resource loading process in xwalk.
 */

public class XWalkMediaPlayerResourceLoadingFilter extends
        MediaPlayerBridge.ResourceLoadingFilter {
    @Override
    public boolean shouldOverrideResourceLoading(MediaPlayer mediaPlayer,
            Context context, Uri uri) {
        if (uri.getScheme().equals(AndroidProtocolHandler.APP_SCHEME)) {
            uri = AndroidProtocolHandler.appUriToFileUri(uri);
        }

        String scheme = uri.getScheme();

        if (!scheme.equals(AndroidProtocolHandler.FILE_SCHEME)) return false;

        try {
            AssetFileDescriptor afd =
                    context.getAssets().openFd(AndroidProtocolHandler.getAssetPath(uri));
            mediaPlayer.setDataSource(
                    afd.getFileDescriptor(), afd.getStartOffset(), afd.getLength());

            return true;
        } catch (Exception e) {
            return false;
        }
    }
}
