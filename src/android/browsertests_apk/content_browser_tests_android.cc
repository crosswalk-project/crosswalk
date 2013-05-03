// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This class sets up the environment for running the content browser tests
// inside an android application.

#include <android/log.h>

#include "base/android/base_jni_registrar.h"
#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/android/scoped_java_ref.h"
#include "base/base_switches.h"
#include "base/command_line.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/string_util.h"
#include "base/stringprintf.h"
#include "base/strings/string_tokenizer.h"
#include "cameo/src/android/shell_jni_registrar.h"
#include "cameo/src/shell_main_delegate.h"
#include "content/public/app/android_library_loader_hooks.h"
#include "content/public/app/content_main.h"
#include "content/public/common/content_switches.h"
#include "content/public/test/test_launcher.h"
#include "content/test/browser_test_message_pump_android.h"
#include "jni/ContentBrowserTestsActivity_jni.h"
#include "testing/android/native_test_util.h"

using testing::native_test_util::ArgsToArgv;
using testing::native_test_util::CreateFIFO;
using testing::native_test_util::ParseArgsFromCommandLineFile;
using testing::native_test_util::RedirectStream;
using testing::native_test_util::ScopedMainEntryLogger;

// The main function of the program to be wrapped as an apk.
extern int main(int argc, char** argv);

namespace {

// The test runner script writes the command line file in
// "/data/local/tmp".
static const char kCommandLineFilePath[] =
    "/data/local/tmp/content-browser-tests-command-line";

}  // namespace

namespace content {

static void RunTests(JNIEnv* env,
                     jobject obj,
                     jstring jfiles_dir,
                     jobject app_context) {

  // Command line basic initialization, will be fully initialized later.
  static const char* const kInitialArgv[] = { "ContentBrowserTestsActivity" };
  CommandLine::Init(arraysize(kInitialArgv), kInitialArgv);

  // Set the application context in base.
  base::android::ScopedJavaLocalRef<jobject> scoped_context(
      env, env->NewLocalRef(app_context));
  base::android::InitApplicationContext(scoped_context);
  base::android::RegisterJni(env);

  std::vector<std::string> args;
  ParseArgsFromCommandLineFile(kCommandLineFilePath, &args);

  std::vector<char*> argv;
  int argc = ArgsToArgv(args, &argv);

  // Fully initialize command line with arguments.
  CommandLine* command_line = CommandLine::ForCurrentProcess();
  command_line->AppendArguments(CommandLine(argc, &argv[0]), false);

  // Append required switches.
  command_line->AppendSwitch(content::kSingleProcessTestsFlag);
  command_line->AppendSwitch(switches::kUseFakeDeviceForMediaStream);

  // Create fifo and redirect stdout and stderr to it.
  base::FilePath files_dir(
      base::android::ConvertJavaStringToUTF8(env, jfiles_dir));
  base::FilePath fifo_path(files_dir.Append(base::FilePath("test.fifo")));
  CreateFIFO(fifo_path.value().c_str());
  RedirectStream(stdout, fifo_path.value().c_str(), "w");
  dup2(STDOUT_FILENO, STDERR_FILENO);

  ScopedMainEntryLogger scoped_main_entry_logger;
  main(argc, &argv[0]);
}
}  // namespace content

// This is called by the VM when the shared library is first loaded.
JNI_EXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
  base::android::InitVM(vm);
  JNIEnv* env = base::android::AttachCurrentThread();

  if (!content::RegisterLibraryLoaderEntryHook(env))
    return -1;

  if (!content::android::RegisterShellJni(env))
    return -1;

  if (!content::BrowserTestMessagePumpAndroid::RegisterJni(env))
    return -1;

  if (!content::RegisterNativesImpl(env))
    return -1;

  content::SetContentMainDelegate(new content::ShellMainDelegate());
  return JNI_VERSION_1_4;
}
