// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/cookie_manager.h"

#include <string>

#include "android_webview/browser/scoped_allow_wait_for_legacy_web_view_api.h"
#include "android_webview/native/aw_browser_dependency_factory.h"
#include "base/android/jni_string.h"
#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/lazy_instance.h"
#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread_restrictions.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/common/url_constants.h"
#include "jni/XWalkCookieManager_jni.h"
#include "net/cookies/cookie_monster.h"
#include "net/cookies/cookie_options.h"
#include "net/url_request/url_request_context.h"
#include "xwalk/runtime/browser/android/xwalk_cookie_access_policy.h"

using base::android::ConvertJavaStringToUTF8;
using base::android::ConvertJavaStringToUTF16;
using content::BrowserThread;
using net::CookieList;
using net::CookieMonster;

// All functions on the CookieManager can be called from any thread, including
// threads without a message loop. BrowserThread::FILE is used to call methods
// on CookieMonster that needs to be called, and called back, on a chrome
// thread.

namespace xwalk {

namespace {

class CookieManager {
 public:
  static CookieManager* GetInstance();

  void SetCookieMonster(net::CookieMonster* cookie_monster);
  net::CookieMonster* GetCookieMonster();

  void SetAcceptCookie(bool accept);
  bool AcceptCookie();
  void SetCookie(const GURL& host, const std::string& cookie_value);
  std::string GetCookie(const GURL& host);
  void RemoveSessionCookie();
  void RemoveAllCookie();
  void RemoveExpiredCookie();
  void FlushCookieStore();
  bool HasCookies();
  bool AllowFileSchemeCookies();
  void SetAcceptFileSchemeCookies(bool accept);

 private:
  friend struct base::DefaultLazyInstanceTraits<CookieManager>;

  CookieManager();
  ~CookieManager();

  typedef base::Callback<void(base::WaitableEvent*)> CookieTask;
  void ExecCookieTask(const CookieTask& task,
                      const bool wait_for_completion);

  void SetCookieAsyncHelper(
      const GURL& host,
      const std::string& value,
      base::WaitableEvent* completion);
  void SetCookieCompleted(bool success);

  void GetCookieValueAsyncHelper(
      const GURL& host,
      std::string* result,
      base::WaitableEvent* completion);
  void GetCookieValueCompleted(base::WaitableEvent* completion,
                               std::string* result,
                               const std::string& value);

  void RemoveSessionCookieAsyncHelper(base::WaitableEvent* completion);
  void RemoveAllCookieAsyncHelper(base::WaitableEvent* completion);
  void RemoveCookiesCompleted(int num_deleted);

  void FlushCookieStoreAsyncHelper(base::WaitableEvent* completion);

  void HasCookiesAsyncHelper(bool* result,
                             base::WaitableEvent* completion);
  void HasCookiesCompleted(base::WaitableEvent* completion,
                           bool* result,
                           const CookieList& cookies);

  scoped_refptr<net::CookieMonster> cookie_monster_;

