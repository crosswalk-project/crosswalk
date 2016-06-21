// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_XWALK_AUTOFILL_CLIENT_H_
#define XWALK_RUNTIME_BROWSER_XWALK_AUTOFILL_CLIENT_H_

#include <string>
#include <vector>

#include "base/compiler_specific.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service_factory.h"
#include "components/autofill/core/browser/autofill_client.h"
#include "content/public/browser/web_contents_user_data.h"

namespace autofill {
class AutofillMetrics;
class AutofillPopupDelegate;
class CardUnmaskDelegate;
class CreditCard;
class FormStructure;
class PasswordGenerator;
class PersonalDataManager;
struct FormData;
}

namespace content {
class WebContents;
}

namespace gfx {
class RectF;
}

class PersonalDataManager;
class PrefService;

namespace xwalk {

// Manager delegate for the autofill functionality. XWalkView
// supports enabling autocomplete feature for each XWalkView instance
// (different than the browser which supports enabling/disabling for
// a profile). Since there is only one pref service for a given browser
// context, we cannot enable this feature via UserPrefs. Rather, we always
// keep the feature enabled at the pref service, and control it via
// the delegates.
class XWalkAutofillClient : public autofill::AutofillClient {
 public:
  ~XWalkAutofillClient() override;

  void SetSaveFormData(bool enabled);
  bool GetSaveFormData();

  // AutofillClient:
  autofill::PersonalDataManager* GetPersonalDataManager() override;
  scoped_refptr<autofill::AutofillWebDataService> GetDatabase() override;
  PrefService* GetPrefs() override;
  sync_driver::SyncService* GetSyncService() override;
  IdentityProvider* GetIdentityProvider() override;
  rappor::RapporService* GetRapporService() override;
  void ShowAutofillSettings() override;
  void ShowUnmaskPrompt(
      const autofill::CreditCard& card,
      UnmaskCardReason reason,
      base::WeakPtr<autofill::CardUnmaskDelegate> delegate) override;
  void OnUnmaskVerificationResult(PaymentsRpcResult result) override;
  void ConfirmSaveCreditCardLocally(
      const autofill::CreditCard& card,
      const base::Closure& callback) override;
  void ConfirmSaveCreditCardToCloud(
      const autofill::CreditCard& card,
      std::unique_ptr<base::DictionaryValue> legal_message,
      const base::Closure& callback) override;
  void LoadRiskData(
      const base::Callback<void(const std::string&)>& callback) override;
  bool HasCreditCardScanFeature() override;
  void ScanCreditCard(const CreditCardScanCallback& callback) override;
  void HideAutofillPopup() override;
  void ShowAutofillPopup(
      const gfx::RectF& element_bounds,
      base::i18n::TextDirection text_direction,
      const std::vector<autofill::Suggestion>& suggestions,
      base::WeakPtr<autofill::AutofillPopupDelegate> delegate) override;
  void UpdateAutofillPopupDataListValues(
      const std::vector<base::string16>& values,
      const std::vector<base::string16>& labels) override;
  bool IsAutocompleteEnabled() override;
  void PropagateAutofillPredictions(
      content::RenderFrameHost* rfh,
      const std::vector<autofill::FormStructure*>& forms) override;
  void DidFillOrPreviewField(
      const base::string16& autofilled_value,
      const base::string16& profile_full_name) override;
  void OnFirstUserGestureObserved() override;
  bool IsContextSecure(const GURL& form_origin) override;
  void SuggestionSelected(int position);

  virtual void ShowAutofillPopupImpl(
    const gfx::RectF& element_bounds,
    base::i18n::TextDirection text_direction,
    const std::vector<autofill::Suggestion>& suggestions) = 0;

  virtual void HideAutofillPopupImpl() = 0;

 protected:
  explicit XWalkAutofillClient(content::WebContents* web_contents);

  // The web_contents associated with this delegate.
  content::WebContents* web_contents_;
  base::WeakPtr<autofill::AutofillPopupDelegate> delegate_;

 private:
  friend class content::WebContentsUserData<XWalkAutofillClient>;

  bool save_form_data_;

  // The current Autofill query values.
  std::vector<autofill::Suggestion> suggestions_;

  DISALLOW_COPY_AND_ASSIGN(XWalkAutofillClient);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_XWALK_AUTOFILL_CLIENT_H_
