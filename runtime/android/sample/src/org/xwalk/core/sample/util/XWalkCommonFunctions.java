// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample.util;

import android.app.Activity;
import android.content.Intent;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;

public class XWalkCommonFunctions {
    public void XWalkCommonFunctions() {
    }

    Button createButton(String text, final Activity activity, final Intent intent) {
        Button btn = new Button(activity);
        btn.setText(text);
        btn.setOnClickListener(new View.OnClickListener() {
            public void onClick(View view) {
                startActivityOnClick(activity, intent);
            }
        });
        return btn;
    }

    Intent createNewIntent(Activity activity, Class<?> cls) {
        Intent intent = new Intent(activity, cls);
        intent.putExtra("name", "crosswalk");
        return intent;
    }

    void startActivityOnClick(Activity activity, Intent intent) {
        activity.startActivity(intent);
    }

    public void createComponentsAndAddView(LinearLayout ll, Activity activity, Class<?> cls, String text) {
        Intent intent = createNewIntent(activity, cls);
        Button btn = createButton(text, activity, intent);
        ll.addView(btn);
    }
}
