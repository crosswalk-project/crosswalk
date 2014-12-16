// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/application_file_util.h"

#include <algorithm>
#include <map>
#include <vector>

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/i18n/rtl.h"
#include "base/json/json_file_value_serializer.h"
#include "base/logging.h"
#include "base/metrics/histogram.h"
#include "base/path_service.h"
#include "base/strings/string16.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/thread_restrictions.h"
#include "net/base/escape.h"
#include "net/base/file_stream.h"
#include "third_party/libxml/src/include/libxml/tree.h"
#include "ui/base/l10n/l10n_util.h"
#include "xwalk/application/common/application_data.h"
#include "xwalk/application/common/application_manifest_constants.h"
#include "xwalk/application/common/constants.h"
#include "xwalk/application/common/manifest.h"
#include "xwalk/application/common/manifest_handler.h"

#if defined(OS_TIZEN)
#include "xwalk/application/common/id_util.h"
#endif

namespace errors = xwalk::application_manifest_errors;
namespace keys = xwalk::application_manifest_keys;
namespace widget_keys = xwalk::application_widget_keys;

namespace {
const char kAttributePrefix[] = "@";
const char kNamespaceKey[] = "@namespace";
const char kTextKey[] = "#text";

const char kContentKey[] = "content";

const xmlChar kWidgetNodeKey[] = "widget";
const xmlChar kNameNodeKey[] = "name";
const xmlChar kDescriptionNodeKey[] = "description";
const xmlChar kAuthorNodeKey[] = "author";
const xmlChar kLicenseNodeKey[] = "license";
const xmlChar kVersionAttributeKey[] = "version";
const xmlChar kShortAttributeKey[] = "short";
const xmlChar kDirAttributeKey[] = "dir";

const char kDirLTRKey[] = "ltr";
const char kDirRTLKey[] = "rtl";
const char kDirLROKey[] = "lro";
const char kDirRLOKey[] = "rlo";

const char* kSingletonElements[] = {
  "allow-navigation",
  "author",
  "content-security-policy-report-only",
  "content-security-policy",
  "content"
};

inline char* ToCharPointer(void* ptr) {
  return reinterpret_cast<char *>(ptr);
}

inline const char* ToConstCharPointer(const void* ptr) {
  return reinterpret_cast<const char*>(ptr);
}

base::string16 ToSting16(const xmlChar* string_ptr) {
  return base::UTF8ToUTF16(std::string(ToConstCharPointer(string_ptr)));
}

base::string16 GetDirText(const base::string16& text, const std::string& dir) {
  if (dir == kDirLTRKey)
    return base::i18n::kLeftToRightEmbeddingMark
           + text
           + base::i18n::kPopDirectionalFormatting;

  if (dir == kDirRTLKey)
    return base::i18n::kRightToLeftEmbeddingMark
           + text
           + base::i18n::kPopDirectionalFormatting;

  if (dir == kDirLROKey)
    return base::i18n::kLeftToRightOverride
           + text
           + base::i18n::kPopDirectionalFormatting;

  if (dir == kDirRLOKey)
    return base::i18n::kRightToLeftOverride
           + text
           + base::i18n::kPopDirectionalFormatting;

  return text;
}

std::string GetNodeDir(xmlNode* node, const std::string& inherit_dir) {
  DCHECK(node);
  std::string dir(inherit_dir);

  xmlAttr* prop = NULL;
  for (prop = node->properties; prop; prop = prop->next) {
    if (xmlStrEqual(prop->name, kDirAttributeKey)) {
      char* prop_value = ToCharPointer(xmlNodeListGetString(
          node->doc, prop->children, 1));
      dir = prop_value;
      xmlFree(prop_value);
      break;
    }
  }

  return dir;
}

base::string16 GetNodeText(xmlNode* root, const std::string& inherit_dir) {
  DCHECK(root);
  if (root->type != XML_ELEMENT_NODE)
    return base::string16();

  std::string current_dir(GetNodeDir(root, inherit_dir));
  base::string16 text;
  for (xmlNode* node = root->children; node; node = node->next) {
    if (node->type == XML_TEXT_NODE || node->type == XML_CDATA_SECTION_NODE) {
      text = text + base::i18n::StripWrappingBidiControlCharacters(
                        ToSting16(node->content));
    } else {
      text = text + GetNodeText(node, current_dir);
    }
  }
  return GetDirText(text, current_dir);
}

// According to widget specification, this two prop need to support dir.
// see detail on http://www.w3.org/TR/widgets/#the-dir-attribute
inline bool IsPropSupportDir(xmlNode* root, xmlAttr* prop) {
  if (xmlStrEqual(root->name, kWidgetNodeKey)
     && xmlStrEqual(prop->name, kVersionAttributeKey))
    return true;
  if (xmlStrEqual(root->name, kNameNodeKey)
     && xmlStrEqual(prop->name, kShortAttributeKey))
    return true;
  return false;
}

// Only this four items need to support span and ignore other element.
// Besides xmlNodeListGetString can not support dir prop of span.
// See http://www.w3.org/TR/widgets/#the-span-element-and-its-attributes
inline bool IsElementSupportSpanAndDir(xmlNode* root) {
  if (xmlStrEqual(root->name, kNameNodeKey)
     || xmlStrEqual(root->name, kDescriptionNodeKey)
     || xmlStrEqual(root->name, kAuthorNodeKey)
     || xmlStrEqual(root->name, kLicenseNodeKey))
    return true;
  return false;
}

bool IsSingletonElement(const std::string& name) {
  for (int i = 0; i < arraysize(kSingletonElements); ++i)
    if (kSingletonElements[i] == name)
#if defined(OS_TIZEN)
      // On Tizen platform, need to check namespace of 'content'
      // element further, a content element with tizen namespace
      // will replace the one with widget namespace.
      return name != kContentKey;
#else
      return true;
#endif
  return false;
}

}  // namespace

