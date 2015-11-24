/*
 * This will be a native module, extension js stub can use
 * This is a internal js helper module in the extension's js context.
 * Extension JavaScript stub can get this module by: requireNative("jsStub");
 * Currently, this helper just be used by "auto-generated" js-stubs,
 * following is an example js-stub using the heper.
 * Usage example:
 *     var jsStub = requireNative("jsStub").jsStub;
 *     var helper = jsStub.create(exports, extension);
 *
 *     jsStub.makeEventTarget(exports);
 *     helper.addEvent("updatePrefix");
 *     helper.addEvent("click");
 *
 *     jsStub.defineProperty(exports, "prefix");
 *
 *     exports.getPrefix = function(callback0_function) {
 *       helper.invokeNative("getPrefix", [callback0_function], false);
 *     };
 *     exports.testEvent = function() {
 *       helper.invokeNative("testEvent", [], false);
 *     };
 *     exports.echo = function(arg0_String) {
 *       return helper.invokeNative("echo", [arg0_String], true);
 *     };
 *     exports.getPrefixPromise = function() {
 *       return new Promise(function(resolve, reject) {
 *           helper.invokeNative("getPrefixPromise", [resolve, reject]);
 *       })
 *     };
 *
 *     exports.getPrefixPromise = function() {
 *       return new Promise(function(resolve, reject) {
 *           var cb = function(data, error) {
 *             if (error) {
 *               reject(error);
 *             } else {
 *               if (wrapReturns) {
 *                 resolve(wrapReturns(data));
 *               } else {
 *                 resolve(data);
 *               }
 *             }
 *           };
 *           helper.invokeNative("getPrefixPromise", cb);
 *       })
 *     };
 */
var v8tools;
var channel;
var MSG_TO_OBJECT = "postMessageToObject";
var MSG_TO_CLASS = "postMessageToClass";
var MSG_TO_EXTENSION = "postMessageToExtension";

var jsStub = function(base, rootStub) {
  // Refer to the exposed extension object.
  this.base = base;

  // Point to the extension stub helper.
  this.rootStub = rootStub || this;

  // Retain the properties which is exposed by native.
  this.properties = {};
};

function messageHandler(msgType, targetHelper, msg) {
  if (!msg.cmd)
    console.warn("No valid Java CMD.");
  switch (msg.cmd) {
    case "updateProperty":
      // Case: property is changed by native and need
      // to sync its value to JS side.
      if (targetHelper.properties.hasOwnProperty(msg.name) >= 0) {
        targetHelper.properties[msg.name] = targetHelper.getNativeProperty(msgType, msg.name);
      }
      break;
    case "dispatchEvent":
      // Dispatch event from native.
      if (msg.type) {
        var type = String(msg.type);
        var listeners = targetHelper.event_listeners[type];

        var wrapEvent;
        for (var i in listeners) {
          wrapEvent = targetHelper.event_synthesizers[type];
          listeners[i](new wrapEvent(type, msg.event));
        }
      }
      break;

    default:
      console.warn("Unsupported Java CMD:" + msg.cmd);
  }
}

