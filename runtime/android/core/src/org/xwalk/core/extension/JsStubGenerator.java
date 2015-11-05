package org.xwalk.core.extension;

import android.util.Log;

import java.lang.annotation.Annotation;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.Map;

import org.xwalk.core.extension.ReflectionHelper.MemberInfo;
import org.xwalk.core.extension.ReflectionHelper.MemberType;

public class JsStubGenerator {
    static public String TAG = "JsStubGenerator";
    static public final String MSG_TO_OBJECT = "postMessageToObject";
    static public final String MSG_TO_CLASS = "postMessageToClass";
    static public final String MSG_TO_EXTENSION = "postMessageToExtension";
    ReflectionHelper reflection;
    String jsHeader =
            "var v8tools = requireNative(\"v8tools\");\n" +
            "var jsStubModule = requireNative(\"jsStub\");\n" +
            "jsStubModule.init(extension, v8tools);\n" +
            "var jsStub = jsStubModule.jsStub;\n" +
            "var helper = jsStub.createRootStub(exports);\n";

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
     * The parent class XWalkExternalExtension will generate the JavaScript stub if the jsApi
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
     * of XWalkExternalExtension.
     */
    String generate() {
        String result = "";
        MemberInfo entry = reflection.getEntryPoint();
        if (entry != null) result = generateEntryPoint(entry);

        result = (result.length() > 0) ? result : jsHeader;
        if (reflection.getEventList() != null) {
            result += generateEventTarget(reflection);
        }

        Map<String, MemberInfo> members = reflection.getMembers();
        for (String key : members.keySet()) {
            MemberInfo m = members.get(key);
            if (m.isEntryPoint) continue;
            switch (m.type) {
                case JS_PROPERTY:
                    result += generateProperty(MSG_TO_EXTENSION, m);
                    break;
                case JS_METHOD:
                    result += generateMethod(MSG_TO_EXTENSION, m, true);
                    break;
                case JS_CONSTRUCTOR:
                    result += generateConstructor(m, true);
                    break;
                default:
                    break;
            }
        }
        return result + "\n";
    }

    String generateEntryPoint(MemberInfo entry) {
        // Is a binding Object.
        if (entry.type == MemberType.JS_PROPERTY) {
            Class<?> type = ((Field)(entry.accesser)).getType();
            String funcName = type.getSimpleName();
            return jsHeader + String.format("%s(exports, helper);\n", getPrototypeName(funcName));

        }

        if (entry.type == MemberType.JS_METHOD) {
            return String.format("exports = %s;\n %s\n %s",
                    getInternalName(entry.jsName), jsHeader,
                    generateMethod(MSG_TO_EXTENSION, entry, false));

        }

        if (entry.type == MemberType.JS_CONSTRUCTOR) {
            return String.format("exports = %s;\n %s\n %s",
                    entry.jsName, jsHeader, generateConstructor(entry, false));
        }

        return "";
    }

    String[] classGenerator(ReflectionHelper targetReflect) {
        String result = "";
        String staticResult = "";
        if (targetReflect.getEventList() != null) {
            String eventStr = generateEventTarget(targetReflect);
            result += eventStr;
            staticResult += eventStr;
        }

        Map<String, MemberInfo> members = targetReflect.getMembers();
        String memberStr;
        String msgType;
        // @JsConstructor should always used in the extension class, not
        // in binding class, so ignore constructor type in binding classes.
        for (String key : members.keySet()) {
            MemberInfo m = members.get(key);
            msgType = m.isStatic ? MSG_TO_CLASS: MSG_TO_OBJECT;
            switch (m.type) {
                case JS_PROPERTY:
                    memberStr = generateProperty(msgType, m);
                    break;
                case JS_METHOD:
                    memberStr = generateMethod(msgType, m, true);
                    break;
                default:
                    memberStr = "";
                    break;
            }
            if (m.isStatic) {
                staticResult += memberStr;
            } else {
                result += memberStr;
            }
        }
        return (new String[] {result, staticResult}); 
    }

    String destroyBindingObject(ReflectionHelper targetReflect) {
        String result = "exports.destroy = function() {\n";
        Map<String, MemberInfo> members = targetReflect.getMembers();
        for (String key : members.keySet()) {
            result += "delete exports[\"" + key + "\"];\n";
        }
        result += "helper.destroy();\n";
        result += "delete exports[\"__stubHelper\"];\n";
        result += "delete exports[\"destroy\"];\n";
        result += "};";
        return result; 
    }

