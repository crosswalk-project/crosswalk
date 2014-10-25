// Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_MANIFEST_HANDLERS_UTIL_H_
#define XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_MANIFEST_HANDLERS_UTIL_H_

#include <string>

#include "base/logging.h"
#include "base/values.h"

namespace xwalk {
namespace application {

class Manifest;

extern const char kErrMsgInvalidDictionary[];
extern const char kErrMsgInvalidList[];
extern const char kErrMsgNoMandatoryKey[];
extern const char kErrMsgInvalidKeyValue[];

// If the error parameter is specified it is filled with the given message
// otherwise it does nothing.
void SafeSetError(const std::string& message,
    std::string* error);

// If the error parameter is specified it is filled with concatenation
// of message and arg parameters otherwise it does nothing.
void SafeSetError(const std::string& message,
    const std::string& arg, std::string* error);

// If the error parameter is specified it is filled with the given message
// otherwise it does nothing.
void SafeSetError(const std::string& message,
    base::string16* error);

// If the error parameter is specified it is filled with concatenation
// of message and arg parameters otherwise it does nothing.
void SafeSetError(const std::string& message,
    const std::string& arg, base::string16* error);

// Retrieves a mandatory dictionary from specified manifest and specified key.
// Returns true if the ditionary is found or false otherwise. If the error
// parameter is specified it is also filled with proper message.
bool GetMandatoryDictionary(const Manifest& manifest, const std::string& key,
    const base::DictionaryValue** dict, base::string16* error);

// Retrieves an optional dictionary from specified manifest and specified key.
// If the dictionary is found the function returns true and fills dict
// parameter. If the dictionary is not found the function returns true and fills
// dict parameter with nullptr. If an error occurs it returns false
// and fills error parameter if it is set.
bool GetOptionalDictionary(const Manifest& manifest, const std::string& key,
    const base::DictionaryValue** dict, base::string16* error);

// Converts given text value to a value of specific type. Returns true
// if convertion is successful or false otherwise.
template <typename ValueType>
bool StringToValue(const std::string& str_value, ValueType* value) {
  NOTREACHED() << "Use one of already defined template specializations"
                  " or define a new one.";
  return false;
}

// Converts given text value to a string value. Returns true
// if convertion is successful or false otherwise.
template <>
bool StringToValue(const std::string& str_value, std::string* value);

// Converts given text value to a boolean value. Returns true
// if convertion is successful or false otherwise.
template <>
bool StringToValue(const std::string& str_value, bool* value);

// Converts given text value to an integer value. Returns true
// if convertion is successful or false otherwise.
template <>
bool StringToValue(const std::string& str_value, int* value);

// Converts given text value to a floating point value. Returns true
// if convertion is successful or false otherwise.
template <>
bool StringToValue(const std::string& str_value, double* value);

// Retrieves a mandatory value from specified dictionary and specified key.
// Returns true if the value is found or false otherwise. If the error parameter
// is specified it is also filled with proper message.
template <typename ValueType>
bool GetMandatoryValue(const base::DictionaryValue& dict,
    const std::string& key, ValueType* value, base::string16* error) {
  DCHECK(value);

  std::string tmp;
  if (!dict.GetString(key, &tmp)) {
    SafeSetError(kErrMsgNoMandatoryKey, key, error);
    return false;
  }

  bool result = StringToValue(tmp, value);
  if (!result)
    SafeSetError(kErrMsgInvalidKeyValue, key, error);
  return result;
}

// Retrieves an optional value from specified dictionary and specified key.
// If the value is found the function returns true and fills value
// parameter. If the value is not found the function returns true and fills
// value parameter with default value. If an error occurs it returns false
// and fills error parameter if it is set.
template <typename ValueType>
bool GetOptionalValue(const base::DictionaryValue& dict,
    const std::string& key, ValueType default_value, ValueType* value,
    base::string16* error) {
  DCHECK(value);

  std::string tmp;
  if (!dict.GetString(key, &tmp)) {
    *value = default_value;
    return true;
  }

  bool result = StringToValue(tmp, value);
  if (!result)
    SafeSetError(kErrMsgInvalidKeyValue, key, error);
  return result;
}

// Represents a valid count of dictionaries found by ForEachDictionaryCall
class ValidCount {
 public:
  ValidCount(const ValidCount& other);
  ValidCount& operator=(const ValidCount& other);

  static ValidCount Any();
  static ValidCount Exactly(int value);
  static ValidCount Min(int value);
  static ValidCount Max(int value);
  static ValidCount Range(int min, int max);

  int Min() const;
  int Max() const;

 private:
  ValidCount(int min, int max);

  int min_;
  int max_;
};

// Validates the count of dictionaries found by ForEachDictionaryCall
class CountValidator {
 public:
  explicit CountValidator(const ValidCount& valid_count);

  bool IncreaseCount(const std::string& key, base::string16* error);
  bool SumUp(const std::string& key, base::string16* error) const;

 private:
  bool Validate(const std::string& key, base::string16* error) const;

  ValidCount valid_count_;
  int count_;
};

// Helper function for ForEachDictionaryCall. Do not use directly.
template <typename HandlerType, typename HandlerDataType>
bool ForDictionaryCall(const base::Value& value, const std::string& key,
    HandlerType handler, HandlerDataType* handler_data,
    CountValidator* count_validator, base::string16* error) {
  DCHECK(handler_data);
  DCHECK(count_validator);

  if (!count_validator->IncreaseCount(key, error))
    return false;

  const base::DictionaryValue* inner_dict;
  if (!value.GetAsDictionary(&inner_dict)) {
    SafeSetError(kErrMsgInvalidDictionary, key, error);
    return false;
  }

  if (!handler(*inner_dict, key, handler_data, error))
    return false;

  return true;
}

// A helper function calling 'handler' for each dictionary contained
// in 'dict' under a 'key'. This helper function takes two template arguments:
//  - a function with following prototype:
//    bool HandlerExample(const base::Value& value, const std::string& key,
//        HandlerDataType* handler_data, base::string16* error);
//  - a HandlerDataType object where the above function stores data
template <typename HandlerType, typename HandlerDataType>
bool ForEachDictionaryCall(const base::DictionaryValue& dict,
    const std::string& key, const ValidCount& valid_count, HandlerType handler,
    HandlerDataType* handler_data, base::string16* error) {
  DCHECK(handler_data);

  CountValidator count_validator(valid_count);

  const base::Value* value = nullptr;
  if (dict.Get(key, &value) && value) {
    if (value->IsType(base::Value::TYPE_DICTIONARY)) {
      if (!ForDictionaryCall(*value, key, handler, handler_data,
          &count_validator, error))
        return false;
    } else if (value->IsType(base::Value::TYPE_LIST)) {
      const base::ListValue* list;
      if (!value->GetAsList(&list)) {
        SafeSetError(kErrMsgInvalidList, key, error);
        return false;
      }
      for (const base::Value* value : *list) {
        if (!ForDictionaryCall(*value, key, handler, handler_data,
            &count_validator, error))
          return false;
      }
    }
  }

  return count_validator.SumUp(key, error);
}

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_MANIFEST_HANDLERS_MANIFEST_HANDLERS_UTIL_H_
