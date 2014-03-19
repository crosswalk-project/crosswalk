// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.xwview.shell;

import java.util.ArrayList;

import org.xwalk.core.XWalkView;

import android.app.ActionBar;
import android.content.Context;
import android.content.Intent;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;

public class SectionsPagerAdapter extends FragmentPagerAdapter {
    private ArrayList<Fragment> mFragmentList;
    ActionBar mActionBar;
    Context mContext;

    public SectionsPagerAdapter(Context context, FragmentManager fm, ActionBar actionBar) {
        super(fm);
        mFragmentList = new ArrayList<Fragment>();
        XWalkViewSectionFragment fragment1 = new XWalkViewSectionFragment();
        XWalkViewSectionFragment fragment2 = new XWalkViewSectionFragment();
        mFragmentList.add(fragment1);
        mFragmentList.add(fragment2);

        mActionBar = actionBar;
        mContext = context;
    }

    @Override
    public Fragment getItem(int position) {
        if (position < mFragmentList.size()){
            return mFragmentList.get(position);
        }
        return null;
    }

    @Override
    public int getCount() {
        return mFragmentList.size();
    }

    @Override
    public CharSequence getPageTitle(int position) {
        String title;
        if(position < mFragmentList.size()) {
            XWalkViewSectionFragment fragment = (XWalkViewSectionFragment)mFragmentList.get(position);
            XWalkView xwalkView = fragment.getXWalkView();
            if (xwalkView != null) {
                title = xwalkView.getTitle();
                if (!title.isEmpty())
                    return title;
            }
        }
        return mContext.getString(R.string.new_tab);
    }

    public void setPageTitle(XWalkView view, String title) {
        for (int i=0; i<mFragmentList.size(); i++) {
            XWalkViewSectionFragment fragment = (XWalkViewSectionFragment)mFragmentList.get(i);
            XWalkView xwalkView = fragment.getXWalkView();
            if (xwalkView == view) {
                mActionBar.getTabAt(i).setText(title);
            }
        }
    }

    public void onPause() {
        for (int i=0; i<mFragmentList.size(); i++) {
            XWalkViewSectionFragment fragment = (XWalkViewSectionFragment)mFragmentList.get(i);
            XWalkView xwalkView = fragment.getXWalkView();
            if (xwalkView != null) xwalkView.onPause();
        }
    }

    public void onResume() {
        for (int i=0; i<mFragmentList.size(); i++) {
            XWalkViewSectionFragment fragment = (XWalkViewSectionFragment)mFragmentList.get(i);
            XWalkView xwalkView = fragment.getXWalkView();
            if (xwalkView != null) xwalkView.onResume();
        }
    }

    public void onDestroy() {
        for (int i=0; i<mFragmentList.size(); i++) {
            XWalkViewSectionFragment fragment = (XWalkViewSectionFragment)mFragmentList.get(i);
            XWalkView xwalkView = fragment.getXWalkView();
            if (xwalkView != null) xwalkView.onDestroy();
        }
    }

    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        for (int i=0; i<mFragmentList.size(); i++) {
            XWalkViewSectionFragment fragment = (XWalkViewSectionFragment)mFragmentList.get(i);
            XWalkView xwalkView = fragment.getXWalkView();
            if (xwalkView != null) {
                xwalkView.onActivityResult(requestCode, resultCode, data);
            }
        }
    }
}
