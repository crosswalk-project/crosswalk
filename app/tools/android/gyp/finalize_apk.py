#!/usr/bin/env python
#
# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=F0401
"""Signs and zipaligns APK.
"""

import optparse
import shutil
import sys

from util import build_utils

def SignApk(keystore_path, unsigned_path, signed_path, alias, code):
  intermediate_path = unsigned_path + '.copy'
  shutil.copy(unsigned_path, intermediate_path)
  sign_cmd = [
      'jarsigner',
      '-sigalg', 'MD5withRSA',
      '-digestalg', 'SHA1',
      '-keystore', keystore_path,
      '-storepass', code,
      intermediate_path, alias
      ]
  build_utils.CheckCallDie(sign_cmd)
  shutil.move(intermediate_path, signed_path)


def AlignApk(zipalign_path, unaligned_path, final_path):
  align_cmd = [
      zipalign_path,
      '-f', '4',  # 4 bytes
      unaligned_path,
      final_path
      ]
  build_utils.CheckCallDie(align_cmd)


def main():
  parser = optparse.OptionParser()

  parser.add_option('--zipalign-path', help='Path to the zipalign tool.')
  parser.add_option('--unsigned-apk-path', help='Path to input unsigned APK.')
  parser.add_option('--final-apk-path',
      help='Path to output signed and aligned APK.')
  parser.add_option('--keystore-path', help='Path to keystore for signing.')
  parser.add_option('--keystore-alias', help='Alias name of keystore.')
  parser.add_option('--keystore-passcode', help='Passcode of keystore.')
  parser.add_option('--stamp', help='Path to touch on success.')
  options, _ = parser.parse_args()

  signed_apk_path = options.unsigned_apk_path + '.signed.apk'
  SignApk(options.keystore_path, options.unsigned_apk_path,
          signed_apk_path, options.keystore_alias, options.keystore_passcode)
  AlignApk(options.zipalign_path, signed_apk_path, options.final_apk_path)

  if options.stamp:
    build_utils.Touch(options.stamp)


if __name__ == '__main__':
  sys.exit(main())
