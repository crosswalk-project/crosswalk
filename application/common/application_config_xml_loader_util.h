// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_COMMON_APPLICATION_CONFIG_XML_LOADER_UTIL_H_
#define XWALK_APPLICATION_COMMON_APPLICATION_CONFIG_XML_LOADER_UTIL_H_

#include <string>

#include "base/strings/string_util.h"
#include "base/values.h"
#include "third_party/libxml/src/include/libxml/tree.h"

namespace xwalk {
namespace application {
namespace config_xml_loader {
extern const char kAttributePrefix[];
extern const char kNamespaceKey[];
extern const char kNamespaceNameConnectKey[];
extern const char kTextKey[];

// Element name keys.
extern const xmlChar kWidgetNameKey[];
extern const xmlChar kNameNameKey[];
extern const xmlChar kDescriptionNameKey[];
extern const xmlChar kAuthorNameKey[];
extern const xmlChar kLicenseNameKey[];
extern const xmlChar kIconNameKey[];
extern const xmlChar kContentNameKey[];
extern const xmlChar kFeatureNameKey[];
extern const xmlChar kPreferenceNameKey[];
extern const xmlChar kParamNameKey[];
extern const xmlChar kSpanNameKey[];

// Namespace value.
extern const xmlChar kNamespaceValue[];

// Attribute keys.
extern const xmlChar kAttributeDirKey[];
extern const xmlChar kAttributeLangKey[];
extern const xmlChar kAttributeIdKey[];
extern const xmlChar kAttributeVersionKey[];
extern const xmlChar kAttributeHeightKey[];
extern const xmlChar kAttributeWidthKey[];
extern const xmlChar kAttributeViewmodesKey[];
extern const xmlChar kAttributeDefaultlocaleKey[];
extern const xmlChar kAttributeShortKey[];
extern const xmlChar kAttributeHrefKey[];
extern const xmlChar kAttributeEmailKey[];
extern const xmlChar kAttributeSrcKey[];
extern const xmlChar kAttributeTypeKey[];
extern const xmlChar kAttributeEncodingKey[];
extern const xmlChar kAttributeRequiredKey[];
extern const xmlChar kAttributeNameKey[];
extern const xmlChar kAttributeValueKey[];
extern const xmlChar kAttributeReadonlyKey[];

inline std::string ToString(const xmlChar* string_ptr) {
  if (!string_ptr)
    return base::EmptyString();
  return std::string(reinterpret_cast<const char*>(string_ptr));
}

// Get a dir text, and this text is from node(maybe a attribute)
base::string16 GetDirText(const base::string16& text, xmlNodePtr node);

// Get a node text when the node may support span and dir.
// For example :
// <name dir="ltr">n<span dir="lro">a<span dir="rlo">m</span></span>e</name>
// we will get value => "n[LRO]a[RLO]m[PDF][PDF]e"
base::string16 GetNodeText(xmlNodePtr root);

// Insert an DictionaryValue of element to parent element DictionaryValue,
// if the element is already exist(same name) and can_be_list is true, we will
// have a ListValue in element_name dictionary of parent element, or if
// can_be_list if false, the element will be ignored.
bool InsertElement(base::DictionaryValue* parent_value,
                   xmlNodePtr node,
                   base::DictionaryValue* element_value,
                   bool can_be_list);

// Load extensions XML node into Dictionary structure.
base::DictionaryValue* LoadExtensionsXMLNode(xmlNode* root);

base::string16 GetAttribute(xmlNodePtr node,
                            const xmlChar* key,
                            bool is_dir_attr = false);

enum AttributeType {
  NORMAL,
  DIR,
  IRI
};

bool SetAttribute(base::DictionaryValue* element_value,
                  xmlNodePtr node,
                  const xmlChar* key,
                  AttributeType attribute_type = NORMAL);

}  // namespace config_xml_loader
}  // namespace application
}  // namespace xwalk


#endif  // XWALK_APPLICATION_COMMON_APPLICATION_CONFIG_XML_LOADER_UTIL_H_
