// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/tizen/signature_validator.h"

#include <set>
#include <string>
#include "base/files/file_enumerator.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "libxml/tree.h"
#include "libxml/parser.h"
#include "libxml/xmlschemas.h"
#include "third_party/re2/re2/re2.h"
#include "xwalk/application/common/tizen/signature_data.h"
#include "xwalk/application/common/tizen/signature_parser.h"
#include "xwalk/application/common/tizen/signature_xmlsec_adaptor.h"

namespace {

const int kXMLLogSize = 1024;
const char kAuthorSignatureName[] = "author-signature.xml";
const char kDistributorSignatureRex[] = "^signature([1-9][0-9]*)\\.xml";
const char kTokenRoleAuthorURI[] =
  "http://www.w3.org/ns/widgets-digsig#role-author";
const char kTokenRoleDistributor[] =
  "http://www.w3.org/ns/widgets-digsig#role-distributor";
const char kTokenProfileURI[] =
  "http://www.w3.org/ns/widgets-digsig#profile";
const char kSignatureSchemaPath[] = "/usr/share/xwalk/signature_schema.xsd";

//  A wrapper of LOG(ERROR) function, which  is used as parameter of function
//  xmlSchemaSetValidErrors
void LogErrorLibxml2(void *, const char *msg, ...) {
  char buffer[kXMLLogSize];
  va_list args;
  va_start(args, msg);
  vsnprintf(buffer, sizeof(buffer), msg, args);
  va_end(args);
  LOG(ERROR) << "ERROR: " << buffer;
}

//  A wrapper of LOG(WARNING) function, which  is used as parameter of function
//  xmlSchemaSetValidErrors
void LogWarningLibxml2(void *, const char *msg, ...) {
  char buffer[kXMLLogSize];
  va_list args;
  va_start(args, msg);
  vsnprintf(buffer, sizeof(buffer), msg, args);
  va_end(args);
  LOG(WARNING) << "Warning: " << buffer;
}

class SignatureFile {
 public:
  SignatureFile(const std::string& file_name, int file_number)
    : file_name_(file_name), file_number_(file_number) {
  }

  std::string file_name() const {
    return file_name_;
  }

  int file_number() const {
    return file_number_;
  }

  bool operator<(const SignatureFile &second) const {
    return file_number_ < second.file_number();
  }

