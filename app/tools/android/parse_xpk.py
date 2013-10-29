#!/usr/bin/env python
# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# pylint: disable=C0301
"""The script is used to parse an XPK file.

It will do:
1. Check the magic file header;
2. Verify the signature of the XPK file;
3. Extract the content of the XPK file to some folder.

The format of XPK file can be found at
https://github.com/crosswalk-project/crosswalk-website/wiki/Crosswalk-package-management

This file is used by make_apk.py.
"""

import optparse
import os
import struct
import sys
import zipfile

EXIT_CODE_CRYPTO_NOT_FOUND = 1
EXIT_CODE_NO_XPK_FILE = 2
EXIT_CODE_XPK_FILE_NOT_EXIST = 3
EXIT_CODE_MAGIC_FAILED = 4
EXIT_CODE_VERIFICATION_FAILED = 5
EXIT_CODE_XPK_FILE_IO_ERROR = 6

XPK_MAGIC_HEAD = 'CrWk'

errorMessageMap = {
  EXIT_CODE_CRYPTO_NOT_FOUND: 'Python module Crypto('\
      'https://www.dlitz.net/software/pycrypto/) is needed',
  EXIT_CODE_NO_XPK_FILE: 'Please specify XPK file by --file',
  EXIT_CODE_XPK_FILE_NOT_EXIST: 'The XPK file you specified does not exist',
  EXIT_CODE_MAGIC_FAILED: 'The file you specified is not in XPK format',
  EXIT_CODE_VERIFICATION_FAILED:
      'Signature verification failed for the XPK file',
  EXIT_CODE_XPK_FILE_IO_ERROR: 'Error happened when reading the XPK file',
}


def HandleError(err_code):
  print 'Error: %s' % errorMessageMap[err_code]
  sys.exit(err_code)


try:
  from Crypto.PublicKey import RSA
  from Crypto.Signature import PKCS1_v1_5
  from Crypto.Hash import SHA
except ImportError:
  HandleError(EXIT_CODE_CRYPTO_NOT_FOUND)


def CheckMagic(input_file):
  magic = input_file.read(4)
  if magic != XPK_MAGIC_HEAD:
    HandleError(EXIT_CODE_MAGIC_FAILED)


def GetPubkeySignature(input_file):
  """Return (pubkey, signature) pair"""
  pubkey_size, signature_size = struct.unpack('II', input_file.read(8))
  return (input_file.read(pubkey_size), input_file.read(signature_size))


def ExtractXPKContent(input_file, zip_path):
  zip_file = open(zip_path, 'wb')
  zip_file.write(input_file.read())
  zip_file.close()


def VerifySignature(pubkey, signature, zip_path):
  zip_file = open(zip_path, 'rb')
  key = RSA.importKey(pubkey)
  content = SHA.new(zip_file.read())
  zip_file.close()
  verifier = PKCS1_v1_5.new(key)
  if not verifier.verify(content, signature):
    HandleError(EXIT_CODE_VERIFICATION_FAILED)


def main():
  option_parser = optparse.OptionParser()
  option_parser.add_option('--file', '-f', help='Path to the xpk file')
  option_parser.add_option('--out', '-o', help='Path to extract the xpk to')

  opts, _ = option_parser.parse_args()

  if opts.file == None:
    HandleError(EXIT_CODE_NO_XPK_FILE)

  app_name = os.path.splitext(os.path.basename(opts.file))[0]

  if opts.out == None:
    opts.out = app_name

  if os.path.isfile(opts.file):
    zip_path = None
    try:
      xpk_file = open(opts.file, 'rb')
      CheckMagic(xpk_file)
      pubkey, signature = GetPubkeySignature(xpk_file)
      zip_path = '%s.zip' % app_name
      ExtractXPKContent(xpk_file, zip_path)
      VerifySignature(pubkey, signature, zip_path)
      zipfile.ZipFile(zip_path).extractall(opts.out)
    except SystemExit, ec:
      return ec.code
    except IOError:
      HandleError(EXIT_CODE_XPK_FILE_IO_ERROR)
    finally:
      xpk_file.close()
      if zip_path and os.path.isfile(zip_path):
        os.remove(zip_path)
  else:
    HandleError(EXIT_CODE_XPK_FILE_NOT_EXIST)


if __name__ == '__main__':
  sys.exit(main())
