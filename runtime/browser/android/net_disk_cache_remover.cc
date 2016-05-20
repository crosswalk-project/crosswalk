// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/net_disk_cache_remover.h"

#include "base/bind_helpers.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/storage_partition.h"
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

void CallDoomEntry(Backend** backend, const std::string& key, int rv) {
  DCHECK(rv == net::OK);
  (*backend)->DoomEntry(key, base::Bind(&Noop));
}

void ClearHttpDiskCacheOfContext(URLRequestContextGetter* context_getter,
                                 const std::string& key) {
  typedef Backend* BackendPtr;  // Make line below easier to understand.
  BackendPtr* backend_ptr = new BackendPtr(NULL);
  CompletionCallback callback;
  if (!key.empty()) {
    callback = base::Bind(&CallDoomEntry,
                          base::Owned(backend_ptr),
                          key);
  } else {
    callback = base::Bind(&CallDoomAllEntries,
                          base::Owned(backend_ptr));
  }

  int rv = context_getter->GetURLRequestContext()->
    http_transaction_factory()->GetCache()->GetBackend(backend_ptr, callback);

  // If not net::ERR_IO_PENDING, then backend pointer is updated but callback
  // is not called, so call it explicitly.
  if (rv != net::ERR_IO_PENDING)
    callback.Run(net::OK);
}

void ClearHttpDiskCacheOnIoThread(
    URLRequestContextGetter* main_context_getter,
    URLRequestContextGetter* media_context_getter,
    const std::string& key) {
  ClearHttpDiskCacheOfContext(main_context_getter, key);
  ClearHttpDiskCacheOfContext(media_context_getter, key);
}

}  // namespace

namespace xwalk {

void RemoveHttpDiskCache(content::RenderProcessHost* render_process_host,
                         const std::string& key) {
  BrowserThread::PostTask(
      BrowserThread::IO, FROM_HERE,
      base::Bind(&ClearHttpDiskCacheOnIoThread,
                 base::Unretained(render_process_host->GetStoragePartition()->
                     GetURLRequestContext()),
                 base::Unretained(render_process_host->GetStoragePartition()->
                     GetMediaURLRequestContext()),
                 key));
}

}  // namespace xwalk
