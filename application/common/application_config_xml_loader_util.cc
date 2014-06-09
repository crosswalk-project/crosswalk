// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/application_config_xml_loader_util.h"

#include "base/i18n/rtl.h"
#include "base/strings/string16.h"
#include "base/strings/utf_string_conversions.h"
#include "url/gurl.h"

namespace xwalk {
namespace application {
namespace {
const char kDirLTRValue[] = "ltr";
const char kDirRTLValue[] = "rtl";
const char kDirLROValue[] = "lro";
const char kDirRLOValue[] = "rlo";
const base::char16 kLRE = base::i18n::kLeftToRightEmbeddingMark;
const base::char16 kRLE = base::i18n::kRightToLeftEmbeddingMark;
const base::char16 kLRO = base::i18n::kLeftToRightOverride;
const base::char16 kRLO = base::i18n::kRightToLeftOverride;
const base::char16 kPDF = base::i18n::kPopDirectionalFormatting;
}  // namespace
namespace config_xml_loader {
const char kAttributePrefix[] = "@";
const char kNamespaceKey[] = "@namespace";
const char kNamespaceNameConnectKey[] = ":";
const char kTextKey[] = "#text";

// Element name keys.
const xmlChar kWidgetNameKey[] = "widget";
const xmlChar kNameNameKey[] = "name";
const xmlChar kDescriptionNameKey[] = "description";
const xmlChar kAuthorNameKey[] = "author";
const xmlChar kLicenseNameKey[] = "license";
const xmlChar kIconNameKey[] = "icon";
const xmlChar kContentNameKey[] = "content";
const xmlChar kFeatureNameKey[] = "feature";
const xmlChar kPreferenceNameKey[] = "preference";
const xmlChar kParamNameKey[] = "param";
const xmlChar kSpanNameKey[] = "span";

// Namespace value.
const xmlChar kNamespaceValue[] = "http://www.w3.org/ns/widgets";

// Attribute keys.
const xmlChar kAttributeDirKey[] = "dir";
const xmlChar kAttributeLangKey[] = "lang";
const xmlChar kAttributeIdKey[] = "id";
const xmlChar kAttributeVersionKey[] = "version";
const xmlChar kAttributeHeightKey[] = "height";
const xmlChar kAttributeWidthKey[] = "width";
const xmlChar kAttributeViewmodesKey[] = "viewmodes";
const xmlChar kAttributeDefaultlocaleKey[] = "defaultlocale";
const xmlChar kAttributeShortKey[] = "short";
const xmlChar kAttributeHrefKey[] = "href";
const xmlChar kAttributeEmailKey[] = "email";
const xmlChar kAttributeSrcKey[] = "src";
const xmlChar kAttributeTypeKey[] = "type";
const xmlChar kAttributeEncodingKey[] = "encoding";
const xmlChar kAttributeRequiredKey[] = "required";
const xmlChar kAttributeNameKey[] = "name";
const xmlChar kAttributeValueKey[] = "value";
const xmlChar kAttributeReadonlyKey[] = "readonly";

inline base::string16 ToString16(const xmlChar* string_ptr) {
  return base::UTF8ToUTF16(ToString(string_ptr));
}

// To attritube key in DictionaryValue
inline std::string ToDVAttrKey(const xmlChar* string_ptr) {
  return kAttributePrefix + ToString(string_ptr);
}

// Get a dir text, and this text is from node(maybe a attribute)
base::string16 GetDirText(const base::string16& text, xmlNodePtr node) {
  std::string dir;
  for (; node; node = node->parent) {
    xmlChar* value_ptr = xmlGetProp(node, kAttributeDirKey);
    if (value_ptr) {
      dir = ToString(value_ptr);
      xmlFree(value_ptr);
      break;
    }
  }
  if (dir == kDirLTRValue)
    return kLRE + text + kPDF;
  if (dir == kDirRTLValue)
    return kRLE + text + kPDF;
  if (dir == kDirLROValue)
    return kLRO + text + kPDF;
  if (dir == kDirRLOValue)
    return kRLO + text + kPDF;
  return text;
}

// Get a node text when the node may support span and dir.
// For example :
// <name dir="ltr">n<span dir="lro">a<span dir="rlo">m</span></span>e</name>
// we will get value => "n[LRO]a[RLO]m[PDF][PDF]e"
base::string16 GetNodeText(xmlNodePtr root) {
  DCHECK(root);
  if (root->type != XML_ELEMENT_NODE)
    return base::string16();

  base::string16 text;
  for (xmlNode* node = root->children; node; node = node->next) {
    if (node->type == XML_TEXT_NODE || node->type == XML_CDATA_SECTION_NODE) {
      text = text + base::i18n::StripWrappingBidiControlCharacters(
                        ToString16(node->content));
    } else {
      text = text + GetNodeText(node);
    }
  }
  return GetDirText(text, root);
}

std::string GetNamespacePrefix(xmlNode* root) {
  DCHECK(root);
  if (!root->ns || !root->ns->prefix)
    return base::EmptyString();
  return ToString(root->ns->prefix) + kNamespaceNameConnectKey;
}

// Insert an DictionaryValue of element to parent element DictionaryValue,
// if the element is already exist(same name) and can_be_list is true, we will
// have a ListValue in element_name dictionary of parent element, or if
// can_be_list if false, the element will be ignored.
bool InsertElement(base::DictionaryValue* parent_value,
                   xmlNodePtr node,
                   base::DictionaryValue* element_value,
                   bool can_be_list) {
  if (!parent_value || !element_value || !node)
    return false;
  std::string element_name = ToString(node->name);
  if (!parent_value->HasKey(element_name)) {
    parent_value->Set(element_name, element_value);
    return true;
  } else if (!can_be_list) {
    return false;
  }

  base::Value* temp;
  parent_value->Get(element_name, &temp);
  DCHECK(temp);

  base::ListValue* list;
  base::DictionaryValue* dict;
  if (temp->GetAsList(&list)) {
    list->Append(element_value);
  } else if (temp->GetAsDictionary(&dict)) {
    list = new base::ListValue();
    list->Append(dict->DeepCopy());
    list->Append(element_value);
    parent_value->Set(element_name, list);
  }
  return true;
}

base::string16 NodeListGetString(xmlDocPtr doc, xmlNodePtr children) {
  xmlChar* value_ptr = xmlNodeListGetString(doc, children, 1);
  base::string16 value(ToString16(value_ptr));
  xmlFree(value_ptr);
  return value;
}

// Load extensions XML node into Dictionary structure.
base::DictionaryValue* LoadExtensionsXMLNode(xmlNode* root) {
  DCHECK(root);
  scoped_ptr<base::DictionaryValue> value(new base::DictionaryValue);
  if (root->type != XML_ELEMENT_NODE)
    return NULL;
  if (root->ns)
    value->SetString(kNamespaceKey, ToString(root->ns->href));
  for (xmlAttr* attr = root->properties; attr; attr = attr->next) {
    value->SetString(ToDVAttrKey(attr->name),
                     NodeListGetString(root->doc, attr->children));
  }
  for (xmlNode* node = root->children; node; node = node->next) {
    InsertElement(value.get(), node, LoadExtensionsXMLNode(node), true);
  }
  value->SetString(kTextKey, NodeListGetString(root->doc, root->children));
  return value.release();
}

base::string16 GetAttribute(xmlNodePtr node,
                            const xmlChar* key,
                            bool is_dir_attr) {
  xmlChar* value_ptr = xmlGetProp(node, key);
  base::string16 value(base::CollapseWhitespace(ToString16(value_ptr), false));
  xmlFree(value_ptr);
  if (is_dir_attr)
    return GetDirText(value, node);
  return value;
}

bool SetAttribute(base::DictionaryValue* element_value,
                  xmlNodePtr node,
                  const xmlChar* key,
                  AttributeType attribute_type) {
  base::string16 value(GetAttribute(node, key, attribute_type == DIR));
  if (value.empty() || (attribute_type == IRI && !GURL(value).is_valid()))
    return false;
  element_value->SetString(ToDVAttrKey(key), value);
  return true;
}
}  // namespace config_xml_loader
}  // namespace application
}  // namespace xwalk
