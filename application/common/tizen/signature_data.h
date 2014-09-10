// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_TIZEN_SIGNATURE_DATA_H_
#define XWALK_APPLICATION_COMMON_TIZEN_SIGNATURE_DATA_H_

#include <list>
#include <set>
#include <string>

#include "base/files/file_path.h"
#include "libxml/tree.h"
#include "libxml/xpath.h"

namespace xwalk {
namespace application {

class SignatureData {
 public:
  SignatureData(const std::string& signature_file_name, int signature_number);
  ~SignatureData();

  std::string signature_file_name() const {
    return signature_file_name_;
  }

  const std::set<std::string>& reference_set() const {
    return reference_set_;
  }

  void set_reference_set(const std::set<std::string>& reference_set) {
    reference_set_ = reference_set;
  }

  std::string role_uri() const {
    return role_uri_;
  }

  void set_role_uri(const std::string& role_uri) {
    role_uri_ = role_uri;
  }

  std::string profile_uri() const {
    return profile_uri_;
  }

  void set_profile_uri(const std::string& profile_uri) {
    profile_uri_ = profile_uri;
  }

  std::string object_id() const {
    return object_id_;
  }

  void set_object_id(const std::string& object_id) {
    object_id_ = object_id;
  }

  std::string signature_value() const {
    return signature_value_;
  }

  void set_signature_value(const std::string& signature_value) {
    signature_value_ = signature_value;
  }

  std::string canonicalization_method() const {
    return canonicalization_method_;
  }

  void set_canonicalization_method(const std::string& canonicalization_method) {
    canonicalization_method_ = canonicalization_method;
  }

  std::string signature_method() const {
    return signature_method_;
  }

  void set_signature_method(const std::string signature_method) {
    signature_method_ = signature_method;
  }

  const std::list<std::string>& certificate_list() const {
    return certificate_list_;
  }

  void set_certificate_list(const std::list<std::string>& certificate_list) {
    certificate_list_ = certificate_list;
  }

  bool isAuthorSignature() const {
    return signature_number_ == -1;
  }

  base::FilePath GetExtractedWidgetPath() const;

 private:
  std::string signature_file_name_;
  // This number is taken from distributor signature file name.
  // Author signature do not contain any number on the file name.
  // Author signature should have signature number equal to -1.
  int signature_number_;
  std::string role_uri_;
  std::string profile_uri_;
  std::string signature_value_;
  std::string identifier;
  std::string object_id_;
  std::string canonicalization_method_;
  std::string signature_method_;
  std::set<std::string> reference_set_;
  std::list<std::string> certificate_list_;

  DISALLOW_COPY_AND_ASSIGN(SignatureData);
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_TIZEN_SIGNATURE_DATA_H_
