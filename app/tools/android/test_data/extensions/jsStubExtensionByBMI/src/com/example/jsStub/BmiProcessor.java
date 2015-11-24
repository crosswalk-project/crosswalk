// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package com.example.jsStub;

import org.xwalk.app.runtime.extension.*;

import java.lang.Byte;
import java.lang.Integer;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

class BmiProcessor extends BindingObjectAutoJS {

    @JsApi(withPromise = true)
    public void invertColor(ByteBuffer buffer, String callbackId) {
        buffer.order(ByteOrder.LITTLE_ENDIAN);
        int offset = buffer.position();
        int bytesPerInt = Integer.SIZE / Byte.SIZE;

        int width = buffer.getInt(offset);
        int height = buffer.getInt(offset + bytesPerInt);
        int dataOffset = offset + bytesPerInt * 2;
        int count = buffer.array().length - dataOffset;

        // Binary message: callbackId(int), width(int), height(int), colorImageData
        ByteBuffer message = ByteBuffer.allocate((int) (3 * bytesPerInt + count));
        message.order(ByteOrder.LITTLE_ENDIAN);
        message.rewind();
        message.putInt(Integer.parseInt(callbackId));
        message.putInt(width);
        message.putInt(height);
        for (int i = 0; i < count; i += 4) {
            message.put((byte)(0xff - buffer.get(dataOffset + i)));
            message.put((byte)(0xff - buffer.get(dataOffset + i + 1)));
            message.put((byte)(0xff - buffer.get(dataOffset + i + 2)));
            message.put((byte)0xff);
        }
        // Invert the color.
        // The 'callbackId' should be included in the byte buffer.
        invokeJsCallback(message.array());
    }
}
