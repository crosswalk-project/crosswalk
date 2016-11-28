// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/android/cookie_manager.h"

#include <string>

#include "base/android/jni_string.h"
#include "base/android/path_utils.h"
#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/message_loop/message_loop.h"
#include "base/lazy_instance.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/single_thread_task_runner.h"
#include "base/synchronization/lock.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/sequenced_worker_pool.h"
#include "base/threading/thread.h"
#include "base/threading/thread_restrictions.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/cookie_store_factory.h"
#include "content/public/common/url_constants.h"

#include "jni/XWalkCookieManagerInternal_jni.h"
#include "net/cookies/cookie_monster.h"
#include "net/cookies/cookie_options.h"
#include "net/cookies/cookie_store.h"
#include "net/url_request/url_request_context.h"
#include "ui/base/resource/resource_bundle.h"
#include "xwalk/runtime/browser/android/scoped_allow_wait_for_legacy_web_view_api.h"
#include "xwalk/runtime/browser/android/xwalk_cookie_access_policy.h"
#include "xwalk/runtime/browser/xwalk_browser_main_parts_android.h"
#include "xwalk/runtime/common/xwalk_switches.h"

using base::FilePath;
using base::android::ConvertJavaStringToUTF8;
using base::android::ConvertJavaStringToUTF16;
using content::BrowserThread;
using net::CookieList;

// All functions on the CookieManager can be called from any thread, including
// threads without a message loop. BrowserThread::FILE is used to call methods
// on CookieMonster that needs to be called, and called back, on a chrome
// thread.

namespace xwalk {

namespace {

// Are cookies allowed for file:// URLs by default?
const bool kDefaultFileSchemeAllowed = false;

// CookieManager creates and owns XWalkView's CookieStore, in addition to
// handling calls into the CookieStore from Java.
//
// Since Java calls can be made on the IO Thread, and must synchronously return
// a result, and the CookieStore API allows it to asynchronously return results,
// the CookieStore must be run on its own thread, to prevent deadlock.
class CookieManager {
 public:
  static CookieManager* GetInstance();

  // Returns the TaskRunner on which the CookieStore lives.
  base::SingleThreadTaskRunner* GetCookieStoreTaskRunner();
  // Returns the CookieStore, creating it if necessary. This must only be called
  // on the CookieStore TaskRunner.
  net::CookieStore* GetCookieStore();

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
                           const net::CookieList& cookies);

  // This protects the following two bools, as they're used on multiple threads.
  base::Lock accept_file_scheme_cookies_lock_;
  // True if cookies should be allowed for file URLs. Can only be changed prior
  // to creating the CookieStore.
  bool accept_file_scheme_cookies_;
  // True once the cookie store has been created. Just used to track when
  // |accept_file_scheme_cookies_| can no longer be modified.
  bool cookie_store_created_;

  base::Thread cookie_store_client_thread_;
  base::Thread cookie_store_backend_thread_;

  scoped_refptr<base::SingleThreadTaskRunner> cookie_store_task_runner_;
  std::unique_ptr<net::CookieStore> cookie_store_;

  DISALLOW_COPY_AND_ASSIGN(CookieManager);
};

base::LazyInstance<CookieManager>::Leaky g_lazy_instance;

}  // namespace

// static
CookieManager* CookieManager::GetInstance() {
  return g_lazy_instance.Pointer();
}

CookieManager::CookieManager()
    : accept_file_scheme_cookies_(kDefaultFileSchemeAllowed),
      cookie_store_created_(false),
      cookie_store_client_thread_("CookieMonsterClient"),
      cookie_store_backend_thread_("CookieMonsterBackend") {
      cookie_store_client_thread_.Start();
      cookie_store_backend_thread_.Start();
      cookie_store_task_runner_ = cookie_store_client_thread_.task_runner();
}

CookieManager::~CookieManager() {
}

// Executes the |task| on the FILE thread. |wait_for_completion| should only be
// true if the Java API method returns a value or is explicitly stated to be
// synchronous.
void CookieManager::ExecCookieTask(const CookieTask& task,
                                   const bool wait_for_completion) {
  base::WaitableEvent completion(
      base::WaitableEvent::ResetPolicy::AUTOMATIC,
      base::WaitableEvent::InitialState::NOT_SIGNALED);
  cookie_store_task_runner_->PostTask(FROM_HERE,
      base::Bind(task, wait_for_completion ? &completion : nullptr));
  if (wait_for_completion) {
    ScopedAllowWaitForLegacyWebViewApi wait;
    completion.Wait();
  }
}

base::SingleThreadTaskRunner* CookieManager::GetCookieStoreTaskRunner() {
  return cookie_store_task_runner_.get();
}

