// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved. 
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.content.Context;
import android.view.View;
import android.view.ViewGroup;

import org.chromium.base.CalledByNative;
import org.chromium.base.JNINamespace;
import org.chromium.content.browser.ContentViewCore;
import org.chromium.ui.autofill.AutofillPopup;
import org.chromium.ui.autofill.AutofillSuggestion;

// Java counterpart to the XWalkAutofillManagerDelegate. This class is owned by
// XWalkContents and has a weak reference from native side.
@JNINamespace("xwalk")
public class XWalkAutofillManagerDelegate {

    private final int mNativeXWalkAutofillManagerDelegate;
    private AutofillPopup mAutofillPopup;
    private ViewGroup mContainerView;
    private ContentViewCore mContentViewCore;

    @CalledByNative
    public static XWalkAutofillManagerDelegate create(int nativeDelegate) {
        return new XWalkAutofillManagerDelegate(nativeDelegate);
    }

    private XWalkAutofillManagerDelegate(int nativeXWalkAutofillManagerDelegate) {
        mNativeXWalkAutofillManagerDelegate = nativeXWalkAutofillManagerDelegate;
    }

    public void init(ContentViewCore contentViewCore) {
        mContentViewCore = contentViewCore;
        mContainerView = contentViewCore.getContainerView();
    }

    @CalledByNative
    private void showAutofillPopup(float x, float y, float width, float height,
            AutofillSuggestion[] suggestions) {
        if (mContentViewCore == null) return;

        if (mAutofillPopup == null) {
            mAutofillPopup = new AutofillPopup(
                mContentViewCore.getContext(),
                mContentViewCore.getViewAndroidDelegate(),
                new AutofillPopup.AutofillPopupDelegate() {
                    @Override
                    public void requestHide() { }

                    @Override
                    public void suggestionSelected(int listIndex) {
                        nativeSuggestionSelected(mNativeXWalkAutofillManagerDelegate, listIndex);
                    }
                });
        }
        mAutofillPopup.setAnchorRect(x, y, width, height);
        mAutofillPopup.show(suggestions);
    }

    @CalledByNative
    public void hideAutofillPopup() {
        if (mAutofillPopup == null) return;
        mAutofillPopup.hide();
        mAutofillPopup = null;
    }

    @CalledByNative
    private static AutofillSuggestion[] createAutofillSuggestionArray(int size) {
        return new AutofillSuggestion[size];
    }

    /**
     * @param array AutofillSuggestion array that should get a new suggestion added.
     * @param index Index in the array where to place a new suggestion.
     * @param name Name of the suggestion.
     * @param label Label of the suggestion.
     * @param uniqueId Unique suggestion id.
     */
    @CalledByNative
    private static void addToAutofillSuggestionArray(AutofillSuggestion[] array, int index,
            String name, String label, int uniqueId) {
        array[index] = new AutofillSuggestion(name, label, uniqueId);
    }

    private native void nativeSuggestionSelected(int nativeXWalkAutofillManagerDelegate, int position);
}
