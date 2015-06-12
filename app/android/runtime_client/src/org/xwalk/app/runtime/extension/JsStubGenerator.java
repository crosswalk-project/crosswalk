package org.xwalk.app.runtime.extension;

import android.util.Log;

import java.lang.annotation.Annotation;
import java.lang.reflect.Method;
import java.util.Map;

import org.xwalk.app.runtime.extension.ReflectionHelper.MemberInfo;

public class JsStubGenerator {
    static public String TAG = "JsStubGenerator";
    ReflectionHelper reflection;
    String jsHeader =
            "var jsStub = requireNative(\"jsStub\").jsStub;\n" +
            "var helper = jsStub.create(exports, extension);\n";

    JsStubGenerator (ReflectionHelper extReflection) {
        reflection = extReflection;
    }

    /*
     * Typically, an external Crosswalk extension for Android will contain three parts:
     * 1. the java files of the native implementation
     * 2. a JavaScript stub for the extension
     * 3. a manifest.json file pointing out: extension name, main class of the extension,
     *    and the JavaScript stub file name.
     * 
     * With JavaScript stub auto-generation feature, you do not need to write a JavaScript
     * stub, nor to point out the JavaScript stub file in manifest.json.
     *
     * The parent class XWalkExtensionClient will generate the JavaScript stub if the jsApi
     * is found null or empty string in its constructor. The generated JavaScript stub will
     * also leverage an internal JavaScript module "jsStub", please refer following file for
     * more information: SOURCE/xwalk/extensions/renderer/xwalk_js_stub_wrapper.js
     * 
     * What's more, this parent class also provides extra helper methods for descendants to
     * communicate with JavaScript by a object oriented way, other than raw messages.
     * For Example:
     *     invokeJsCallback()
     *     dispatchEvent()
     *     updateProperty()
     *     LogJs()
     *
     * This is the generation logic triggered if the jsApi is null or empty in the constructor
     * of XWalkExtensionClient.
     */
    String generate() {
        String result = "";
        if (reflection.getEventList() != null) {
            result += generateEventTarget();
        }

        Map<String, MemberInfo> members = reflection.getMembers();
        for (String key : members.keySet()) {
            MemberInfo m = members.get(key);
            switch (m.type) {
                case JS_PROPERTY:
                    result += generateProperty(key, m);
                    break;
                case JS_METHOD:
                    result += generateMethod(key, m);
                    break;
                default:
                    break;
            }
        }
        return jsHeader + result + "\n";
    }

    String generateEventTarget() {
        String[] eventList = reflection.getEventList();
        if (eventList == null || eventList.length == 0) {
            return "";
        }

        String gen = "jsStub.makeEventTarget(exports);\n";
        for (String e : eventList) {
            gen += "helper.addEvent(\"" + e + "\");\n";
        }
        return gen;
    }

    String generateProperty(String name, MemberInfo m) {
        if (m.isWritable) {
            return "jsStub.defineProperty(exports, \"" + name + "\", true);\n";
        } else {
            return "jsStub.defineProperty(exports, \"" + name + "\");\n";
        }
    }

    String generatePromiseMethod(String name, String jsArgs) {
        String argStr = "{\"resolve\": resolve, \"reject\":reject}";
        if (jsArgs.length() > 0) {
            argStr = jsArgs + ", " + argStr;
        }
        return String.format(
                "exports.%s = function(%s) {\n" +
                "  return new Promise(function(resolve, reject) {\n" +
                "     helper.invokeNative(\"%s\", [%s]);\n" +
                "  })\n" +
                "};\n",
                name, jsArgs, name, argStr);
    }

    String generateMethod(String name, MemberInfo mInfo) {
        Method m = (Method)mInfo.accesser;
        Class<?>[] pTypes = m.getParameterTypes();
        Annotation[][] anns = m.getParameterAnnotations();
        String jsArgs = "";
        for (int i = 0; i < pTypes.length; ++i) {
            Class<?> p = pTypes[i];
            String pStr;
            if (anns[i].length > 0 && anns[i][0] instanceof JsCallback) {
                if (((JsCallback)anns[i][0]).isPromise()) {
                    if (i != pTypes.length - 1) {
                        Log.w(TAG, "@JsCallback(isPromise = true) must annotated on "
                                + "last parameter, the tailing ones will be ignored.");
                    }
                    // Generate method that returns promise.
                    return generatePromiseMethod(name, jsArgs);
                }
                // Indicate that this argument is a callback.
                pStr = "callback" + i + "_function";
            } else {
                pStr = "arg" + i + "_" + p.getSimpleName();
            }

            if (jsArgs.length() > 0)
                jsArgs += ", ";
            jsArgs += pStr;
        }

        boolean isSync = !(m.getReturnType().equals(Void.TYPE));
        return String.format(
                "exports.%s = function(%s) {\n" +
                ((isSync) ? "  return " : "  ") +
                "helper.invokeNative(\"%s\", [%s], %b);\n" +
                "};\n",
                name, jsArgs, name, jsArgs, isSync);
    }
}
