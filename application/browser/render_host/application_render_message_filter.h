// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_RENDER_HOST_APPLICATION_RENDER_MESSAGE_FILTER_H_
#define XWALK_APPLICATION_BROWSER_RENDER_HOST_APPLICATION_RENDER_MESSAGE_FILTER_H_

#include "base/memory/weak_ptr.h"
#include "content/public/browser/browser_message_filter.h"
#include "xwalk/application/browser/application_system.h"

namespace xwalk {
namespace application {

class ApplicationRenderMessageFilter : public content::BrowserMessageFilter {
 public:
  ApplicationRenderMessageFilter(int render_process_id,
                                 ApplicationSystem* application_system);

  virtual void OverrideThreadForMessage(
     const IPC::Message& message,
     content::BrowserThread::ID* thread) OVERRIDE;
  virtual bool OnMessageReceived(const IPC::Message& message,
                                 bool* message_was_ok) OVERRIDE;
  int render_process_id() { return render_process_id_; }

 private:
  friend class content::BrowserThread;
  friend class base::DeleteHelper<ApplicationRenderMessageFilter>;

  virtual ~ApplicationRenderMessageFilter();
  void OnApplicationShouldSuspendAck(int sequence_id);
  void OnApplicationSuspendAck();

  int render_process_id_;
  ApplicationSystem* application_system_;
  base::WeakPtrFactory<ApplicationRenderMessageFilter> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(ApplicationRenderMessageFilter);
};

}  // namespace application
}  // namespace xwalk
#endif  // XWALK_APPLICATION_BROWSER_RENDER_HOST_APPLICATION_RENDER_MESSAGE_FILTER_H_

