// Copyright 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/tizen/signature_parser.h"

#include <list>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include "base/files/file_util.h"
#include "base/logging.h"
#include "libxml/parser.h"
#include "libxml/xmlschemas.h"
#include "libxml/xpathInternals.h"
#include "libxml/xmlreader.h"
#include "third_party/libxml/chromium/libxml_utils.h"

namespace {
const char kExpectedXmlns[] = "http://www.w3.org/2000/09/xmldsig#";
// TAG TOKENS
const char kTokenSignature[] = "Signature";
const char kTokenSignedInfo[] = "SignedInfo";
const char kTokenCanonicalizationMethod[] = "CanonicalizationMethod";
const char kTokenSignatureMethod[] = "SignatureMethod";
const char kTokenReference[] = "Reference";
const char kTokenTransforms[] = "Transforms";
const char kTokenTransform[] = "Transform";
const char kTokenDigestMethod[] = "DigestMethod";
const char kTokenDigestValue[] = "DigestValue";
const char kTokenSignatureValue[] = "SignatureValue";
const char kTokenkeyInfo[] = "KeyInfo";
const char kTokenX509Data[] = "X509Data";
const char kTokenX509Certificate[] = "X509Certificate";
const char kTokenObject[] = "Object";
const char kTokenSignatureProperties[] = "SignatureProperties";
const char kTokenSignatureProperty[] = "SignatureProperty";

// ATTRIBUTE TOKENS
const char kTokenAlgorithm[] = "Algorithm";
const char kTokenURI[] = "URI";
const char kTokenID[] = "Id";

// ATTRIBUTE VALUES
const char kTokenAttrProfile[] = "profile";
const char kTokenAttrRole[] = "role";
const char kTokenAttrIdentifier[] = "identifier";

bool TagNameEquals(const xmlNodePtr node,
    const char* expected_name, const xmlNsPtr expected_namespace) {
  if (node->ns != expected_namespace)
    return false;

  return 0 == strcmp(expected_name, reinterpret_cast<const char*>(node->name));
}

// Returns child nodes of |root| with name |name|.
std::vector<xmlNodePtr> GetChildren(
    const xmlNodePtr root, const xmlNsPtr xml_namespace, const char* name) {
  std::vector<xmlNodePtr> result;
  for (xmlNodePtr child = root->children; child != NULL; child = child->next) {
    if (!TagNameEquals(child, name, xml_namespace))
      continue;

    result.push_back(child);
  }
  return result;
}

// Returns the first child node of |root| with name |name|.
xmlNodePtr GetFirstChild(
    const xmlNodePtr root, const xmlNsPtr xml_namespace, const char* name) {
  xmlNodePtr result = NULL;
  for (xmlNodePtr child = root->children; child != NULL; child = child->next) {
    if (TagNameEquals(child, name, xml_namespace)) {
      result = child;
      break;
    }
  }
  return result;
}

// Returns the value of a named attribute, or the empty string.
std::string GetAttribute(
    const xmlNodePtr node, const char* attribute_name) {
  const xmlChar* name =
    reinterpret_cast<const xmlChar*>(attribute_name);
  for (xmlAttr* attr = node->properties; attr != NULL; attr = attr->next) {
    if (!xmlStrcmp(attr->name, name) && attr->children &&
        attr->children->content)
      return std::string(reinterpret_cast<const char*>(
            attr->children->content));
  }
  return std::string();
}

// Returns a pointer to the xmlNs on |node| with the |expected_href|, or
// NULL if there isn't one with that href.
xmlNsPtr GetNamespace(const xmlNodePtr node, const char* expected_href) {
  const xmlChar* href = reinterpret_cast<const xmlChar*>(expected_href);
  for (xmlNsPtr ns = node->ns; ns != NULL; ns = ns->next) {
    if (ns->href && !xmlStrcmp(ns->href, href))
      return ns;
  }
  return NULL;
}

}  // namespace

