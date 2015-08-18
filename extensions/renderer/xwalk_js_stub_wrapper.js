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
 *           helper.invokeNative("getPrefixPromise", [{"resolve": resolve, "reject":reject}]);
 *       })
 *     };
 */
var v8tools;
var channel;
var jsStub = function(base, rootStub) {
  var nextCallbackId = 1;

  // Refer to the exposed extension object.
  this.base = base;

  // Point to the extension stub helper.
  this.rootStub = rootStub || this;

  // Retain the properties which is exposed by native.
  this.properties = {};
  this.callbacks = [];

  this.getCallbackId = function() {
    while (this.callbacks[nextCallbackId] != undefined)
      ++nextCallbackId;
    return nextCallbackId;
  };
};

function messageHandler(targetHelper, msg) {
  if (!msg.cmd)
    console.warn("No valid Java CMD.");
  switch (msg.cmd) {
    case "invokeCallback":
      if (!msg.callInfo || (typeof msg.callInfo !== "object")) return;
      targetHelper.invokeCallback(msg.callInfo, msg.key, msg.args);
      break;
    case "updateProperty":
      // Case: property is changed by native and need
      // to sync its value to JS side.
      if (targetHelper.properties.hasOwnProperty(msg.name) >= 0) {
        targetHelper.properties[msg.name] = targetHelper.getNativeProperty(msg.name);
      }
      break;
    case "dispatchEvent":
      if (msg.type)
        targetHelper.base.dispatchEvent(msg.type, msg.event);
      break;

    default:
      console.warn("Unsupported Java CMD:" + msg.cmd);
  }
}

function makeExtensionHelper(helper) {
  var nextObjectId = 1;
  var bindingObjects = [];
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
  };

  helper.getBindingObject = function(objectId) {
    return  bindingObjects[objectId];
  };
  helper.getConstructorHelper = function(cName) {
    return jsStub.getHelper(this.constructors[cName]);
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
    var msgObj = JSON.parse(msg);
    if (msgObj.cmd == "error") {
      // supported key: log, info, warn, error
      if (console[msgObj.level] instanceof Function) {
        console[msgObj.level](msgObj.msg);
      } else {
        console.error(msgObj.msg);
      }
      return;
    } 
    if (msgObj.cmd == "onEvent") {
      var observers = this.eventObservers[msgObj.type];
      if (!Array.isArray(observers) ) return;

      for (var o of observers) {
        o.dispatchEvent(msgObj.type, msgObj.event);
      }
      return;
    }
    // Message back to extension itself.
    var targetHelper = this;

    var cName = String(msgObj["constructorName"]).trim();
    // Message back to constructors.
    if (msgObj.objectId == 0 && cName.length > 0) {
      targetHelper = this.getConstructorHelper(cName);
    } else if (msgObj.objectId > 0) {
      // Message back to binding objects.
      var obj = bindingObjects[msgObj.objectId];
      if (!obj) return;
      targetHelper = jsStub.getHelper(obj);
    }
    if (!targetHelper) return;

    messageHandler(targetHelper, msgObj);
  }
}

