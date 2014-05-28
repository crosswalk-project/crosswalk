// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/application_config_xml_loader.h"

#include <set>

#include "base/files/file_path.h"
#include "base/file_util.h"
#include "base/strings/stringprintf.h"
#include "base/values.h"
#include "third_party/libxml/src/include/libxml/tree.h"
#include "xwalk/application/common/application_config_xml_loader_util.h"
#include "xwalk/application/common/application_manifest_constants.h"

namespace xwalk {
namespace application {

using namespace config_xml_loader; // NOLINT

base::DictionaryValue* ConfigXMLLoader::Load(const base::FilePath& xml_path,
                                             std::string* error) {
  Reset();
  xmlDoc * doc = NULL;
  doc = xmlReadFile(xml_path.MaybeAsASCII().c_str(), NULL, 0);
  if (doc == NULL) {
    *error = base::StringPrintf("%s",
        application_manifest_errors::kManifestUnreadable);
    return NULL;
  }
  root_ = xmlDocGetRootElement(doc);
  if (root_->type != XML_ELEMENT_NODE) {
    *error = "Root node of config.xml is not an element node.";
    return NULL;
  }
  if (xmlStrEqual(kWidgetNameKey, root_->name)) {
    *error = "Root node name of config.xml should be widget.";
    return NULL;
  }

  if (!LoadWidgetNamesapce()) {
    *error = error_;
    return NULL;
  }

  if (!LoadWidgetAttribute()) {
    *error = error_;
    return NULL;
  }

  for (xmlNodePtr node = root_->children; node; node = node->next) {
    if (!LoadWidgetEachChildElement(node))
      InsertElement(widget_.get(), node, LoadExtensionsXMLNode(node), true);
  }

  scoped_ptr<base::DictionaryValue> result(new base::DictionaryValue);
  result->Set(ToString(root_->name), widget_.release());
  return result.release();
}

void ConfigXMLLoader::Reset() {
  error_.clear();
  root_ = NULL;
  widget_.reset(new base::DictionaryValue);
  preference_names_.clear();
}

bool ConfigXMLLoader::LoadWidgetNamesapce() {
  for (xmlNsPtr ns = root_->ns; ns; ns= ns->next) {
    if (xmlStrEqual(kNamespaceValue, ns->href))
      return true;
  }
  error_ = base::StringPrintf("There is no namespace %s.", kNamespaceValue);
  return false;
}

bool ConfigXMLLoader::LoadWidgetAttribute() {
  SetAttribute(widget_.get(), root_, kAttributeDefaultlocaleKey);
  SetAttribute(widget_.get(), root_, kAttributeIdKey, IRI);
  SetAttribute(widget_.get(), root_, kAttributeVersionKey, DIR);
  SetAttribute(widget_.get(), root_, kAttributeHeightKey);
  SetAttribute(widget_.get(), root_, kAttributeWidthKey);
  SetAttribute(widget_.get(), root_, kAttributeViewmodesKey);
  return true;
}

bool ConfigXMLLoader::LoadWidgetEachChildElement(xmlNodePtr node) {
  scoped_ptr<base::DictionaryValue> element(new base::DictionaryValue);

  if (xmlStrEqual(kNameNameKey, node->name)) {
    SetAttribute(element.get(), node, kAttributeLangKey);
    SetAttribute(element.get(), node, kAttributeShortKey, DIR);
    element->SetString(
        kTextKey, base::CollapseWhitespace(GetNodeText(node), false));
    InsertElement(widget_.get(), node, element.release(), true);
    return true;
  }

  if (xmlStrEqual(kDescriptionNameKey, node->name)) {
    SetAttribute(element.get(), node, kAttributeLangKey);
    element->SetString(kTextKey, GetNodeText(node));
    InsertElement(widget_.get(), node, element.release(), true);
    return true;
  }

  if (xmlStrEqual(kAuthorNameKey, node->name)) {
    SetAttribute(element.get(), node, kAttributeHrefKey, IRI);
    SetAttribute(element.get(), node, kAttributeEmailKey);
    element->SetString(
        kTextKey, base::CollapseWhitespace(GetNodeText(node), false));
    InsertElement(widget_.get(), node, element.release(), false);
    return true;
  }

  if (xmlStrEqual(kLicenseNameKey, node->name)) {
    SetAttribute(element.get(), node, kAttributeHrefKey, IRI);
    element->SetString(kTextKey, GetNodeText(node));
    InsertElement(widget_.get(), node, element.release(), false);
    return true;
  }

  if (xmlStrEqual(kIconNameKey, node->name)) {
    if (!SetAttribute(element.get(), node, kAttributeSrcKey))
      return true;
    SetAttribute(element.get(), node, kAttributeWidthKey);
    SetAttribute(element.get(), node, kAttributeHeightKey);
    InsertElement(widget_.get(), node, element.release(), true);
    return true;
  }

  if (xmlStrEqual(kContentNameKey, node->name)) {
    if (!SetAttribute(element.get(), node, kAttributeSrcKey))
      return true;
    SetAttribute(element.get(), node, kAttributeTypeKey);
    SetAttribute(element.get(), node, kAttributeEncodingKey);
    InsertElement(widget_.get(), node, element.release(), false);
    return true;
  }

  if (xmlStrEqual(kFeatureNameKey, node->name)) {
    // TODO(hongzhang) : Load feature element according to widget_ process
    // rules.
    InsertElement(widget_.get(), node, LoadExtensionsXMLNode(node), true);
    return true;
  }

  if (xmlStrEqual(kPreferenceNameKey, node->name)) {
    base::string16 preference_name = GetAttribute(node, kAttributeNameKey);
    if (preference_names_.find(preference_name) != preference_names_.end())
      return true;
    SetAttribute(element.get(), node, kAttributeNameKey);
    SetAttribute(element.get(), node, kAttributeValueKey);
    SetAttribute(element.get(), node, kAttributeReadonlyKey);
    InsertElement(widget_.get(), node, element.release(), true);
    return false;
  }

  return false;
}

}  // namespace application
}  // namespace xwalk
