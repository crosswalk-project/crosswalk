// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/xwalk_autofill_client.h"

#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "base/logging.h"
#include "base/prefs/pref_registry_simple.h"
#include "base/prefs/pref_service.h"
#include "base/prefs/pref_service_factory.h"
#include "components/autofill/core/browser/autofill_popup_delegate.h"
#include "components/autofill/core/browser/suggestion.h"
#include "components/autofill/core/browser/webdata/autofill_webdata_service.h"
#include "components/autofill/core/common/autofill_pref_names.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/ssl_status.h"
#include "jni/XWalkAutofillClient_jni.h"
#include "xwalk/runtime/browser/android/xwalk_content.h"
#include "xwalk/runtime/browser/xwalk_browser_context.h"
#include "xwalk/runtime/browser/xwalk_content_browser_client.h"

using base::android::AttachCurrentThread;
using base::android::ConvertUTF16ToJavaString;
using base::android::ScopedJavaLocalRef;
using content::WebContents;

DEFINE_WEB_CONTENTS_USER_DATA_KEY(xwalk::XWalkAutofillClient);

namespace xwalk {

// Ownership: The native object is created (if autofill enabled) and owned by
// XWalkContent. The native object creates the java peer which handles most
// autofill functionality at the java side. The java peer is owned by Java
// XWalkContent. The native object only maintains a weak ref to it.
XWalkAutofillClient::XWalkAutofillClient(WebContents* contents)
    : web_contents_(contents), save_form_data_(false) {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> delegate;
  delegate.Reset(
      Java_XWalkAutofillClient_create(env, reinterpret_cast<intptr_t>(this)));
  XWalkContent* xwalk_content = XWalkContent::FromWebContents(web_contents_);
  xwalk_content->SetXWalkAutofillClient(delegate.obj());
  java_ref_ = JavaObjectWeakGlobalRef(env, delegate.obj());
}

XWalkAutofillClient::~XWalkAutofillClient() {
  HideAutofillPopup();
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
                        text_direction == base::i18n::RIGHT_TO_LEFT,
                        suggestions);
}

void XWalkAutofillClient::ShowAutofillPopupImpl(
    const gfx::RectF& element_bounds,
    bool is_rtl,
    const std::vector<autofill::Suggestion>& suggestions) {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null()) return;

  // We need an array of AutofillSuggestion.
  size_t count = suggestions.size();

  ScopedJavaLocalRef<jobjectArray> data_array =
      Java_XWalkAutofillClient_createAutofillSuggestionArray(env, count);

  for (size_t i = 0; i < count; ++i) {
    ScopedJavaLocalRef<jstring> name =
        ConvertUTF16ToJavaString(env, suggestions[i].value);
    ScopedJavaLocalRef<jstring> label =
        ConvertUTF16ToJavaString(env, suggestions[i].label);
    Java_XWalkAutofillClient_addToAutofillSuggestionArray(
        env, data_array.obj(), i, name.obj(), label.obj(),
        suggestions[i].frontend_id);
  }

  Java_XWalkAutofillClient_showAutofillPopup(env,
                                             obj.obj(),
                                             element_bounds.x(),
                                             element_bounds.y(),
                                             element_bounds.width(),
                                             element_bounds.height(),
                                             is_rtl,
                                             data_array.obj());
}

void XWalkAutofillClient::UpdateAutofillPopupDataListValues(
    const std::vector<base::string16>& values,
    const std::vector<base::string16>& labels) {
  // Leaving as an empty method since updating autofill popup window
  // dynamically does not seem to be a useful feature for xwalkview.
  // See crrev.com/18102002 if need to implement.
}

void XWalkAutofillClient::HideAutofillPopup() {
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> obj = java_ref_.get(env);
  if (obj.is_null()) return;
  delegate_.reset();
  Java_XWalkAutofillClient_hideAutofillPopup(env, obj.obj());
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

void XWalkAutofillClient::LinkClicked(const GURL& url,
                                      WindowOpenDisposition disposition) {
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

void XWalkAutofillClient::SuggestionSelected(JNIEnv* env,
                                             jobject object,
                                             jint position) {
  if (delegate_) {
    delegate_->DidAcceptSuggestion(suggestions_[position].value,
                                   suggestions_[position].frontend_id,
                                   position);
  }
}

void XWalkAutofillClient::HideRequestAutocompleteDialog() {
  NOTIMPLEMENTED();
}

void XWalkAutofillClient::ShowAutofillSettings() {
  NOTIMPLEMENTED();
}

void XWalkAutofillClient::ShowUnmaskPrompt(
    const autofill::CreditCard& card,
    base::WeakPtr<autofill::CardUnmaskDelegate> delegate) {
  NOTIMPLEMENTED();
}

void XWalkAutofillClient::OnUnmaskVerificationResult(GetRealPanResult result) {
  NOTIMPLEMENTED();
}

void XWalkAutofillClient::ConfirmSaveCreditCard(
    const base::Closure& save_card_callback) {
  NOTIMPLEMENTED();
}

bool XWalkAutofillClient::HasCreditCardScanFeature() {
  return false;
}

void XWalkAutofillClient::ScanCreditCard(
    const CreditCardScanCallback& callback) {
  NOTIMPLEMENTED();
}

void XWalkAutofillClient::ShowRequestAutocompleteDialog(
    const autofill::FormData& form,
    content::RenderFrameHost* rfh,
    const ResultCallback& callback) {
  NOTIMPLEMENTED();
}

bool RegisterXWalkAutofillClient(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace xwalk
