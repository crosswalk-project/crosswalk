// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_password_manager_client.h"

#include "base/bind_helpers.h"
#include "base/command_line.h"
#include "base/lazy_instance.h"
#include "base/prefs/pref_service.h"
#include "components/autofill/content/browser/content_autofill_driver.h"
#include "components/autofill/content/browser/content_autofill_driver_factory.h"
#include "components/autofill/content/common/autofill_messages.h"
#include "components/autofill/core/browser/password_generator.h"
#include "components/autofill/core/common/password_form.h"
#include "components/password_manager/content/browser/content_password_manager_driver.h"
#include "components/password_manager/content/browser/password_manager_internals_service_factory.h"
#include "components/password_manager/content/common/credential_manager_messages.h"
#include "components/password_manager/core/browser/browser_save_password_progress_logger.h"
#include "components/password_manager/core/browser/log_receiver.h"
#include "components/password_manager/core/browser/login_database.h"
#include "components/password_manager/core/browser/password_form_manager.h"
#include "components/password_manager/core/browser/password_store_default.h"
#include "components/password_manager/core/browser/password_manager_internals_service.h"
#include "components/password_manager/core/browser/password_manager_metrics_util.h"
#include "components/password_manager/core/browser/password_manager_util.h"
#include "components/password_manager/core/common/credential_manager_types.h"
#include "components/password_manager/core/common/password_manager_pref_names.h"
#include "components/password_manager/core/common/password_manager_switches.h"
#include "components/password_manager/sync/browser/sync_store_result_filter.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"

using base::LazyInstance;
using password_manager::ContentPasswordManagerDriverFactory;
using password_manager::PasswordManagerInternalsService;
using password_manager::PasswordManagerInternalsServiceFactory;

DEFINE_WEB_CONTENTS_USER_DATA_KEY(xwalk::XWalkPasswordManagerClient);

namespace xwalk {
class PasswordStoreService;

LazyInstance<scoped_ptr<PasswordStoreService> > g_password_store_service =
    LAZY_INSTANCE_INITIALIZER;

class PasswordStoreService {
 public:
  PasswordStoreService() = default;
  ~PasswordStoreService() {
    if (password_store_.get()) {
      password_store_->Shutdown();
    }
  }

  void Initialize(const base::FilePath& db_path) {
    // Given that LoginDatabase::Init() takes ~100ms on average; it will be
    // called by PasswordStore::Init() on the background thread to avoid UI
    // jank.
    scoped_ptr<password_manager::LoginDatabase> login_db(
        new password_manager::LoginDatabase(db_path));
    scoped_refptr<base::SingleThreadTaskRunner> main_thread_runner(
        base::ThreadTaskRunnerHandle::Get());
    scoped_refptr<base::SingleThreadTaskRunner> db_thread_runner(
        content::BrowserThread::GetMessageLoopProxyForThread(
            content::BrowserThread::DB));

    password_store_ = new password_manager::PasswordStoreDefault(
        main_thread_runner, db_thread_runner, login_db.Pass());
    DCHECK(password_store_);
    password_store_->Init(base::Bind(&StartSyncProxyDummy));
  }

  password_manager::PasswordStore* GetPasswordStore() {
    return password_store_.get();
  }

 private:
  static void StartSyncProxyDummy(syncer::ModelType type) {}

