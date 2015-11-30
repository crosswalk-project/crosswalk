// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package com.example.jsStub;

import android.util.Log;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.xwalk.app.runtime.extension.*;

/*
 * Example for JavaScript stub auto-generation feature on Android.
 * This extension will expose an object:
 * {
 *   readOnlyProperty: [string] read only
 *   writablePrefix: [string] writable
 *   testEvent: [Function], an async method
 *   echo: [Function], a sync method
 *   getPrifix: [Function], an async method with callback
 *   getPrifixPromise: [Function], a method returns Promise
 *
 *   onupdatePrefix: [Function] writable, property of event handler entry
 *   onclick: [Function] writable, property of event handler entry
 *   onnewStudent: [Function] writable, property of event handler entry
 *
 *   addEventListener: [Function], interface method as an EventTarget
 *   dispatchEvent: [Function], interface method as an EventTarget
 *   removeEventListener: [Function], interface method as an EventTarget
 * }
 */
public class Echo extends BindingObjectAutoJS {

  // Expose a read only JS property.
  @JsApi
  public String readOnlyProperty = "0x0000";

  // Expose a writable JS property.
  @JsApi(isWritable = true)
  public String writablePrefix = "From Java:";

  // The event list must be "static" field.
  @JsApi(isEventList = true)
  public static String[] events = {"updatePrefix", "click", "newStudent"};

  // In some cases, Java objects need to be passed to JavaScript,
  // such as: arguments passed to "invokeJsCallback",
  // event data passed to "dispatchEvent", etc.
  //
  // There are three ways to serialize Java objects:
  // 1. use directly serializable objects, like primitive types, String, and JSONObject.
  // 2. provide a specified method "toJSONString" in the objects to be serialized.
  // 3. rely on the auto-serializing feature in the system, but it may be unqualified for
  //    complicated Java objects. Please do use this feature on simple sturctures.
  
  // Event object specified serializing method "toJSONObject", this must be public method.
  public class Student {
      private String name;
      private int age;

      public Student(String name, int age) {
          this.name = name;
          this.age = age;
      }

      // For Java side usage.
      public String getName() {
          return name;
      }

      // For Java side usage.
      public int getAge() {
          return age;
      }

      // The specified serialization method for JS side.
      public String toJSONString() {
          try {
              JSONObject eventData = new JSONObject();
              eventData.put("name", name);
              eventData.put("age", age);
              return eventData.toString();
          } catch (Exception e) {
              e.printStackTrace();
              return "{}";
          }
      }
  }

  // Event object without customized serializing method.
  // All public fields will be automatically serialized to
  // JSON then passed to JavaScript, and exposed as an object.
  // Please, do keep it simple. The auto-serializing logic do
  // not support complicated Java objects.
  public class Event {
      public String type;
      public int dataInt;
      public String dataStr;
      public Event(String t, int dInt, String dStr) {
          type = t;
          dataInt = dInt;
          dataStr = dStr;
      }
  }

  // This method will be invoked to trigger events.
  @JsApi
  public void testEvent() {
      readOnlyProperty = "0xFFFF";
      updateProperty("readOnlyProperty");
      try {
          // Trigger an event with serializable event data.
          JSONObject event = new JSONObject();
          event.put("type", "updatePrefix");
          event.put("data", "Event data.");
          dispatchEvent("updatePrefix", event);
          
          // Trigger another event with object data will be auto serialized.
          // Only public fields will be serialized and please do keep it simple.
          Event e1 = new Event("click", 99, "helloWorld!");
          dispatchEvent(e1.type, e1);

          // Trigger an event with customised serializable event.
          dispatchEvent("newStudent", new Student("John", 20));

      } catch(Exception e) {
          e.printStackTrace();
      }
  }

  // Expose a sync JS method.
  @JsApi
  public String echo(String msg) {
    return writablePrefix + msg;
  }

  // Expose an async JS method with callback.
  @JsApi
  public void getPrefix(String callbackId) {
    invokeJsCallback(callbackId, writablePrefix);
    return;
  }

  // Expose a JS method returns promise.
  @JsApi(withPromise = true)
  public void getPrefixPromise(String callbackId) {
    Boolean flag = true;
    if (flag)
      invokeJsCallback(callbackId, writablePrefix, "");
    else
      invokeJsCallback(callbackId, "", "OnError in getPrefixPromise");
    return;
  }
}
