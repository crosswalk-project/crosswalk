// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package com.example.jsStub;

import org.xwalk.app.runtime.extension.*;

class Person extends XWalkExtensionBindingObject {
    private XWalkExtensionClient extensionClient;
    @JsApi
    public static String constructorName = "Person";

    private static int nextPersonId = 1;
    @JsApi
    public String name;
    int personId;
    int age;

    public void onJsDestroyed() {
        ((PersonExtension)extensionClient).onRemoveBindingObject(this);
    }
    
    public void onJsBinded() {
        ((PersonExtension)extensionClient).onAddBindingObject(this);
    }

    @JsApi(isEventList = true)
    public static String[] eventList = {"event1", "event2"};

    @JsApi
    public static int getNextPersonId() {
        return nextPersonId;
    }
    public Person(String vName, int vAge, XWalkExtensionClient extClient) {
        super(extClient);
        extensionClient = extClient;
        personId = nextPersonId++;
        name = vName;
        age = vAge;
    }

    @JsApi
    public int getAge() {
        return age;
    }

    public int getPersonId() {
        return personId;
    }

    @JsApi
    public boolean isOlder(int age) {
        return (this.age > age);
    }

    @JsApi
    public void setAge(int vAge) {
        age = vAge; 
    }

    @JsApi
    public void triggerEvents() {
        dispatchEvent("event1", "data for event1");
        dispatchEvent("event2", "data for event2");
    }
}
