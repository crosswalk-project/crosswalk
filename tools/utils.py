#!/usr/bin/env python

''' This script provides utils for python scripts in cameo.
'''

import os
import sys
import subprocess

def TryAddDepotToolsToPythonPath():
  depot_tools = FindDepotToolsInPath()
  if depot_tools:
    sys.path.append(depot_tools)

def FindDepotToolsInPath():
  paths = os.getenv('PATH').split(os.path.pathsep)
  for path in paths:
    if os.path.basename(path) == 'depot_tools':
      return path
  return None

def IsWindows():
  return sys.platform == 'cygwin' or sys.platform.startswith('win')

def IsLinux():
  return sys.platform.startswith('linux')

def IsMac():
  return sys.platform.startswith('darwin')

def GitExe():
  if IsWindows():
    return 'git.bat'
  else:
    return 'git'

def GetCommandOutput(command, cwd=None):
  proc = subprocess.Popen(command, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, bufsize=1,
                          cwd=cwd)
  output = proc.communicate()[0]
  result = proc.returncode
  if result:
    raise Exception('%s: %s' % (subprocess.list2cmdline(command), output))
  return output
