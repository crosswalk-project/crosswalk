// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXPERIMENTAL_PRESENTATION_PRESENTATION_EXTENSION_H_
#define XWALK_EXPERIMENTAL_PRESENTATION_PRESENTATION_EXTENSION_H_

#include <vector>

#include "xwalk/experimental/presentation/presentation_display_manager.h"
#include "xwalk/extensions/common/xwalk_extension.h"

namespace xwalk {
namespace experimental {

class PresentationInstance;

// A XWalk extension for Presentation API spec implementation. It is placed
// under experimental namespace since the spec is still in editor draft and
// might be changed in future after a wider discussion.
//
// Presentation API spec: http://otcshare.github.io/presentation-spec/index.html
class PresentationExtension : public extensions::XWalkExtension,
                           public PresentationDisplayManager::Observer {
 public:
  PresentationExtension();
  virtual ~PresentationExtension();

  // XWalkExtension implementation.
  virtual const char* GetJavaScriptAPI() OVERRIDE;
  virtual extensions::XWalkExtensionInstance* CreateInstance() OVERRIDE;

  void AddInstance(PresentationInstance* instance);
  void RemoveInstance(PresentationInstance* instance);

  bool display_available() const { return display_available_; }

 private:
  virtual void OnDisplayAvailabilityChanged(bool is_available) OVERRIDE;

  std::vector<PresentationInstance*> instance_list_;

  // All extension instances share one display manager.
  scoped_ptr<PresentationDisplayManager> display_manager_;

  // Indicates if there is an available display for showing presentation.
  bool display_available_;
};

class PresentationInstance : public extensions::XWalkExtensionInstance {
 public:
  explicit PresentationInstance(PresentationExtension* extension);
  virtual ~PresentationInstance();

  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE;
  virtual void HandleSyncMessage(scoped_ptr<base::Value> msg) OVERRIDE;

  void OnDisplayAvailabilityChanged(bool is_available);

 private:
  PresentationExtension* extension_;
};

}  // namespace experimental
}  // namespace xwalk

#endif  // XWALK_EXPERIMENTAL_PRESENTATION_PRESENTATION_EXTENSION_H_
