package com.example.extension;

import org.xwalk.core.extension.XWalkExternalExtension;
import org.xwalk.core.extension.XWalkExtensionContextClient;

public class MyExtension extends XWalkExternalExtension {
    // Don't change the parameters in Constructor because XWalk needs to call this constructor.
    public MyExtension(String name, String JsApi, XWalkExtensionContextClient context) {
        super(name, JsApi, context);
    }

    @Override
    public void onMessage(int instanceID, String message){
        postMessage(instanceID, "From java:" + message);
    }

    @Override
    public String onSyncMessage(int instanceID, String message) {
        return "From java sync:" + message;
    }

    @Override
    public void onBinaryMessage(int instanceId, byte[] message) {
        postBinaryMessage(instanceId, message);
    }
}