namespace xwalk {
namespace application {
bool ParseSignedInfoElement(
    const xmlNodePtr node, const xmlNsPtr signature_ns, SignatureData* data) {
  xmlNodePtr signed_info_node =
    GetFirstChild(node, signature_ns, kTokenSignedInfo);
  if (!signed_info_node) {
    LOG(ERROR) << "Missing SignedInfo tag.";
    return false;
  }

  // Parse <CanonicalizationMethod>
  xmlNodePtr canonicalization_method_node =
    GetFirstChild(signed_info_node, signature_ns, kTokenCanonicalizationMethod);
  if (!canonicalization_method_node) {
    LOG(ERROR) << "Missing SignedInfo tag.";
    return false;
  }
  std::string canonicalization_method =
    GetAttribute(canonicalization_method_node, kTokenAlgorithm);
  data->set_canonicalization_method(canonicalization_method);

  // Parse <SignatureMethod>
  xmlNodePtr signature_method_node =
    GetFirstChild(signed_info_node, signature_ns, kTokenSignatureMethod);
  if (!signature_method_node) {
    LOG(ERROR) << "Missing SignatureMethod tag.";
    return false;
  }
  std::string signature_method =
    GetAttribute(signature_method_node, kTokenAlgorithm);
  data->set_signature_method(signature_method);

  // Parse <Reference>
  std::vector<xmlNodePtr> reference_vec =
    GetChildren(signed_info_node, signature_ns, kTokenReference);
  if (reference_vec.empty()) {
    LOG(ERROR) << "Missing Reference tag.";
    return false;
  }

  std::string uri;
  xmlNodePtr refer_node;
  std::set<std::string> reference_set;
  for (size_t i = 0; i < reference_vec.size(); ++i) {
    refer_node = reference_vec[i];
    uri = GetAttribute(refer_node, kTokenURI);
    if (uri.empty()) {
      LOG(ERROR) << "Missing URI attribute.";
      return false;
    }
    reference_set.insert(uri);
  }
  data->set_reference_set(reference_set);

  return true;
}

bool ParseSignatureValueElement(
    const xmlNodePtr node, const xmlNsPtr ns, SignatureData* data) {
  xmlNodePtr sign_value_node = GetFirstChild(node, ns, kTokenSignatureValue);
  if (!sign_value_node) {
    LOG(ERROR) << "Missing SignatureValue tag.";
    return false;
  }
  std::string signature_value = XmlStringToStdString(
      xmlNodeGetContent(sign_value_node));
  data->set_signature_value(signature_value);
  return true;
}

bool ParseKeyInfoElement(
    const xmlNodePtr node, const xmlNsPtr ns, SignatureData* data) {
  xmlNodePtr key_info_node = GetFirstChild(node, ns, kTokenkeyInfo);
  if (!key_info_node) {
    LOG(INFO) << "Missing KeyInfo tag, it is allowed by schema.xsd.";
    return true;
  }

  // KeyInfo may contain keys, names, certificates and other public key
  // management. Now I only handle X509 certifcates which is commonly used.
  // TODO(Xu): Other types of element
  xmlNodePtr X509_data_node =
    GetFirstChild(key_info_node, ns, kTokenX509Data);
  if (!X509_data_node) {
    LOG(INFO) << "Missing X509Data tag.";
    return true;
  }

  // Parse <X509Certificate>
  std::vector<xmlNodePtr> cert_vec =
    GetChildren(X509_data_node, ns, kTokenX509Certificate);
  if (cert_vec.empty()) {
    LOG(ERROR) << "Missing X509Certificate tag.";
    return false;
  }

  std::list<std::string> certificate_list;
  for (std::vector<xmlNode*>::iterator it = cert_vec.begin();
      it != cert_vec.end(); ++it) {
    xmlNodePtr certificate_node = *it;
    certificate_list.push_back(
        XmlStringToStdString(xmlNodeGetContent(certificate_node)));
  }
  data->set_certificate_list(certificate_list);
  return true;
}

bool ParseObjectElement(
    const xmlNodePtr node, const xmlNsPtr ns, SignatureData* data) {
  xmlNodePtr object_node = GetFirstChild(node, ns, kTokenObject);
  if (!object_node) {
    LOG(ERROR) << "Missing Object tag.";
    return false;
  }

  std::string object_id = GetAttribute(object_node, kTokenID);
  data->set_object_id(object_id);
  // Parse <SignatureProperties>
  xmlNodePtr properties_node =
    GetFirstChild(object_node, ns, kTokenSignatureProperties);
  if (!properties_node) {
    LOG(ERROR) << "Missing Object tag.";
    return false;
  }

  std::vector<xmlNodePtr> prop_vec =
    GetChildren(properties_node, ns, kTokenSignatureProperty);
  std::string Id, uri, element_name, profile_uri, role_uri;
  xmlNodePtr sign_property_node, child;
  for (size_t i = 0; i < prop_vec.size(); i++) {
    sign_property_node = prop_vec[i];
    Id = GetAttribute(sign_property_node, kTokenID);
    child = sign_property_node->children;
    if (!child) {
      LOG(ERROR) << "Failing to find " << element_name
        << "  element.";
      return false;
    }

    if (Id.compare(kTokenAttrProfile) == 0) {
      profile_uri = GetAttribute(child, kTokenURI);
      data->set_profile_uri(profile_uri);
    }
    if (Id.compare(kTokenAttrRole) == 0) {
      role_uri = GetAttribute(child, kTokenURI);
      data->set_role_uri(role_uri);
    }
  }

  return true;
}

bool ParseXML(xmlDocPtr docPtr, SignatureData* data) {
  xmlNodePtr root = xmlDocGetRootElement(docPtr);
  if (!root) {
    LOG(ERROR) << "Missinging root node.";
    return false;
  }

  // Look for the required namespace declaration.
  xmlNsPtr signature_ns = GetNamespace(root, kExpectedXmlns);
  if (!signature_ns) {
    LOG(ERROR) << "Missinging or incorrect xmlns on signature tag.";
    return false;
  }
  if (!TagNameEquals(root, kTokenSignature, signature_ns)) {
    LOG(ERROR) << "Missinging Signature tag.";
    return false;
  }

  if (!ParseSignedInfoElement(root, signature_ns, data))
    return false;

  if (!ParseSignatureValueElement(root, signature_ns, data))
    return false;

  if (!ParseKeyInfoElement(root, signature_ns, data))
    return false;

  if (!ParseObjectElement(root, signature_ns, data))
    return false;

  return true;
}

// static
scoped_ptr<SignatureData> SignatureParser::CreateSignatureData(
    const base::FilePath& signature_path, int signature_number) {
  std::string file_name = signature_path.MaybeAsASCII();
  scoped_ptr<SignatureData>
    data(new SignatureData(file_name, signature_number));

  xmlInitParser();
  xmlDocPtr doc = xmlParseFile(file_name.c_str());
  if (!doc) {
    LOG(ERROR) << "Opening signature " << file_name << " failed.";
    return scoped_ptr<SignatureData>();
  }

  if (!ParseXML(doc, data.get())) {
    LOG(ERROR) << "Parsering failed.";
    xmlFreeDoc(doc);
    return scoped_ptr<SignatureData>();
  }

  xmlFreeDoc(doc);
  xmlCleanupParser();
  return data.Pass();
}
}  // namespace application
}  // namespace xwalk