function makeExtensionHelper(helper) {
  var nextObjectId = 1;
  var bindingObjects = [];
  var nextCallbackId = 1;
  helper.callbacks = {};

  helper.getCallbackId = function() {
    while (helper.callbacks[String(nextCallbackId)] != undefined)
      ++nextCallbackId;
    return String(nextCallbackId);
  };
  helper.setNextCallbackId = function(cid) {
    nextCallbackId = Number(cid);
  };
  helper.constructors = {};
  helper.eventObservers = {};
  helper.getObjectId = function() {
    while (bindingObjects[nextObjectId] != undefined)
      ++nextObjectId;
    return nextObjectId;
  };
  helper.addBindingObject = function(objectId, newObject) {
    bindingObjects[objectId] = newObject;
  };

  helper.removeBindingObject = function(objectId) {
    bindingObjects[objectId] = undefined;
    nextObjectId = objectId;
  };

  helper.getBindingObject = function(objectId) {
    return  bindingObjects[objectId];
  };
  helper.getConstructorHelper = function(ctorName) {
    return jsStub.getHelper(this.constructors[ctorName]);
  };

  helper.addEventObserver = function(type, obj) {
    if (type in this.eventObservers) {
      var observers = this.eventObservers[type];
      if (observers.indexOf(obj) < 0)
        observers.push(obj);
    } else {
      this.eventObservers[type] = [obj];
    }
  };

  helper.removeEventObserver = function(type, obj) {
    if (type in this.eventObservers) {
      var observers = this.eventObservers[type];
      var index = observers.indexOf(obj);
      if (index > -1)
        observers.splice(index, 1);
    };
  };

  // Wrap the object message handler for extension object.
  helper.handleMessage = function (msg) {
    var rootStub = this.rootStub;
    function invokeMethod(callbackId, args) {
      if (!callbackId) return;

      var listener = rootStub.callbacks[callbackId];
      if (!(listener instanceof Function)) return;

      if (!(listener.apply(null, args))) {
        delete rootStub.callbacks[callbackId];
        rootStub.setNextCallbackId(callbackId);
      }
    }

    // Handle the message.
    if (msg instanceof ArrayBuffer) {
      // Binary Message.
      var int32_array = new Int32Array(msg, 0 , 1);
      invokeMethod(int32_array[0], [msg]);
      return;
    }

    var msgObj = JSON.parse(msg);

    // Message posted from info.PostResult().
    if (Array.isArray(msgObj)) {
      invokeMethod(msgObj.shift(), msgObj);
      return;
    }
      // Message posted from exposed interfaces in BindingObjectAutoJs.
      if (msgObj.cmd == "onEvent") {
        var observers = this.eventObservers[msgObj.type];
        if (!Array.isArray(observers) ) return;

        for (var o of observers) {
          o.dispatchEvent(msgObj.type, msgObj.event);
        }
        return;
      }

      if (msgObj.cmd == "invokeCallback") {
        invokeMethod(msgObj.callbackId, msgObj.args);
        return;
      }
      // Message about properties.
      var targetHelper = this;
      var msgType = MSG_TO_EXTENSION;

      var ctorName = String(msgObj["constructorName"]).trim();
      // Message back to constructors.
      if (msgObj.objectId == 0 && ctorName.length > 0) {
        targetHelper = this.getConstructorHelper(ctorName);
        msgType = MSG_TO_CLASS;
      } else if (msgObj.objectId > 0) {
        // Message back to binding objects.
        var obj = bindingObjects[msgObj.objectId];
        if (!obj) return;
        targetHelper = jsStub.getHelper(obj);
        msgType = MSG_TO_OBJECT;
      }
      if (!targetHelper) return;

      messageHandler(msgType, targetHelper, msgObj);
  }
}

jsStub.createRootStub = function(base) {
  var helper = jsStub.getHelper(base, null);

  // Add some specail properties for extension object only.
  // These properties will be unavaliable for binding-object helpers.
  makeExtensionHelper(helper);

  // Set up messageListener for extension object.
  channel.setMessageListener(function (msg) {
    helper.handleMessage(msg);
  });
  return helper;
};

jsStub.getHelper = function(base, rootStub) {
  if (!(base.__stubHelper instanceof jsStub)) {
    Object.defineProperty(base, "__stubHelper", {
      // TODO: set to false if find good way to do clear destroy.
      "configurable": true,
      "enumerable": false,
      "value":new jsStub(base, rootStub) 
    });
  }
  return base.__stubHelper;
};

