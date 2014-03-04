// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function focus(element) {
  // Focus the target element in order to send keys to it.
  // First, the currently active element is blurred, if it is different from
  // the target element. We do not want to blur an element unnecessarily,
  // because this may cause us to lose the current cursor position in the
  // element.
  // Secondly, we focus the target element.
  // Thirdly, if the target element is newly focused and is a text input, we
  // set the cursor position at the end.
  // Fourthly, we check if the new active element is the target element. If not,
  // we throw an error.
  // Additional notes:
  //   - |document.activeElement| is the currently focused element, or body if
  //     no element is focused
  //   - Even if |document.hasFocus()| returns true and the active element is
  //     the body, sometimes we still need to focus the body element for send
  //     keys to work. Not sure why
  //   - You cannot focus a descendant of a content editable node
  //   - V8 throws a TypeError when calling setSelectionRange for a non-text
  //     input, which still have setSelectionRange defined. For chrome 29+, V8
  //     throws a DOMException with code InvalidStateError.
  var doc = element.ownerDocument || element;
  var prevActiveElement = doc.activeElement;
  if (element != prevActiveElement && prevActiveElement)
    prevActiveElement.blur();
  element.focus();
  if (element != prevActiveElement && element.value &&
      element.value.length && element.setSelectionRange) {
    try {
      element.setSelectionRange(element.value.length, element.value.length);
    } catch (error) {
      if (!(error instanceof TypeError) && !(error instanceof DOMException &&
          error.code == DOMException.INVALID_STATE_ERR))
        throw error;
    }
  }
  if (element != doc.activeElement)
    throw new Error('cannot focus element');
}
