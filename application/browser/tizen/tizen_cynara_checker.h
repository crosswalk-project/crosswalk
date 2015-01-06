// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_TIZEN_TIZEN_CYNARA_CHECKER_H_
#define XWALK_APPLICATION_BROWSER_TIZEN_TIZEN_CYNARA_CHECKER_H_

#include <cynara-client.h>
#include <map>
#include <string>

#include "base/threading/thread.h"

namespace xwalk {

// Helper class to check cynara, one Application has one instance
class TizenCynaraChecker {
 public:
  TizenCynaraChecker();
  virtual ~TizenCynaraChecker();

  // Cynara needs the socket file descriptor, through which BP is communicating
  // with RP, to indentify the correspondent RP.
  bool Init(cynara* cynara_handler, int socket_fd);
  bool Initialized() { return cynara_handler_ != nullptr; }

  using ResultCallback = base::Callback<void(bool result)>;

  // The parameter session is not used currently.
  void CheckCynaraASync(const char* session,
                       const char* privilege,
                       const ResultCallback& callback);

 private:
    // The parameter session is not used currently.
  void CheckCynaraTask(const char* session,
                       const char* privilege,
                       const ResultCallback& callback);

  // This class is resposible to free memory
  char* client_id_;
  char* user_id_;

  cynara* cynara_handler_;

  // For the same privilege, we only check Cynara one time.
  struct CheckResult {
    CheckResult();
    bool checked;
    bool permission;
  };
  std::map<std::string, CheckResult> check_result_cache_;
  scoped_ptr<base::Thread> thread_;
};

}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_TIZEN_TIZEN_CYNARA_CHECKER_H_
