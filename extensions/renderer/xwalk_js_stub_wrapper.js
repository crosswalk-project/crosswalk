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
var jsStub = function(base, channel) {
  var nextCallbackId = 1;

  // Refer to the exposed extension object.
  this.base = base;

  // Refer to the global extension variable, used to send message.
  this.channel = channel;
  // Retain the properties which is exposed by native.
  this.properties = {};
  this.callbacks = [];

  this.getCallbackId = function() {
    while (this.callbacks[nextCallbackId] != undefined)
      ++nextCallbackId;
    return nextCallbackId;
  };
};

jsStub.create = function(base, channel) {
  var helper = jsStub.getHelper(base, channel);
  channel.setMessageListener(function (msg) {
    helper.handleMessage(msg);
  });
  return helper;
};

jsStub.getHelper = function(base, channel) {
  if (!(base.__stubHelper instanceof jsStub)) {
    Object.defineProperty(base, "__stubHelper", {
      value:new jsStub(base, channel) 
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
      cmd: "invokeNative",
      name: name,
      args: args
    };
    if (sync)
      return this.sendSyncMessage(msg);
    else
      this.postMessage(msg);
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
  "handleMessage": function(json) {
    var msg = JSON.parse(json);
    if (!msg.cmd)
      console.warn("No valid Java CMD.");
    switch (msg.cmd) {
      case "invokeCallback":
        if (!msg.callInfo || (typeof msg.callInfo !== "object")) return;
        this.invokeCallback(msg.callInfo, msg.key, msg.args);
        break;
      case "updateProperty":
        // Case: property is changed by native and need
        // to sync its value to JS side.
        if (this.properties.hasOwnProperty(msg.name) >= 0) {
          this.properties[msg.name] = this.getNativeProperty(msg.name);
        }
        break;
      case "error":
        // supported key: log, info, warn, error
        console[msg.level](msg.msg);
        break;
      case "dispatchEvent":
        if (msg.type) {
          this.base.dispatchEvent(msg.type, msg.event);
        }
        break;
      default:
        console.warn("Unsupported Java CMD:" + msg.cmd);
    }
  },
  "sendSyncMessage": function(msg) {
    var resultStr = this.channel.internal.sendSyncMessage(JSON.stringify(msg));
    return resultStr.length > 0 ? JSON.parse(resultStr) : undefined;
  },
  "postMessage": function(msg) {
    this.channel.postMessage(JSON.stringify(msg));
  }
};

// expose native property to JavaScript
jsStub.defineProperty = function(obj, prop, writable) {
  var helper = jsStub.getHelper(obj);

  // keep all exported properties in the helper
  helper.properties[prop] = helper.getNativeProperty(prop);

  var desc = {
    'configurable': false,
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