  scoped_refptr<password_manager::PasswordStore> password_store_;
  DISALLOW_COPY_AND_ASSIGN(PasswordStoreService);
};

// Shorten the name to spare line breaks. The code provides enough context
// already.
typedef autofill::SavePasswordProgressLogger Logger;

// static
void XWalkPasswordManagerClient::CreateForWebContentsWithAutofillClient(
    content::WebContents* contents,
    autofill::AutofillClient* autofill_client) {
  if (FromWebContents(contents))
    return;

  contents->SetUserData(
      UserDataKey(),
      new XWalkPasswordManagerClient(contents, autofill_client));
}

XWalkPasswordManagerClient::XWalkPasswordManagerClient(
    content::WebContents* web_contents,
    autofill::AutofillClient* autofill_client)
    : content::WebContentsObserver(web_contents),
      pref_service_(autofill_client->GetPrefs()),
      password_manager_(this),
      driver_factory_(nullptr),
      credential_manager_dispatcher_(web_contents, this),
      can_use_log_router_(false) {
  ContentPasswordManagerDriverFactory::CreateForWebContents(web_contents, this,
                                                            autofill_client);
  driver_factory_ =
      ContentPasswordManagerDriverFactory::FromWebContents(web_contents);

  PasswordManagerInternalsService* service =
      PasswordManagerInternalsServiceFactory::GetForBrowserContext(
          web_contents->GetBrowserContext());
  if (service)
    can_use_log_router_ = service->RegisterClient(this);

  if (!g_password_store_service.Get().get()) {
    g_password_store_service.Get().reset(new PasswordStoreService());
    base::FilePath db_path = web_contents->GetBrowserContext()->GetPath();
    db_path = db_path.Append("xwalklogindb");
    g_password_store_service.Get()->Initialize(db_path);
  }
}

XWalkPasswordManagerClient::~XWalkPasswordManagerClient() {
}

bool XWalkPasswordManagerClient::IsAutomaticPasswordSavingEnabled() const {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(
             password_manager::switches::kEnableAutomaticPasswordSaving);
}

bool XWalkPasswordManagerClient::IsPasswordManagementEnabledForCurrentPage()
    const {
  return true;
}

bool XWalkPasswordManagerClient::IsSavingEnabledForCurrentPage() const {
  return !IsOffTheRecord() &&
         !DidLastPageLoadEncounterSSLErrors() &&
         IsPasswordManagementEnabledForCurrentPage();
}

std::string XWalkPasswordManagerClient::GetSyncUsername() const {
  return "";
}

bool XWalkPasswordManagerClient::IsSyncAccountCredential(
    const std::string& username,
    const std::string& realm) const {
  return false;
}

bool XWalkPasswordManagerClient::PromptUserToSaveOrUpdatePassword(
    scoped_ptr<password_manager::PasswordFormManager> form_to_save,
    password_manager::CredentialSourceType type,
    bool update_password) {
  if (form_to_save->IsBlacklisted())
    return false;
  // TODO(Xiaosong): Prompt infobar UI to let user save password
  return true;
}

bool XWalkPasswordManagerClient::PromptUserToChooseCredentials(
    ScopedVector<autofill::PasswordForm> local_forms,
    ScopedVector<autofill::PasswordForm> federated_forms,
    const GURL& origin,
    base::Callback<void(const password_manager::CredentialInfo&)> callback) {
  return true;
}

void XWalkPasswordManagerClient::ForceSavePassword() {
  password_manager::ContentPasswordManagerDriver* driver =
      driver_factory_->GetDriverForFrame(web_contents()->GetFocusedFrame());
  driver->ForceSavePassword();
}

void XWalkPasswordManagerClient::NotifyUserAutoSignin(
    ScopedVector<autofill::PasswordForm> local_forms) {
}

void XWalkPasswordManagerClient::AutomaticPasswordSave(
    scoped_ptr<password_manager::PasswordFormManager> saved_form) {
}

void XWalkPasswordManagerClient::PasswordWasAutofilled(
    const autofill::PasswordFormMap& best_matches) const {
}

void XWalkPasswordManagerClient::PasswordAutofillWasBlocked(
    const autofill::PasswordFormMap& best_matches) const {
}

void XWalkPasswordManagerClient::HidePasswordGenerationPopup() {
}

PrefService* XWalkPasswordManagerClient::GetPrefs() {
  return pref_service_;
}

password_manager::PasswordStore*
XWalkPasswordManagerClient::GetPasswordStore() const {
  return g_password_store_service.Get()->GetPasswordStore();
}

password_manager::PasswordSyncState
XWalkPasswordManagerClient::GetPasswordSyncState() const {
  return password_manager::NOT_SYNCING_PASSWORDS;
}

void XWalkPasswordManagerClient::OnLogRouterAvailabilityChanged(
    bool router_can_be_used) {
  if (can_use_log_router_ == router_can_be_used)
    return;
  can_use_log_router_ = router_can_be_used;

  NotifyRendererOfLoggingAvailability();
}

void XWalkPasswordManagerClient::LogSavePasswordProgress(
    const std::string& text) const {
  if (!IsLoggingActive())
    return;
  PasswordManagerInternalsService* service =
      PasswordManagerInternalsServiceFactory::GetForBrowserContext(
          web_contents()->GetBrowserContext());
  if (service)
    service->ProcessLog(text);
}

bool XWalkPasswordManagerClient::IsLoggingActive() const {
  // WebUI tabs do not need to log password saving progress. In particular, the
  // internals page itself should not send any logs.
  return can_use_log_router_ && !web_contents()->GetWebUI();
}

bool XWalkPasswordManagerClient::WasLastNavigationHTTPError() const {
  DCHECK(web_contents());

  scoped_ptr<password_manager::BrowserSavePasswordProgressLogger> logger;
  if (IsLoggingActive()) {
    logger.reset(new password_manager::BrowserSavePasswordProgressLogger(this));
    logger->LogMessage(
        Logger::STRING_WAS_LAST_NAVIGATION_HTTP_ERROR_METHOD);
  }

  content::NavigationEntry* entry =
      web_contents()->GetController().GetVisibleEntry();
  if (!entry)
    return false;
  int http_status_code = entry->GetHttpStatusCode();

  if (logger)
    logger->LogNumber(Logger::STRING_HTTP_STATUS_CODE, http_status_code);

  if (http_status_code >= 400 && http_status_code < 600)
    return true;
  return false;
}

bool XWalkPasswordManagerClient::DidLastPageLoadEncounterSSLErrors() const {
  content::NavigationEntry* entry =
      web_contents()->GetController().GetLastCommittedEntry();
  bool ssl_errors = true;
  if (!entry) {
    ssl_errors = false;
  } else {
    ssl_errors = net::IsCertStatusError(entry->GetSSL().cert_status);
  }
  if (IsLoggingActive()) {
    password_manager::BrowserSavePasswordProgressLogger logger(this);
    logger.LogBoolean(Logger::STRING_SSL_ERRORS_PRESENT, ssl_errors);
  }
  return ssl_errors;
}

bool XWalkPasswordManagerClient::IsOffTheRecord() const {
  return web_contents()->GetBrowserContext()->IsOffTheRecord();
}

password_manager::PasswordManager*
XWalkPasswordManagerClient::GetPasswordManager() {
  return &password_manager_;
}

autofill::AutofillManager*
XWalkPasswordManagerClient::GetAutofillManagerForMainFrame() {
  autofill::ContentAutofillDriverFactory* factory =
      autofill::ContentAutofillDriverFactory::FromWebContents(web_contents());
  return factory
             ? factory->DriverForFrame(web_contents()->GetMainFrame())
                   ->autofill_manager()
             : nullptr;
}

bool XWalkPasswordManagerClient::OnMessageReceived(
    const IPC::Message& message,
    content::RenderFrameHost* render_frame_host) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP_WITH_PARAM(XWalkPasswordManagerClient, message,
                                   render_frame_host)
    // Autofill messages:
    IPC_MESSAGE_HANDLER(AutofillHostMsg_ShowPasswordGenerationPopup,
                        ShowPasswordGenerationPopup)
    IPC_MESSAGE_HANDLER(AutofillHostMsg_ShowPasswordEditingPopup,
                        ShowPasswordEditingPopup)
    IPC_END_MESSAGE_MAP()

