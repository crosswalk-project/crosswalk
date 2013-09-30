# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''Utility to remove comments from JSON files so that they can be parsed by
json.loads.
'''

def _Rcount(string, chars):
  '''Returns the number of consecutive characters from |chars| that occur at the
  end of |string|.
  '''
  return len(string) - len(string.rstrip(chars))

def _FindNextToken(string, tokens, start):
  '''Finds the next token in |tokens| that occurs in |string| from |start|.
  Returns a tuple (index, token key).
  '''
  min_index, min_key = (-1, None)
  for k in tokens:
    index = string.find(k, start)
    if index != -1 and (min_index == -1 or index < min_index):
      min_index, min_key = (index, k)
  return (min_index, min_key)

def _ReadString(input, start, output):
  output.append('"')
  start_range, end_range = (start, input.find('"', start))
  # \" escapes the ", \\" doesn't, \\\" does, etc.
  while (end_range != -1 and
         _Rcount(input[start_range:end_range], '\\') % 2 == 1):
    start_range, end_range = (end_range, input.find('"', end_range + 1))
  if end_range == -1:
    return start_range + 1
  output.append(input[start:end_range + 1])
  return end_range + 1

def _ReadComment(input, start, output):
  eol_tokens = ('\n', '\r')
  eol_token_index, eol_token = _FindNextToken(input, eol_tokens, start)
  if eol_token is None:
    return len(input)
  output.append(eol_token)
  return eol_token_index + len(eol_token)

def Nom(input):
  token_actions = {
    '"': _ReadString,
    '//': _ReadComment,
  }
  output = []
  pos = 0
  while pos < len(input):
    token_index, token = _FindNextToken(input, token_actions.keys(), pos)
    if token is None:
      output.append(input[pos:])
      break
    output.append(input[pos:token_index])
    pos = token_actions[token](input, token_index + len(token), output)
  return ''.join(output)
