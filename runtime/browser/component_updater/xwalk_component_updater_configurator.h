// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_COMPONENT_UPDATER_XWALK_COMPONENT_UPDATER_CONFIGURATOR_H_
#define XWALK_RUNTIME_BROWSER_COMPONENT_UPDATER_XWALK_COMPONENT_UPDATER_CONFIGURATOR_H_

#include "base/memory/ref_counted.h"
#include "components/update_client/configurator.h"

namespace base {
class CommandLine;
}

namespace net {
class URLRequestContextGetter;
}

namespace component_updater {

scoped_refptr<update_client::Configurator>
MakeXwalkComponentUpdaterConfigurator(
    const base::CommandLine* cmdline,
    net::URLRequestContextGetter* context_getter);

}  // namespace component_updater

#endif  // XWALK_RUNTIME_BROWSER_COMPONENT_UPDATER_XWALK_COMPONENT_UPDATER_CONFIGURATOR_H_
