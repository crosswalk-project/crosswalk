// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_XWALK_FORM_DATABASE_SERVICE_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_XWALK_FORM_DATABASE_SERVICE_H_

#include <map>

#include "base/basictypes.h"
#include "base/files/file_path.h"
#include "components/autofill/core/browser/webdata/autofill_webdata_service.h"
#include "components/webdata/common/web_data_service_consumer.h"
#include "components/webdata/common/web_database_service.h"

namespace base {
class WaitableEvent;
};

namespace xwalk {

// Handles the database operations necessary to implement the autocomplete
// functionality. This includes creating and initializing the components that
// handle the database backend, and providing a synchronous interface when
// needed (the chromium database components have an async. interface).
class XWalkFormDatabaseService : public WebDataServiceConsumer {
 public:
  explicit XWalkFormDatabaseService(const base::FilePath path);

  virtual ~XWalkFormDatabaseService();

  void Shutdown();

  // Returns whether the database has any data stored. May do
  // IO access and block.
  bool HasFormData();

  // Clear any saved form data. Executes asynchronously.
  void ClearFormData();

  scoped_refptr<autofill::AutofillWebDataService>
      get_autofill_webdata_service();

  // WebDataServiceConsumer implementation.
  virtual void OnWebDataServiceRequestDone(
      WebDataServiceBase::Handle h,
      const WDTypedResult* result) OVERRIDE;

 private:
  struct PendingQuery {
    bool* result;
    base::WaitableEvent* completion;
  };
  typedef std::map<WebDataServiceBase::Handle, PendingQuery> QueryMap;

  void ClearFormDataImpl();
  void HasFormDataImpl(base::WaitableEvent* completion, bool* result);

  QueryMap result_map_;

  scoped_refptr<autofill::AutofillWebDataService> autofill_data_;
  scoped_refptr<WebDatabaseService> web_database_;

  DISALLOW_COPY_AND_ASSIGN(XWalkFormDatabaseService);
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_XWALK_FORM_DATABASE_SERVICE_H_
