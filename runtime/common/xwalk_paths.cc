// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/common/xwalk_paths.h"

#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/path_service.h"
#include "xwalk/runtime/browser/xwalk_runner.h"

#if defined(OS_WIN)
#include "base/base_paths_win.h"
#elif defined(OS_LINUX)
#include "base/environment.h"
#include "base/nix/xdg_util.h"
#elif defined(OS_MACOSX) && !defined(OS_IOS)
#import "base/mac/mac_util.h"
#endif

namespace xwalk {

namespace {

#if defined(OS_MACOSX) && !defined(OS_IOS)

base::FilePath GetVersionedDirectory() {
  // Start out with the path to the running executable.
  base::FilePath path;
  PathService::Get(base::FILE_EXE, &path);

  // One step up to MacOS, another to Contents.
  path = path.DirName().DirName();
  DCHECK_EQ(path.BaseName().value(), "Contents");

  if (base::mac::IsBackgroundOnlyProcess()) {
    // path identifies the helper .app's Contents directory in the browser
    // .app's versioned directory.  Go up two steps to get to the browser
    // .app's versioned directory.
    path = path.DirName().DirName();
  }

  return path;
}

base::FilePath GetFrameworkBundlePath() {
  // It's tempting to use +[NSBundle bundleWithIdentifier:], but it's really
  // slow (about 30ms on 10.5 and 10.6), despite Apple's documentation stating
  // that it may be more efficient than +bundleForClass:.  +bundleForClass:
  // itself takes 1-2ms.  Getting an NSBundle from a path, on the other hand,
  // essentially takes no time at all, at least when the bundle has already
  // been loaded as it will have been in this case.  The FilePath operations
  // needed to compute the framework's path are also effectively free, so that
  // is the approach that is used here.  NSBundle is also documented as being
  // not thread-safe, and thread safety may be a concern here.

  // The framework bundle is at a known path and name from the browser .app's
  // versioned directory.
  return GetVersionedDirectory().Append("XWalk");
}
#endif

// File name of the internal NaCl plugin on different platforms.
const base::FilePath::CharType kInternalNaClPluginFileName[] =
#if defined(OS_WIN)
    FILE_PATH_LITERAL("ppGoogleNaClPluginChrome.dll");
#elif defined(OS_MACOSX)
    // TODO(noelallen) Please verify this extention name is correct.
    FILE_PATH_LITERAL("ppGoogleNaClPluginChrome.plugin");
#else  // Linux and Chrome OS
    FILE_PATH_LITERAL("libppGoogleNaClPluginChrome.so");
#endif

#if defined(OS_LINUX)
base::FilePath GetConfigPath() {
  scoped_ptr<base::Environment> env(base::Environment::Create());
  return base::nix::GetXDGDirectory(
      env.get(), base::nix::kXdgConfigHomeEnvVar, base::nix::kDotConfigDir);
}
#endif

bool GetXWalkDataPath(base::FilePath* path) {
  base::FilePath::StringType xwalk_suffix;
  if (XWalkRunner::GetInstance()->is_running_as_service())
    xwalk_suffix = FILE_PATH_LITERAL("xwalk-service");
  else
    xwalk_suffix = FILE_PATH_LITERAL("xwalk");
  base::FilePath cur;

#if defined(OS_WIN)
  CHECK(PathService::Get(base::DIR_LOCAL_APP_DATA, &cur));
  cur = cur.Append(xwalk_suffix);

#elif defined(OS_TIZEN)
  if (XWalkRunner::GetInstance()->is_running_as_service())
    cur = GetConfigPath().Append(xwalk_suffix);
  else
    cur = base::FilePath("/opt/usr/apps");

#elif defined(OS_LINUX)
  cur = GetConfigPath().Append(xwalk_suffix);

#elif defined(OS_MACOSX)
  CHECK(PathService::Get(base::DIR_APP_DATA, &cur));
  cur = cur.Append(xwalk_suffix);

#else
  NOTIMPLEMENTED() << "Unsupported OS platform.";
  return false;
#endif

  *path = cur;
  return true;
}

bool GetInternalPluginsDirectory(base::FilePath* result) {
#if defined(OS_MACOSX) && !defined(OS_IOS)
  // If called from Chrome, get internal plugins from a subdirectory of the
  // framework.
  if (base::mac::AmIBundled()) {
    *result = xwalk::GetFrameworkBundlePath();
    DCHECK(!result->empty());
    *result = result->Append("Internet Plug-Ins");
    return true;
  }
  // In tests, just look in the module directory (below).
#endif

  // The rest of the world expects plugins in the module directory.
  return PathService::Get(base::DIR_MODULE, result);
}

}  // namespace

bool PathProvider(int key, base::FilePath* path) {
  base::FilePath cur;
  switch (key) {
    case xwalk::DIR_DATA_PATH:
      return GetXWalkDataPath(path);
      break;
    case xwalk::DIR_INTERNAL_PLUGINS:
      if (!GetInternalPluginsDirectory(&cur))
        return false;
      break;
    case xwalk::FILE_NACL_PLUGIN:
      if (!GetInternalPluginsDirectory(&cur))
        return false;
      cur = cur.Append(kInternalNaClPluginFileName);
      break;
    // Where PNaCl files are ultimately located.  The default finds the files
    // inside the InternalPluginsDirectory / build directory, as if it
    // was shipped along with xwalk.  The value can be overridden
    // if it is installed via component updater.
    case xwalk::DIR_PNACL_COMPONENT:
#if defined(OS_MACOSX)
      // PNaCl really belongs in the InternalPluginsDirectory but actually
      // copying it there would result in the files also being shipped, which
      // we don't want yet. So for now, just find them in the directory where
      // they get built.
      if (!PathService::Get(base::DIR_EXE, &cur))
        return false;
      if (base::mac::AmIBundled()) {
        // If we're called from xwalk, it's beside the app (outside the
        // app bundle), if we're called from a unittest, we'll already be
        // outside the bundle so use the exe dir.
        // exe_dir gave us .../Chromium.app/Contents/MacOS/Chromium.
        cur = cur.DirName();
        cur = cur.DirName();
        cur = cur.DirName();
      }
#else
      if (!GetInternalPluginsDirectory(&cur))
        return false;
#endif
      cur = cur.Append(FILE_PATH_LITERAL("pnacl"));
      break;
    case xwalk::DIR_TEST_DATA:
      if (!PathService::Get(base::DIR_SOURCE_ROOT, &cur))
        return false;
      cur = cur.Append(FILE_PATH_LITERAL("xwalk"));
      cur = cur.Append(FILE_PATH_LITERAL("test"));
      cur = cur.Append(FILE_PATH_LITERAL("data"));
      break;
    case xwalk::DIR_WGT_STORAGE_PATH:
      if (!GetXWalkDataPath(&cur))
        return false;
      cur = cur.Append(FILE_PATH_LITERAL("Widget Storage"));
      break;
    default:
      return false;
  }
  *path = cur;
  return true;
}

void RegisterPathProvider() {
  PathService::RegisterProvider(PathProvider, PATH_START, PATH_END);
}

}  // namespace xwalk
