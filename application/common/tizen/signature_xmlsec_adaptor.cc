// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Copyright (C) 2002-2003 Aleksey Sanin.  All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/tizen/signature_xmlsec_adaptor.h"

#include <list>
#include <map>
#include <string>
#include <utility>

#include "base/logging.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "net/cert/x509_certificate.h"
#include "libxml/parser.h"
#include "xmlsec/crypto.h"
#include "xmlsec/io.h"
#include "xmlsec/keysmngr.h"
#include "xmlsec/xmlsec.h"
#include "xmlsec/xmltree.h"
#include "xmlsec/xmldsig.h"
#ifndef XMLSEC_NO_XSLT
#include "libxslt/xslt.h"
#endif  // XMLSEC_NO_XSLT

namespace {

// TODO(XU): Once tizen platform provide certificate manager util APIs,
// we should call API from system to query certificate's file path.
class CertificateUtil {
 public:
  static const std::map<std::string, std::string>& certificate_path() {
    return certificate_path_;
  }

 private:
  static std::map<std::string, std::string> InitCertificatePath() {
    std::map<std::string, std::string> root_certificates;
    root_certificates["Tizen Partner-Manufacturer Distributor Root CA"] =
        "tizen-distributor-root-ca-partner-manufacturer.pem";
    root_certificates["SLP WebApp Temporary CA"] =
        "tizen.root.preproduction.cert.pem";
    root_certificates["Tizen Test Developer Root CA"] =
        "tizen-developer-root-ca.pem";
    root_certificates["Tizen Developers Root"] =
        "tizen-developers-root.pem";
    root_certificates["Tizen Partner Distributor Root CA"] =
        "tizen-distributor-root-ca-partner.pem";
    root_certificates["Tizen Partner-Operator Distributor Root CA"] =
        "tizen-distributor-root-ca-partner-operator.pem";
    root_certificates["Tizen Public Distributor Root CA"] =
        "tizen-distributor-root-ca-public.pem";
    root_certificates["Partner Class Developer Root"] =
        "tizen-partner-class-developer-root.pem";
    root_certificates["Partner Class Root Authority"] =
        "tizen-partner-class-root-authority.pem";
    root_certificates["Platform Class Developer Root"] =
        "tizen-platform-class-developer-root.pem";
    root_certificates["Platform Class Root Authority"] =
        "tizen-platform-class-root-authority.pem";
    root_certificates["Public Class Developer Root"] =
        "tizen-public-class-developer-root.pem";
    root_certificates["Public Class Root Authority"] =
        "tizen-public-class-root-authority.pem";

    return root_certificates;
  }

  static std::map<std::string, std::string> certificate_path_;
};

std::map<std::string, std::string>
    CertificateUtil::certificate_path_ = InitCertificatePath();

class XmlSecContext {
 public:
  static void GetExtractedPath(const xwalk::application::SignatureData& data);
  static xmlSecKeysMngrPtr LoadTrustedCerts(
      const xwalk::application::SignatureData& signature_data);
  static int VerifyFile(
      xmlSecKeysMngrPtr mngr, const xwalk::application::SignatureData& data);

 private:
  static int FileMatchCallback(const char* file_name);
  static void* FileOpenCallback(const char* file_name);
  static int FileReadCallback(void* context, char* buffer, int len);
  static int FileCloseCallback(void* context);
  static void ConvertToPemCert(std::string* cert);
  static base::FilePath GetCertFromStore(const std::string& subject);

