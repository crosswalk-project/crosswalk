// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import org.xwalk.core.internal.XWalkJavascriptResultInternal;

final class XWalkJavascriptResultHandler implements XWalkJavascriptResult{

    private XWalkJavascriptResultInternal internal;

    XWalkJavascriptResultHandler(XWalkJavascriptResultInternal internal) {
        this.internal = internal;
    }

    XWalkJavascriptResultInternal getInternal() {
        return this.internal;
    }

    public void confirm() {
        internal.confirm();
    }

    public void confirmWithResult(final String promptResult) {
        internal.confirmWithResult(promptResult);
    }

    public void cancel() {
        internal.cancel();
    }
}
