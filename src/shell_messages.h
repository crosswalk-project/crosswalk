// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Multiply-included file, no traditional include guard.
#include <string>
#include <vector>

#include "content/public/common/common_param_traits.h"
#include "content/shell/shell_test_configuration.h"
#include "ipc/ipc_message_macros.h"
#include "ipc/ipc_platform_file.h"
#include "third_party/skia/include/core/SkBitmap.h"

#define IPC_MESSAGE_START ShellMsgStart

IPC_STRUCT_TRAITS_BEGIN(content::ShellTestConfiguration)
IPC_STRUCT_TRAITS_MEMBER(current_working_directory)
IPC_STRUCT_TRAITS_MEMBER(temp_path)
IPC_STRUCT_TRAITS_MEMBER(test_url)
IPC_STRUCT_TRAITS_MEMBER(enable_pixel_dumping)
IPC_STRUCT_TRAITS_MEMBER(layout_test_timeout)
IPC_STRUCT_TRAITS_MEMBER(allow_external_pages)
IPC_STRUCT_TRAITS_MEMBER(expected_pixel_hash)
IPC_STRUCT_TRAITS_END()

// Tells the renderer to reset all test runners.
IPC_MESSAGE_ROUTED0(ShellViewMsg_Reset)

// Sets the path to the WebKit checkout.
IPC_MESSAGE_CONTROL1(ShellViewMsg_SetWebKitSourceDir,
                     base::FilePath /* webkit source dir */)

// Loads the hyphen dictionary used for layout tests.
IPC_MESSAGE_CONTROL1(ShellViewMsg_LoadHyphenDictionary,
                     IPC::PlatformFileForTransit /* dict_file */)

// Sets the initial configuration to use for layout tests.
IPC_MESSAGE_ROUTED1(ShellViewMsg_SetTestConfiguration,
                    content::ShellTestConfiguration)

// Tells the main window that a secondary window in a different process invoked
// notifyDone().
IPC_MESSAGE_ROUTED0(ShellViewMsg_NotifyDone)

// Pushes a snapshot of the current session history from the browser process.
// This includes only information about those RenderViews that are in the
// same process as the main window of the layout test and that are the current
// active RenderView of their WebContents.
IPC_MESSAGE_ROUTED3(
    ShellViewMsg_SessionHistory,
    std::vector<int> /* routing_ids */,
    std::vector<std::vector<std::string> > /* session_histories */,
    std::vector<unsigned> /* current_entry_indexes */)

// Send a text dump of the WebContents to the render host.
IPC_MESSAGE_ROUTED1(ShellViewHostMsg_TextDump,
                    std::string /* dump */)

// Send an image dump of the WebContents to the render host.
IPC_MESSAGE_ROUTED2(ShellViewHostMsg_ImageDump,
                    std::string /* actual pixel hash */,
                    SkBitmap /* image */)

// Send an audio dump to the render host.
IPC_MESSAGE_ROUTED1(ShellViewHostMsg_AudioDump,
                    std::vector<unsigned char> /* audio data */)

IPC_MESSAGE_ROUTED1(ShellViewHostMsg_TestFinished,
                    bool /* did_timeout */)

IPC_MESSAGE_ROUTED0(ShellViewHostMsg_ResetDone)

IPC_MESSAGE_ROUTED0(ShellViewHostMsg_TestFinishedInSecondaryWindow)

// WebTestDelegate related.
IPC_MESSAGE_ROUTED1(ShellViewHostMsg_OverridePreferences,
                    webkit_glue::WebPreferences /* preferences */)
IPC_SYNC_MESSAGE_ROUTED1_1(ShellViewHostMsg_RegisterIsolatedFileSystem,
                           std::vector<base::FilePath> /* absolute_filenames */,
                           std::string /* filesystem_id */)
IPC_SYNC_MESSAGE_ROUTED1_1(ShellViewHostMsg_ReadFileToString,
                           base::FilePath /* local path */,
                           std::string /* contents */)
IPC_MESSAGE_ROUTED1(ShellViewHostMsg_PrintMessage,
                    std::string /* message */)
IPC_MESSAGE_ROUTED0(ShellViewHostMsg_ShowDevTools)
IPC_MESSAGE_ROUTED0(ShellViewHostMsg_CloseDevTools)
IPC_MESSAGE_ROUTED1(ShellViewHostMsg_GoToOffset,
                    int /* offset */)
IPC_MESSAGE_ROUTED0(ShellViewHostMsg_Reload)
IPC_MESSAGE_ROUTED2(ShellViewHostMsg_LoadURLForFrame,
                    GURL /* url */,
                    std::string /* frame_name */)
IPC_MESSAGE_ROUTED0(ShellViewHostMsg_ClearAllDatabases)
IPC_MESSAGE_ROUTED1(ShellViewHostMsg_SetDatabaseQuota,
                    int /* quota */)
IPC_MESSAGE_ROUTED1(ShellViewHostMsg_AcceptAllCookies,
                    bool /* accept */)
IPC_MESSAGE_ROUTED1(ShellViewHostMsg_SetDeviceScaleFactor,
                    float /* factor */)
IPC_MESSAGE_ROUTED0(ShellViewHostMsg_CaptureSessionHistory)
IPC_MESSAGE_ROUTED0(ShellViewHostMsg_CloseRemainingWindows)
