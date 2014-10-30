package com.example.extension;

import org.xwalk.app.runtime.extension.XWalkExtensionClient;
import org.xwalk.app.runtime.extension.XWalkExtensionContextClient;

public class AdExtension extends XWalkExtensionClient {
    // Don't change the parameters in Constructor because XWalk needs to call this constructor.
    public AdExtension(String name, String JsApi, XWalkExtensionContextClient context) {
        super(name, JsApi, context);
    }
}