jsStub.prototype = {
  "invokeNativeByBMI": function(msgType, name, args, callback) {
    // This interface only for "msgType" == MESSAGE_TO_OBJECT
    if (!(args instanceof ArrayBuffer) || !(this.objectId)) {
      console.warn("Invalid argument(s) for invokeNativeWithBMI.");
      return;
    }
    function alignedWith4Bytes(number) {
      return number + (4 - number % 4);
    };

    var rootStub = this.rootStub;
    const byteLengthOfInt32 = 4;
    name = String(name);
    var callbackID = 0;
    if (callback instanceof Function) {
      callbackID = parseInt(rootStub.getCallbackId());
      rootStub.callbacks[callbackID] = callback;
    }
    var functionName = MSG_TO_OBJECT;
    var methodName = name;
    var methodArgs = args;
    var objectId = String(this.objectId);

    var allignFuncNameLen = alignedWith4Bytes(functionName.length);
    var allignMethodNameLen = alignedWith4Bytes(methodName.length);
    var allignObjectId = alignedWith4Bytes(objectId.length);

    // Final ArrayBuffer includes funcNameLen(int32),funcName(string), callbackID(int32),
    // objectIdLen(int32), objectId(string), methodNameLen(int32), methodName(string),
    // methodArgs(ArrayBuffer)
    var byteLen = byteLengthOfInt32 + allignFuncNameLen + 2 * byteLengthOfInt32 +
      allignObjectId + byteLengthOfInt32 + allignMethodNameLen + methodArgs.byteLength;
    var arrayBuffer = new ArrayBuffer(byteLen);
    var offset = 0;
    var view = new Int32Array(arrayBuffer, offset, 1);
    view[0] = functionName.length;

    offset += byteLengthOfInt32;
    view = new Uint8Array(arrayBuffer, offset, functionName.length);
    for (var i = 0; i < functionName.length; i++) {
      view[i] = functionName.charCodeAt(i);
    }

    offset += allignFuncNameLen;
    view = new Int32Array(arrayBuffer, offset, 2);
    view[0] = callbackID;
    view[1] = objectId.length;

    offset += 2 * byteLengthOfInt32;
    view = new Uint8Array(arrayBuffer, offset, objectId.length);
    for (var i = 0; i < objectId.length; i++) {
      view[i] = objectId.charCodeAt(i);
    }

    offset += allignObjectId;
    view = new Int32Array(arrayBuffer, offset, 1);
    view[0] = methodName.length;

    offset += byteLengthOfInt32;
    view = new Uint8Array(arrayBuffer, offset, methodName.length);
    for (var i = 0; i < methodName.length; i++) {
      view[i] = methodName.charCodeAt(i);
    }

    offset += allignMethodNameLen;
    view = new Uint8Array(arrayBuffer, offset);
    view.set(new Uint8Array(methodArgs), 0);

    channel.postMessage(arrayBuffer);
  },

  "invokeNative": function(msgType, name, args, sync) {
    var rootStub = this.rootStub;
    name = String(name);
    var isNewInstance = (name[0] == '+') ? true : false;

    if (!Array.isArray(args)) {
      console.warn("invokeNative: args is not an array.");
      args = Array(args);
    }

    args.forEach(function(val, index, a) {
      if (val instanceof Function) {
        // Retain callbacks in JS stub, replace them to related number ID
        var cid = rootStub.getCallbackId();
        rootStub.callbacks[cid] = val;
        a[index] = cid;
      }
    })

    var msg = {
      type: msgType,
      cmd: isNewInstance ? "newInstance" : "invokeNative",
      name: isNewInstance ? name.substr(1) : name,
      args: args
    };

    if (isNewInstance) {
      var objectId = rootStub.getObjectId();
      msg.args = [objectId, args];
      // Use sync channel for newInstance actions.
      // "newInstance" action should return true/false.
      return (this.sendSyncMessage(msg)? objectId : 0);
    }

    if (sync)
      return this.sendSyncMessage(msg);
    else
      this.postMessage(msg);
  },
  // Only Binding object need this lifecycle tracker.
  "registerLifecycleTracker": function() {
    // TODO(Donna): This will never be triggered,
    // because we kept all the binding Object in jsStub.
    var baseObject = this.base;
    var helper = this;
    Object.defineProperty(baseObject, "_tracker", {
      value: v8tools.lifecycleTracker(),
    });

    var objId = helper.objectId;
    var msg = {
      type: MSG_TO_EXTENSION,
      cmd: "invokeNative",
      name: "JSObjectCollected",
      args: [objId]
    };
    baseObject._tracker.destructor = function() {
      // TODO(Donna): find better way to coorperate with "destroy".
      //channel.postMessage(msg);
    };
  },
  "destroy": function() {
    var objId = this.objectId;
    var msg = {
      type: MSG_TO_EXTENSION,
      cmd: "invokeNative",
      name: "JSObjectCollected",
      args: [objId]
    };
    this.postMessage(msg);
    this.rootStub.removeBindingObject(objId);
    delete this.objectId;
    delete this.event_listeners;
    delete this.base;
    delete this.rootStub;
    delete this.callbacks;
    delete this.properties;
    delete this.postMessage;
    delete this.sendSyncMessage;
    delete this.invokeNative;
    delete this.getNativeProperty;
    delete this.setNativeProperty;
    delete this.destroy;
  },
  "getNativeProperty": function(msgType, name) {
    return this.sendSyncMessage({
      type: msgType,
      cmd: "getProperty",
      name: name,
      args: []
    });
  },
  "setNativeProperty": function(msgType, name, value) {
    this.sendSyncMessage({
      type: msgType,
      cmd: "setProperty",
      name: name,
      args: [value]
    });
  },
  "sendSyncMessage": function(msg) {
    // Add objectId for each message.
    msg["objectId"] = this.objectId ? this.objectId : 0;
    if (msg.type == "postMessageToClass") {
      msg.args = [this.constructorJsName, msg.args];
    }

    var resultStr = channel.internal.sendSyncMessage(JSON.stringify(msg));
    return resultStr.length > 0 ? JSON.parse(resultStr) : undefined;
  },
  "postMessage": function(msg) {
    // Add objectId for each message.
    msg["objectId"] = this.objectId ? this.objectId : 0;
    if (msg.type == "postMessageToClass") {
      msg.args = [this.constructorJsName, msg.args];
    }

    channel.postMessage(JSON.stringify(msg));
  }
};