jsStub.create = function(base) {
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

function isSerializable(obj) {
  if (!(obj instanceof Object))
    return true;
  if (obj instanceof Function)
    return false;
  if (obj instanceof Boolean ||
      obj instanceof Date ||
      obj instanceof Number ||
      obj instanceof RegExp ||
      obj instanceof String)
    return true;
  for (var p of Object.getOwnPropertyNames(obj))
    if (!isSerializable(obj[p]))
      return false;
  return true;
}

jsStub.prototype = {
  "invokeNative": function(name, args, sync) {
    name = String(name);
    var isNewInstance = (name[0] == '+') ? true : false;

    if (!Array.isArray(args)) {
      console.warn("invokeNative: args is not an array.");
      args = Array(args);
    }
    // Retain callbacks in JS stub, replace them to related number ID
    var call = [];
    var cid = this.getCallbackId();
    args.forEach(function(val, vid, a) {
      if (!isSerializable(val)) {
        call[vid] = val;
        a[vid] = { "cid": cid, "vid": vid };
      }
    })

    if (call.length > 0) {
      this.callbacks[cid] = call;
    }

    var msg = {
      cmd: isNewInstance ? "newInstance" : "invokeNative",
      name: isNewInstance ? name.substr(1) : name,
      args: args
    };

    if (isNewInstance) {
      msg.bindingObjectId = this.getObjectId();
      // Use sync channel for newInstance actions.
      // "newInstance" action should return true/false.
      return (this.sendSyncMessage(msg)? msg.bindingObjectId : 0);
    }

    if (sync)
      return this.sendSyncMessage(msg);
    else
      this.postMessage(msg);
  },
  // Only Binding object need this lifecycle tracker.
  "registerLifecycleTracker": function() {
    var baseObject = this.base;
    var helper = this;
    Object.defineProperty(baseObject, "_tracker", {
      value: v8tools.lifecycleTracker(),
    });

    var objId = helper.objectId;
    var msg = {
      cmd: "jsObjectCollected",
      jsObjectId: objId
    };
    baseObject._tracker.destructor = function() {
      channel.postMessage(msg);
    };
  },
  "destroy": function() {
    var objId = this.objectId;
    var msg = {
      cmd: "jsObjectCollected",
      jsObjectId: objId
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
    delete this.invokeCallback;;
    delete this.destroy;
  },
  "getNativeProperty": function(name) {
    return this.sendSyncMessage({
      cmd: "getProperty",
      name: name
    });
  },
  "setNativeProperty": function(name, value) {
    this.sendSyncMessage({
      cmd: "setProperty",
      name: name,
      value: value
    });
  },
  "invokeCallback": function(callInfo, key, args) {
      var cid = callInfo.cid;
      if (!cid) return;

      var vid = callInfo.vid;
      var obj = this.callbacks[cid][vid];
      if (!obj || !((typeof obj == "object") || (obj instanceof Function))) return;

      if (typeof(key) === 'number' || key instanceof Number)
          obj = obj[key];
      else if (typeof(key) === 'string' || key instanceof String)
          key.split('.').forEach(function(p) { obj = obj[p]; });

      if (obj instanceof Function)
          obj.apply(null, JSON.parse(args));
  },
  "sendSyncMessage": function(msg) {
    // Add objectId for each message.
    msg["objectId"] = this.objectId ? this.objectId : 0;
    msg["constructorJsName"] = this.constructorJsName ? this.constructorJsName : "";

    var resultStr = channel.internal.sendSyncMessage(JSON.stringify(msg));
    return resultStr.length > 0 ? JSON.parse(resultStr) : undefined;
  },
  "postMessage": function(msg) {
    // Add objectId for each message.
    msg["objectId"] = this.objectId;
    msg["constructorJsName"] = this.constructorJsName ? this.constructorJsName : "";

    channel.postMessage(JSON.stringify(msg));
  }
};

// expose native property to JavaScript
jsStub.defineProperty = function(obj, prop, writable) {
  var helper = jsStub.getHelper(obj);

  // keep all exported properties in the helper
  helper.properties[prop] = helper.getNativeProperty(prop);

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
      helper.setNativeProperty(prop, v);
      helper.properties[prop] = v;
    }
  }
  Object.defineProperty(obj, prop, desc);
};

// EventTarget implementation:
// When dispatch event in the Java side, we will broadcast the
// message to all instances, so all frames will handle the event.
jsStub.makeEventTarget = function(base) {
  var helper = jsStub.getHelper(base);
  helper.event_listeners = {};

  helper.addEvent = function(type) {
    Object.defineProperty(helper, "_on" + type, {
      writable : true,
    });

    Object.defineProperty(base, "on" + type, {
      get: function() {
        return helper["_on" + type];
      },
      set: function(listener) {
        var old_listener = helper["_on" + type];
        if (old_listener === listener)
          return;

        if (old_listener)
          base.removeEventListener(type, old_listener);

        helper["_on" + type] = listener;
        base.addEventListener(type, listener);
      },
      enumerable: true,
    });
  };

  function dispatchEvent(type, event) {
    if (!(type instanceof String || typeof(type) === "string") 
        || !(("on" + type) in base)) {
      console.warn("Invalid or unsupported event type:" + type);
      return;
    }
    if (!(type in helper.event_listeners))
      return;

    var listeners = helper.event_listeners[type];
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
