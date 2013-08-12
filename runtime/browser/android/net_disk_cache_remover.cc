// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/net_disk_cache_remover.h"

#include "base/bind_helpers.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents.h"
#include "net/disk_cache/disk_cache.h"
#include "net/http/http_cache.h"
#include "net/http/http_transaction_factory.h"
#include "net/url_request/url_request_context_getter.h"
#include "net/url_request/url_request_context.h"
#include "net/base/completion_callback.h"

using content::BrowserThread;
using disk_cache::Backend;
using net::CompletionCallback;
using net::URLRequestContextGetter;

namespace {
// Everything is called and accessed on the IO thread.

void Noop(int rv) {
  DCHECK(rv == net::OK);
}

void CallDoomAllEntries(Backend** backend, int rv) {
  DCHECK(rv == net::OK);
  (*backend)->DoomAllEntries(base::Bind(&Noop));
}

void ClearHttpDiskCacheOfContext(URLRequestContextGetter* context_getter) {
  typedef Backend* BackendPtr;  // Make line below easier to understand.
  BackendPtr* backend_ptr = new BackendPtr(NULL);
  CompletionCallback callback(base::Bind(&CallDoomAllEntries,
                                         base::Owned(backend_ptr)));

  int rv = context_getter->GetURLRequestContext()->
    http_transaction_factory()->GetCache()->GetBackend(backend_ptr, callback);

  // If not net::ERR_IO_PENDING, then backend pointer is updated but callback
  // is not called, so call it explicitly.
  if (rv != net::ERR_IO_PENDING)
    callback.Run(net::OK);
}

void ClearHttpDiskCacheOnIoThread(
    URLRequestContextGetter* main_context_getter,
    URLRequestContextGetter* media_context_getter) {
  ClearHttpDiskCacheOfContext(main_context_getter);
  ClearHttpDiskCacheOfContext(media_context_getter);
}

}  // namespace

namespace xwalk {

void RemoveHttpDiskCache(content::BrowserContext* browser_context,
                        int renderer_child_id) {
  URLRequestContextGetter* main_context_getter =
      browser_context->GetRequestContextForRenderProcess(renderer_child_id);
  URLRequestContextGetter* media_context_getter =
      browser_context->GetMediaRequestContextForRenderProcess(
          renderer_child_id);

  BrowserThread::PostTask(
      BrowserThread::IO, FROM_HERE,
      base::Bind(&ClearHttpDiskCacheOnIoThread,
                 base::Unretained(main_context_getter),
                 base::Unretained(media_context_getter)));
}

}  // namespace xwalk