  static std::string prefix_path_;
  static std::pair<void*, bool> file_wrapper_;
};

std::string XmlSecContext::prefix_path_;
std::pair<void*, bool> XmlSecContext::file_wrapper_;

void XmlSecContext::GetExtractedPath(
    const xwalk::application::SignatureData& data) {
  XmlSecContext::prefix_path_ = data.GetExtractedWidgetPath().MaybeAsASCII();
}

int XmlSecContext::FileMatchCallback(const char* file_name) {
  std::string path = XmlSecContext::prefix_path_ + std::string(file_name);
  return xmlFileMatch(path.c_str());
}

void* XmlSecContext::FileOpenCallback(const char* file_name) {
  std::string path = XmlSecContext::prefix_path_ + std::string(file_name);
  XmlSecContext::file_wrapper_ =
      std::make_pair(xmlFileOpen(path.c_str()), false);
  return &(XmlSecContext::file_wrapper_);
}

int XmlSecContext::FileReadCallback(void* context, char* buffer, int len) {
  std::pair<void*, bool>* file_wrapper =
      static_cast<std::pair<void*, bool>*>(context);
  DCHECK(file_wrapper);
  if (file_wrapper->second)
    return 0;

  int output = xmlFileRead(file_wrapper->first, buffer, len);
  if (output == 0) {
    file_wrapper->second = true;
    xmlFileClose(file_wrapper->first);
  }
  return output;
}

int XmlSecContext::FileCloseCallback(void* context) {
  std::pair<void*, bool>* file_wrapper =
      static_cast<std::pair<void*, bool>*>(context);
  DCHECK(file_wrapper);
  int output = 0;
  if (!file_wrapper->second)
    output = xmlFileClose(file_wrapper->first);

  return output;
}

xmlSecKeysMngrPtr XmlSecContext::LoadTrustedCerts(
    const xwalk::application::SignatureData& signature_data) {
  xmlSecKeysMngrPtr mngr = xmlSecKeysMngrCreate();
  if (!mngr) {
    LOG(ERROR) << "Error: failed to create keys manager.";
    return NULL;
  }
  if (xmlSecCryptoAppDefaultKeysMngrInit(mngr) < 0) {
    LOG(ERROR) << "Error: failed to initialize keys manager.";
    xmlSecKeysMngrDestroy(mngr);
    return NULL;
  }

  std::list<std::string> certificate_list = signature_data.certificate_list();
  std::string cert;
  std::string issuer;
  for (std::list<std::string>::iterator it = certificate_list.begin();
      it != certificate_list.end(); ++it) {
    cert = *it;
    XmlSecContext::ConvertToPemCert(&cert);
    net::CertificateList certs =
        net::X509Certificate::CreateCertificateListFromBytes(
            cert.data(), cert.length(), net::X509Certificate::FORMAT_AUTO);
    issuer = certs[0]->issuer().GetDisplayName();

    if (xmlSecCryptoAppKeysMngrCertLoadMemory(mngr,
        reinterpret_cast<const unsigned char*>(cert.c_str()), cert.size(),
        xmlSecKeyDataFormatCertPem, xmlSecKeyDataTypeTrusted) < 0) {
      LOG(ERROR) << "Error: failed to load pem certificate.";
      xmlSecKeysMngrDestroy(mngr);
      return NULL;
    }
  }

  const base::FilePath& root_cert_path =
      XmlSecContext::GetCertFromStore(issuer);
  if (!base::PathExists(root_cert_path)) {
    LOG(ERROR) << "Failed to find root certificate.";
    return NULL;
  }

  if (xmlSecCryptoAppKeysMngrCertLoad(mngr,
      root_cert_path.MaybeAsASCII().c_str(), xmlSecKeyDataFormatPem,
      xmlSecKeyDataTypeTrusted) < 0) {
    LOG(ERROR) << "Error: failed to load root certificate";
    xmlSecKeysMngrDestroy(mngr);
    return NULL;
  }

  return mngr;
}

// Verifies XML signature in #xml_file
// Returns 0 on success or a negative value if an error occurs.
int XmlSecContext::VerifyFile(xmlSecKeysMngrPtr mngr,
                              const xwalk::application::SignatureData& data) {
  LOG(INFO) << "Verify " << data.signature_file_name();
  xmlSecIOCleanupCallbacks();
  XmlSecContext::GetExtractedPath(data);
  xmlSecIORegisterCallbacks(
      XmlSecContext::FileMatchCallback,
      XmlSecContext::FileOpenCallback,
      XmlSecContext::FileReadCallback,
      XmlSecContext::FileCloseCallback);

  xmlDocPtr doc = xmlParseFile(data.signature_file_name().c_str());
  if (!doc) {
    LOG(ERROR) << "Error: failed to parse " << data.signature_file_name();
    return -1;
  }

  if (!xmlDocGetRootElement(doc)) {
    LOG(ERROR) << "Error: unable to get root element.";
    xmlFreeDoc(doc);
    return -1;
  }

  xmlNodePtr node = xmlSecFindNode(
      xmlDocGetRootElement(doc), xmlSecNodeSignature, xmlSecDSigNs);
  if (!node) {
    LOG(ERROR) << "Error: unable to find SecNodeSignature node.";
    xmlFreeDoc(doc);
    return -1;
  }

  xmlSecDSigCtxPtr dsig_ctx = xmlSecDSigCtxCreate(mngr);
  if (!dsig_ctx) {
    LOG(ERROR) << "Error: failed to create signature context.";
    xmlFreeDoc(doc);
    return -1;
  }

  if (xmlSecDSigCtxVerify(dsig_ctx, node) < 0) {
    LOG(ERROR) << "Error: signature verify.";
    xmlFreeDoc(doc);
    xmlSecDSigCtxDestroy(dsig_ctx);
    return -1;
  }

  int res = -1;
  if (dsig_ctx->status != xmlSecDSigStatusSucceeded)
    LOG(ERROR) << "Signature " << data.signature_file_name() <<" is INVALID";

  LOG(INFO) << "Signature  "<< data.signature_file_name() << " is OK.";
  res = 0;

  xmlFreeDoc(doc);
  xmlSecDSigCtxDestroy(dsig_ctx);
  return res;
}

void XmlSecContext::ConvertToPemCert(std::string* cert) {
  *cert = "-----BEGIN CERTIFICATE-----" + *cert;
  *cert = *cert + "-----END CERTIFICATE-----";
}

base::FilePath XmlSecContext::GetCertFromStore(const std::string& subject) {
  const char cert_prefix_path[] = "/usr/share/ca-certificates/tizen/";
  std::map<std::string, std::string>::const_iterator iter =
      CertificateUtil::certificate_path().find(subject);

  if (iter == CertificateUtil::certificate_path().end()) {
    LOG(ERROR) << "Failing to find root certificate.";
    return base::FilePath("");
  }
  LOG(INFO) << "root cert path is " << cert_prefix_path + iter->second;
  return base::FilePath(cert_prefix_path + iter->second);
}

}  // namespace