net::CookieStore* CookieManager::GetCookieStore() {
  DCHECK(cookie_store_task_runner_->RunsTasksOnCurrentThread());

  if (!cookie_store_) {
    FilePath user_data_dir;
    xwalk::GetUserDataDir(&user_data_dir);
    base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
    base::FilePath profile = user_data_dir.Append(
        command_line->GetSwitchValuePath(switches::kXWalkProfileName));
    FilePath cookie_store_path = profile.Append(FILE_PATH_LITERAL("Cookies"));

    content::CookieStoreConfig cookie_config(
        cookie_store_path, content::CookieStoreConfig::RESTORED_SESSION_COOKIES,
        nullptr, nullptr);
    cookie_config.client_task_runner = cookie_store_task_runner_;
    cookie_config.background_task_runner =
        cookie_store_backend_thread_.task_runner();

    {
      base::AutoLock lock(accept_file_scheme_cookies_lock_);

      // There are some unknowns about how to correctly handle file:// cookies,
      // and our implementation for this is not robust.  http://crbug.com/582985
      //
      // TODO(mmenke): This call should be removed once we can deprecate and
      // remove the Android WebView 'CookieManager::setAcceptFileSchemeCookies'
      // method. Until then, note that this is just not a great idea.
      cookie_config.cookieable_schemes.insert(
          cookie_config.cookieable_schemes.begin(),
          net::CookieMonster::kDefaultCookieableSchemes,
          net::CookieMonster::kDefaultCookieableSchemes +
              net::CookieMonster::kDefaultCookieableSchemesCount);
      if (accept_file_scheme_cookies_)
        cookie_config.cookieable_schemes.push_back(url::kFileScheme);
      cookie_store_created_ = true;
    }

    cookie_store_ = content::CreateCookieStore(cookie_config);
  }

  return cookie_store_.get();
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

  GetCookieStore()->SetCookieWithOptionsAsync(
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

  GetCookieStore()->GetCookiesWithOptionsAsync(
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
  GetCookieStore()->DeleteSessionCookiesAsync(
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
  GetCookieStore()->DeleteAllAsync(
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
  GetCookieStore()->FlushStore(base::Bind(&base::DoNothing));
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
  GetCookieStore()->GetAllCookiesAsync(
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
  base::AutoLock lock(accept_file_scheme_cookies_lock_);
  return accept_file_scheme_cookies_;
}

void CookieManager::SetAcceptFileSchemeCookies(bool accept) {
    base::AutoLock lock(accept_file_scheme_cookies_lock_);
    // Can only modify this before the cookie store is created.
    if (!cookie_store_created_) {
      accept_file_scheme_cookies_ = accept;
    }
}

static void SetAcceptCookie(JNIEnv* env,
                            const JavaParamRef<jobject>& obj,
                            jboolean accept) {
  CookieManager::GetInstance()->SetAcceptCookie(accept);
}

static jboolean AcceptCookie(JNIEnv* env, const JavaParamRef<jobject>& obj) {
  return CookieManager::GetInstance()->AcceptCookie();
}

static void SetCookie(JNIEnv* env,
                      const JavaParamRef<jobject>& obj,
                      const JavaParamRef<jstring>& url,
                      const JavaParamRef<jstring>& value) {
  GURL host(ConvertJavaStringToUTF16(env, url));
  std::string cookie_value(ConvertJavaStringToUTF8(env, value));

  CookieManager::GetInstance()->SetCookie(host, cookie_value);
}

static ScopedJavaLocalRef<jstring>
GetCookie(JNIEnv* env,
          const JavaParamRef<jobject>& obj,
          const JavaParamRef<jstring>& url) {
  GURL host(ConvertJavaStringToUTF16(env, url));

  return base::android::ConvertUTF8ToJavaString(
      env, CookieManager::GetInstance()->GetCookie(host));
}

static void RemoveSessionCookie(JNIEnv* env, const JavaParamRef<jobject>& obj) {
  CookieManager::GetInstance()->RemoveSessionCookie();
}

static void RemoveAllCookie(JNIEnv* env, const JavaParamRef<jobject>& obj) {
  CookieManager::GetInstance()->RemoveAllCookie();
}

static void RemoveExpiredCookie(JNIEnv* env, const JavaParamRef<jobject>& obj) {
  CookieManager::GetInstance()->RemoveExpiredCookie();
}

static void FlushCookieStore(JNIEnv* env, const JavaParamRef<jobject>& obj) {
  CookieManager::GetInstance()->FlushCookieStore();
}

static jboolean HasCookies(JNIEnv* env, const JavaParamRef<jobject>& obj) {
  return CookieManager::GetInstance()->HasCookies();
}

static jboolean AllowFileSchemeCookies(JNIEnv* env,
                                       const JavaParamRef<jobject>& obj) {
  return CookieManager::GetInstance()->AllowFileSchemeCookies();
}

static void SetAcceptFileSchemeCookies(JNIEnv* env,
                                       const JavaParamRef<jobject>& obj,
                                       jboolean accept) {
  return CookieManager::GetInstance()->SetAcceptFileSchemeCookies(accept);
}

// The following two methods are used to avoid a circular project dependency.
// TODO(mmenke):  This is weird. Maybe there should be a leaky Singleton in
// browser/net that creates and owns there?

scoped_refptr<base::SingleThreadTaskRunner> GetCookieStoreTaskRunner() {
  return CookieManager::GetInstance()->GetCookieStoreTaskRunner();
}

net::CookieStore* GetCookieStore() {
  return CookieManager::GetInstance()->GetCookieStore();
}

bool RegisterCookieManager(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace xwalk
