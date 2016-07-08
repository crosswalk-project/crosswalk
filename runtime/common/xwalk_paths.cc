// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/common/xwalk_paths.h"

#include <memory>

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"

#if defined(OS_WIN)
#include "base/base_paths_win.h"
#include "base/win/scoped_co_mem.h"
#include <knownfolders.h>
#include <shlobj.h>
#elif defined(OS_LINUX)
#include "base/environment.h"
#include "base/nix/xdg_util.h"
#elif defined(OS_MACOSX) && !defined(OS_IOS)
#import "base/mac/foundation_util.h"
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
    FILE_PATH_LITERAL("internal-nacl-plugin");

#if defined(OS_LINUX)
base::FilePath GetConfigPath() {
  std::unique_ptr<base::Environment> env(base::Environment::Create());
  return base::nix::GetXDGDirectory(
      env.get(), base::nix::kXdgConfigHomeEnvVar, base::nix::kDotConfigDir);
}
#endif

bool GetXWalkDataPath(base::FilePath* path) {
  base::FilePath::StringType xwalk_suffix;
  xwalk_suffix = FILE_PATH_LITERAL("xwalk");
  base::FilePath cur;

#if defined(OS_WIN)
  CHECK(PathService::Get(base::DIR_LOCAL_APP_DATA, &cur));
  cur = cur.Append(xwalk_suffix);

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

#if defined(OS_WIN)
// Generic function to call SHGetFolderPath().
bool GetUserDirectory(int csidl_folder, base::FilePath* result) {
  wchar_t path_buf[MAX_PATH];
  path_buf[0] = 0;
  if (FAILED(SHGetFolderPath(NULL, csidl_folder, NULL,
      SHGFP_TYPE_CURRENT, path_buf))) {
    return false;
  }
  *result = base::FilePath(path_buf);
  return true;
}

bool GetUserDownloadDirectory(base::FilePath* result) {
  typedef HRESULT(WINAPI *GetKnownFolderPath)(
      REFKNOWNFOLDERID, DWORD, HANDLE, PWSTR*);
  GetKnownFolderPath known_folder_path = reinterpret_cast<GetKnownFolderPath>(
      GetProcAddress(GetModuleHandle(L"shell32.dll"), "SHGetKnownFolderPath"));
  base::win::ScopedCoMem<wchar_t> path_buf;
  if (known_folder_path &&
      SUCCEEDED(known_folder_path(FOLDERID_Downloads, 0, NULL, &path_buf))) {
    *result = base::FilePath(base::string16(path_buf));
    return true;
  }

  // Fallback to MyDocuments if the above didn't work.
  if (!GetUserDirectory(CSIDL_MYDOCUMENTS, result))
    return false;

  *result = result->Append(L"Downloads");
  return true;
}
#endif

bool GetDownloadPath(base::FilePath* result) {
#if defined(OS_LINUX)
  *result = base::nix::GetXDGUserDirectory("DOWNLOAD", "Downloads");
  return true;
#elif defined(OS_WIN)
  return GetUserDownloadDirectory(result);
#else
  if (!PathService::Get(DIR_DATA_PATH, result))
    return false;
  ignore_result(result->Append(FILE_PATH_LITERAL("Downloads")));
  return true;
#endif
}

}  // namespace

bool PathProvider(int key, base::FilePath* path) {
  base::FilePath cur;
  switch (key) {
    case xwalk::DIR_DATA_PATH:
      return GetXWalkDataPath(path);
      break;
    case xwalk::DIR_LOGS:
#ifdef NDEBUG
      // Release builds write to the data dir
      return PathService::Get(xwalk::DIR_DATA_PATH, path);
#else
      // Debug builds write next to the binary (in the build tree)
#if defined(OS_MACOSX)
      if (!PathService::Get(base::DIR_EXE, path))
        return false;
      if (base::mac::AmIBundled()) {
        *path = path->DirName();
        *path = path->DirName();
        *path = path->DirName();
      }
      return true;
#else
      return PathService::Get(base::DIR_EXE, path);
#endif  // defined(OS_MACOSX)
#endif  // NDEBUG
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
    case xwalk::DIR_APPLICATION_PATH:
      if (!GetXWalkDataPath(&cur))
        return false;
      cur = cur.Append(FILE_PATH_LITERAL("applications"));
      break;
    case xwalk::DIR_DOWNLOAD_PATH:
      if (!GetDownloadPath(&cur))
        return false;
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