namespace xwalk {
namespace application {

// static
bool SignatureXmlSecAdaptor::ValidateFile(
    const SignatureData& signature_data, const base::FilePath& widget_path) {
  xmlInitParser();
  xmlSubstituteEntitiesDefault(1);
#ifndef XMLSEC_NO_XSLT
  xsltSecurityPrefsPtr xslt_sec_prefs = xsltNewSecurityPrefs();
  xsltSetSecurityPrefs(
      xslt_sec_prefs, XSLT_SECPREF_READ_FILE, xsltSecurityForbid);
  xsltSetSecurityPrefs(
      xslt_sec_prefs, XSLT_SECPREF_WRITE_FILE, xsltSecurityForbid);
  xsltSetSecurityPrefs(
      xslt_sec_prefs, XSLT_SECPREF_CREATE_DIRECTORY, xsltSecurityForbid);
  xsltSetSecurityPrefs(
      xslt_sec_prefs, XSLT_SECPREF_READ_NETWORK, xsltSecurityForbid);
  xsltSetSecurityPrefs(
      xslt_sec_prefs, XSLT_SECPREF_WRITE_NETWORK, xsltSecurityForbid);
  xsltSetDefaultSecurityPrefs(xslt_sec_prefs);
#endif  // XMLSEC_NO_XSLT

  if (xmlSecInit() < 0) {
    LOG(ERROR) << "Error: xmlsec initialization failed.";
    return false;
  }

  if (xmlSecCheckVersion() != 1) {
    LOG(ERROR) << "Error: loaded xmlsec library version is not compatible.";
    return false;
  }

#ifdef XMLSEC_CRYPTO_DYNAMIC_LOADING
  if (xmlSecCryptoDLLoadLibrary(BAD_CAST XMLSEC_CRYPTO) < 0) {
    LOG(ERROR) << "Error: unable to load default xmlsec-crypto library.";
    return false;
  }
#endif  // XMLSEC_CRYPTO_DYNAMIC_LOADING

  if (xmlSecCryptoAppInit(NULL) < 0) {
    LOG(ERROR) << "Error: crypto initialization failed.";
    return false;
  }

  if (xmlSecCryptoInit() < 0) {
    LOG(ERROR) << "Error: xmlsec-crypto initialization failed.";
    return false;
  }

  xmlSecKeysMngrPtr mngr = XmlSecContext::LoadTrustedCerts(signature_data);
  if (!mngr)
    return false;

  if (XmlSecContext::VerifyFile(mngr, signature_data) < 0) {
    xmlSecKeysMngrDestroy(mngr);
    return false;
  }

  xmlSecKeysMngrDestroy(mngr);
  xmlSecCryptoShutdown();
  xmlSecCryptoAppShutdown();
  xmlSecShutdown();

#ifndef XMLSEC_NO_XSLT
  xsltFreeSecurityPrefs(xslt_sec_prefs);
  xsltCleanupGlobals();
#endif  // XMLSEC_NO_XSLT
  xmlCleanupParser();

  return true;
}

}  // namespace application
}  // namespace xwalk