namespace xwalk {
namespace application {

FileDeleter::FileDeleter(const base::FilePath& path, bool recursive)
    : path_(path),
      recursive_(recursive) {}

FileDeleter::~FileDeleter() {
  base::DeleteFile(path_, recursive_);
}

namespace {

// Load XML node into Dictionary structure.
// The keys for the XML node to Dictionary mapping are described below:
// XML                                 Dictionary
// <e></e>                             "e":{"#text": ""}
// <e>textA</e>                        "e":{"#text":"textA"}
// <e attr="val">textA</e>             "e":{ "@attr":"val", "#text": "textA"}
// <e> <a>textA</a> <b>textB</b> </e>  "e":{
//                                       "a":{"#text":"textA"}
//                                       "b":{"#text":"textB"}
//                                     }
// <e> <a>textX</a> <a>textY</a> </e>  "e":{
//                                       "a":[ {"#text":"textX"},
//                                             {"#text":"textY"}]
//                                     }
// <e> textX <a>textY</a> </e>         "e":{ "#text":"textX",
//                                           "a":{"#text":"textY"}
//                                     }
//
// For elements that are specified under a namespace, the dictionary
// will add '@namespace' key for them, e.g.,
// XML:
// <e xmln="linkA" xmlns:N="LinkB">
//   <sub-e1> text1 </sub-e>
//   <N:sub-e2 text2 />
// </e>
// will be saved in Dictionary as,
// "e":{
//   "#text": "",
//   "@namespace": "linkA"
//   "sub-e1": {
//     "#text": "text1",
//     "@namespace": "linkA"
//   },
//   "sub-e2": {
//     "#text":"text2"
//     "@namespace": "linkB"
//   }
// }
base::DictionaryValue* LoadXMLNode(
    xmlNode* root, const std::string& inherit_dir = "") {
  scoped_ptr<base::DictionaryValue> value(new base::DictionaryValue);
  if (root->type != XML_ELEMENT_NODE)
    return NULL;

  std::string current_dir(GetNodeDir(root, inherit_dir));

  xmlAttr* prop = NULL;
  for (prop = root->properties; prop; prop = prop->next) {
    xmlChar* value_ptr = xmlNodeListGetString(root->doc, prop->children, 1);
    base::string16 prop_value(ToSting16(value_ptr));
    xmlFree(value_ptr);

    if (IsPropSupportDir(root, prop))
      prop_value = GetDirText(prop_value, current_dir);

    value->SetString(
        std::string(kAttributePrefix) + ToConstCharPointer(prop->name),
        prop_value);
  }

  if (root->ns)
    value->SetString(kNamespaceKey, ToConstCharPointer(root->ns->href));

  for (xmlNode* node = root->children; node; node = node->next) {
    std::string sub_node_name(ToConstCharPointer(node->name));
    base::DictionaryValue* sub_value = LoadXMLNode(node, current_dir);
    if (!sub_value)
      continue;

    if (!value->HasKey(sub_node_name)) {
      value->Set(sub_node_name, sub_value);
      continue;
    } else if (IsSingletonElement(sub_node_name)) {
      continue;
#if defined(OS_TIZEN)
    } else if (sub_node_name == kContentKey) {
      std::string current_namespace, new_namespace;
      base::DictionaryValue* current_value;
      value->GetDictionary(sub_node_name, &current_value);

      current_value->GetString(kNamespaceKey, &current_namespace);
      sub_value->GetString(kNamespaceKey, &new_namespace);
      if (current_namespace != new_namespace &&
          new_namespace == widget_keys::kTizenNamespacePrefix)
        value->Set(sub_node_name, sub_value);
      continue;
#endif
    }

    base::Value* temp;
    value->Get(sub_node_name, &temp);
    DCHECK(temp);

    if (temp->IsType(base::Value::TYPE_LIST)) {
      base::ListValue* list;
      temp->GetAsList(&list);
      list->Append(sub_value);
    } else {
      DCHECK(temp->IsType(base::Value::TYPE_DICTIONARY));
      base::DictionaryValue* dict;
      temp->GetAsDictionary(&dict);
      base::DictionaryValue* prev_value = dict->DeepCopy();

      base::ListValue* list = new base::ListValue();
      list->Append(prev_value);
      list->Append(sub_value);
      value->Set(sub_node_name, list);
    }
  }

  base::string16 text;
  if (IsElementSupportSpanAndDir(root)) {
    text = GetNodeText(root, current_dir);
  } else {
    xmlChar* text_ptr = xmlNodeListGetString(root->doc, root->children, 1);
    if (text_ptr) {
      text = ToSting16(text_ptr);
      xmlFree(text_ptr);
    }
  }

  if (!text.empty())
    value->SetString(kTextKey, text);

  return value.release();
}

}  // namespace

template <Manifest::Type>
scoped_ptr<Manifest> LoadManifest(
    const base::FilePath& manifest_path, std::string* error);

template <>
scoped_ptr<Manifest> LoadManifest<Manifest::TYPE_MANIFEST>(
    const base::FilePath& manifest_path, std::string* error) {
  JSONFileValueSerializer serializer(manifest_path);
  scoped_ptr<base::Value> root(serializer.Deserialize(NULL, error));
  if (!root) {
    if (error->empty()) {
      // If |error| is empty, than the file could not be read.
      // It would be cleaner to have the JSON reader give a specific error
      // in this case, but other code tests for a file error with
      // error->empty().  For now, be consistent.
      *error = base::StringPrintf("%s", errors::kManifestUnreadable);
    } else {
      *error = base::StringPrintf("%s  %s",
          errors::kManifestParseError, error->c_str());
    }
    return scoped_ptr<Manifest>();
  }

  if (!root->IsType(base::Value::TYPE_DICTIONARY)) {
    *error = base::StringPrintf("%s", errors::kManifestUnreadable);
    return scoped_ptr<Manifest>();
  }

  scoped_ptr<base::DictionaryValue> dv = make_scoped_ptr(
      static_cast<base::DictionaryValue*>(root.release()));
#if defined(OS_TIZEN)
  // Ignore any Tizen application ID, as this is automatically generated.
  dv->Remove(keys::kTizenAppIdKey, NULL);
#endif

  return make_scoped_ptr(new Manifest(dv.Pass(), Manifest::TYPE_MANIFEST));
}

template <>
scoped_ptr<Manifest> LoadManifest<Manifest::TYPE_WIDGET>(
    const base::FilePath& manifest_path,
    std::string* error) {
  xmlDoc * doc = NULL;
  xmlNode* root_node = NULL;
  doc = xmlReadFile(manifest_path.MaybeAsASCII().c_str(), NULL, 0);
  if (doc == NULL) {
    *error = base::StringPrintf("%s", errors::kManifestUnreadable);
    return scoped_ptr<Manifest>();
  }
  root_node = xmlDocGetRootElement(doc);
  base::DictionaryValue* dv = LoadXMLNode(root_node);
  scoped_ptr<base::DictionaryValue> result(new base::DictionaryValue);
  if (dv)
    result->Set(ToConstCharPointer(root_node->name), dv);

  return make_scoped_ptr(new Manifest(result.Pass(), Manifest::TYPE_WIDGET));
}

scoped_ptr<Manifest> LoadManifest(const base::FilePath& manifest_path,
    Manifest::Type type, std::string* error) {
  if (type == Manifest::TYPE_MANIFEST)
    return LoadManifest<Manifest::TYPE_MANIFEST>(manifest_path, error);

  if (type == Manifest::TYPE_WIDGET)
    return LoadManifest<Manifest::TYPE_WIDGET>(manifest_path, error);

  *error = base::StringPrintf("%s", errors::kManifestUnreadable);
  return scoped_ptr<Manifest>();
}

base::FilePath GetManifestPath(
    const base::FilePath& app_directory, Manifest::Type type) {
  base::FilePath manifest_path;
  switch (type) {
    case Manifest::TYPE_WIDGET:
      manifest_path = app_directory.Append(kManifestWgtFilename);
      break;
    case Manifest::TYPE_MANIFEST:
      manifest_path = app_directory.Append(kManifestXpkFilename);
      break;
    default:
      NOTREACHED();
  }

  return manifest_path;
}

scoped_refptr<ApplicationData> LoadApplication(
    const base::FilePath& app_root, const std::string& app_id,
    ApplicationData::SourceType source_type, Manifest::Type manifest_type,
    std::string* error) {
  base::FilePath manifest_path = GetManifestPath(app_root, manifest_type);

  scoped_ptr<Manifest> manifest = LoadManifest(
      manifest_path, manifest_type, error);
  if (!manifest)
    return NULL;

  return ApplicationData::Create(
      app_root, app_id, source_type, manifest.Pass(), error);
}

base::FilePath ApplicationURLToRelativeFilePath(const GURL& url) {
  std::string url_path = url.path();
  if (url_path.empty() || url_path[0] != '/')
    return base::FilePath();

  // Drop the leading slashes and convert %-encoded UTF8 to regular UTF8.
  std::string file_path = net::UnescapeURLComponent(url_path,
      net::UnescapeRule::SPACES | net::UnescapeRule::URL_SPECIAL_CHARS);
  size_t skip = file_path.find_first_not_of("/\\");
  if (skip != file_path.npos)
    file_path = file_path.substr(skip);

  base::FilePath path =
#if defined(OS_POSIX)
    base::FilePath(file_path);
#elif defined(OS_WIN)
    base::FilePath(base::UTF8ToWide(file_path));
#else
    base::FilePath();
    NOTIMPLEMENTED();
#endif

  // It's still possible for someone to construct an annoying URL whose path
  // would still wind up not being considered relative at this point.
  // For example: app://id/c:////foo.html
  if (path.IsAbsolute())
    return base::FilePath();

  return path;
}

}  // namespace application
}  // namespace xwalk
