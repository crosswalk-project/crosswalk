#!/usr/bin/env python

# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys

# For tty that supports ANSI color, define the color value.
# For window console, the win32 API `SetConsoleTextAttribute` has to be
# used to set the console color.
class AnsiColor:
  BLACK   = '\033[' + str(30) + 'm'
  RED     = '\033[' + str(31) + 'm'
  GREEN   = '\033[' + str(32) + 'm'
  YELLOW  = '\033[' + str(33) + 'm'
  BLUE    = '\033[' + str(34) + 'm'
  MAGENTA = '\033[' + str(35) + 'm'
  CYAN    = '\033[' + str(36) + 'm'
  WHITE   = '\033[' + str(37) + 'm'
  RESET   = '\033[' + str(39) + 'm'

  def __init__(self):
    pass


def TtySupportAnsiColor():
  if sys.platform == 'win32':
    return False
  is_a_tty = hasattr(sys.stdout, 'isatty') and sys.stdout.isatty()
  return is_a_tty


def PrintError(message):
  color_message = message
  if TtySupportAnsiColor():
    color_message = AnsiColor.RED + message + AnsiColor.RESET
  print(color_message)


def PrintWarning(message):
  color_message = message
  if TtySupportAnsiColor():
    color_message = AnsiColor.CYAN + message + AnsiColor.RESET
  print(color_message)


def PrintSuccess(message):
  color_message = message
  if TtySupportAnsiColor():
    color_message = AnsiColor.GREEN + message + AnsiColor.RESET
  print(color_message)


if __name__ == '__main__':
  PrintError('Error')
  PrintWarning('Warning')
  PrintSuccess('Success')
