// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package com.example.jsStub;

import android.util.Log;
import java.util.Map;
import java.util.HashMap;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.xwalk.app.runtime.extension.*;

/*
 * An example for constructor extension
 */
public class PersonExtension extends XWalkExtensionClient {
    private Map<Integer, Person> personMap = new HashMap<Integer, Person>();
    public PersonExtension(String extensionName, String jsApi, XWalkExtensionContextClient context) {
        super(extensionName, jsApi, context);
    }

    public void onAddBindingObject(Object obj) {
        if (obj.getClass().equals(Person.class)) {
            Person newP = (Person)obj;
            personMap.put(newP.getPersonId(), newP);
        }
    }

    public void onRemoveBindingObject(Object obj) {
        if (obj.getClass().equals(Person.class)) {
            int key = ((Person)obj).getPersonId();
            if(personMap.containsKey(key))
                personMap.remove(key);
        }
    }

    @JsConstructor(mainClass = Person.class)
        public Person onPersonInstance(String name, int age) {
            return new Person(name, age, this);
        }

    @JsApi
    public boolean hasPersonByName(String name) {
        for(Person p : personMap.values()) {
            if (p.name.equals(name)) return true;
        }
        return false;
    }

    @JsApi
    public int getPersonNumber() {
        return personMap.size();
    }

}
