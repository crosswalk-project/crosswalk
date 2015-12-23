// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/component_updater/xwalk_component_updater_configurator.h"

#include <string>
#include <vector>

#include "base/threading/sequenced_worker_pool.h"
#include "base/version.h"
#include "components/component_updater/configurator_impl.h"
#include "components/update_client/component_patcher_operation.h"
#include "content/public/browser/browser_thread.h"

namespace component_updater {

namespace {

class XwalkConfigurator : public update_client::Configurator {
 public:
  XwalkConfigurator(const base::CommandLine* cmdline,
                    net::URLRequestContextGetter* url_request_getter);

  // update_client::Configurator overrides.
  int InitialDelay() const override;
  int NextCheckDelay() const override;
  int StepDelay() const override;
  int OnDemandDelay() const override;
  int UpdateDelay() const override;
  std::vector<GURL> UpdateUrl() const override;
  std::vector<GURL> PingUrl() const override;
  base::Version GetBrowserVersion() const override;
  std::string GetChannel() const override;
  std::string GetLang() const override;
  std::string GetOSLongName() const override;
  std::string ExtraRequestParams() const override;
  net::URLRequestContextGetter* RequestContext() const override;
  scoped_refptr<update_client::OutOfProcessPatcher> CreateOutOfProcessPatcher()
      const override;
  bool DeltasEnabled() const override;
  bool UseBackgroundDownloader() const override;
  scoped_refptr<base::SequencedTaskRunner> GetSequencedTaskRunner()
      const override;

 private:
  friend class base::RefCountedThreadSafe<XwalkConfigurator>;

  ConfiguratorImpl configurator_impl_;

  ~XwalkConfigurator() override {}
};

XwalkConfigurator::XwalkConfigurator(
    const base::CommandLine* cmdline,
    net::URLRequestContextGetter* url_request_getter)
    : configurator_impl_(cmdline, url_request_getter) {}

int XwalkConfigurator::InitialDelay() const {
  return configurator_impl_.InitialDelay();
}

int XwalkConfigurator::NextCheckDelay() const {
  return configurator_impl_.NextCheckDelay();
}

int XwalkConfigurator::StepDelay() const {
  return configurator_impl_.StepDelay();
}

int XwalkConfigurator::OnDemandDelay() const {
  return configurator_impl_.OnDemandDelay();
}

int XwalkConfigurator::UpdateDelay() const {
  return configurator_impl_.UpdateDelay();
}

std::vector<GURL> XwalkConfigurator::UpdateUrl() const {
  return configurator_impl_.UpdateUrl();
}

std::vector<GURL> XwalkConfigurator::PingUrl() const {
  return configurator_impl_.PingUrl();
}

base::Version XwalkConfigurator::GetBrowserVersion() const {
  return configurator_impl_.GetBrowserVersion();
}

std::string XwalkConfigurator::GetChannel() const {
  NOTIMPLEMENTED();
  return "";
}

std::string XwalkConfigurator::GetLang() const {
  NOTIMPLEMENTED();
  return "";
}

std::string XwalkConfigurator::GetOSLongName() const {
  return configurator_impl_.GetOSLongName();
}

std::string XwalkConfigurator::ExtraRequestParams() const {
  return configurator_impl_.ExtraRequestParams();
}

net::URLRequestContextGetter* XwalkConfigurator::RequestContext() const {
  return configurator_impl_.RequestContext();
}

scoped_refptr<update_client::OutOfProcessPatcher>
XwalkConfigurator::CreateOutOfProcessPatcher() const {
  NOTIMPLEMENTED();
  return nullptr;
}

bool XwalkConfigurator::DeltasEnabled() const {
  return configurator_impl_.DeltasEnabled();
}

bool XwalkConfigurator::UseBackgroundDownloader() const {
  return configurator_impl_.UseBackgroundDownloader();
}

scoped_refptr<base::SequencedTaskRunner>
XwalkConfigurator::GetSequencedTaskRunner() const {
  return content::BrowserThread::GetBlockingPool()
      ->GetSequencedTaskRunnerWithShutdownBehavior(
          content::BrowserThread::GetBlockingPool()->GetSequenceToken(),
          base::SequencedWorkerPool::SKIP_ON_SHUTDOWN);
}

}  // namespace

scoped_refptr<update_client::Configurator>
MakeXwalkComponentUpdaterConfigurator(
    const base::CommandLine* cmdline,
    net::URLRequestContextGetter* context_getter) {
  return new XwalkConfigurator(cmdline, context_getter);
}

}  // namespace component_updater
