// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

using System;

namespace xwalk
{
public class XWalkExtension
{
    public XWalkExtension() {
    }
    public String ExtensionName() {
        return "foo";
    }
    public int ExtensionAPI() {
        return 3;
    }
}
}