    String generateEventTarget(ReflectionHelper targetReflect) {
        String[] eventList = targetReflect.getEventList();
        if (eventList == null || eventList.length == 0) {
            return "";
        }

        String gen = "jsStub.makeEventTarget(exports);\n";
        for (String e : eventList) {
            gen += "helper.addEvent(\"" + e + "\");\n";
        }
        return gen;
    }

    String generateProperty(String msgType, MemberInfo m) {
        String name = m.jsName;
        return String.format(
                "jsStub.defineProperty(\"%s\", exports, \"%s\", %b);\n",
                msgType, name, m.isWritable);
    }

    String generatePromiseMethod(String msgType, MemberInfo mInfo) {
        String name = mInfo.jsName;
        String wrapArgs = mInfo.wrapArgs.length() > 0 ? mInfo.wrapArgs : "null"; 
        String wrapReturns = mInfo.wrapReturns.length() > 0 ? mInfo.wrapReturns : "null"; 
        return String.format(
                "jsStub.addMethodWithPromise(\"%s\", exports, \"%s\", %s, %s);\n",
                msgType, name, wrapArgs, wrapReturns);

    }

    String getArgString(Method m, boolean withPromise) {
        if (m == null) return "";

        Class<?>[] pTypes = m.getParameterTypes();
        Annotation[][] anns = m.getParameterAnnotations();
        String jsArgs = "";
        int length = withPromise ? (pTypes.length - 1 ) : pTypes.length;
        for (int i = 0; i < length; ++i) {
            Class<?> p = pTypes[i];
            String pStr = "arg" + i + "_" + p.getSimpleName();
            if (jsArgs.length() > 0)
                jsArgs += ", ";
            jsArgs += pStr;
        }
        return jsArgs;
    }

    String generateMethod(String msgType, MemberInfo mInfo, boolean isMember) {
        // Generate method that returns promise.
        if (mInfo.withPromise) return generatePromiseMethod(msgType,mInfo);

        String name = mInfo.jsName;
        Method m = (Method)mInfo.accesser;
        String iName = getInternalName(name);
        Annotation[][] anns = m.getParameterAnnotations();
        String jsArgs = getArgString(m, mInfo.withPromise);


        boolean isSync = !(m.getReturnType().equals(Void.TYPE));
        String funcBody = String.format(
                "function %s(%s) {\n" +
                ((isSync) ? "  return " : "  ") +
                "helper.invokeNative(\"%s\", \"%s\", [%s], %b);\n" +
                "};\n",
                iName, jsArgs, msgType, name, jsArgs, isSync);

        String memberStr =  isMember ? String.format("exports[\"%s\"] = %s;\n", name, iName) : "";

        return funcBody + memberStr;
    }

    String getInternalName(String name) {
        return "__" + name;
    }

    String getPrototypeName(String funcName) {
        return "__" + funcName + "_prototype";
    }

    String generateConstructor(MemberInfo mInfo, boolean isMember) {
        String name = mInfo.jsName;
        String protoFunc = getPrototypeName(name);
        String argStr = getArgString((Method)mInfo.accesser, false);
        ReflectionHelper targetReflect = reflection.getConstructorReflection(name);
        String[] classStr = classGenerator(targetReflect);

        String protoStr = String.format(
                "function %s(exports, helper){\n" + "%s\n" + "%s\n" + "}\n",
                protoFunc, classStr[0], destroyBindingObject(targetReflect));

        String self = String.format(
                "function %s(%s) {\n" +
                "var newObject = this;\n" +
                "var objectId =\n" +
                "Number(helper.invokeNative(\"%s\", \"+%s\", [%s], true));\n" +
                "if (!objectId) throw \"Error to create instance for constructor:%s.\";\n" +
                "var objectHelper = jsStub.getHelper(newObject, helper);\n" +
                "objectHelper.objectId = objectId;\n" +
                "objectHelper.constructorJsName = \"%s\";\n" +
                "objectHelper.registerLifecycleTracker();" +
                "%s(newObject, objectHelper);\n" +
                "helper.addBindingObject(objectId, newObject);}\n" +
                "helper.constructors[\"%s\"] = %s;\n",
                name, argStr, MSG_TO_EXTENSION, name, argStr, name, name,
                protoFunc, name, name);

        String staticStr = String.format(
                "(function(exports, helper){\n" +
                "  helper.constructorJsName = \"%s\";\n" +
                "%s\n" +
                "})(%s, jsStub.getHelper(%s, helper));\n",
                name, classStr[1], name, name);

        String memberStr =  isMember ? String.format("exports[\"%s\"] = %s;\n", name, name) : "";

        return protoStr + self + staticStr + memberStr;
    }
}
