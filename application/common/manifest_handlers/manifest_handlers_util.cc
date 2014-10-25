// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/manifest_handlers_util.h"

#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "xwalk/application/common/manifest.h"

namespace xwalk {
namespace application {

const char kErrMsgInvalidDictionary[] =
    "Cannot get key value as a dictionary. Key name: ";
const char kErrMsgInvalidList[] =
    "Cannot get key value as a list. Key name: ";
const char kErrMsgNoMandatoryKey[] =
    "Cannot find mandatory key. Key name: ";
const char kErrMsgInvalidKeyValue[] =
    "Invalid key value. Key name: ";
const char kErrMsgTooSmallKeyCount[] =
    "Too few keys found. Key name: ";
const char kErrMsgTooBigKeyCount[] =
    "Too many keys found. Key name: ";

void SafeSetError(const std::string& message,
    std::string* error) {
  if (error)
    *error = message;
}

void SafeSetError(const std::string& message,
    const std::string& arg, std::string* error) {
  if (error)
    *error = message + arg;
}

void SafeSetError(const std::string& message,
    base::string16* error) {
  if (error)
    *error = base::ASCIIToUTF16(message);
}

void SafeSetError(const std::string& message,
    const std::string& arg, base::string16* error) {
  if (error)
    *error = base::ASCIIToUTF16(message + arg);
}

bool GetMandatoryDictionary(const Manifest& manifest, const std::string& key,
    const base::DictionaryValue** dict, base::string16* error) {
  if (!manifest.HasPath(key)) {
    SafeSetError(kErrMsgNoMandatoryKey, key, error);
    return false;
  }

  if (!manifest.GetDictionary(key, dict) || !*dict) {
    SafeSetError(kErrMsgInvalidDictionary, key, error);
    return false;
  }

  return true;
}

bool GetOptionalDictionary(const Manifest& manifest, const std::string& key,
    const base::DictionaryValue** dict, base::string16* error) {
  if (!manifest.HasPath(key)) {
    dict = nullptr;
    return true;
  }

  if (!manifest.GetDictionary(key, dict) || !*dict) {
    SafeSetError(kErrMsgInvalidDictionary, key, error);
    return false;
  }

  return true;
}

template <>
bool StringToValue(const std::string& str_value, std::string* value) {
  DCHECK(value);
  *value = str_value;
  return true;
}

template <>
bool StringToValue(const std::string& str_value, bool* value) {
  DCHECK(value);
  if (str_value == "true") {
    *value = true;
    return true;
  }
  if (str_value == "false") {
    *value = false;
    return true;
  }
  return false;
}

template <>
bool StringToValue(const std::string& str_value, int* value) {
  DCHECK(value);
  return base::StringToInt(str_value, value);
}

template <>
bool StringToValue(const std::string& str_value, double* value) {
  DCHECK(value);
  return base::StringToDouble(str_value, value);
}

namespace {

const int infinity = -1;

}  // namespace

ValidCount::ValidCount(int min, int max)
    : min_(min), max_(max) {
}

ValidCount::ValidCount(const ValidCount& other)
    : min_(other.min_), max_(other.max_) {
}

ValidCount& ValidCount::operator=(const ValidCount& other) {
  min_ = other.min_;
  max_ = other.max_;
}

ValidCount ValidCount::Any() {
  return ValidCount(0, infinity);
}

ValidCount ValidCount::Exactly(int value) {
  DCHECK_GE(value, 1);
  return ValidCount(value, value);
}

ValidCount ValidCount::Min(int value) {
  DCHECK_GE(value, 0);
  return ValidCount(value, infinity);
}

ValidCount ValidCount::Max(int value) {
  DCHECK_GE(value, 1);
  return ValidCount(0, value);
}

ValidCount ValidCount::Range(int min, int max) {
  DCHECK_LE(min, max);
  DCHECK_GE(min, 0);
  DCHECK_GE(max, 1);
  return ValidCount(min, max);
}

int ValidCount::Min() const {
  return min_;
}

int ValidCount::Max() const {
  return max_;
}

CountValidator::CountValidator(const ValidCount& valid_count)
  : valid_count_(valid_count), count_(0) {
}

bool CountValidator::IncreaseCount(const std::string& key,
    base::string16* error) {
  ++count_;
  return Validate(key, error);
}

bool CountValidator::SumUp(const std::string& key,
    base::string16* error) const {
  return Validate(key, error);
}

bool CountValidator::Validate(const std::string& key,
    base::string16* error) const {
  if (valid_count_.Min() > 0 && count_ == 0) {
    SafeSetError(kErrMsgNoMandatoryKey, key, error);
    return false;
  } else if (count_ < valid_count_.Min()) {
    SafeSetError(kErrMsgTooSmallKeyCount, key, error);
    return false;
  } else if (valid_count_.Max() != infinity && count_ > valid_count_.Max()) {
    SafeSetError(kErrMsgTooBigKeyCount, key, error);
    return false;
  }
  return true;
}

}  // namespace application
}  // namespace xwalk
