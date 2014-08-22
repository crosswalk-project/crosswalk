// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Copyright (C) 2002-2003 Aleksey Sanin.  All Rights Reserved.
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
#include "libxml/tree.h"
#include "libxml/parser.h"
#include "libxml/xmlschemas.h"
#ifndef XMLSEC_NO_XSLT
#include "libxslt/xslt.h"
#endif  // XMLSEC_NO_XSLT
#include "xmlsec/xmlsec.h"
#include "xmlsec/xmltree.h"
#include "xmlsec/xmldsig.h"
#include "xmlsec/crypto.h"
#include "xmlsec/io.h"
#include "xmlsec/keyinfo.h"

namespace {

// TODO(XU): Once tizen platform provide certificate manager util APIs,
// we should call API from system to query certificate's file path.
class CertificateUtil {
 public:
  typedef std::map<std::string, std::string> CertificatePath;
  static CertificatePath certificate_path;

 private:
  static CertificatePath InitCertificatePath() {
    CertificatePath root_certificates;
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
};

CertificateUtil::CertificatePath
    CertificateUtil::certificate_path = InitCertificatePath();

base::FilePath GetCertFromStore(const std::string& subject) {
  const char cert_prefix_path[] = "/usr/share/ca-certificates/tizen/";
  std::map<std::string, std::string>::iterator iter =
      CertificateUtil::certificate_path.find(subject);

  if (iter == CertificateUtil::certificate_path.end()) {
    LOG(ERROR) << "Failing to find root certificate.";
    return base::FilePath("");
  }
  LOG(INFO) << "root cert path is " << cert_prefix_path + iter->second;
  return base::FilePath(cert_prefix_path + iter->second);
}

void ConvertToPemCert(std::string* cert) {
  *cert = "-----BEGIN CERTIFICATE-----" + *cert;
  *cert = *cert + "-----END CERTIFICATE-----";
}

std::string prefix_path;
std::pair <void*, bool> fw;
void GetExtractedPath(const xwalk::application::SignatureData& data) {
  prefix_path = data.GetExtractedWidgetPath().MaybeAsASCII();
}

int FileMatchCallback(const char* file_name) {
  std::string path = prefix_path + std::string(file_name);
  return xmlFileMatch(path.c_str());
}

void* FileOpenCallback(const char* file_name) {
  std::string path = prefix_path + std::string(file_name);
  fw = std::make_pair(xmlFileOpen(path.c_str()), false);
  return &fw;
}

int FileReadCallback(void* context, char* buffer, int len) {
  std::pair <void*, bool>* pair = static_cast<std::pair<void*, bool>*>(context);
  DCHECK(pair);
  if (pair->second)
    return 0;

  int output = xmlFileRead(pair->first, buffer, len);
  if (output == 0) {
    pair->second = true;
    xmlFileClose(pair->first);
  }
  return output;
}

int FileCloseCallback(void* context) {
  std::pair <void*, bool>* pair = static_cast<std::pair<void*, bool>*>(context);
  DCHECK(pair);
  int output = 0;
  if (!(pair->second))
    output = xmlFileClose(pair->first);

  return output;
}

xmlSecKeysMngrPtr
LoadTrustedCerts(const xwalk::application::SignatureData& data) {
  xmlSecKeysMngrPtr mngr;
  // Create and initialize keys manager, we use a simple list based
  // keys manager
  mngr = xmlSecKeysMngrCreate();
  if (!mngr) {
    LOG(ERROR) << "Error: failed to create keys manager.";
    return NULL;
  }
  if (xmlSecCryptoAppDefaultKeysMngrInit(mngr) < 0) {
    LOG(ERROR) << "Error: failed to initialize keys manager.";
    xmlSecKeysMngrDestroy(mngr);
    return NULL;
  }

  std::list<std::string> certificate_list = data.certificate_list();
  std::string cert;
  std::string issuer;
  for (std::list<std::string>::iterator it = certificate_list.begin();
      it != certificate_list.end(); ++it) {
    cert = *it;
    ConvertToPemCert(&cert);
    net::CertificateList certs =
        net::X509Certificate::CreateCertificateListFromBytes(
            cert.data(), cert.length(), net::X509Certificate::FORMAT_AUTO);
    issuer = certs[0]->issuer().GetDisplayName();
    unsigned char* data1 = new unsigned char[cert.size()];
    strncpy(reinterpret_cast<char*>(data1), cert.c_str(), cert.size());
    // Load trusted cert
    if (xmlSecCryptoAppKeysMngrCertLoadMemory(
          mngr,
          data1,
          cert.size(),
          xmlSecKeyDataFormatCertPem,
          xmlSecKeyDataTypeTrusted) < 0) {
      LOG(ERROR) << "Error: failed to load pem certificate.";
      xmlSecKeysMngrDestroy(mngr);
      delete data1;
      return NULL;
    }
    delete data1;
  }

  const base::FilePath& root_cert_path = GetCertFromStore(issuer);
  if (!base::PathExists(root_cert_path)) {
    LOG(ERROR) << "Failed to find root certificate.";
    return NULL;
  }

  if (xmlSecCryptoAppKeysMngrCertLoad(
        mngr,
        root_cert_path.MaybeAsASCII().c_str(),
        xmlSecKeyDataFormatPem,
        xmlSecKeyDataTypeTrusted) < 0) {
    LOG(ERROR) << "Error: failed to load root certificate";
    xmlSecKeysMngrDestroy(mngr);
    return NULL;
  }

  return mngr;
}

// Verifies XML signature in #xml_file
// Returns 0 on success or a negative value if an error occurs.
int VerifyFile(xmlSecKeysMngrPtr mngr,
               const xwalk::application::SignatureData& data) {
  LOG(INFO) << "Verify " << data.signature_file_name();
  xmlSecIOCleanupCallbacks();
  GetExtractedPath(data);
  xmlSecIORegisterCallbacks(
      FileMatchCallback,
      FileOpenCallback,
      FileReadCallback,
      FileCloseCallback);

  // Load file
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

  // Find start node
  xmlNodePtr node = xmlSecFindNode(
      xmlDocGetRootElement(doc), xmlSecNodeSignature, xmlSecDSigNs);
  if (!node) {
    LOG(ERROR) << "Error: unable to find SecNodeSignature node.";
    xmlFreeDoc(doc);
    return -1;
  }

  // Create signature context
  xmlSecDSigCtxPtr dsigCtx = xmlSecDSigCtxCreate(mngr);
  if (!dsigCtx) {
    LOG(ERROR) << "Error: failed to create signature context.";
    xmlFreeDoc(doc);
    return -1;
  }

  // Verify signature
  if (xmlSecDSigCtxVerify(dsigCtx, node) < 0) {
    LOG(ERROR) << "Error: signature verify.";
    xmlFreeDoc(doc);
    xmlSecDSigCtxDestroy(dsigCtx);
    return -1;
  }

  // Print verification result to stdout
  int res = -1;
  if (dsigCtx->status != xmlSecDSigStatusSucceeded)
    LOG(ERROR) << "Signature " << data.signature_file_name() <<" is INVALID";

  LOG(INFO) << "Signature  "<< data.signature_file_name() << " is OK.";
  res = 0;

  xmlFreeDoc(doc);
  xmlSecDSigCtxDestroy(dsigCtx);
  return res;
}

}  // anonymous namespace

namespace xwalk {
namespace application {

// static
bool SignatureXmlSecAdaptor::ValidateFile(
    const SignatureData& signature_data, const base::FilePath& widget_path) {
  xmlInitParser();
  xmlSecKeysMngrPtr mngr;
  LIBXML_TEST_VERSION
    xmlLoadExtDtdDefaultValue = XML_DETECT_IDS | XML_COMPLETE_ATTRS;
  xmlSubstituteEntitiesDefault(1);
#ifndef XMLSEC_NO_XSLT
  // Init libxslt
  xsltSecurityPrefsPtr xsltSecPrefs = NULL;
  xmlIndentTreeOutput = 1;
  // Disable everything
  xsltSecPrefs = xsltNewSecurityPrefs();
  xsltSetSecurityPrefs(
      xsltSecPrefs, XSLT_SECPREF_READ_FILE, xsltSecurityForbid);
  xsltSetSecurityPrefs(
      xsltSecPrefs, XSLT_SECPREF_WRITE_FILE, xsltSecurityForbid);
  xsltSetSecurityPrefs(
      xsltSecPrefs, XSLT_SECPREF_CREATE_DIRECTORY, xsltSecurityForbid);
  xsltSetSecurityPrefs(
      xsltSecPrefs, XSLT_SECPREF_READ_NETWORK, xsltSecurityForbid);
  xsltSetSecurityPrefs(
      xsltSecPrefs, XSLT_SECPREF_WRITE_NETWORK, xsltSecurityForbid);
  xsltSetDefaultSecurityPrefs(xsltSecPrefs);
#endif  // XMLSEC_NO_XSLT

  // Init xmlsec library
  if (xmlSecInit() < 0) {
    LOG(ERROR) << "Error: xmlsec initialization failed.";
    return false;
  }

  // Check loaded library version
  if (xmlSecCheckVersion() != 1) {
    LOG(ERROR) << "Error: loaded xmlsec library version is not compatible.";
    return false;
  }

  // Load default crypto engine if we are supporting dynamic
  // loading for xmlsec-crypto libraries. Use the crypto library
  // name ("openssl", "nss", etc.) to load corresponding
  // xmlsec-crypto library.
#ifdef XMLSEC_CRYPTO_DYNAMIC_LOADING
  if (xmlSecCryptoDLLoadLibrary(BAD_CAST XMLSEC_CRYPTO) < 0) {
    LOG(ERROR) << "Error: unable to load default xmlsec-crypto library.";
    return false;
  }
#endif  // XMLSEC_CRYPTO_DYNAMIC_LOADING

  // Init crypto library
  if (xmlSecCryptoAppInit(NULL) < 0) {
    LOG(ERROR) << "Error: crypto initialization failed.";
    return false;
  }

  // Init xmlsec-crypto library
  if (xmlSecCryptoInit() < 0) {
    LOG(ERROR) << "Error: xmlsec-crypto initialization failed.";
    return false;
  }

  // Create keys manager and load trusted certificates
  mngr = LoadTrustedCerts(signature_data);
  if (!mngr)
    return false;

  // Verify file
  if (VerifyFile(mngr, signature_data) < 0) {
    xmlSecKeysMngrDestroy(mngr);
    return false;
  }

  // Clean up
  xmlSecKeysMngrDestroy(mngr);
  xmlSecCryptoShutdown();
  xmlSecCryptoAppShutdown();
  xmlSecShutdown();

  // Shutdown libxslt/libxml
#ifndef XMLSEC_NO_XSLT
  xsltFreeSecurityPrefs(xsltSecPrefs);
  xsltCleanupGlobals();
#endif  // XMLSEC_NO_XSLT
  xmlCleanupParser();

  return true;
}

}  // namespace application
}  // namespace xwalk