  DISALLOW_COPY_AND_ASSIGN(CookieManager);
};

base::LazyInstance<CookieManager>::Leaky g_lazy_instance;

// static
CookieManager* CookieManager::GetInstance() {
  return g_lazy_instance.Pointer();
}

CookieManager::CookieManager() {
}

CookieManager::~CookieManager() {
}

// Executes the |task| on the FILE thread. |wait_for_completion| should only be
// true if the Java API method returns a value or is explicitly stated to be
// synchronous.
void CookieManager::ExecCookieTask(const CookieTask& task,
                                   const bool wait_for_completion) {
  base::WaitableEvent completion(false, false);

  DCHECK(cookie_monster_.get());

  BrowserThread::PostTask(BrowserThread::FILE, FROM_HERE,
      base::Bind(task, wait_for_completion ? &completion : NULL));

  if (wait_for_completion) {
    ScopedAllowWaitForLegacyWebViewApi wait;
    completion.Wait();
  }
}

void CookieManager::SetCookieMonster(net::CookieMonster* cookie_monster) {
  DCHECK(!cookie_monster_.get());
  cookie_monster_ = cookie_monster;
}

net::CookieMonster* CookieManager::GetCookieMonster() {
  return cookie_monster_.get();
}

void CookieManager::SetAcceptCookie(bool accept) {
  XWalkCookieAccessPolicy::GetInstance()->SetGlobalAllowAccess(accept);
}

bool CookieManager::AcceptCookie() {
  return XWalkCookieAccessPolicy::GetInstance()->GetGlobalAllowAccess();
}

void CookieManager::SetCookie(const GURL& host,
                              const std::string& cookie_value) {
  ExecCookieTask(base::Bind(&CookieManager::SetCookieAsyncHelper,
                            base::Unretained(this),
                            host,
                            cookie_value), false);
}

void CookieManager::SetCookieAsyncHelper(
    const GURL& host,
    const std::string& value,
    base::WaitableEvent* completion) {
  DCHECK(!completion);
  net::CookieOptions options;
  options.set_include_httponly();

  cookie_monster_->SetCookieWithOptionsAsync(
      host, value, options,
      base::Bind(&CookieManager::SetCookieCompleted, base::Unretained(this)));
}

void CookieManager::SetCookieCompleted(bool success) {
  // The CookieManager API does not return a value for SetCookie,
  // so we don't need to propagate the |success| value back to the caller.
}

std::string CookieManager::GetCookie(const GURL& host) {
  std::string cookie_value;
  ExecCookieTask(base::Bind(&CookieManager::GetCookieValueAsyncHelper,
                            base::Unretained(this),
                            host,
                            &cookie_value), true);

  return cookie_value;
}

void CookieManager::GetCookieValueAsyncHelper(
    const GURL& host,
    std::string* result,
    base::WaitableEvent* completion) {
  net::CookieOptions options;
  options.set_include_httponly();

  cookie_monster_->GetCookiesWithOptionsAsync(
      host,
      options,
      base::Bind(&CookieManager::GetCookieValueCompleted,
                 base::Unretained(this),
                 completion,
                 result));
}

void CookieManager::GetCookieValueCompleted(base::WaitableEvent* completion,
                                            std::string* result,
                                            const std::string& value) {
  *result = value;
  DCHECK(completion);
  completion->Signal();
}

void CookieManager::RemoveSessionCookie() {
  ExecCookieTask(base::Bind(&CookieManager::RemoveSessionCookieAsyncHelper,
                            base::Unretained(this)), false);
}

void CookieManager::RemoveSessionCookieAsyncHelper(
    base::WaitableEvent* completion) {
  DCHECK(!completion);
  cookie_monster_->DeleteSessionCookiesAsync(
      base::Bind(&CookieManager::RemoveCookiesCompleted,
                 base::Unretained(this)));
}

void CookieManager::RemoveCookiesCompleted(int num_deleted) {
  // The CookieManager API does not return a value for removeSessionCookie or
  // removeAllCookie, so we don't need to propagate the |num_deleted| value
  // back to the caller.
}

void CookieManager::RemoveAllCookie() {
  ExecCookieTask(base::Bind(&CookieManager::RemoveAllCookieAsyncHelper,
                            base::Unretained(this)), false);
}

// TODO(kristianm): Pass a null callback so it will not be invoked
// across threads.
void CookieManager::RemoveAllCookieAsyncHelper(
    base::WaitableEvent* completion) {
  DCHECK(!completion);
  cookie_monster_->DeleteAllAsync(
      base::Bind(&CookieManager::RemoveCookiesCompleted,
                 base::Unretained(this)));
}

void CookieManager::RemoveExpiredCookie() {
  // HasCookies will call GetAllCookiesAsync, which in turn will force a GC.
  HasCookies();
}

void CookieManager::FlushCookieStoreAsyncHelper(
    base::WaitableEvent* completion) {
  DCHECK(!completion);
  cookie_monster_->FlushStore(base::Bind(&base::DoNothing));
}

void CookieManager::FlushCookieStore() {
  ExecCookieTask(base::Bind(&CookieManager::FlushCookieStoreAsyncHelper,
                            base::Unretained(this)), false);
}

bool CookieManager::HasCookies() {
  bool has_cookies;
  ExecCookieTask(base::Bind(&CookieManager::HasCookiesAsyncHelper,
                            base::Unretained(this),
                            &has_cookies), true);
  return has_cookies;
}

// TODO(kristianm): Simplify this, copying the entire list around
// should not be needed.
void CookieManager::HasCookiesAsyncHelper(bool* result,
                                  base::WaitableEvent* completion) {
  cookie_monster_->GetAllCookiesAsync(
      base::Bind(&CookieManager::HasCookiesCompleted,
                 base::Unretained(this),
                 completion,
                 result));
}

void CookieManager::HasCookiesCompleted(base::WaitableEvent* completion,
                                        bool* result,
                                        const CookieList& cookies) {
  *result = cookies.size() != 0;
  DCHECK(completion);
  completion->Signal();
}

bool CookieManager::AllowFileSchemeCookies() {
  return cookie_monster_->IsCookieableScheme(content::kFileScheme);
}

void CookieManager::SetAcceptFileSchemeCookies(bool accept) {
  // The docs on CookieManager base class state the API mustn't be called after
  // creating a CookieManager instance (which contradicts its own internal
  // implementation) but this code does rely on the essence of that comment, as
  // the monster will DCHECK here if it has already been lazy initialized (i.e.
  // if cookies have been read or written from the store). If that turns out to
  // be a problemin future, it looks like it maybe possible to relax the
  // DCHECK.
  cookie_monster_->SetEnableFileScheme(accept);
}

}  // namespace

static void SetAcceptCookie(JNIEnv* env, jobject obj, jboolean accept) {
  CookieManager::GetInstance()->SetAcceptCookie(accept);
}

static jboolean AcceptCookie(JNIEnv* env, jobject obj) {
  return CookieManager::GetInstance()->AcceptCookie();
}

static void SetCookie(JNIEnv* env, jobject obj, jstring url, jstring value) {
  GURL host(ConvertJavaStringToUTF16(env, url));
  std::string cookie_value(ConvertJavaStringToUTF8(env, value));

  CookieManager::GetInstance()->SetCookie(host, cookie_value);
}

static jstring GetCookie(JNIEnv* env, jobject obj, jstring url) {
  GURL host(ConvertJavaStringToUTF16(env, url));

  return base::android::ConvertUTF8ToJavaString(
      env,
      CookieManager::GetInstance()->GetCookie(host)).Release();
}

static void RemoveSessionCookie(JNIEnv* env, jobject obj) {
  CookieManager::GetInstance()->RemoveSessionCookie();
}

static void RemoveAllCookie(JNIEnv* env, jobject obj) {
  CookieManager::GetInstance()->RemoveAllCookie();
}

static void RemoveExpiredCookie(JNIEnv* env, jobject obj) {
  CookieManager::GetInstance()->RemoveExpiredCookie();
}

static void FlushCookieStore(JNIEnv* env, jobject obj) {
  CookieManager::GetInstance()->FlushCookieStore();
}

static jboolean HasCookies(JNIEnv* env, jobject obj) {
  return CookieManager::GetInstance()->HasCookies();
}

static jboolean AllowFileSchemeCookies(JNIEnv* env, jobject obj) {
  return CookieManager::GetInstance()->AllowFileSchemeCookies();
}

static void SetAcceptFileSchemeCookies(JNIEnv* env, jobject obj,
                                       jboolean accept) {
  return CookieManager::GetInstance()->SetAcceptFileSchemeCookies(accept);
}

void SetCookieMonsterOnNetworkStackInit(net::CookieMonster* cookie_monster) {
  CookieManager::GetInstance()->SetCookieMonster(cookie_monster);
}

net::CookieMonster* GetCookieMonster() {
  return CookieManager::GetInstance()->GetCookieMonster();
}

bool RegisterCookieManager(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace xwalk
