// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.app.runtime.extension;

import android.util.Log;

import java.util.HashMap;
import java.util.Map;

public class XWalkExtensionBindingObjectStore {
    // Store of all binding objects
    private Map<Integer, XWalkExtensionBindingObject> bindingObjects;
    int instanceId;
    XWalkExtensionClient extension;

    public XWalkExtensionBindingObjectStore(XWalkExtensionClient ext, int extInstanceId) {
        bindingObjects = new HashMap<Integer, XWalkExtensionBindingObject>();
        extension = ext;
        instanceId = extInstanceId;
    }

    public boolean addBindingObject(int objectId, XWalkExtensionBindingObject obj) {
        if (bindingObjects.containsKey(objectId)) {
            Log.w("Extension-" + extension.getExtensionName(),"Existing binding object:\n" + objectId);
            return false;
        }

        obj.init(instanceId, objectId);
        bindingObjects.put(objectId, obj);
        obj.onJsBinded();
        return true;
    }

    public XWalkExtensionBindingObject getBindingObject(int objectId) {
       return bindingObjects.get(objectId);
    }

    public XWalkExtensionBindingObject removeBindingObject(int objectId) {
       XWalkExtensionBindingObject obj = bindingObjects.remove(objectId);
       if (obj != null) obj.onJsDestroyed();

       return obj;
    }
}
