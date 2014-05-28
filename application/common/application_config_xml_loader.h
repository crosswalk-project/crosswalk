// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_APPLICATION_CONFIG_XML_LOADER_H_
#define XWALK_APPLICATION_COMMON_APPLICATION_CONFIG_XML_LOADER_H_

#include <string>
#include <set>

#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "base/strings/string16.h"
#include "base/values.h"
#include "third_party/libxml/src/include/libxml/tree.h"

namespace xwalk {
namespace application {
// Load config.xml into a DictionaryValue with rules according to
// http://www.w3.org/TR/widgets/#step-7-process-the-configuration-document
// For example config.xml
// <?xml version="1.0" encoding="UTF-8"?>
// <widget id="example:" xmlns="http://www.w3.org/ns/widgets"
//     xmlns:tizen="http://tizen.org/ns/widgets">
//  <name>English</name>
//  <name xml:lang="zh-cn">Chinese</name>
//  <author>example author</author>
//  <author>author is zero or one, so this element will be ignored</author>
//  <tizen:application id="UM93QrpfK6.example" package="UM93QrpfK6"
//     required_version="2.1"/>
// </widget>
// DictionaryValue :
// widget : {
//   "@namespace" : "http://www.w3.org/ns/widgets",
//   "@id" : "example:",
//   "name" : [
//     {
//       "#text" : "English"
//     },
//     {
//       "@lang" : "zh-cn"
//       "#text" : "Chinese"
//     }
//   ],
//   "author" : {
//     "#text" : "example author"
//   },
//   "tizen:application" {
//     "@namespace" : "http://tizen.org/ns/widgets",
//     "@id" : "UM93QrpfK6.example",
//     "@package" : "UM93QrpfK6",
//     "@required_version" : "2.1"
//   }
// }
// Note : all user defined element(for example <tizen:application>) will
// be a list if there are more than one element in document.
class ConfigXMLLoader {
 public:
  base::DictionaryValue* Load(const base::FilePath& xml_path,
                              std::string* error);

 protected:
  virtual void Reset();
  virtual bool LoadWidgetNamesapce();
  virtual bool LoadWidgetAttribute();
  virtual bool LoadWidgetEachChildElement(xmlNodePtr node);

  std::string error_;
  xmlNodePtr root_;
  scoped_ptr<base::DictionaryValue> widget_;
  std::set<base::string16> preference_names_;
};


}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_COMMON_APPLICATION_CONFIG_XML_LOADER_H_
