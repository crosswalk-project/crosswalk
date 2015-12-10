package org.xwalk.core.extension;

import android.util.Log;

import java.lang.annotation.Annotation;
import java.lang.reflect.AccessibleObject;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Member;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.json.JSONArray;
import org.json.JSONObject;

class ReflectionHelper {
    private static final String TAG = "JsStubReflectHelper";
    private Class<?> myClass;
    private Map<String, MemberInfo> members = new HashMap<String, MemberInfo>();
    // The map for constructors, (subJavaClassName, exposedJsConstructorName).
    private Map<String, String> bindingClasses = new HashMap<String, String>();
    // The map for constructors, (jsName, ReflectionHelper).
    private Map<String, ReflectionHelper> constructorReflections = new HashMap<String, ReflectionHelper>();
    private String[] eventList = null;
    private MemberInfo entryPoint = null;
    static Set<Class<?>> primitives = new HashSet<>();

    public enum MemberType {
        JS_METHOD,
        JS_PROPERTY,
        JS_CONSTRUCTOR
    }

    public class MemberInfo {
        MemberType type;
        boolean isWritable;
        AccessibleObject accesser;
        boolean isEntryPoint;
        // jsName and javaName are only different for constructors.
        String jsName;
        String javaName;
        // Only for JS_CONSTRUCTOR.
        Class<?> mainClass;
        boolean isStatic;
        boolean withPromise;
        String wrapArgs = "";
        String wrapReturns = "";
    }

    public ReflectionHelper(Class<?> clazz) {
        myClass = clazz;
        init();
    }

