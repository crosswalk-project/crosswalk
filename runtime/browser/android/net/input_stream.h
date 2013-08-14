// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_BROWSER_ANDROID_NET_INPUT_STREAM_H_
#define XWALK_RUNTIME_BROWSER_ANDROID_NET_INPUT_STREAM_H_

#include "base/basictypes.h"

namespace net {
class IOBuffer;
}

namespace xwalk {

// Abstract wrapper used to access the InputStream Java class.
// This class is safe to pass around between threads (the destructor,
// constructor and methods can be called on different threads) but calling
// methods concurrently might have undefined results.
class InputStream {
 public:
  virtual ~InputStream() {}

  // Sets |bytes_available| to the number of bytes that can be read (or skipped
  // over) from this input stream without blocking by the next caller of a
  // method for this input stream.
  // Returns true if completed successfully or false if an exception was
  // thrown.
  virtual bool BytesAvailable(int* bytes_available) const = 0;

  // Skips over and discards |n| bytes of data from this input stream. Sets
  // |bytes_skipped| to the number of of bytes skipped.
  // Returns true if completed successfully or false if an exception was
  // thrown.
  virtual bool Skip(int64_t n, int64_t* bytes_skipped) = 0;

  // Reads at most |length| bytes into |dest|. Sets |bytes_read| to the total
  // number of bytes read into |dest| or 0 if there is no more data because the
  // end of the stream was reached.
  // |dest| must be at least |length| in size.
  // Returns true if completed successfully or false if an exception was
  // thrown.
  virtual bool Read(net::IOBuffer* dest, int length, int* bytes_read) = 0;

 protected:
  InputStream() {}
};

}  // namespace xwalk

#endif  // XWALK_RUNTIME_BROWSER_ANDROID_NET_INPUT_STREAM_H_
