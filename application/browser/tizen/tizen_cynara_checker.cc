// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/browser/tizen/tizen_cynara_checker.h"

#include <cynara-creds-socket.h>

#include "base/bind.h"
#include "base/logging.h"

namespace xwalk {

TizenCynaraChecker::TizenCynaraChecker()
    : client_id_(nullptr),
      user_id_(nullptr),
      cynara_handler_(nullptr),
      thread_(new base::Thread("cynara_checking_thread")) {
}

bool TizenCynaraChecker::Init(cynara* cynara_handler, int socket_fd) {
  LOG(ERROR) << "enter cynara check";

  if (Initialized())
    return true;

  if (cynara_handler == nullptr)
    return false;
  else
    cynara_handler_ = cynara_handler;

  int ret;
  ret = cynara_creds_socket_get_client(socket_fd,
          CLIENT_METHOD_SMACK, &client_id_);
  if (ret != CYNARA_API_SUCCESS) {
    LOG(ERROR) << "cynara failed to get client id, error code " + ret;
    return false;
  }
  ret = cynara_creds_socket_get_user(socket_fd, USER_METHOD_UID, &user_id_);
  if (ret != CYNARA_API_SUCCESS) {
    LOG(ERROR) << "cynara failed to get user id, error code " + ret;
    return false;
  }

  if (!thread_.get()->Start()) {
    LOG(ERROR) << "Failed to create Cynara checking thread";
    return false;
  }

  return true;
}

void TizenCynaraChecker::CheckCynaraASync(const char* session,
        const char* privilege,
        const ResultCallback& callback) {
  thread_.get()->task_runner()->PostTask(FROM_HERE,
                                  base::Bind(
                                    &TizenCynaraChecker::CheckCynaraTask,
                                    base::Unretained(this),
                                    session,
                                    privilege,
                                    callback));
}

void TizenCynaraChecker::CheckCynaraTask(const char* session,
                       const char* privilege,
                       const ResultCallback& callback) {
  // If it's checked once already, the sequenced will be completed directly.
  if (check_result_cache_[privilege].checked) {
    callback.Run(check_result_cache_[privilege].permission);
    return;
  }

  bool permission = cynara_check(cynara_handler_,
                        client_id_, session, user_id_, privilege);
  check_result_cache_[privilege].checked = true;
  check_result_cache_[privilege].permission = permission;
  callback.Run(permission);
}

TizenCynaraChecker::~TizenCynaraChecker() {
  // Stop checking thread before Cynara client being invalid
  thread_.get()->Stop();

  if (client_id_)
    free(client_id_);
  if (user_id_)
    free(user_id_);
}

TizenCynaraChecker::CheckResult::CheckResult()
  : checked(false),
    permission(false) {
}

}  // namespace xwalk
