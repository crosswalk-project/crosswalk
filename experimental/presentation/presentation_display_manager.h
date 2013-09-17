// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXPERIMENTAL_PRESENTATION_PRESENTATION_DISPLAY_MANAGER_H_
#define XWALK_EXPERIMENTAL_PRESENTATION_PRESENTATION_DISPLAY_MANAGER_H_

#include <vector>

#include "base/memory/ref_counted.h"
#include "base/observer_list_threadsafe.h"
#include "ui/gfx/display.h"

namespace xwalk {
namespace experimental {

// A presentation display manager is responsible for retrieving the secondary
// displays from system and notifying the secondary display arrival/removal
// event to its observers.
class PresentationDisplayManager {
 public:
  class Observer {
   public:
    virtual ~Observer() {}

    // Called when the availability of presentation display is changed. The
    // presentation display is available when the first secondary display
    // connected, and unavailable when the last secondary display is
    // disconnected.
    virtual void OnDisplayAvailabilityChanged(bool is_available) = 0;
  };

  static PresentationDisplayManager* GetForTesting();

  PresentationDisplayManager();
  virtual ~PresentationDisplayManager();

  // Ensure the display manager is initialized correctly. Can be called on
  // any thread.
  virtual void EnsureInitialized();

  // Add/remove an observer for display availability change.
  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);

  // Called on UI thread to retrieve all connected secondary displays.
  std::vector<gfx::Display> GetSecondaryDisplays() const {
    return secondary_displays_;
  }

  // Called on UI thread to retrieve the secondary display info.
  gfx::Display GetDisplayInfo(int display_id);

 private:
  friend class PresentationExtensionTest;

  // Add/remove the seconary display.
  void AddSecondaryDisplay(const gfx::Display& display);
  void RemoveSecondaryDisplay(const gfx::Display& display);

  std::vector<gfx::Display> secondary_displays_;

  // Indicate if the display manager gets initialized.
  bool initialized_;

  // The observer can be added on any thread.
  scoped_refptr<ObserverListThreadSafe<Observer> > observers_;
};

}  // namespace experimental
}  // namespace xwalk

#endif  // XWALK_EXPERIMENTAL_PRESENTATION_PRESENTATION_DISPLAY_MANAGER_H_
