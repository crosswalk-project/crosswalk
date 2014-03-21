// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample;

import java.util.ArrayList;
import java.util.List;

import android.app.ListActivity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;

public class XWalkEmbeddingAPISample extends ListActivity {

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setListAdapter(new SampleAdapter());
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    @Override
    protected void onListItemClick(ListView lv, View v, int pos, long id) {
        SampleInfo info = (SampleInfo) getListAdapter().getItem(pos);
        startActivity(info.intent);
    }

    static class SampleInfo {
        String name;
        Intent intent;

        SampleInfo(String name, Intent intent) {
            this.name = name;
            this.intent = intent;
        }
    }

    class SampleAdapter extends BaseAdapter {
        private ArrayList<SampleInfo> mItems;

        public SampleAdapter() {
            Intent intent = new Intent(Intent.ACTION_MAIN, null);
            intent.setPackage(getPackageName());
            intent.addCategory(Intent.CATEGORY_SAMPLE_CODE);

            PackageManager pm = getPackageManager();
            List<ResolveInfo> infos = pm.queryIntentActivities(intent, 0);

            mItems = new ArrayList<SampleInfo>();

            final int count = infos.size();
            for (int i = 0; i < count; i++) {
                final ResolveInfo info = infos.get(i);
                final CharSequence labelSeq = info.loadLabel(pm);
                String label = labelSeq != null ? labelSeq.toString() : info.activityInfo.name;

                Intent target = new Intent();
                target.setClassName(info.activityInfo.applicationInfo.packageName,
                        info.activityInfo.name);
                SampleInfo sample = new SampleInfo(label, target);
                mItems.add(sample);
            }
        }

        @Override
        public int getCount() {
            return mItems.size();
        }

        @Override
        public Object getItem(int position) {
            return mItems.get(position);
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            if (convertView == null) {
                convertView = getLayoutInflater().inflate(android.R.layout.simple_list_item_1,
                        parent, false);
            }
            TextView tv = (TextView) convertView.findViewById(android.R.id.text1);
            SampleInfo info = mItems.get(position);
            tv.setText(info.name);
            return convertView;
        }
    }
}
