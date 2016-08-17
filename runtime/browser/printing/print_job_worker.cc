// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/printing/print_job_worker.h"

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/location.h"
#include "base/message_loop/message_loop.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/values.h"
#include "build/build_config.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "printing/print_job_constants.h"
#include "printing/printed_document.h"
#include "printing/printed_page.h"
#include "printing/printing_utils.h"
#include "ui/base/l10n/l10n_util.h"
#include "xwalk/runtime/browser/printing/print_job.h"
#include "xwalk/runtime/browser/xwalk_content_browser_client.h"
#include "xwalk/runtime/common/xwalk_notification_types.h"

using content::BrowserThread;

namespace xwalk {

namespace {

// Helper function to ensure |owner| is valid until at least |callback| returns.
void HoldRefCallback(const scoped_refptr<PrintJobWorkerOwner>& owner,
                     const base::Closure& callback) {
  callback.Run();
}

class PrintingContextDelegate : public printing::PrintingContext::Delegate {
 public:
  PrintingContextDelegate(int render_process_id, int render_view_id);
  ~PrintingContextDelegate() override;

  gfx::NativeView GetParentView() override;
  std::string GetAppLocale() override;

  // Not exposed to PrintingContext::Delegate because of dependency issues.
  content::WebContents* GetWebContents();

 private:
  int render_process_id_;
  int render_view_id_;
};

PrintingContextDelegate::PrintingContextDelegate(int render_process_id,
                                                 int render_view_id)
    : render_process_id_(render_process_id),
      render_view_id_(render_view_id) {
}

PrintingContextDelegate::~PrintingContextDelegate() {
}

gfx::NativeView PrintingContextDelegate::GetParentView() {
  content::WebContents* wc = GetWebContents();
  return wc ? wc->GetNativeView() : nullptr;
}

content::WebContents* PrintingContextDelegate::GetWebContents() {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  content::RenderViewHost* view =
      content::RenderViewHost::FromID(render_process_id_, render_view_id_);
  return view ? content::WebContents::FromRenderViewHost(view) : nullptr;
}

std::string PrintingContextDelegate::GetAppLocale() {
  return XWalkContentBrowserClient::Get()->GetApplicationLocale();
}

void NotificationCallback(PrintJobWorkerOwner* print_job,
                          JobEventDetails::Type detail_type,
                          printing::PrintedDocument* document,
                          printing::PrintedPage* page) {
  JobEventDetails* details = new JobEventDetails(detail_type, document, page);
  content::NotificationService::current()->Notify(
      NOTIFICATION_XWALK_PRINT_JOB_EVENT,
      // We know that is is a PrintJob object in this circumstance.
      content::Source<PrintJob>(static_cast<PrintJob*>(print_job)),
      content::Details<JobEventDetails>(details));
}

void PostOnOwnerThread(const scoped_refptr<PrintJobWorkerOwner>& owner,
                       const printing::PrintingContext::PrintSettingsCallback& callback,
                       printing::PrintingContext::Result result) {
  owner->PostTask(FROM_HERE, base::Bind(&HoldRefCallback, owner,
                                        base::Bind(callback, result)));
}

}  // namespace

PrintJobWorker::PrintJobWorker(int render_process_id,
                               int render_view_id,
                               PrintJobWorkerOwner* owner)
    : owner_(owner), thread_("Printing_Worker"), weak_factory_(this) {
  // The object is created in the IO thread.
  DCHECK(owner_->RunsTasksOnCurrentThread());

  printing_context_delegate_.reset(
      new PrintingContextDelegate(render_process_id, render_view_id));
  printing_context_ = printing::PrintingContext::Create(printing_context_delegate_.get());
}

PrintJobWorker::~PrintJobWorker() {
  // The object is normally deleted in the UI thread, but when the user
  // cancels printing or in the case of print preview, the worker is destroyed
  // on the I/O thread.
  DCHECK(owner_->RunsTasksOnCurrentThread());
  Stop();
}

void PrintJobWorker::SetNewOwner(PrintJobWorkerOwner* new_owner) {
  DCHECK(page_number_ == printing::PageNumber::npos());
  owner_ = new_owner;
}

void PrintJobWorker::GetSettings(
    bool ask_user_for_settings,
    int document_page_count,
    bool has_selection,
    printing::MarginType margin_type,
    bool is_scripted) {
  DCHECK(task_runner_->RunsTasksOnCurrentThread());
  DCHECK_EQ(page_number_, printing::PageNumber::npos());

  // Recursive task processing is needed for the dialog in case it needs to be
  // destroyed by a task.
  // TODO(thestig): This code is wrong. SetNestableTasksAllowed(true) is needed
  // on the thread where the PrintDlgEx is called, and definitely both calls
  // should happen on the same thread. See http://crbug.com/73466
  // MessageLoop::current()->SetNestableTasksAllowed(true);
  printing_context_->set_margin_type(margin_type);

  // When we delegate to a destination, we don't ask the user for settings.
  // TODO(mad): Ask the destination for settings.
  if (ask_user_for_settings) {
    BrowserThread::PostTask(
        BrowserThread::UI, FROM_HERE,
        base::Bind(&HoldRefCallback, make_scoped_refptr(owner_),
                   base::Bind(&PrintJobWorker::GetSettingsWithUI,
                              base::Unretained(this),
                              document_page_count,
                              has_selection,
                              is_scripted)));
  } else {
    BrowserThread::PostTask(
        BrowserThread::UI, FROM_HERE,
        base::Bind(&HoldRefCallback, make_scoped_refptr(owner_),
                   base::Bind(&PrintJobWorker::UseDefaultSettings,
                              base::Unretained(this))));
  }
}

void PrintJobWorker::SetSettings(
    std::unique_ptr<base::DictionaryValue> new_settings) {
  DCHECK(task_runner_->RunsTasksOnCurrentThread());

  BrowserThread::PostTask(
      BrowserThread::UI,
      FROM_HERE,
      base::Bind(&HoldRefCallback,
                 make_scoped_refptr(owner_),
                 base::Bind(&PrintJobWorker::UpdatePrintSettings,
                            base::Unretained(this),
                            base::Passed(&new_settings))));
}

void PrintJobWorker::UpdatePrintSettings(
    std::unique_ptr<base::DictionaryValue> new_settings) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  printing::PrintingContext::Result result =
      printing_context_->UpdatePrintSettings(*new_settings);
  GetSettingsDone(result);
}

