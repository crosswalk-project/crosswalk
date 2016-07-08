// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/net/xwalk_cookie_store_wrapper.h"

#include <string>

#include "base/memory/ref_counted_delete_on_message_loop.h"
#include "base/threading/thread_task_runner_handle.h"
#include "url/gurl.h"
#include "xwalk/runtime/browser/android/net/init_native_callback.h"

namespace xwalk {

namespace {

// Posts |task| to the thread that the global CookieStore lives on.
void PostTaskToCookieStoreTaskRunner(const base::Closure& task) {
  GetCookieStoreTaskRunner()->PostTask(FROM_HERE, task);
}

// Wraps a subscription to cookie change notifications for the global
// CookieStore for a consumer that lives on another thread. Handles passing
// messages between thread, and destroys itself when the consumer unsubscribes.
// Must be created on the consumer's thread. Each instance only supports a
// single subscription.
class SubscriptionWrapper {
 public:
  SubscriptionWrapper() : weak_factory_(this) {}

  std::unique_ptr<net::CookieStore::CookieChangedSubscription> Subscribe(
      const GURL& url,
      const std::string& name,
      const net::CookieStore::CookieChangedCallback& callback) {
    // This class is only intended to be used for a single subscription.
    DCHECK(callback_list_.empty());

    nested_subscription_ =
        new NestedSubscription(url, name, weak_factory_.GetWeakPtr());
    return callback_list_.Add(callback);
  }

 private:
  // The NestedSubscription is responsible for creating and managing the
  // underlying subscription to the real CookieStore, and posting notifications
  // back to |callback_list_|.
  class NestedSubscription
      : public base::RefCountedDeleteOnMessageLoop<NestedSubscription> {
   public:
    NestedSubscription(const GURL& url,
                       const std::string& name,
                       base::WeakPtr<SubscriptionWrapper> subscription_wrapper)
        : base::RefCountedDeleteOnMessageLoop<NestedSubscription>(
              GetCookieStoreTaskRunner()),
          subscription_wrapper_(subscription_wrapper),
          client_task_runner_(base::ThreadTaskRunnerHandle::Get()) {
      PostTaskToCookieStoreTaskRunner(
          base::Bind(&NestedSubscription::Subscribe, this, url, name));
    }

   private:
    friend class base::RefCountedDeleteOnMessageLoop<NestedSubscription>;
    friend class base::DeleteHelper<NestedSubscription>;

    ~NestedSubscription() {}

    void Subscribe(const GURL& url, const std::string& name) {
      GetCookieStore()->AddCallbackForCookie(
          url, name, base::Bind(&NestedSubscription::OnChanged, this));
    }

    void OnChanged(const net::CanonicalCookie& cookie, bool removed) {
      client_task_runner_->PostTask(
          FROM_HERE, base::Bind(&SubscriptionWrapper::OnChanged,
                                subscription_wrapper_, cookie, removed));
    }

    base::WeakPtr<SubscriptionWrapper> subscription_wrapper_;
    scoped_refptr<base::TaskRunner> client_task_runner_;

    std::unique_ptr<net::CookieStore::CookieChangedSubscription> subscription_;

    DISALLOW_COPY_AND_ASSIGN(NestedSubscription);
  };

  void OnChanged(const net::CanonicalCookie& cookie, bool removed) {
    callback_list_.Notify(cookie, removed);
  }

  // The "list" only had one entry, so can just clean up now.
  void OnUnsubscribe() { delete this; }