 private:
  std::string file_name_;
  int file_number_;
};
typedef std::set<SignatureFile> SignatureFileSet;

const SignatureFileSet GetSignatureFiles(const base::FilePath& widget_path) {
  SignatureFileSet signature_set;
  std::string file_name;
  int number;
  base::FileEnumerator iter(widget_path, false, base::FileEnumerator::FILES,
      FILE_PATH_LITERAL("*.xml"));

  for (base::FilePath name = iter.Next(); !name.empty(); name = iter.Next()) {
    file_name = name.BaseName().MaybeAsASCII();
    if (file_name.compare(kAuthorSignatureName) == 0) {
      // Find author signature file.
      signature_set.insert(SignatureFile(file_name, -1));
    }
    if (re2::RE2::FullMatch(file_name, kDistributorSignatureRex, &number)) {
      // Find distributor signature file.
      signature_set.insert(SignatureFile(file_name, number));
    }
  }
  return signature_set;
}

bool XMLSchemaValidate(
    const SignatureFile& signature_file, const base::FilePath& widget_path) {
  xmlDocPtr schema_doc = xmlReadFile(
      kSignatureSchemaPath, NULL, XML_PARSE_NONET|XML_PARSE_NOENT);
  if (NULL == schema_doc) {
    LOG(ERROR) << "Reading schema file failed.";
    return false;
  }

  xmlSchemaParserCtxtPtr ctx = xmlSchemaNewParserCtxt(kSignatureSchemaPath);
  if (ctx == NULL) {
    LOG(ERROR) << "Initing xml schema parser context failed.";
    return false;
  }

  xmlSchemaPtr xschema = xmlSchemaParse(ctx);
  if (xschema == NULL) {
    LOG(ERROR) << "Parsing xml schema failed.";
    return false;
  }

  xmlSchemaValidCtxtPtr vctx = xmlSchemaNewValidCtxt(xschema);
  if (vctx == NULL) {
    LOG(ERROR) << "Initing xml schema context failed.";
    return false;
  }
  xmlSchemaSetValidErrors(vctx, (xmlSchemaValidityErrorFunc)&LogErrorLibxml2,
      (xmlSchemaValidityWarningFunc)&LogWarningLibxml2, NULL);

  int ret = xmlSchemaValidateFile(vctx, widget_path.Append(
        signature_file.file_name()).MaybeAsASCII().c_str(), 0);

  if (ret != 0) {
    LOG(ERROR) << "Validating " << signature_file.file_name()
               << " schema failed.";
    return false;
  }
  return true;
}

bool CheckObjectID(
    const xwalk::application::SignatureData& signature_data) {
  std::string object_id = signature_data.object_id();
  std::set<std::string> reference_set = signature_data.reference_set();

  std::set<std::string>::const_iterator result =
    reference_set.find(std::string("#") + object_id);
  if (result == reference_set.end()) {
    LOG(ERROR) << "No reference to object.";
    return false;
  }
  return true;
}

bool CheckRoleURI(
    const xwalk::application::SignatureData& signature_data) {
  std::string role_uri = signature_data.role_uri();

  if (role_uri.empty()) {
    LOG(ERROR) << "URI attribute in Role tag couldn't be empty.";
    return false;
  }

  if (role_uri != kTokenRoleAuthorURI && signature_data.isAuthorSignature()) {
    LOG(ERROR) << "URI attribute in Role tag does not "
               << "match with signature filename.";
    return false;
  }

  if (role_uri != kTokenRoleDistributor &&
      !signature_data.isAuthorSignature()) {
    LOG(ERROR) << "URI attribute in Role tag does not "
               << "match with signature filename.";
    return false;
  }

  return true;
}

bool CheckProfileURI(const xwalk::application::SignatureData& signature_data) {
  if (kTokenProfileURI != signature_data.profile_uri()) {
    LOG(ERROR) << "Profile tag contains unsupported value in URI attribute.";
    return false;
  }
  return true;
}

bool CheckReference(
    const xwalk::application::SignatureData& signature_data) {
  base::FilePath widget_path = signature_data.GetExtractedWidgetPath();
  int prefix_length = widget_path.value().length();
  std::string file_name;
  std::set<std::string> reference_set = signature_data.reference_set();
  base::FileEnumerator iter(widget_path, true, base::FileEnumerator::FILES);

  for (base::FilePath name = iter.Next(); !name.empty(); name = iter.Next()) {
    file_name = name.value().substr(prefix_length);
    if (file_name.compare(kAuthorSignatureName) == 0 ||
        re2::RE2::FullMatch(file_name, kDistributorSignatureRex)) {
      // Skip signtature file.
      continue;
    }
    std::set<std::string>::iterator ref_iter = reference_set.find(file_name);
    if (ref_iter == reference_set.end()) {
      LOG(ERROR) << file_name << "is not in signature ds:Reference.";
      return false;
    }
  }
  return true;
}

}  // anonymous namespace

namespace xwalk {
namespace application {
// static
SignatureValidator::Status SignatureValidator::Check(
    const base::FilePath& widget_path) {
  LOG(INFO) << "Verifying widget signature file.";
  // Process every signature files (author and distributor) according to
  // http://www.w3.org/TR/widgets-digsig/#signature-verification.
  const SignatureFileSet& signature_set = GetSignatureFiles(widget_path);
  if (signature_set.empty()) {
    LOG(INFO) << "No signed signature in the package.";
    return UNTRUSTED;
  }

  SignatureFileSet::reverse_iterator iter = signature_set.rbegin();
  bool ret = false;
  for (; iter != signature_set.rend(); ++iter) {
    // Verify whether signature xml is a valid [XMLDSIG] document.
    if (!XMLSchemaValidate(*iter, widget_path)) {
      LOG(ERROR) << "Validating " << iter->file_name() << "schema failed.";
      return INVALID;
    }

    scoped_ptr<SignatureData> data = SignatureParser::CreateSignatureData(
        widget_path.Append(iter->file_name()), iter->file_number());
    // Check whether each file in the widget can be found from ds:Reference.
    if (!CheckReference(*data.get()))
      return INVALID;

    // Validate the profile property.
    if (!CheckProfileURI(*data.get()))
      return INVALID;

    // Validate the identifier property.
    if (!CheckObjectID(*data.get()))
      return INVALID;

    // Validate role property.
    if (!CheckRoleURI(*data.get()))
      return INVALID;

    // Perform reference validation and signature validation on signature
    if (!SignatureXmlSecAdaptor::ValidateFile(*data.get(), widget_path))
      return INVALID;
  }
  return VALID;
}

}  // namespace application
}  // namespace xwalk