void PrintJobWorker::GetSettingsDone(printing::PrintingContext::Result result) {
  // Most PrintingContext functions may start a message loop and process
  // message recursively, so disable recursive task processing.
  // TODO(thestig): See above comment. SetNestableTasksAllowed(false) needs to
  // be called on the same thread as the previous call.  See
  // http://crbug.com/73466
  // MessageLoop::current()->SetNestableTasksAllowed(false);

  // We can't use OnFailure() here since owner_ may not support notifications.

  // PrintJob will create the new PrintedDocument.
  owner_->PostTask(FROM_HERE,
                   base::Bind(&PrintJobWorkerOwner::GetSettingsDone,
                              make_scoped_refptr(owner_),
                              printing_context_->settings(),
                              result));
}

void PrintJobWorker::GetSettingsWithUI(
    int document_page_count,
    bool has_selection,
    bool is_scripted) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);

  // weak_factory_ creates pointers valid only on owner_ thread.
  printing_context_->AskUserForSettings(
      document_page_count, has_selection, is_scripted,
      base::Bind(&PostOnOwnerThread, make_scoped_refptr(owner_),
                 base::Bind(&PrintJobWorker::GetSettingsDone,
                            weak_factory_.GetWeakPtr())));
}

void PrintJobWorker::UseDefaultSettings() {
  printing::PrintingContext::Result result = printing_context_->UseDefaultSettings();
  GetSettingsDone(result);
}

void PrintJobWorker::StartPrinting(printing::PrintedDocument* new_document) {
  DCHECK(task_runner_->RunsTasksOnCurrentThread());
  DCHECK_EQ(page_number_, printing::PageNumber::npos());
  DCHECK_EQ(document_.get(), new_document);
  DCHECK(document_.get());

  if (!document_.get() || page_number_ != printing::PageNumber::npos() ||
      document_.get() != new_document) {
    return;
  }

  base::string16 document_name =
      printing::SimplifyDocumentTitle(document_->name());
  printing::PrintingContext::Result result =
      printing_context_->NewDocument(document_name);
  if (result != printing::PrintingContext::OK) {
    OnFailure();
    return;
  }

  // Try to print already cached data. It may already have been generated for
  // the print preview.
  OnNewPage();
  // Don't touch this anymore since the instance could be destroyed. It happens
  // if all the pages are printed a one sweep and the client doesn't have a
  // handle to us anymore. There's a timing issue involved between the worker
  // thread and the UI thread. Take no chance.
}

void PrintJobWorker::OnDocumentChanged(printing::PrintedDocument* new_document) {
  DCHECK(task_runner_->RunsTasksOnCurrentThread());
  DCHECK_EQ(page_number_, printing::PageNumber::npos());

  if (page_number_ != printing::PageNumber::npos())
    return;

  document_ = new_document;
}

