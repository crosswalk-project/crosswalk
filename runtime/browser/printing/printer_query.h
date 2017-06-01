// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_PRINTING_PRINTER_QUERY_H_
#define XWALK_RUNTIME_BROWSER_PRINTING_PRINTER_QUERY_H_

#include <memory>

#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/macros.h"
#include "printing/print_job_constants.h"
#include "xwalk/runtime/browser/printing/print_job_worker_owner.h"

namespace base {
class DictionaryValue;
}

namespace xwalk {

class PrintJobWorker;

// Query the printer for settings.
class PrinterQuery : public PrintJobWorkerOwner {
 public:
  // GetSettings() UI parameter.
  enum class GetSettingsAskParam {
    DEFAULTS,
    ASK_USER,
  };

  PrinterQuery(int render_process_id, int render_view_id);

  // PrintJobWorkerOwner implementation.
  void GetSettingsDone(const printing::PrintSettings& new_settings,
                       printing::PrintingContext::Result result) override;
  PrintJobWorker* DetachWorker(PrintJobWorkerOwner* new_owner) override;
  const printing::PrintSettings& settings() const override;
  int cookie() const override;

  // Initializes the printing context. It is fine to call this function multiple
  // times to reinitialize the settings. |web_contents_observer| can be queried
  // to find the owner of the print setting dialog box. It is unused when
  // |ask_for_user_settings| is DEFAULTS.
  void GetSettings(
      GetSettingsAskParam ask_user_for_settings,
      int expected_page_count,
      bool has_selection,
      printing::MarginType margin_type,
      bool is_scripted,
      const base::Closure& callback);

  // Updates the current settings with |new_settings| dictionary values.
  void SetSettings(std::unique_ptr<base::DictionaryValue> new_settings,
                   const base::Closure& callback);

  // Stops the worker thread since the client is done with this object.
  void StopWorker();

  // Returns true if a GetSettings() call is pending completion.
  bool is_callback_pending() const;

  printing::PrintingContext::Result last_status() const { return last_status_; }

  // Returns if a worker thread is still associated to this instance.
  bool is_valid() const;

 private:
  ~PrinterQuery() override;

  // Lazy create the worker thread. There is one worker thread per print job.
  void StartWorker(const base::Closure& callback);

  // All the UI is done in a worker thread because many Win32 print functions
  // are blocking and enters a message loop without your consent. There is one
  // worker thread per print job.
  std::unique_ptr<PrintJobWorker> worker_;

  // Cache of the print context settings for access in the UI thread.
  printing::PrintSettings settings_;

  // Is the Print... dialog box currently shown.
  bool is_print_dialog_box_shown_;

  // Cookie that make this instance unique.
  int cookie_;

  // Results from the last GetSettingsDone() callback.
  printing::PrintingContext::Result last_status_;

  // Callback waiting to be run.
  base::Closure callback_;

  DISALLOW_COPY_AND_ASSIGN(PrinterQuery);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_PRINTING_PRINTER_QUERY_H_
