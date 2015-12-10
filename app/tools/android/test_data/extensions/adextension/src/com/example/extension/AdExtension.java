package com.example.extension;

import org.xwalk.core.extension.XWalkExternalExtension;
import org.xwalk.core.extension.XWalkExtensionContextClient;

public class AdExtension extends XWalkExternalExtension {
    // Don't change the parameters in Constructor because XWalk needs to call this constructor.
    public AdExtension(String name, String JsApi, XWalkExtensionContextClient context) {
        super(name, JsApi, context);
    }
}
