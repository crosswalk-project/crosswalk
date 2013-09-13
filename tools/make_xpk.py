#!/usr/bin/env python

# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Generate XPK package from package resources and the author private key.
"""
import argparse
import os
from Crypto.PublicKey import RSA
from Crypto import Random
from Crypto.Signature import PKCS1_v1_5
from Crypto.Hash import SHA
import traceback
import zipfile
import struct

class XPKGenerator(object):
  def __init__(self, source_dir, key_file, output_file):
    """
    source_dir  : the path to package resource directory.
    key_file    : the path to RSA private key file, if the file is invalid,
                  generator will create it automatically.
    output_file : the output XPK file path.
    """
    self.source_dir_ = source_dir
    self.output_file_ = output_file
    if not os.path.exists(key_file):
      try:
        print('Start to generate RSA key')
        rng = Random.new().read
        self.RSAkey = RSA.generate(1024, rng)
        kfile = open(key_file,'w')
        kfile.write(self.RSAkey.exportKey('PEM'))
        kfile.close()
        print('Finished generating RSA key, saved as %s' % key_file)
      except IOError:
        if os.path.exists(key_file):
          os.remove(key_file)
        traceback.print_exc()
    else:
      self.RSAkey = RSA.importKey(open(key_file, 'r').read())
    self.pubkey = self.RSAkey.publickey().exportKey('DER')

  def Generate(self):
    if not os.path.exists(self.source_dir_):
      print("The source directory %s is invalid." % self.source_dir_)
      return
    try:
      zip_file = '%s.tmp' % self.output_file_
      self.__Compress(self.source_dir_, zip_file)
      signer = PKCS1_v1_5.new(self.RSAkey)
      zfile = open(zip_file, 'rb')
      sha = SHA.new(zfile.read())
      signature = signer.sign(sha)
      xpk = open(self.output_file_, 'wb')
      zfile.seek(0)
      print('Generating XPK package: %s' % self.output_file_)
      xpk.write('\x43\x72\x57\x6B')
      xpk.write(struct.pack('<I', len(self.pubkey)))
      xpk.write(struct.pack('<I', len(signature)))
      xpk.write(self.pubkey)
      xpk.write(signature)
      xpk.write(zfile.read())
      zfile.close()
      xpk.close()
      print('Generated new XPK package %s successfully.'
            % self.output_file_)
    except IOError:
      if os.path.exists(self.output_file_):
        os.remove(self.output_file_)
      traceback.print_exc()
    finally:
      if os.path.exists(zip_file):
        os.remove(zip_file)

  @classmethod
  def __Compress(cls, src, dst):
    try:
      print('Adding resources from %s into package.' % src)
      zfile = zipfile.ZipFile(dst, 'w')
      abs_src = os.path.abspath(src)
      for dirname, _, files in os.walk(src):
        for filename in files:
          absname = os.path.abspath(os.path.join(dirname, filename))
          relativename = absname[len(abs_src) + 1:]
          zfile.write(absname, relativename)
      zfile.close()
      print('Generated package successfully.')
    except IOError:
      if os.path.exists(dst):
        os.remove(dst)
      traceback.print_exc()

def main():
  parser = argparse.ArgumentParser(
      description='XPKGenerator arguments parser')
  parser.add_argument('input',
      help='Directory path to Crosswalk package resources')
  parser.add_argument(
      'key',
      help='Path to private key file, a new private ' \
           'key file will be generated if it is invalid.')
  parser.add_argument(
      '-o', '--output',
      help='Path to generated XPK file',
      default='default')
  args = parser.parse_args()

  output_file = args.output
  if output_file == 'default':
    head, tail = os.path.split(args.input)
    while len(tail) == 0:
      head, tail = os.path.split(head)
    output_file = tail + '.xpk'
  generator = XPKGenerator(args.input, args.key, output_file)
  generator.Generate()

if __name__ == '__main__':
  main()
