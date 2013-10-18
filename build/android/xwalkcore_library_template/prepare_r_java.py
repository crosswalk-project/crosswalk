#!/usr/bin/env python

"""Copy R.java to additional packages
"""

import optparse
import os
import re
import sys


RE_PACKAGE = re.compile('^package ([a-zA-Z_]+(?:|\.[a-zA-Z_]+)*);$')
RE_CLASS = re.compile('^(?:|[ ]*)public static final class ([a-zA-Z_]+) {$')
RE_VALUE = re.compile('^([ ]*)public static int ([a-zA-Z_]+)=(0x[0-9a-f]{8});$')


def CopyRjavaToPackage(gen_path, app_package, target_package):
  r_java = os.path.join(gen_path, app_package.replace('.', os.sep), 'R.java')
  if not os.path.isfile(r_java):
    print '%s not exist' % r_java
    sys.exit(1)
  target_folder = os.path.join(gen_path, target_package.replace('.', os.sep))
  if not os.path.isdir(target_folder):
    os.makedirs(target_folder)
  target_java_file = open(os.path.join(target_folder, 'R.java'), 'w')
  current_class = None
  got_package = False
  for line in open(r_java, 'r').readlines():
    if not got_package:
      # Looking for package declaration
      match_package = RE_PACKAGE.match(line)
      if match_package and match_package.groups()[0] == app_package:
        got_package = True
        target_java_file.write('package %s;\n' % target_package)
      else:
        target_java_file.write(line)
      continue
    # Trying to match class pattern first
    match_class = RE_CLASS.match(line)
    if match_class:
      current_class = match_class.groups()[0]
      target_java_file.write(line)
      continue

    if current_class:
      match_value = RE_VALUE.match(line)
      if match_value:
        target_java_file.write(
            '%spublic static int %s=%s.R.%s.%s;\n' % (match_value.groups()[0],
                                                      match_value.groups()[1],
                                                      app_package,
                                                      current_class,
                                                      match_value.groups()[1]))
        continue

    target_java_file.write(line)


def main():
  option_parser = optparse.OptionParser()

  option_parser.add_option('--app-package', default=None,
      help='The package which provides R.java')
  option_parser.add_option('--packages', default=None,
      help='The additional packages to be placed R.java, '
           'delimated by semicolon')
  option_parser.add_option('--gen-path', default=None,
      help='The gen folder')

  opts, _ = option_parser.parse_args()

  if opts.packages == None or opts.packages.strip() == '':
    return 0

  if opts.gen_path == None:
    print 'gen path not specified'
    return 1

  if opts.app_package == None:
    print 'app package not specified'
    return 1

  for package in opts.packages.strip().split(';'):
    CopyRjavaToPackage(opts.gen_path, opts.app_package, package)

  return 0


if '__main__' == __name__:
  sys.exit(main())