  scoped_refptr<NestedSubscription> nested_subscription_;
  net::CookieStore::CookieChangedCallbackList callback_list_;
  base::WeakPtrFactory<SubscriptionWrapper> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(SubscriptionWrapper);
};

void SetCookieWithOptionsAsyncOnCookieThread(
    const GURL& url,
    const std::string& cookie_line,
    const net::CookieOptions& options,
    const net::CookieStore::SetCookiesCallback& callback) {
  GetCookieStore()->SetCookieWithOptionsAsync(url, cookie_line, options,
                                              callback);
}

void SetCookieWithDetailsAsyncOnCookieThread(
    const GURL& url,
    const std::string& name,
    const std::string& value,
    const std::string& domain,
    const std::string& path,
    base::Time creation_time,
    base::Time expiration_time,
    base::Time last_access_time,
    bool secure,
    bool http_only,
    net::CookieSameSite same_site,
    bool enforce_strict_secure,
    net::CookiePriority priority,
    const net::CookieStore::SetCookiesCallback& callback) {
  GetCookieStore()->SetCookieWithDetailsAsync(
      url, name, value, domain, path, creation_time, expiration_time,
      last_access_time, secure, http_only, same_site, enforce_strict_secure,
      priority, callback);
}

void GetCookiesWithOptionsAsyncOnCookieThread(
    const GURL& url,
    const net::CookieOptions& options,
    const net::CookieStore::GetCookiesCallback& callback) {
  GetCookieStore()->GetCookiesWithOptionsAsync(url, options, callback);
}

void GetCookieListWithOptionsAsyncOnCookieThread(
    const GURL& url,
    const net::CookieOptions& options,
    const net::CookieStore::GetCookieListCallback& callback) {
  GetCookieStore()->GetCookieListWithOptionsAsync(url, options, callback);
}

void GetAllCookiesAsyncOnCookieThread(
    const net::CookieStore::GetCookieListCallback& callback) {
  GetCookieStore()->GetAllCookiesAsync(callback);
}

void DeleteCookieAsyncOnCookieThread(const GURL& url,
                                     const std::string& cookie_name,
                                     const base::Closure& callback) {
  GetCookieStore()->DeleteCookieAsync(url, cookie_name, callback);
}

void DeleteCanonicalCookieAsyncOnCookieThread(
    const net::CanonicalCookie& cookie,
    const net::CookieStore::DeleteCallback& callback) {
  GetCookieStore()->DeleteCanonicalCookieAsync(cookie, callback);
}

void DeleteAllCreatedBetweenAsyncOnCookieThread(
    const base::Time& delete_begin,
    const base::Time& delete_end,
    const net::CookieStore::DeleteCallback& callback) {
  GetCookieStore()->DeleteAllCreatedBetweenAsync(delete_begin, delete_end,
                                                 callback);
}

void DeleteAllCreatedBetweenWithPredicateAsyncOnCookieThread(
    const base::Time& delete_begin,
    const base::Time& delete_end,
    const net::CookieStore::CookiePredicate& predicate,
    const net::CookieStore::DeleteCallback& callback) {
  GetCookieStore()->DeleteAllCreatedBetweenWithPredicateAsync(
      delete_begin, delete_end, predicate, callback);
}

void DeleteSessionCookiesAsyncOnCookieThread(
    const net::CookieStore::DeleteCallback& callback) {
  GetCookieStore()->DeleteSessionCookiesAsync(callback);
}

void FlushStoreOnCookieThread(const base::Closure& callback) {
  GetCookieStore()->FlushStore(callback);
}

void SetForceKeepSessionStateOnCookieThread() {
  GetCookieStore()->SetForceKeepSessionState();
}

}  // namespace

XWalkCookieStoreWrapper::XWalkCookieStoreWrapper()
    : client_task_runner_(base::ThreadTaskRunnerHandle::Get()),
      weak_factory_(this) {}

XWalkCookieStoreWrapper::~XWalkCookieStoreWrapper() {}

void XWalkCookieStoreWrapper::SetCookieWithOptionsAsync(
    const GURL& url,
    const std::string& cookie_line,
    const net::CookieOptions& options,
    const net::CookieStore::SetCookiesCallback& callback) {
  DCHECK(client_task_runner_->RunsTasksOnCurrentThread());
  PostTaskToCookieStoreTaskRunner(
      base::Bind(&SetCookieWithOptionsAsyncOnCookieThread, url, cookie_line,
                 options, CreateWrappedCallback<bool>(callback)));
}

void XWalkCookieStoreWrapper::SetCookieWithDetailsAsync(
    const GURL& url,
    const std::string& name,
    const std::string& value,
    const std::string& domain,
    const std::string& path,
    base::Time creation_time,
    base::Time expiration_time,
    base::Time last_access_time,
    bool secure,
    bool http_only,
    net::CookieSameSite same_site,
    bool enforce_strict_secure,
    net::CookiePriority priority,
    const SetCookiesCallback& callback) {
  DCHECK(client_task_runner_->RunsTasksOnCurrentThread());
  PostTaskToCookieStoreTaskRunner(
      base::Bind(&SetCookieWithDetailsAsyncOnCookieThread, url, name, value,
                 domain, path, creation_time, expiration_time, last_access_time,
                 secure, http_only, same_site, enforce_strict_secure, priority,
                 CreateWrappedCallback<bool>(callback)));
}

void XWalkCookieStoreWrapper::GetCookiesWithOptionsAsync(
    const GURL& url,
    const net::CookieOptions& options,
    const GetCookiesCallback& callback) {
  DCHECK(client_task_runner_->RunsTasksOnCurrentThread());
  PostTaskToCookieStoreTaskRunner(
      base::Bind(&GetCookiesWithOptionsAsyncOnCookieThread, url, options,
                 CreateWrappedCallback<const std::string&>(callback)));
}

void XWalkCookieStoreWrapper::GetCookieListWithOptionsAsync(
    const GURL& url,
    const net::CookieOptions& options,
    const GetCookieListCallback& callback) {
  DCHECK(client_task_runner_->RunsTasksOnCurrentThread());
  PostTaskToCookieStoreTaskRunner(
      base::Bind(&GetCookieListWithOptionsAsyncOnCookieThread, url, options,
                 CreateWrappedCallback<const net::CookieList&>(callback)));
}

void XWalkCookieStoreWrapper::GetAllCookiesAsync(
    const GetCookieListCallback& callback) {
  DCHECK(client_task_runner_->RunsTasksOnCurrentThread());
  PostTaskToCookieStoreTaskRunner(
      base::Bind(&GetAllCookiesAsyncOnCookieThread,
                 CreateWrappedCallback<const net::CookieList&>(callback)));
}

void XWalkCookieStoreWrapper::DeleteCookieAsync(const GURL& url,
    const std::string& cookie_name,
    const base::Closure& callback) {
  DCHECK(client_task_runner_->RunsTasksOnCurrentThread());
  PostTaskToCookieStoreTaskRunner(
      base::Bind(&DeleteCookieAsyncOnCookieThread, url, cookie_name,
                 CreateWrappedClosureCallback(callback)));
}

void XWalkCookieStoreWrapper::DeleteCanonicalCookieAsync(
    const net::CanonicalCookie& cookie,
    const DeleteCallback& callback) {
  DCHECK(client_task_runner_->RunsTasksOnCurrentThread());
  PostTaskToCookieStoreTaskRunner(
      base::Bind(&DeleteCanonicalCookieAsyncOnCookieThread, cookie,
                 CreateWrappedCallback<int>(callback)));
}

void XWalkCookieStoreWrapper::DeleteAllCreatedBetweenAsync(
    const base::Time& delete_begin,
    const base::Time& delete_end,
    const DeleteCallback& callback) {
  DCHECK(client_task_runner_->RunsTasksOnCurrentThread());
  PostTaskToCookieStoreTaskRunner(
      base::Bind(&DeleteAllCreatedBetweenAsyncOnCookieThread, delete_begin,
                 delete_end, CreateWrappedCallback<int>(callback)));
}

void XWalkCookieStoreWrapper::DeleteAllCreatedBetweenWithPredicateAsync(
    const base::Time& delete_begin,
    const base::Time& delete_end,
    const net::CookieStore::CookiePredicate& predicate,
    const DeleteCallback& callback) {
  DCHECK(client_task_runner_->RunsTasksOnCurrentThread());
  PostTaskToCookieStoreTaskRunner(base::Bind(
      &DeleteAllCreatedBetweenWithPredicateAsyncOnCookieThread,
      delete_begin,
      delete_end,
      predicate,
      CreateWrappedCallback<int>(callback)));
}

void XWalkCookieStoreWrapper::DeleteSessionCookiesAsync(
    const DeleteCallback& callback) {
  DCHECK(client_task_runner_->RunsTasksOnCurrentThread());
  PostTaskToCookieStoreTaskRunner(
      base::Bind(&DeleteSessionCookiesAsyncOnCookieThread,
                 CreateWrappedCallback<int>(callback)));
}

void XWalkCookieStoreWrapper::FlushStore(const base::Closure& callback) {
  DCHECK(client_task_runner_->RunsTasksOnCurrentThread());
  PostTaskToCookieStoreTaskRunner(base::Bind(
      &FlushStoreOnCookieThread, CreateWrappedClosureCallback(callback)));
}

void XWalkCookieStoreWrapper::SetForceKeepSessionState() {
  DCHECK(client_task_runner_->RunsTasksOnCurrentThread());
  PostTaskToCookieStoreTaskRunner(
      base::Bind(&SetForceKeepSessionStateOnCookieThread));
}

std::unique_ptr<net::CookieStore::CookieChangedSubscription>
XWalkCookieStoreWrapper::AddCallbackForCookie(
    const GURL& url,
    const std::string& name,
    const CookieChangedCallback& callback) {
  DCHECK(client_task_runner_->RunsTasksOnCurrentThread());

  // The SubscriptionWrapper is owned by the subscription itself, and has no
  // connection to the XWalkCookieStoreWrapper after creation. Other CookieStore
  // implementations DCHECK if a subscription outlasts the cookie store,
  // unfortunately, this design makes DCHECKing if there's an outstanding
  // subscription when the XWalkCookieStoreWrapper is destroyed a bit ugly.
  // TODO(mmenke):  Still worth adding a DCHECK?
  SubscriptionWrapper* subscription = new SubscriptionWrapper();
  return subscription->Subscribe(url, name, callback);
}

bool XWalkCookieStoreWrapper::IsEphemeral() {
  return GetCookieStore()->IsEphemeral();
}

base::Closure XWalkCookieStoreWrapper::CreateWrappedClosureCallback(
    const base::Closure& callback) {
  if (callback.is_null())
    return callback;
  return base::Bind(base::IgnoreResult(&base::TaskRunner::PostTask),
                    client_task_runner_, FROM_HERE,
                    base::Bind(&XWalkCookieStoreWrapper::RunClosureCallback,
                               weak_factory_.GetWeakPtr(), callback));
}

void XWalkCookieStoreWrapper::RunClosureCallback(
    const base::Closure& callback) {
  DCHECK(client_task_runner_->RunsTasksOnCurrentThread());
  callback.Run();
}

}  // namespace xwalk
