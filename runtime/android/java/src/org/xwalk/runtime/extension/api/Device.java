package org.xwalk.runtime.extension.api;

import org.xwalk.runtime.extension.XWalkExtension;
import org.xwalk.runtime.extension.XWalkExtensionContext;

// TODO(Donna): Add the implementation.
public class Device extends XWalkExtension {
    final public static String NAME = "Device";
    final public static String VERSION = "0.1.0";
    final public static String JS_API_PATH = "js_api/device.js";

    public Device(String JsApiContent, XWalkExtensionContext context) {
        super(NAME, VERSION, JsApiContent, context);
    }

    @Override
    public void onMessage(String message) {
        return;
    }
}
