// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_XWALK_AUTOFILL_MANAGER_DELEGATE_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_XWALK_AUTOFILL_MANAGER_DELEGATE_H_

#include <jni.h>
#include <vector>

#include "base/android/jni_helper.h"
#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/prefs/pref_registry_simple.h"
#include "base/prefs/pref_service_builder.h"
#include "components/autofill/core/browser/autofill_manager_delegate.h"
#include "content/public/browser/web_contents_user_data.h"

namespace autofill {
class AutofillMetrics;
class AutofillPopupDelegate;
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

// Manager delegate for the autofill functionality. Crosswalk
// supports enabling autocomplete feature for each xwalk instance
// (different than the browser which supports enabling/disabling for
// a profile). Since there is only one pref service for a given browser
// context, we cannot enable this feature via UserPrefs. Rather, we always
// keep the feature enabled at the pref service, and control it via
// the delegates.
class XWalkAutofillManagerDelegate
    : public autofill::AutofillManagerDelegate,
      public content::WebContentsUserData<XWalkAutofillManagerDelegate> {

 public:
  virtual ~XWalkAutofillManagerDelegate();

  void SetSaveFormData(bool enabled);
  bool GetSaveFormData();

  // AutofillManagerDelegate implementation.
  virtual autofill::PersonalDataManager* GetPersonalDataManager() OVERRIDE;
  virtual PrefService* GetPrefs() OVERRIDE;
  virtual void HideRequestAutocompleteDialog() OVERRIDE;
  virtual void ShowAutofillSettings() OVERRIDE;
  virtual void ConfirmSaveCreditCard(
      const autofill::AutofillMetrics& metric_logger,
      const autofill::CreditCard& credit_card,
      const base::Closure& save_card_callback) OVERRIDE;
  virtual void ShowRequestAutocompleteDialog(
      const autofill::FormData& form,
      const GURL& source_url,
      const base::Callback<void(const autofill::FormStructure*)>& callback)
      OVERRIDE;
  virtual void ShowAutofillPopup(
      const gfx::RectF& element_bounds,
      base::i18n::TextDirection text_direction,
      const std::vector<string16>& values,
      const std::vector<string16>& labels,
      const std::vector<string16>& icons,
      const std::vector<int>& identifiers,
      base::WeakPtr<autofill::AutofillPopupDelegate> delegate) OVERRIDE;
  virtual void UpdateAutofillPopupDataListValues(
      const std::vector<base::string16>& values,
      const std::vector<base::string16>& labels) OVERRIDE;
  virtual void HideAutofillPopup() OVERRIDE;
  virtual bool IsAutocompleteEnabled() OVERRIDE;
  virtual void DetectAccountCreationForms(
      const std::vector<autofill::FormStructure*>& forms) OVERRIDE;

  void SuggestionSelected(JNIEnv* env,
                          jobject obj,
                          jint position);
 private:
  explicit XWalkAutofillManagerDelegate(content::WebContents* web_contents);
  friend class content::WebContentsUserData<XWalkAutofillManagerDelegate>;

  void ShowAutofillPopupImpl(const gfx::RectF& element_bounds,
                             const std::vector<string16>& values,
                             const std::vector<string16>& labels,
                             const std::vector<int>& identifiers);

  // The web_contents associated with this delegate.
  content::WebContents* web_contents_;
  bool save_form_data_;
  JavaObjectWeakGlobalRef java_ref_;

  // The current Autofill query values.
  std::vector<string16> values_;
  std::vector<int> identifiers_;
  base::WeakPtr<autofill::AutofillPopupDelegate> delegate_;

  DISALLOW_COPY_AND_ASSIGN(XWalkAutofillManagerDelegate);
};

bool RegisterXWalkAutofillManagerDelegate(JNIEnv* env);

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_XWALK_AUTOFILL_MANAGER_DELEGATE_H_