void PrintJobWorker::OnNewPage() {
  if (!document_.get())  // Spurious message.
    return;

  // message_loop() could return NULL when the print job is cancelled.
  DCHECK(task_runner_->RunsTasksOnCurrentThread());

  if (page_number_ == printing::PageNumber::npos()) {
    // Find first page to print.
    int page_count = document_->page_count();
    if (!page_count) {
      // We still don't know how many pages the document contains. We can't
      // start to print the document yet since the header/footer may refer to
      // the document's page count.
      return;
    }
    // We have enough information to initialize page_number_.
    page_number_.Init(document_->settings(), page_count);
  }
  DCHECK_NE(page_number_, printing::PageNumber::npos());

  while (true) {
    // Is the page available?
    scoped_refptr<printing::PrintedPage> page = document_->GetPage(page_number_.ToInt());
    if (!page.get()) {
      // We need to wait for the page to be available.
      base::ThreadTaskRunnerHandle::Get()->PostDelayedTask(
          FROM_HERE,
          base::Bind(&PrintJobWorker::OnNewPage, weak_factory_.GetWeakPtr()),
          base::TimeDelta::FromMilliseconds(500));
      break;
    }
    // The page is there, print it.
    SpoolPage(page.get());
    ++page_number_;
    if (page_number_ == printing::PageNumber::npos()) {
      OnDocumentDone();
      // Don't touch this anymore since the instance could be destroyed.
      break;
    }
  }
}

void PrintJobWorker::Cancel() {
  // This is the only function that can be called from any thread.
  printing_context_->Cancel();
  // Cannot touch any member variable since we don't know in which thread
  // context we run.
}

bool PrintJobWorker::IsRunning() const {
  return thread_.IsRunning();
}

bool PrintJobWorker::PostTask(const tracked_objects::Location& from_here,
                              const base::Closure& task) {
  if (task_runner_.get())
    return task_runner_->PostTask(from_here, task);
  return false;
}

void PrintJobWorker::StopSoon() {
  thread_.StopSoon();
}

void PrintJobWorker::Stop() {
  thread_.Stop();
}

bool PrintJobWorker::Start() {
  bool result = thread_.Start();
  task_runner_ = thread_.task_runner();
  return result;
}

void PrintJobWorker::OnDocumentDone() {
  DCHECK(task_runner_->RunsTasksOnCurrentThread());
  DCHECK_EQ(page_number_, printing::PageNumber::npos());
  DCHECK(document_.get());

  if (printing_context_->DocumentDone() != printing::PrintingContext::OK) {
    OnFailure();
    return;
  }

  owner_->PostTask(FROM_HERE,
                   base::Bind(&NotificationCallback, base::RetainedRef(owner_),
                              JobEventDetails::DOC_DONE,
                              base::RetainedRef(document_), nullptr));

  // Makes sure the variables are reinitialized.
  document_ = NULL;
}

void PrintJobWorker::SpoolPage(printing::PrintedPage* page) {
  DCHECK(task_runner_->RunsTasksOnCurrentThread());
  DCHECK_NE(page_number_, printing::PageNumber::npos());

  // Signal everyone that the page is about to be printed.
  owner_->PostTask(
      FROM_HERE,
      base::Bind(&NotificationCallback, base::RetainedRef(owner_),
                 JobEventDetails::NEW_PAGE, base::RetainedRef(document_),
                 base::RetainedRef(page)));

  // Preprocess.
  if (printing_context_->NewPage() != printing::PrintingContext::OK) {
    OnFailure();
    return;
  }

  // Actual printing.
#if defined(OS_WIN) || defined(OS_MACOSX)
  document_->RenderPrintedPage(*page, printing_context_->context());
#elif defined(OS_POSIX)
  document_->RenderPrintedPage(*page, printing_context_.get());
#endif

  // Postprocess.
  if (printing_context_->PageDone() != printing::PrintingContext::OK) {
    OnFailure();
    return;
  }

  // Signal everyone that the page is printed.
  owner_->PostTask(
      FROM_HERE,
      base::Bind(&NotificationCallback, base::RetainedRef(owner_),
                 JobEventDetails::PAGE_DONE, base::RetainedRef(document_),
                 base::RetainedRef(page)));
}

void PrintJobWorker::OnFailure() {
  DCHECK(task_runner_->RunsTasksOnCurrentThread());

  // We may loose our last reference by broadcasting the FAILED event.
  scoped_refptr<PrintJobWorkerOwner> handle(owner_);

  owner_->PostTask(FROM_HERE,
                   base::Bind(&NotificationCallback, base::RetainedRef(owner_),
                              JobEventDetails::FAILED,
                              base::RetainedRef(document_), nullptr));
  Cancel();

  // Makes sure the variables are reinitialized.
  document_ = NULL;
  page_number_ = printing::PageNumber::npos();
}

}  // namespace xwalk