// expose native property to JavaScript
jsStub.defineProperty = function(msgType, obj, prop, writable) {
  var helper = jsStub.getHelper(obj);

  // keep all exported properties in the helper
  helper.properties[prop] = helper.getNativeProperty(msgType, prop);

  var desc = {
    // TODO: set to false if find good way to do clear destroy.
    'configurable': true,
    'enumerable': true,
    'get': function() {
      return helper.properties[prop];
    }
  }
  if (writable) {
    desc.set = function(v) {
      helper.setNativeProperty(msgType, prop, v);
      helper.properties[prop] = v;
    }
  }
  Object.defineProperty(obj, prop, desc);
};

jsStub.addMethodWithPromise = function(msgType, obj, name, wrapArgs, wrapReturns) {
  var helper = jsStub.getHelper(obj);
  var desc = {
    'enumerable': true,
    'value': function() {
      var args = Array.prototype.slice.call(arguments);
      var useBMI = false;
      if (wrapArgs instanceof Function) {
        args = wrapArgs(args);
        if (!args) {
          return new Promise(function(resolve, reject) {
            reject('Package the parameters failed. Invalid parameters');
          });
        }
        useBMI = (args instanceof ArrayBuffer) ? true : false;
      } else if (args.length == 1 && (args[0] instanceof ArrayBuffer)) {
        args = args[0];
        useBMI = true;
      }

      return new Promise(function(resolve, reject) {
        var callback = function(data, error) {
          if (error) {
            reject(error);
          } else if (wrapReturns instanceof Function) {
            resolve(wrapReturns(data));
          } else {
            resolve(data);
          }
        };

        if (useBMI) {
          helper.invokeNativeByBMI(msgType, name, args, callback);
        } else {
          args.push(callback);
          helper.invokeNative(msgType, name, args);
        }
      });
    }
  };
  Object.defineProperty(obj, name, desc);
};
// EventTarget implementation:
// When dispatch event in the Java side, we will broadcast the
// message to all instances, so all frames will handle the event.
jsStub.makeEventTarget = function(base) {
  var helper = jsStub.getHelper(base);
  helper.event_listeners = {};
  helper.event_synthesizers = {};

  var DefaultEvent = function(type, data) {
    this.type = type;

    if (data)
      this.data = data;
  };

  helper.addEvent = function(type, wrapEvent) {
    Object.defineProperty(helper, "_on" + type, {
      writable : true,
    });

    Object.defineProperty(base, "on" + type, {
      get: function() {
        return helper["_on" + type];
      },
      set: function(listener) {
        var old_listener = helper["_on" + type];
        if (!(listener instanceof Function)) {
          helper["_on" + type] = null;
          base.removeEventListener(type, old_listener);
          return;
        }

        if (old_listener === listener)
          return;

        if (old_listener)
          base.removeEventListener(type, old_listener);

        helper["_on" + type] = listener;
        base.addEventListener(type, listener);
      },
      enumerable: true,
    });

    if (wrapEvent)
      helper.event_synthesizers[type] = wrapEvent;
    else
      helper.event_synthesizers[type] = DefaultEvent;
  };

  function dispatchEvent(event) {
    if (!event.type)
      return;
    if (!(event.type in helper.event_listeners))
      return;

    var listeners = helper.event_listeners[event.type];
    for (var i in listeners)
      listeners[i](event);
  };

  function addEventListener(type, listener) {
    if (!(listener instanceof Function)) {
      console.warn("Illegal argument, expect a function.");
      return;
    }

    if (!(type instanceof String || typeof(type) === "string")
        || !(("on" + type) in base)) {
      console.warn("Invalid or unsupported event type:" + type);
      return;
    }

    if (type in helper.event_listeners) {
      var listeners = helper.event_listeners[type];
      if (listeners.indexOf(listener) == -1)
        listeners.push(listener);
    } else {
      // Add this object as a event observer.
      helper.rootStub.addEventObserver(type, base);
      helper.event_listeners[type] = [listener];
    }
  };

  function removeEventListener(type, listener) {
    if (!(listener instanceof Function)) {
      console.warn("Illegal argument, expect a function.");
      return;
    }

    if (!(type in helper.event_listeners)) {
      console.warn("Invalid or unsupported event type:" + type);
      return;
    }

    var listeners = helper.event_listeners[type];
    var index = listeners.indexOf(listener);
    if (index > -1)
      listeners.splice(index, 1);

    // Remove this object as a event observer.
    if (listeners.length == 0)
      helper.rootStub.removeEventObserver(type, base);
  };

  Object.defineProperties(base, {
    "addEventListener" : {
      value : addEventListener,
      enumerable : true,
    },
    "removeEventListener" : {
      value : removeEventListener,
      enumerable : true,
    },
    "dispatchEvent" : {
      value : dispatchEvent,
      enumerable : true,
    },
  });
};
exports.jsStub = jsStub;
exports.init = function(extension, v8toolsObject) {
  channel = extension;
  v8tools = v8toolsObject;
};
