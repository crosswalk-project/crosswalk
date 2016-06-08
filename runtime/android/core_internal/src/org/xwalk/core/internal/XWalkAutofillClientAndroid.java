// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

import android.view.ViewGroup;

import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JNINamespace;
import org.chromium.content.browser.ContentViewCore;
import org.chromium.ui.autofill.AutofillDelegate;
import org.chromium.ui.autofill.AutofillPopup;
import org.chromium.ui.autofill.AutofillSuggestion;
import org.chromium.ui.DropdownItem;

/**
 * Java counterpart to the XWalkAutofillClient. This class is owned by XWalkContent and has
 * a weak reference from native side.
 */
@JNINamespace("xwalk")
public class XWalkAutofillClientAndroid {

    private final long mNativeXWalkAutofillClientAndroid;
    private AutofillPopup mAutofillPopup;
    private ContentViewCore mContentViewCore;

    @CalledByNative
    public static XWalkAutofillClientAndroid create(long nativeClient) {
        return new XWalkAutofillClientAndroid(nativeClient);
    }

    private XWalkAutofillClientAndroid(long nativeXWalkAutofillClient) {
        mNativeXWalkAutofillClientAndroid = nativeXWalkAutofillClient;
    }

    public void init(ContentViewCore contentViewCore) {
        mContentViewCore = contentViewCore;
    }

    @CalledByNative
    private void showAutofillPopup(float x, float y, float width, float height,
            boolean isRtl, AutofillSuggestion[] suggestions) {

        if (mContentViewCore == null) return;

        if (mAutofillPopup == null) {
            mAutofillPopup = new AutofillPopup(
                mContentViewCore.getContext(),
                mContentViewCore.getViewAndroidDelegate(),
                new AutofillDelegate() {
                    @Override
                    public void dismissed() { }
                    @Override
                    public void suggestionSelected(int listIndex) {
                        nativeSuggestionSelected(mNativeXWalkAutofillClientAndroid, listIndex);
                    }
                    @Override
                    public void deleteSuggestion(int listIndex) { }
                });
        }
        mAutofillPopup.setAnchorRect(x, y, width, height);
        mAutofillPopup.filterAndShow(suggestions, isRtl);
    }

    @CalledByNative
    public void hideAutofillPopup() {
        if (mAutofillPopup == null) return;
        mAutofillPopup.dismiss();
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
        array[index] = new AutofillSuggestion(name, label, DropdownItem.NO_ICON, uniqueId, false, false);
    }

    private native void nativeSuggestionSelected(long nativeXWalkAutofillClientAndroid,
            int position);
}
