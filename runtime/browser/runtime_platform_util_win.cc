// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/runtime_platform_util.h"

#include <commdlg.h>
#include <dwmapi.h>
#include <shellapi.h>
#include <shlobj.h>
#include <windows.h>

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/files/file_util.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/win/registry.h"
#include "base/win/scoped_co_mem.h"
#include "base/win/scoped_comptr.h"
#include "base/win/windows_version.h"
#include "content/public/browser/browser_thread.h"
#include "ui/base/win/shell.h"
#include "ui/gfx/native_widget_types.h"
#include "url/gurl.h"

using content::BrowserThread;

namespace {

void ShowItemInFolderOnFileThread(const base::FilePath& full_path) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::FILE));
  base::FilePath dir = full_path.DirName().AsEndingWithSeparator();
  // ParseDisplayName will fail if the directory is "C:", it must be "C:\\".
  if (dir.empty())
    return;

  typedef HRESULT (WINAPI *SHOpenFolderAndSelectItemsFuncPtr)(
      PCIDLIST_ABSOLUTE pidl_Folder,
      UINT cidl,
      PCUITEMID_CHILD_ARRAY pidls,
      DWORD flags);

  static SHOpenFolderAndSelectItemsFuncPtr open_folder_and_select_itemsPtr =
    NULL;
  static bool initialize_open_folder_proc = true;
  if (initialize_open_folder_proc) {
    initialize_open_folder_proc = false;
    // The SHOpenFolderAndSelectItems API is exposed by shell32 version 6
    // and does not exist in Win2K. We attempt to retrieve this function export
    // from shell32 and if it does not exist, we just invoke ShellExecute to
    // open the folder thus losing the functionality to select the item in
    // the process.
    HMODULE shell32_base = GetModuleHandle(L"shell32.dll");
    if (!shell32_base) {
      NOTREACHED() << " " << __FUNCTION__ << "(): Can't open shell32.dll";
      return;
    }
    open_folder_and_select_itemsPtr =
        reinterpret_cast<SHOpenFolderAndSelectItemsFuncPtr>
            (GetProcAddress(shell32_base, "SHOpenFolderAndSelectItems"));
  }
  if (!open_folder_and_select_itemsPtr) {
    ShellExecute(NULL, L"open", dir.value().c_str(), NULL, NULL, SW_SHOW);
    return;
  }

  base::win::ScopedComPtr<IShellFolder> desktop;
  HRESULT hr = SHGetDesktopFolder(desktop.Receive());
  if (FAILED(hr))
    return;

  base::win::ScopedCoMem<ITEMIDLIST> dir_item;
  hr = desktop->ParseDisplayName(NULL, NULL,
                                 const_cast<wchar_t *>(dir.value().c_str()),
                                 NULL, &dir_item, NULL);
  if (FAILED(hr))
    return;

  base::win::ScopedCoMem<ITEMIDLIST> file_item;
  hr = desktop->ParseDisplayName(NULL, NULL,
      const_cast<wchar_t *>(full_path.value().c_str()),
      NULL, &file_item, NULL);
  if (FAILED(hr))
    return;

  const ITEMIDLIST* highlight[] = {
    {file_item},
  };

  hr = (*open_folder_and_select_itemsPtr)(dir_item, arraysize(highlight),
                                          highlight, NULL);

  if (FAILED(hr)) {
    // On some systems, the above call mysteriously fails with "file not
    // found" even though the file is there.  In these cases, ShellExecute()
    // seems to work as a fallback (although it won't select the file).
    if (hr == ERROR_FILE_NOT_FOUND) {
      ShellExecute(NULL, L"open", dir.value().c_str(), NULL, NULL, SW_SHOW);
    } else {
      LPTSTR message = NULL;
      FormatMessage(
          FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
          0, hr, 0, reinterpret_cast<LPTSTR>(&message), 0, NULL);
      LOG(WARNING) << " " << __FUNCTION__
                   << "(): Can't open full_path = \""
                   << full_path.value() << "\""
                   << " hr = " << hr
                   << " " << reinterpret_cast<LPTSTR>(&message);
      if (message)
        LocalFree(message);
    }
  }
}

// Old ShellExecute crashes the process when the command for a given scheme
// is empty. This function tells if it is.
bool ValidateShellCommandForScheme(const std::string& scheme) {
  base::win::RegKey key;
  std::wstring registry_path = base::ASCIIToUTF16(scheme) +
                               L"\\shell\\open\\command";
  key.Open(HKEY_CLASSES_ROOT, registry_path.c_str(), KEY_READ);
  if (!key.Valid())
    return false;
  DWORD size = 0;
  key.ReadValue(NULL, NULL, &size, NULL);
  if (size <= 2)
    return false;
  return true;
}

}  // namespace

namespace platform_util {

void ShowItemInFolder(const base::FilePath& full_path) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  BrowserThread::PostTask(BrowserThread::FILE, FROM_HERE,
      base::Bind(&ShowItemInFolderOnFileThread, full_path));
}

void OpenItem(const base::FilePath& full_path, OpenItemType item_type) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  switch (item_type) {
  case OPEN_FILE:
    BrowserThread::PostTask(
      BrowserThread::FILE, FROM_HERE,
      base::Bind(base::IgnoreResult(&ui::win::OpenFileViaShell), full_path));
    break;
  case OPEN_FOLDER:
    BrowserThread::PostTask(
      BrowserThread::FILE, FROM_HERE,
      base::Bind(base::IgnoreResult(&ui::win::OpenFolderViaShell), full_path));
    break;
  }
}