    void getMemberInfo(AccessibleObject[] accessers, MemberType type) {
        for (AccessibleObject a : accessers) {
            if (!a.isAnnotationPresent(JsApi.class) && !a.isAnnotationPresent(JsConstructor.class)) continue;

            MemberInfo mInfo = new MemberInfo();
            String name = ((Member) a).getName();
            mInfo.javaName = name;
            mInfo.accesser = a;
            mInfo.isStatic = Modifier.isStatic(((Member)a).getModifiers());
            if (a.isAnnotationPresent(JsApi.class)) {
                JsApi mAnno = a.getAnnotation(JsApi.class);

                // Get eventList from properties.
                if (type == MemberType.JS_PROPERTY && mAnno.isEventList()) {
                    if (!((Field)a).getType().equals(String[].class)) {
                        Log.w(TAG, "Invalid type for Supported JS event list" + name);
                        continue;
                    }
                    try {
                        // Event List should be a class property with "static".
                        eventList = (String[])(((Field)a).get(null));
                    } catch (IllegalArgumentException | IllegalAccessException e ) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    }
                    continue;
                }

                mInfo.type = type;
                mInfo.isWritable = mAnno.isWritable();
                mInfo.isEntryPoint = mAnno.isEntryPoint();
                mInfo.withPromise = mAnno.withPromise();
                mInfo.jsName = name; 
                mInfo.wrapArgs = mAnno.wrapArgs();
                mInfo.wrapReturns = mAnno.wrapReturns();
            } else if (a.isAnnotationPresent(JsConstructor.class)) {
                if (type != MemberType.JS_METHOD) {
                    Log.w(TAG, "Invalid @JsConstructor on non-function member:" + name);
                    continue;
                }
                JsConstructor cAnno = a.getAnnotation(JsConstructor.class);
                mInfo.type = MemberType.JS_CONSTRUCTOR;
                mInfo.isEntryPoint = cAnno.isEntryPoint();
                mInfo.mainClass = cAnno.mainClass();
                // Currently Constructor with promise is not supported.
                mInfo.withPromise = false;
                // TODO: more detail checking for main class.
                // Is there a way to throw compile error if main class missing?
                if (mInfo.mainClass == null) continue;

                mInfo.jsName = mInfo.mainClass.getSimpleName(); 
                // Create relections for constructor main classes.
                bindingClasses.put(mInfo.mainClass.getName(), mInfo.jsName);
                constructorReflections.put(mInfo.jsName, new ReflectionHelper(mInfo.mainClass));
            }

            if (mInfo.isEntryPoint) {
                // Always get the first entry point setting.
                if (entryPoint != null) {
                    Log.w(TAG, "Entry point already exist, try to set another:" + mInfo.jsName);
                    continue;
                }
                // Flag isEntryPoint only meaningful for methods, constructors and BindingObjects.
                if (type == MemberType.JS_PROPERTY
                        && !(isBindingClass(((Field)(mInfo.accesser)).getType()))) {
                    Log.w(TAG, "Invalid entry point setting on property:" + name);
                    continue;
                }
                // The first entry point will be used.
                entryPoint = mInfo;
            }
            if (members.containsKey(mInfo.jsName)) {
                Log.w(TAG, "Conflict namespace - " + mInfo.jsName);
                continue;
            }  
            members.put(mInfo.jsName, mInfo);
        }
    }

    boolean isBindingClass(Class<?> clz) {
        return BindingObject.class.isAssignableFrom(clz);
    }

    void init() {
        primitives.add(Byte.class);
        primitives.add(Integer.class);
        primitives.add(Long.class);
        primitives.add(Double.class);
        primitives.add(Character.class);
        primitives.add(Float.class);
        primitives.add(Boolean.class);
        primitives.add(Short.class);

        // Find all functions.
        getMemberInfo(myClass.getDeclaredMethods(), MemberType.JS_METHOD);

        // Find all properties
        getMemberInfo(myClass.getDeclaredFields(), MemberType.JS_PROPERTY);
    }

    // Register all the exposed APIs TO message handler.
    // @handler the message handler APIs will be registered in.
    // @object the target object of the handler.
    public static void registerHandlers(ReflectionHelper reflection,
            MessageHandler handler, Object object) {
        if (reflection == null || handler == null) return;

        for (String key : reflection.getMembers().keySet()) {
            MemberInfo m = reflection.getMembers().get(key);
            handler.register(m.jsName, m.javaName, m.type, object, reflection);
        }
    }

    Map<String, MemberInfo> getMembers() {
        return members;
    }

    ReflectionHelper getConstructorReflection(String jsName) {
        if (!constructorReflections.containsKey(jsName)) return null;

        return constructorReflections.get(jsName);
    }

    ReflectionHelper getReflectionByBindingClass(String className) {
        if (!bindingClasses.containsKey(className)) return null;

        return getConstructorReflection(bindingClasses.get(className));
    }

    Boolean hasMethod(String name) {
        if (!members.containsKey(name)) return false;

        MemberInfo m = members.get(name);
        return ((m.type == MemberType.JS_METHOD) || (m.type == MemberType.JS_CONSTRUCTOR));
    }

    Boolean hasProperty(String name) {
        if (!members.containsKey(name)) return false;

        return members.get(name).type == MemberType.JS_PROPERTY;
    }

    MemberInfo getMemberInfo(String name) {
        return members.get(name);
    }

    /*
     * Use case: construct Java object array from JSON array which is passed by JS
     * 1. restore original Java object in the array
     * 2. if the parameter is a callbackID, then combine the instanceID with it
     */
    Object[] getArgsFromJson(XWalkExternalExtension ext, int instanceID, Method m, JSONArray args) {
        Class<?>[] pTypes = m.getParameterTypes();
        Object[] oArgs = new Object[pTypes.length];
        boolean isStatic = Modifier.isStatic(m.getModifiers());
        for (int i = 0; i < pTypes.length; ++i) {
            try {
                Class<?> p = pTypes[i];
                // Identify the static methods which need the information to post message back to JS.
                if (isStatic && p.equals(JsContextInfo.class)) {
                    oArgs[i++] = new JsContextInfo(instanceID, ext, m.getClass(),
                                                   Integer.toString(0));
                } else {
                    // TODO: check if this is enough for other types.
                    oArgs[i] = args.get(i);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return oArgs;
    }

    public static boolean isSerializable(Object obj) {
        Class<?> clz = obj.getClass();

        return clz.isPrimitive() ||
                primitives.contains(clz) ||
                obj instanceof String ||
                obj instanceof Map ||
                obj instanceof JSONArray ||
                obj instanceof JSONObject;
    }

    public static Object toSerializableObject(Object obj) {
        // Case for objects have no specified method to JSON string.
        if (obj.getClass().isArray()) {
            JSONArray result = new JSONArray();
            Object [] arr = (Object[]) obj;
            for (int i = 0; i < arr.length; ++i) {
                try {
                    result.put(i, toSerializableObject(arr[i]));
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
            return result;
        }
        // The original serializable object.
        if (isSerializable(obj)) return obj;

        // Customized serializable object.
        //
        // If the object is not directly serializable, we check if it is customised
        // serializable. That means the developer implemented the public serialization
        // method "toJSONString".
        try {
            Method m = obj.getClass().getMethod("toJSONString", new Class<?>[0]);
            String jsonStr = (String)(m.invoke(obj, new Object[0]));
            if (jsonStr.trim().charAt(0) == '[') {
                return new JSONArray(jsonStr);
            } else {
                return new JSONObject(jsonStr);
            }
        } catch (Exception e) {
            Log.w(TAG, "No serialization method: \"toJSONString\", or errors happened.");
        }

        /*
         * For ordinary objects, we will just serialize the accessible fields.
         */
        try {
            Class<?> c = obj.getClass();
            JSONObject json = new JSONObject();
            Field[] fields = c.getFields();
            for (Field f : fields) {
                json.put(f.getName(), f.get(obj));
            }
            return json;
        } catch (Exception e) {
            Log.e(TAG, "Field to serialize object to JSON.");
            e.printStackTrace();
            return null;
        }
    }

    /*
     * Use case: return the Java object back to JS after invokeNativeMethod
     * 1. quote string in proper way
     * 2. serialize the normal Java object
     */
    public static String objToJSON(Object obj) {
        // We expect the object is JSONObject or primive type.
        if (obj == null) return "null";

        Object sObj = toSerializableObject(obj);
        return (sObj instanceof String) ?
                JSONObject.quote(sObj.toString()) : sObj.toString();
    }

    Object invokeMethod(XWalkExternalExtension ext, int instanceID, Object obj,
                        String mName, JSONArray args) throws Exception {
        if (!hasMethod(mName)) {
            throw new NoSuchMethodException("No such method:" + mName);
        }
        if (!(getMemberInfo(mName).isStatic) && !(myClass.isInstance(obj))) {
            throw new InvocationTargetException(new Exception("Invalid target to set property:" + mName));
        }
        Method m = (Method)members.get(mName).accesser;
        if (!m.isAccessible()) {
            m.setAccessible(true);
        }
        Object[] oArgs = getArgsFromJson(ext, instanceID, m, args);
        return m.invoke(obj, oArgs);
    }

    Object getProperty(Object obj, String pName) throws Exception {
        if (!hasProperty(pName)) {
            throw new NoSuchFieldException("No such property:" + pName);
        }
        if (!(getMemberInfo(pName).isStatic) && !(myClass.isInstance(obj))) {
            throw new InvocationTargetException(new Exception("Invalid target to set property:" + pName));
        }

        Field f = (Field)members.get(pName).accesser;
        if (!f.isAccessible()) {
            f.setAccessible(true);
        }
        return f.get(obj);
    }

    void setProperty(Object obj, String pName, Object value) throws Exception {
        if (!hasProperty(pName)) {
            throw new NoSuchFieldException("No such property:" + pName);
        }
        if (!(getMemberInfo(pName).isStatic) && !(myClass.isInstance(obj))) {
            throw new InvocationTargetException(new Exception("Invalid target to set property:" + pName));
        }

        Field f = (Field)members.get(pName).accesser;
        if (!f.isAccessible())
            f.setAccessible(true);
        f.set(obj, value);
    }

    String[] getEventList() {
        return eventList;
    }

    MemberInfo getEntryPoint() {
        return entryPoint;
    }

    boolean isEventSupported(String event) {
        if (eventList == null) return false;
        for (int i = 0; i < eventList.length; ++i) {
            if (eventList[i].equals(event)) return true;
        }
        return false;
    }

    boolean isInstance(Object obj) {
        return myClass.isInstance(obj);
    }

    public Object handleMessage(MessageInfo info, Object targetObj) throws Exception {
        Object result = null;
        try {
            String cmd = info.getCmd();
            JSONArray args;
            // All the methods using BMI, should put the callback at last.
            if (info.getBinaryArgs() != null) {
                JSONArray newArgs = new JSONArray();
                newArgs.put(info.getBinaryArgs());
                newArgs.put(info.getCallbackId());
                args = newArgs;
            } else {
                args = info.getArgs();
            }
            String memberName = info.getJsName();
            XWalkExternalExtension ext = info.getExtension();
            int instanceId = info.getInstanceId();
            switch (cmd) {
                case "invokeNative":
                    result = invokeMethod(ext, instanceId, targetObj, memberName, args);
                    break;
                case "newInstance":
                    BindingObject newObj = (BindingObject)(invokeMethod(
                                            ext,instanceId, targetObj, memberName, args));
                    result = info.getInstanceHelper().addBindingObject(info.getObjectId(), newObj);
                    break;
                case "getProperty":
                    result = getProperty(targetObj, memberName);
                    break;
                case "setProperty":
                    setProperty(targetObj, memberName, args.get(0));
                    break;
                default:
                    Log.w(TAG, "Unsupported cmd: " + cmd);
                    break;
            }
        } catch (Exception e) {
            Log.w(TAG, "Invalid message, error msg:\n" + e.toString());
            e.printStackTrace();
        }
        return result;
    }
}
