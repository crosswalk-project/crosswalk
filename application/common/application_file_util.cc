// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/application_file_util.h"

#include <algorithm>
#include <map>
#include <vector>

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/scoped_temp_dir.h"
#include "base/file_util.h"
#include "base/json/json_file_value_serializer.h"
#include "base/logging.h"
#include "base/metrics/histogram.h"
#include "base/path_service.h"
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
#include "xwalk/application/common/install_warning.h"
#include "xwalk/application/common/manifest.h"
#include "xwalk/application/common/manifest_handler.h"

namespace errors = xwalk::application_manifest_errors;
namespace keys = xwalk::application_manifest_keys;
namespace widget_keys = xwalk::application_widget_keys;

namespace {
const char kAttributePrefix[] = "@";
const char kNamespaceKey[] = "@namespace";
const char kTextKey[] = "#text";
}

namespace xwalk {
namespace application {

inline char* ToCharPointer(void* ptr) {
  return reinterpret_cast<char *>(ptr);
}

inline const char* ToConstCharPointer(const void* ptr) {
  return reinterpret_cast<const char*>(ptr);
}

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
base::DictionaryValue* LoadXMLNode(xmlNode* root) {
  scoped_ptr<base::DictionaryValue> value(new base::DictionaryValue);
  if (root->type != XML_ELEMENT_NODE)
    return NULL;

  xmlAttr* prop = NULL;
  for (prop = root->properties; prop; prop = prop->next) {
    char* prop_value = ToCharPointer(xmlNodeListGetString(
        root->doc, prop->children, 1));
    value->SetString(
        std::string(kAttributePrefix) + ToConstCharPointer(prop->name),
        prop_value);
    xmlFree(prop_value);
  }

  if (root->ns)
    value->SetString(kNamespaceKey, ToConstCharPointer(root->ns->href));

  for (xmlNode* node = root->children; node; node = node->next) {
    std::string sub_node_name(ToConstCharPointer(node->name));
    base::DictionaryValue* sub_value = LoadXMLNode(node);
    if (!sub_value)
      continue;

    if (!value->HasKey(sub_node_name)) {
      value->Set(sub_node_name, sub_value);
      continue;
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
      base::DictionaryValue* prev_value(new base::DictionaryValue());
      prev_value = dict->DeepCopy();

      base::ListValue* list = new base::ListValue();
      list->Append(prev_value);
      list->Append(sub_value);
      value->Set(sub_node_name, list);
    }
  }

  char* text = ToCharPointer(
      xmlNodeListGetString(root->doc, root->children, 1));
  if (!text) {
    value->SetString(kTextKey, std::string());
  } else {
    value->SetString(kTextKey, text);
  }
  xmlFree(text);

  return value.release();
}

scoped_refptr<ApplicationData> LoadApplication(
    const base::FilePath& application_path,
    Manifest::SourceType source_type,
    std::string* error) {
  return LoadApplication(application_path, std::string(),
                         source_type, error);
}

scoped_refptr<ApplicationData> LoadApplication(
    const base::FilePath& application_path,
    const std::string& application_id,
    Manifest::SourceType source_type,
    std::string* error) {
  scoped_ptr<base::DictionaryValue> manifest(
      LoadManifest(application_path, error));
  if (!manifest.get())
    return NULL;

  scoped_refptr<ApplicationData> application = ApplicationData::Create(
                                                             application_path,
                                                             source_type,
                                                             *manifest,
                                                             application_id,
                                                             error);
  if (!application.get())
    return NULL;

  std::vector<InstallWarning> warnings;
  ManifestHandlerRegistry* registry =
      manifest->HasKey(widget_keys::kWidgetKey)
      ? ManifestHandlerRegistry::GetInstance(Manifest::TYPE_WGT)
      : ManifestHandlerRegistry::GetInstance(Manifest::TYPE_XPK);

  if (!registry->ValidateAppManifest(application, error, &warnings))
    return NULL;

  if (!warnings.empty()) {
    LOG(WARNING) << "There are some warnings when validating the application "
                 << application->ID();
  }

  return application;
}

static base::DictionaryValue* LoadManifestXpk(
    const base::FilePath& manifest_path,
    std::string* error) {
  JSONFileValueSerializer serializer(manifest_path);
  scoped_ptr<base::Value> root(serializer.Deserialize(NULL, error));
  if (!root.get()) {
    if (error->empty()) {
      // If |error| is empty, than the file could not be read.
      // It would be cleaner to have the JSON reader give a specific error
      // in this case, but other code tests for a file error with
      // error->empty().  For now, be consistent.
      *error = base::StringPrintf("%s", errors::kManifestUnreadable);
    } else {
      *error = base::StringPrintf("%s  %s",
                                  errors::kManifestParseError,
                                  error->c_str());
    }
    return NULL;
  }

  if (!root->IsType(base::Value::TYPE_DICTIONARY)) {
    *error = base::StringPrintf("%s", errors::kManifestUnreadable);
    return NULL;
  }

  base::DictionaryValue* dv =
      static_cast<base::DictionaryValue*>(root.release());
#if defined(OS_TIZEN)
  // Ignore any Tizen application ID, as this is automatically generated.
  dv->Remove(keys::kTizenAppIdKey, NULL);
#endif

  return dv;
}

static base::DictionaryValue* LoadManifestWgt(
    const base::FilePath& manifest_path,
    std::string* error) {
  xmlDoc * doc = NULL;
  xmlNode* root_node = NULL;
  doc = xmlReadFile(manifest_path.MaybeAsASCII().c_str(), NULL, 0);
  if (doc == NULL) {
    *error = base::StringPrintf("%s", errors::kManifestUnreadable);
    return NULL;
  }
  root_node = xmlDocGetRootElement(doc);
  base::DictionaryValue* dv = LoadXMLNode(root_node);
  scoped_ptr<base::DictionaryValue> result(new base::DictionaryValue);
  if (dv)
    result->Set(ToConstCharPointer(root_node->name), dv);

  return result.release();
}

base::DictionaryValue* LoadManifest(const base::FilePath& application_path,
      std::string* error) {
  base::FilePath manifest_path;

  manifest_path = application_path.Append(kManifestXpkFilename);
  if (base::PathExists(manifest_path))
    return LoadManifestXpk(manifest_path, error);

  manifest_path = application_path.Append(kManifestWgtFilename);
  if (base::PathExists(manifest_path))
    return LoadManifestWgt(manifest_path, error);

  *error = base::StringPrintf("%s", errors::kManifestUnreadable);
  return NULL;
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
