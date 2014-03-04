// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/test/chromedriver/net/sync_websocket_impl.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/location.h"
#include "base/single_thread_task_runner.h"
#include "base/synchronization/waitable_event.h"
#include "net/base/net_errors.h"
#include "net/url_request/url_request_context_getter.h"
#include "url/gurl.h"

SyncWebSocketImpl::SyncWebSocketImpl(
    net::URLRequestContextGetter* context_getter)
    : core_(new Core(context_getter)) {}

SyncWebSocketImpl::~SyncWebSocketImpl() {}

bool SyncWebSocketImpl::IsConnected() {
  return core_->IsConnected();
}

bool SyncWebSocketImpl::Connect(const GURL& url) {
  return core_->Connect(url);
}

bool SyncWebSocketImpl::Send(const std::string& message) {
  return core_->Send(message);
}

SyncWebSocket::StatusCode SyncWebSocketImpl::ReceiveNextMessage(
    std::string* message, const base::TimeDelta& timeout) {
  return core_->ReceiveNextMessage(message, timeout);
}

bool SyncWebSocketImpl::HasNextMessage() {
  return core_->HasNextMessage();
}

SyncWebSocketImpl::Core::Core(net::URLRequestContextGetter* context_getter)
    : context_getter_(context_getter),
      is_connected_(false),
      on_update_event_(&lock_) {}

bool SyncWebSocketImpl::Core::IsConnected() {
  base::AutoLock lock(lock_);
  return is_connected_;
}

bool SyncWebSocketImpl::Core::Connect(const GURL& url) {
  bool success = false;
  base::WaitableEvent event(false, false);
  context_getter_->GetNetworkTaskRunner()->PostTask(
      FROM_HERE,
      base::Bind(&SyncWebSocketImpl::Core::ConnectOnIO,
                 this, url, &success, &event));
  event.Wait();
  return success;
}

bool SyncWebSocketImpl::Core::Send(const std::string& message) {
  bool success = false;
  base::WaitableEvent event(false, false);
  context_getter_->GetNetworkTaskRunner()->PostTask(
      FROM_HERE,
      base::Bind(&SyncWebSocketImpl::Core::SendOnIO,
                 this, message, &success, &event));
  event.Wait();
  return success;
}

SyncWebSocket::StatusCode SyncWebSocketImpl::Core::ReceiveNextMessage(
    std::string* message,
    const base::TimeDelta& timeout) {
  base::AutoLock lock(lock_);
  base::TimeTicks deadline = base::TimeTicks::Now() + timeout;
  base::TimeDelta next_wait = timeout;
  while (received_queue_.empty() && is_connected_) {
    if (next_wait <= base::TimeDelta())
      return SyncWebSocket::kTimeout;
    on_update_event_.TimedWait(next_wait);
    next_wait = deadline - base::TimeTicks::Now();
  }
  if (!is_connected_)
    return SyncWebSocket::kDisconnected;
  *message = received_queue_.front();
  received_queue_.pop_front();
  return SyncWebSocket::kOk;
}

bool SyncWebSocketImpl::Core::HasNextMessage() {
  base::AutoLock lock(lock_);
  return !received_queue_.empty();
}

void SyncWebSocketImpl::Core::OnMessageReceived(const std::string& message) {
  base::AutoLock lock(lock_);
  received_queue_.push_back(message);
  on_update_event_.Signal();
}

void SyncWebSocketImpl::Core::OnClose() {
  base::AutoLock lock(lock_);
  is_connected_ = false;
  on_update_event_.Signal();
}

SyncWebSocketImpl::Core::~Core() { }

void SyncWebSocketImpl::Core::ConnectOnIO(
    const GURL& url,
    bool* success,
    base::WaitableEvent* event) {
  {
    base::AutoLock lock(lock_);
    received_queue_.clear();
  }
  socket_.reset(new WebSocket(url, this));
  socket_->Connect(base::Bind(
      &SyncWebSocketImpl::Core::OnConnectCompletedOnIO,
      this, success, event));
}

void SyncWebSocketImpl::Core::OnConnectCompletedOnIO(
    bool* success,
    base::WaitableEvent* event,
    int error) {
  *success = (error == net::OK);
  if (*success) {
    base::AutoLock lock(lock_);
    is_connected_ = true;
  }
  event->Signal();
}

void SyncWebSocketImpl::Core::SendOnIO(
    const std::string& message,
    bool* success,
    base::WaitableEvent* event) {
  *success = socket_->Send(message);
  event->Signal();
}

void SyncWebSocketImpl::Core::OnDestruct() const {
  scoped_refptr<base::SingleThreadTaskRunner> network_task_runner =
      context_getter_->GetNetworkTaskRunner();
  if (network_task_runner->BelongsToCurrentThread())
    delete this;
  else
    network_task_runner->DeleteSoon(FROM_HERE, this);
}