    IPC_BEGIN_MESSAGE_MAP(XWalkPasswordManagerClient, message)
    IPC_MESSAGE_HANDLER(AutofillHostMsg_HidePasswordGenerationPopup,
                        HidePasswordGenerationPopup)
    IPC_MESSAGE_HANDLER(AutofillHostMsg_GenerationAvailableForForm,
                        GenerationAvailableForForm)
    IPC_MESSAGE_HANDLER(AutofillHostMsg_PasswordAutofillAgentConstructed,
                        NotifyRendererOfLoggingAvailability)
    // Default:
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void XWalkPasswordManagerClient::WebContentsDestroyed() {
  PasswordManagerInternalsService* service =
      PasswordManagerInternalsServiceFactory::GetForBrowserContext(
          web_contents()->GetBrowserContext());
  if (service)
    service->UnregisterClient(this);
}

void XWalkPasswordManagerClient::ShowPasswordGenerationPopup(
    content::RenderFrameHost* render_frame_host,
    const gfx::RectF& bounds,
    int max_length,
    const autofill::PasswordForm& form) {
}

void XWalkPasswordManagerClient::ShowPasswordEditingPopup(
    content::RenderFrameHost* render_frame_host,
    const gfx::RectF& bounds,
    const autofill::PasswordForm& form) {
}

void XWalkPasswordManagerClient::GenerationAvailableForForm(
    const autofill::PasswordForm& form) {
  password_manager_.GenerationAvailableForForm(form);
}

void XWalkPasswordManagerClient::NotifyRendererOfLoggingAvailability() {
  if (!web_contents())
    return;

  web_contents()->GetRenderViewHost()->Send(new AutofillMsg_SetLoggingState(
      web_contents()->GetRenderViewHost()->GetRoutingID(),
      can_use_log_router_));
}

bool XWalkPasswordManagerClient::IsUpdatePasswordUIEnabled() const {
  return true;
}

const GURL& XWalkPasswordManagerClient::GetMainFrameURL() const {
  return web_contents()->GetVisibleURL();
}

const GURL& XWalkPasswordManagerClient::GetLastCommittedEntryURL() const {
  DCHECK(web_contents());
  content::NavigationEntry* entry =
      web_contents()->GetController().GetLastCommittedEntry();
  if (!entry)
    return GURL::EmptyGURL();

  return entry->GetURL();
}

scoped_ptr<password_manager::CredentialsFilter>
XWalkPasswordManagerClient::CreateStoreResultFilter() const {
  return make_scoped_ptr(new password_manager::SyncStoreResultFilter(this));
}

}  // namespace xwalk
