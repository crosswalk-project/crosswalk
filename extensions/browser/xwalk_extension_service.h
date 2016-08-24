// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_SERVICE_H_
#define XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_SERVICE_H_

#include <stdint.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/callback_forward.h"
#include "base/containers/scoped_ptr_hash_map.h"
#include "base/files/file_path.h"
#include "base/threading/thread.h"
#include "base/values.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "ipc/message_filter.h"
#include "xwalk/extensions/browser/xwalk_extension_process_host.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"
#include "xwalk/extensions/common/xwalk_extension_vector.h"

namespace content {
class RenderProcessHost;
class WebContents;
}

namespace xwalk {
namespace extensions {

class XWalkExtension;
class XWalkExtensionData;
class XWalkExtensionServer;

// This is the entry point for Crosswalk extensions. Its responsible for keeping
// track of the extensions, and enable them on WebContents once they are
// created. It's life time follows the Browser process itself.
class XWalkExtensionService : public content::NotificationObserver,
    public XWalkExtensionProcessHost::Delegate {
 public:
  class Delegate {
   public:
    virtual void CheckAPIAccessControl(
        int render_process_id,
        const std::string& extension_name,
        const std::string& api_name,
        const PermissionCallback& callback) {}
    virtual bool RegisterPermissions(
        int render_process_id,
        const std::string& extension_name,
        const std::string& perm_table);
    virtual void ExtensionProcessCreated(
        int render_process_id,
        const IPC::ChannelHandle& channel_handle) {}

   protected:
    virtual ~Delegate() {}
  };

  explicit XWalkExtensionService(Delegate* delegate);
  ~XWalkExtensionService() override;

  void RegisterExternalExtensionsForPath(const base::FilePath& path);

  // To be called when a new RenderProcessHost is created, will plug the
  // extension system to that render process. See
  // XWalkContentBrowserClient::RenderProcessWillLaunch().
  //
  // The vectors contain the extensions to be used for this render process,
  // ownership of these extensions is taken by the XWalkExtensionService. The
  // vectors will be empty after the call.
  void OnRenderProcessWillLaunch(
      content::RenderProcessHost* host,
      XWalkExtensionVector* ui_thread_extensions,
      XWalkExtensionVector* extension_thread_extensions,
      std::unique_ptr<base::DictionaryValue::Storage> runtime_variables);

  // To be called when a RenderProcess died, so we can gracefully shutdown the
  // associated ExtensionProcess. See Runtime::RenderProcessGone() and
  // XWalkContentBrowserClient::RenderProcessHostGone().
  void OnRenderProcessDied(content::RenderProcessHost* host);

  typedef base::Callback<void(XWalkExtensionVector* extensions)>
      CreateExtensionsCallback;

  static void SetCreateExtensionThreadExtensionsCallbackForTesting(
      const CreateExtensionsCallback& callback);
  static void SetCreateUIThreadExtensionsCallbackForTesting(
      const CreateExtensionsCallback& callback);

  static void SetExternalExtensionsPathForTesting(const base::FilePath& path);

 private:
  void OnRenderProcessHostCreatedInternal(
      content::RenderProcessHost* host,
      XWalkExtensionVector* ui_thread_extensions,
      XWalkExtensionVector* extension_thread_extensions,
      std::unique_ptr<base::DictionaryValue::Storage> runtime_variables);

  // XWalkExtensionProcessHost::Delegate implementation.
  void OnExtensionProcessDied(XWalkExtensionProcessHost* eph,
      int render_process_id) override;

  void OnExtensionProcessCreated(
      int render_process_id,
      const IPC::ChannelHandle handle) override;

  void OnCheckAPIAccessControl(
      int render_process_id,
      const std::string& extension_name,
      const std::string& api_name,
      const PermissionCallback& callback) override;
  bool OnRegisterPermissions(int render_process_id,
                             const std::string& extension_name,
                             const std::string& perm_table) override;

  // NotificationObserver implementation.
  void Observe(int type, const content::NotificationSource& source,
               const content::NotificationDetails& details) override;

  void OnRenderProcessHostClosed(content::RenderProcessHost* host);

  void CreateInProcessExtensionServers(
      content::RenderProcessHost* host,
      XWalkExtensionData* data,
      XWalkExtensionVector* ui_thread_extensions,
      XWalkExtensionVector* extension_thread_extensions);

  void CreateExtensionProcessHost(content::RenderProcessHost* host,
      XWalkExtensionData* data, std::unique_ptr<base::DictionaryValue::Storage> runtime_variables);

  // The server that handles in process extensions will live in the
  // extension_thread_.
  base::Thread extension_thread_;

  content::NotificationRegistrar registrar_;

  Delegate* delegate_;

  base::FilePath external_extensions_path_;

  typedef std::map<int, XWalkExtensionData*> RenderProcessToExtensionDataMap;
  RenderProcessToExtensionDataMap extension_data_map_;

  DISALLOW_COPY_AND_ASSIGN(XWalkExtensionService);
};

// This object intercepts messages destined to a XWalkExtensionServer and
// dispatch them to its task runner. A message loop proxy of a thread is a
// task runner. Like other filters, this filter will run in the IO-thread.
//
// In the case of in process extensions, we will pass the task runner of the
// extension thread.
class ExtensionServerMessageFilter : public IPC::MessageFilter,
  public IPC::Sender {
public:
  ExtensionServerMessageFilter(
      scoped_refptr<base::SequencedTaskRunner> task_runner,
      XWalkExtensionServer* extension_thread_server,
      XWalkExtensionServer* ui_thread_server);

  void Invalidate();

  // IPC::Sender implementation.
  bool Send(IPC::Message* msg_ptr) override;

private:
  ~ExtensionServerMessageFilter() override;
  int64_t GetInstanceIDFromMessage(const IPC::Message& message);
  void RouteMessageToServer(const IPC::Message& message);
  void OnCreateInstance(int64_t instance_id, std::string name);
  void OnGetExtensions(
      std::vector<XWalkExtensionServerMsg_ExtensionRegisterParams>* reply);

  // IPC::ChannelProxy::MessageFilter implementation.
  void OnFilterAdded(IPC::Sender* sender) override;
  void OnFilterRemoved() override;
  void OnChannelClosing() override;
  void OnChannelError() override;
  bool OnMessageReceived(const IPC::Message& message) override;

  base::Lock lock_;
  IPC::Sender* sender_;
  scoped_refptr<base::SequencedTaskRunner> task_runner_;
  XWalkExtensionServer* extension_thread_server_;
  XWalkExtensionServer* ui_thread_server_;
  std::set<int64_t> extension_thread_instances_ids_;
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_BROWSER_XWALK_EXTENSION_SERVICE_H_
