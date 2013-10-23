// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/experimental/presentation/presentation_display_manager.h"

#include "base/command_line.h"
#include "content/public/browser/browser_thread.h"

using content::BrowserThread;

namespace xwalk {
namespace experimental {

// This static variable is only for testing based on the fact there is at most
// one PresentationDisplayManager instance created at runtime.
static PresentationDisplayManager* g_display_manager = NULL;

PresentationDisplayManager::PresentationDisplayManager()
    : initialized_(false),
      observers_(new ObserverListThreadSafe<Observer>()) {
  DCHECK(g_display_manager == NULL);
  g_display_manager = this;
}

PresentationDisplayManager::~PresentationDisplayManager() {
  DCHECK(g_display_manager);
  g_display_manager = NULL;
}

// static
PresentationDisplayManager* PresentationDisplayManager::GetForTesting() {
  return g_display_manager;
}

void PresentationDisplayManager::EnsureInitialized() {
  if (!BrowserThread::CurrentlyOn(BrowserThread::UI)) {
    BrowserThread::PostTask(BrowserThread::UI, FROM_HERE,
        base::Bind(&PresentationDisplayManager::EnsureInitialized,
                   base::Unretained(this)));
    return;
  }

  if (initialized_)
    return;

  // TODO(hmin): Add platform-specific code for initializing display manager.

  initialized_ = true;
}

void PresentationDisplayManager::AddObserver(Observer* obs) {
  observers_->AddObserver(obs);
}

void PresentationDisplayManager::RemoveObserver(Observer* obs) {
  observers_->RemoveObserver(obs);
}

void PresentationDisplayManager::AddSecondaryDisplay(
    const gfx::Display& display) {
  secondary_displays_.push_back(display);
  // Notify its observers when the first secondary display is added.
  if (secondary_displays_.size() == 1)
    observers_->Notify(&Observer::OnDisplayAvailabilityChanged, true);
}

void PresentationDisplayManager::RemoveSecondaryDisplay(
    const gfx::Display& display) {
  std::vector<gfx::Display>::iterator it = secondary_displays_.begin();
  for (; it != secondary_displays_.end(); ++it)
    if (it->id() == display.id()) break;

  if (it == secondary_displays_.end())
    return;

  secondary_displays_.erase(it);
  // Notify its observers when the last secondary display is removed.
  if (secondary_displays_.size() == 0)
    observers_->Notify(&Observer::OnDisplayAvailabilityChanged, false);
}

gfx::Display PresentationDisplayManager::GetDisplayInfo(int display_id) {
  gfx::Display ret;
  std::vector<gfx::Display>::iterator it = secondary_displays_.begin();
  for (; it != secondary_displays_.end(); ++it)
    if (it->id() != display_id) continue;

  if (it != secondary_displays_.end())
    ret = *it;

  return ret;
}

}  // namespace experimental
}  // namespace xwalk
