#!/usr/bin/env python

# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=W0612

import os
import sys
import time


# Constants from the Windows API
STD_OUTPUT_HANDLE = -11

FOREGROUND_BLUE      = 0x0001
FOREGROUND_GREEN     = 0x0002
FOREGROUND_CYAN      = 0x0003
FOREGROUND_RED       = 0x0004
FOREGROUND_MAGENTA   = 0x0005
FOREGROUND_YELLOW    = 0x0006
FOREGROUND_INTENSITY = 0x08


def IsWindows():
  return sys.platform == 'cygwin' or sys.platform.startswith('win')


def IsColorTerminal():
  if not hasattr(sys.stdout, 'isatty'):
    return False
  if not sys.stdout.isatty():
    return False
  if 'COLORTERM' in os.environ:
    return True
  term = os.environ.get('TERM', 'dumb').lower()
  if term in ('xterm', 'linux') or 'color' in term:
    return True
  return False


if not IsColorTerminal():
  import ctypes
  stdout_handle = ctypes.windll.kernel32.GetStdHandle(STD_OUTPUT_HANDLE)


def GetCsbiAttributes(handle):
  if not IsColorTerminal():
    import struct
    csbi = ctypes.create_string_buffer(22)
    res = ctypes.windll.kernel32.GetConsoleScreenBufferInfo(handle, csbi)
    assert res
    (bufx, bufy, curx, cury, wattr, left, top, right, bottom,
     maxx, maxy) = struct.unpack("hhhhHhhhhhh", csbi.raw)
    return wattr


if not IsColorTerminal():
  reset = GetCsbiAttributes(stdout_handle)


def SetColor(color):
  if not IsColorTerminal():
    return ctypes.windll.kernel32.SetConsoleTextAttribute(stdout_handle, color)


def ResetColor():
  if not IsColorTerminal():
    SetColor(reset)


# ANSI escape sequences
HEADER = '\033[95m'  # Magenta
OKBLUE = '\033[94m'  # Blue
OKGREEN = '\033[92m' # Green
WARNING = '\033[93m' # Yellow
FAIL = '\033[91m'    # Red
ENDC = '\033[0m'     # End
BOLD = "\033[1m"     # Bold


def Info(msg):
  if not IsColorTerminal():
    SetColor(FOREGROUND_GREEN)
    print msg
    ResetColor()
  else:
    print OKGREEN + msg + ENDC


def VerboseCommand(msg):
  if not IsColorTerminal():
    SetColor(FOREGROUND_MAGENTA | FOREGROUND_INTENSITY)
    print msg
    ResetColor()
  else:
    print BOLD + HEADER + msg + ENDC


def VerboseOutput(msg):
  if not IsColorTerminal():
    SetColor(FOREGROUND_BLUE)
    print msg
    ResetColor()
  else:
    print OKBLUE + msg + ENDC


def WarningInfo(msg):
  if not IsColorTerminal():
    SetColor(FOREGROUND_YELLOW)
    print msg
    ResetColor()
  else:
    print WARNING + msg + ENDC


def Error(msg):
  if not IsColorTerminal():
    SetColor(FOREGROUND_RED)
    print msg
    ResetColor()
  else:
    print FAIL + msg + ENDC


def ProgressBar():
  if not IsColorTerminal():
    SetColor(FOREGROUND_GREEN)
    sys.stdout.write('#' + '->' + "\b\b")
    ResetColor()
  else:
    sys.stdout.write(OKGREEN + '#' + '->' + "\b\b" + ENDC)
  sys.stdout.flush()
  time.sleep(0.5)
