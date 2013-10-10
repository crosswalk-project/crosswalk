// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/taskbar_util.h"

#include <string>
#include "base/command_line.h"
#include "base/strings/string_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/common/content_switches.h"
#include "crypto/sha2.h"
#include "url/gurl.h"

#if defined(OS_WIN)
#include <shobjidl.h>  // NOLINT(build/include_order)
#endif

namespace xwalk {

const size_t kIdSize = 16;

// Converts a normal hexadecimal string into the alphabet.
// We use the characters 'a'-'p' instead of '0'-'f' to avoid ever having a
// completely numeric host, since some software interprets that as an IP
// address.
void ConvertHexadecimalToIDAlphabet(std::string* id) {
  for (size_t i = 0; i < id->size(); ++i) {
    int val;
    if (base::HexStringToInt(base::StringPiece(id->begin() + i,
                                               id->begin() + i + 1),
                             &val)) {
      (*id)[i] = val + 'a';
    } else {
      (*id)[i] = 'a';
    }
  }
}

// Generates an ID from arbitrary input. The same input string will
// always generate the same output ID.
void GenerateId(const std::string& input, std::string* output) {
  DCHECK(output);
  uint8 hash[kIdSize];
  crypto::SHA256HashString(input, hash, sizeof(hash));
  *output = StringToLowerASCII(base::HexEncode(hash, sizeof(hash)));
  ConvertHexadecimalToIDAlphabet(output);
}

void SetTaskbarGroupIdForProcess() {
  CommandLine* command_line = CommandLine::ForCurrentProcess();
  const CommandLine::StringVector& args = command_line->GetArgs();

  if (args.empty())
    return;

  GURL url(args[0]);
  if (url.is_valid() && url.has_scheme()) {
    std::string appid;
    GenerateId(url.spec(), &appid);
    ::SetCurrentProcessExplicitAppUserModelID(ASCIIToWide(appid).c_str());
  }
}

}  // namespace xwalk
