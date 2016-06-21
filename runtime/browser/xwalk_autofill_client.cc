// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/xwalk_autofill_client.h"

#include <string>

#include "base/logging.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/pref_service_factory.h"
#include "components/autofill/core/browser/autofill_popup_delegate.h"
#include "components/autofill/core/browser/suggestion.h"
#include "components/autofill/core/browser/webdata/autofill_webdata_service.h"
#include "components/autofill/core/common/autofill_pref_names.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/ssl_status.h"
#include "ui/gfx/geometry/rect_f.h"
#include "xwalk/runtime/browser/xwalk_browser_context.h"
#include "xwalk/runtime/browser/xwalk_content_browser_client.h"

using content::WebContents;

namespace xwalk {

XWalkAutofillClient::XWalkAutofillClient(WebContents* contents)
    : web_contents_(contents), save_form_data_(false) {
}

XWalkAutofillClient::~XWalkAutofillClient() {
}

void XWalkAutofillClient::SetSaveFormData(bool enabled) {
  save_form_data_ = enabled;
}

bool XWalkAutofillClient::GetSaveFormData() {
  return save_form_data_;
}

PrefService* XWalkAutofillClient::GetPrefs() {
  return user_prefs::UserPrefs::Get(
      XWalkBrowserContext::GetDefault());
}

sync_driver::SyncService* XWalkAutofillClient::GetSyncService() {
  NOTIMPLEMENTED();
  return nullptr;
}

IdentityProvider* XWalkAutofillClient::GetIdentityProvider() {
  return nullptr;
}

rappor::RapporService* XWalkAutofillClient::GetRapporService() {
  return nullptr;
}

autofill::PersonalDataManager* XWalkAutofillClient::GetPersonalDataManager() {
  return nullptr;
}

scoped_refptr<autofill::AutofillWebDataService>
XWalkAutofillClient::GetDatabase() {
  xwalk::XWalkFormDatabaseService* service =
      static_cast<xwalk::XWalkBrowserContext*>(
          web_contents_->GetBrowserContext())->GetFormDatabaseService();
  return service->get_autofill_webdata_service();
}

void XWalkAutofillClient::HideAutofillPopup() {
  delegate_.reset();
  HideAutofillPopupImpl();
}

void XWalkAutofillClient::ShowAutofillPopup(
    const gfx::RectF& element_bounds,
    base::i18n::TextDirection text_direction,
    const std::vector<autofill::Suggestion>& suggestions,
    base::WeakPtr<autofill::AutofillPopupDelegate> delegate) {
  suggestions_ = suggestions;
  delegate_ = delegate;

  // Convert element_bounds to be in screen space.
  gfx::Rect client_area = web_contents_->GetContainerBounds();
  gfx::RectF element_bounds_in_screen_space =
      element_bounds + client_area.OffsetFromOrigin();

  ShowAutofillPopupImpl(element_bounds_in_screen_space,
                        text_direction,
                        suggestions);
}

void XWalkAutofillClient::UpdateAutofillPopupDataListValues(
    const std::vector<base::string16>& values,
    const std::vector<base::string16>& labels) {
  // Leaving as an empty method since updating autofill popup window
  // dynamically does not seem to be a useful feature for xwalkview.
  // See crrev.com/18102002 if need to implement.
}

bool XWalkAutofillClient::IsAutocompleteEnabled() {
  return GetSaveFormData();
}

void XWalkAutofillClient::PropagateAutofillPredictions(
    content::RenderFrameHost* rfh,
    const std::vector<autofill::FormStructure*>& forms) {
}

void XWalkAutofillClient::DidFillOrPreviewField(
    const base::string16& autofilled_value,
    const base::string16& profile_full_name) {
}

void XWalkAutofillClient::OnFirstUserGestureObserved() {
  NOTIMPLEMENTED();
}

bool XWalkAutofillClient::IsContextSecure(const GURL& form_origin) {
  content::SSLStatus ssl_status;
  content::NavigationEntry* navigation_entry =
      web_contents_->GetController().GetLastCommittedEntry();
  if (!navigation_entry)
     return false;

  ssl_status = navigation_entry->GetSSL();
  // Note: The implementation below is a copy of the one in
  // ChromeAutofillClient::IsContextSecure, and should be kept in sync
  // until crbug.com/505388 gets implemented.
  return ssl_status.security_style ==
      content::SECURITY_STYLE_AUTHENTICATED &&
      ssl_status.content_status == content::SSLStatus::NORMAL_CONTENT;
}

void XWalkAutofillClient::SuggestionSelected(int position) {
  if (delegate_) {
    delegate_->DidAcceptSuggestion(suggestions_[position].value,
                                   suggestions_[position].frontend_id,
                                   position);
  }
}

void XWalkAutofillClient::ShowAutofillSettings() {
  NOTIMPLEMENTED();
}

void XWalkAutofillClient::ShowUnmaskPrompt(
    const autofill::CreditCard& card,
    UnmaskCardReason reason,
    base::WeakPtr<autofill::CardUnmaskDelegate> delegate) {
  NOTIMPLEMENTED();
}

void XWalkAutofillClient::OnUnmaskVerificationResult(PaymentsRpcResult result) {
  NOTIMPLEMENTED();
}

void XWalkAutofillClient::ConfirmSaveCreditCardLocally(
    const autofill::CreditCard& card,
    const base::Closure& save_card_callback) {
  NOTIMPLEMENTED();
}

void XWalkAutofillClient::ConfirmSaveCreditCardToCloud(
      const autofill::CreditCard& card,
      std::unique_ptr<base::DictionaryValue> legal_message,
      const base::Closure& callback) {
  NOTIMPLEMENTED();
}

void XWalkAutofillClient::LoadRiskData(
    const base::Callback<void(const std::string&)>& callback) {
  NOTIMPLEMENTED();
}

bool XWalkAutofillClient::HasCreditCardScanFeature() {
  return false;
}

void XWalkAutofillClient::ScanCreditCard(
    const CreditCardScanCallback& callback) {
  NOTIMPLEMENTED();
}

}  // namespace xwalk
