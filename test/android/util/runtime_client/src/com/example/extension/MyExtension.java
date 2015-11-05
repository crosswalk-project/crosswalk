package com.example.extension;

import org.xwalk.core.extension.XWalkExtensionContextClient;
import org.xwalk.core.extension.XWalkExternalExtension;

import android.util.Log;

public class MyExtension extends XWalkExternalExtension {
    // Don't change the parameters in Constructor because XWalk needs to call this constructor.
    public MyExtension(String name, String JsApiContent, XWalkExtensionContextClient context) {
        super(name, JsApiContent, context);
    }

    @Override
    public void onMessage(int instanceId, String message) {
        postMessage(instanceId, "From java:" + message);
    }

    @Override
    public String onSyncMessage(int instanceId, String message) {
        return "From java sync:" + message;
    }

    @Override
    public void onBinaryMessage(int instanceId, byte[] message) {
        postBinaryMessage(instanceId, message);
    }
}