void OpenExternal(const GURL& url) {
  // Quote the input scheme to be sure that the command does not have
  // parameters unexpected by the external program. This url should already
  // have been escaped.
  std::string escaped_url = url.spec();
  escaped_url.insert(0, "\"");
  escaped_url += "\"";

  // According to Mozilla in uriloader/exthandler/win/nsOSHelperAppService.cpp:
  // "Some versions of windows (Win2k before SP3, Win XP before SP1) crash in
  // ShellExecute on long URLs (bug 161357 on bugzilla.mozilla.org). IE 5 and 6
  // support URLS of 2083 chars in length, 2K is safe."
  const size_t kMaxUrlLength = 2048;
  if (escaped_url.length() > kMaxUrlLength) {
    NOTREACHED();
    return;
  }

  if (base::win::GetVersion() < base::win::VERSION_WIN7) {
    if (!ValidateShellCommandForScheme(url.scheme()))
      return;
  }

  if (reinterpret_cast<ULONG_PTR>(ShellExecuteA(NULL, "open",
                                                escaped_url.c_str(), NULL, NULL,
                                                SW_SHOWNORMAL)) <= 32) {
    // We fail to execute the call. We could display a message to the user.
    // TODO(nsylvain): we should also add a dialog to warn on errors. See
    // bug 1136923.
    return;
  }
}

#if !defined(USE_AURA)
gfx::NativeWindow GetTopLevel(gfx::NativeView view) {
  return ::GetAncestor(view, GA_ROOT);
}

gfx::NativeView GetParent(gfx::NativeView view) {
  return ::GetParent(view);
}

bool IsWindowActive(gfx::NativeWindow window) {
  return ::GetForegroundWindow() == window;
}

void ActivateWindow(gfx::NativeWindow window) {
  ::SetForegroundWindow(window);
}

bool IsVisible(gfx::NativeView view) {
  // MSVC complains if we don't include != 0.
  return ::IsWindowVisible(view) != 0;
}
#endif

static void GetCurrentDisplaySettings(bool *landscape, bool *flipped) {
  DEVMODE dm;
  ZeroMemory(&dm, sizeof(dm));
  dm.dmSize = sizeof(dm);
  if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
    return;
  if (flipped) {
    *flipped = (dm.dmDisplayOrientation == DMDO_270
      || dm.dmDisplayOrientation == DMDO_180);
  }
  if (landscape)
    *landscape = (dm.dmPelsWidth > dm.dmPelsHeight);
}

// SetDisplayAutoRotationPreferences is available on Windows 8 and after.
static void SetDisplayAutoRotationPreferencesWrapper(
  ORIENTATION_PREFERENCE orientation) {
  typedef void(WINAPI *SetDisplayAutoRotationPreferencesPtr)(
    ORIENTATION_PREFERENCE);
  static SetDisplayAutoRotationPreferencesPtr
    set_display_auto_rotation_preferences_func =
      reinterpret_cast<SetDisplayAutoRotationPreferencesPtr>(
        GetProcAddress(GetModuleHandleA("user32.dll"),
        "SetDisplayAutoRotationPreferences"));
  if (set_display_auto_rotation_preferences_func)
    set_display_auto_rotation_preferences_func(orientation);
}

static void SetScreenOrientation(const std::string& orientation) {
  ORIENTATION_PREFERENCE prefs = ORIENTATION_PREFERENCE_NONE;
  bool landscape = true;
  bool flipped = false;
  if (orientation == "portrait-primary") {
    prefs = ORIENTATION_PREFERENCE_PORTRAIT;
  } else if (orientation == "portrait-secondary") {
    prefs = ORIENTATION_PREFERENCE_PORTRAIT_FLIPPED;
  } else if (orientation == "landscape-primary") {
    prefs = ORIENTATION_PREFERENCE_LANDSCAPE;
  } else if (orientation == "landscape-secondary") {
    prefs = ORIENTATION_PREFERENCE_LANDSCAPE_FLIPPED;
  } else if (orientation == "portrait") {
    GetCurrentDisplaySettings(&landscape, &flipped);
    prefs = (flipped && !landscape) ? ORIENTATION_PREFERENCE_PORTRAIT_FLIPPED
                                    : ORIENTATION_PREFERENCE_PORTRAIT;
  } else if (orientation == "landscape") {
    GetCurrentDisplaySettings(&landscape, &flipped);
    prefs = (flipped && landscape) ? ORIENTATION_PREFERENCE_LANDSCAPE_FLIPPED
                                   : ORIENTATION_PREFERENCE_LANDSCAPE;
  } else if (orientation == "natural") {
    GetCurrentDisplaySettings(&landscape, &flipped);
    prefs = landscape ? ORIENTATION_PREFERENCE_LANDSCAPE
                      : ORIENTATION_PREFERENCE_PORTRAIT;
  } else if (orientation == "any") {
    GetCurrentDisplaySettings(&landscape, &flipped);
    if (landscape) {
      prefs = flipped ? ORIENTATION_PREFERENCE_LANDSCAPE_FLIPPED
                      : ORIENTATION_PREFERENCE_LANDSCAPE;
    } else {
      prefs = flipped ? ORIENTATION_PREFERENCE_PORTRAIT_FLIPPED
                      : ORIENTATION_PREFERENCE_PORTRAIT;
    }
  } else {
    NOTREACHED() << "Invalid value for orientation:" << orientation;
  }
  SetDisplayAutoRotationPreferencesWrapper(prefs);
}

void SetPreferredScreenOrientation(const std::string& orientation) {
  if (base::win::GetVersion() >= base::win::VERSION_WIN8)
    SetScreenOrientation(orientation);
}

}  // namespace platform_util
