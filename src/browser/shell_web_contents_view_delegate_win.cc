// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/browser/shell_web_contents_view_delegate.h"

#include "base/command_line.h"
#include "cameo/src/browser/shell.h"
#include "cameo/src/browser/shell_browser_context.h"
#include "cameo/src/browser/shell_browser_main_parts.h"
#include "cameo/src/browser/shell_content_browser_client.h"
#include "cameo/src/browser/shell_devtools_frontend.h"
#include "cameo/src/browser/shell_web_contents_view_delegate_creator.h"
#include "cameo/src/common/shell_switches.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "content/public/common/context_menu_params.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebContextMenuData.h"

using WebKit::WebContextMenuData;

namespace {

enum {
  ShellContextMenuItemCutId = 10001,
  ShellContextMenuItemCopyId,
  ShellContextMenuItemPasteId,
  ShellContextMenuItemDeleteId,
  ShellContextMenuItemOpenLinkId,
  ShellContextMenuItemBackId,
  ShellContextMenuItemForwardId,
  ShellContextMenuItemReloadId,
  ShellContextMenuItemInspectId
};

void MakeContextMenuItem(HMENU menu,
                         int menu_index,
                         LPTSTR text,
                         UINT id,
                         bool enabled) {
  MENUITEMINFO mii = {0};
  mii.cbSize = sizeof(mii);
  mii.fMask = MIIM_FTYPE | MIIM_ID | MIIM_DATA | MIIM_STRING | MIIM_STATE;
  mii.fState = enabled ? MFS_ENABLED : (MF_DISABLED | MFS_GRAYED);
  mii.fType = MFT_STRING;
  mii.wID = id;
  mii.dwTypeData = text;

  InsertMenuItem(menu, menu_index, TRUE, &mii);
}

}  // namespace

namespace cameo {

content::WebContentsViewDelegate* CreateShellWebContentsViewDelegate(
    content::WebContents* web_contents) {
  return new ShellWebContentsViewDelegate(web_contents);
}

ShellWebContentsViewDelegate::ShellWebContentsViewDelegate(
    content::WebContents* web_contents)
    : web_contents_(web_contents) {
}

ShellWebContentsViewDelegate::~ShellWebContentsViewDelegate() {
}

void ShellWebContentsViewDelegate::ShowContextMenu(
    const content::ContextMenuParams& params,
    content::ContextMenuSourceType type) {
  params_ = params;
  bool has_link = !params_.unfiltered_link_url.is_empty();
  bool has_selection = !params_.selection_text.empty();

  HMENU menu = CreateMenu();
  HMENU sub_menu = CreatePopupMenu();
  AppendMenu(menu, MF_STRING | MF_POPUP, (UINT)sub_menu, L"");

  int index = 0;
  if (params_.media_type == WebContextMenuData::MediaTypeNone &&
      !has_link &&
      !has_selection &&
      !params_.is_editable) {
    MakeContextMenuItem(sub_menu,
                        index++,
                        L"Back",
                        ShellContextMenuItemBackId,
                        web_contents_->GetController().CanGoBack());

    MakeContextMenuItem(sub_menu,
                        index++,
                        L"Forward",
                        ShellContextMenuItemForwardId,
                        web_contents_->GetController().CanGoForward());

    MakeContextMenuItem(sub_menu,
                        index++,
                        L"Reload",
                        ShellContextMenuItemReloadId,
                        true);

    AppendMenu(sub_menu, MF_SEPARATOR, 0, NULL);
    index++;
  }

  if (has_link) {
    MakeContextMenuItem(sub_menu,
                        index++,
                        L"Open in New Window",
                        ShellContextMenuItemOpenLinkId,
                        true);
    AppendMenu(sub_menu, MF_SEPARATOR, 0, NULL);
    index++;
  }

  if (params_.is_editable) {
    bool cut_enabled = ((params_.edit_flags & WebContextMenuData::CanCut) != 0);
    MakeContextMenuItem(sub_menu,
                        index++,
                        L"Cut",
                        ShellContextMenuItemCutId,
                        cut_enabled);

    bool copy_enabled =
        ((params_.edit_flags & WebContextMenuData::CanCopy) != 0);
    MakeContextMenuItem(sub_menu,
                        index++,
                        L"Copy",
                        ShellContextMenuItemCopyId,
                        copy_enabled);

    bool paste_enabled =
        ((params_.edit_flags & WebContextMenuData::CanPaste) != 0);
    MakeContextMenuItem(sub_menu,
                        index++,
                        L"Paste",
                        ShellContextMenuItemPasteId,
                        paste_enabled);
    bool delete_enabled =
        ((params_.edit_flags & WebContextMenuData::CanDelete) != 0);
    MakeContextMenuItem(sub_menu,
                        index++,
                        L"Delete",
                        ShellContextMenuItemDeleteId,
                        delete_enabled);

    AppendMenu(sub_menu, MF_SEPARATOR, 0, NULL);
    index++;
  } else if (has_selection) {
    MakeContextMenuItem(sub_menu,
                        index++,
                        L"Copy",
                        ShellContextMenuItemCopyId,
                        true);

    AppendMenu(sub_menu, MF_SEPARATOR, 0, NULL);
    index++;
  }

  MakeContextMenuItem(sub_menu,
                      index++,
                      L"Inspect...",
                      ShellContextMenuItemInspectId,
                      true);
#if defined(USE_AURA)
  NOTIMPLEMENTED();
#else
  gfx::Point screen_point(params.x, params.y);
  POINT point = screen_point.ToPOINT();
  ClientToScreen(web_contents_->GetView()->GetNativeView(), &point);

  int selection =
      TrackPopupMenu(sub_menu,
                     TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
                     point.x, point.y,
                     0,
                     web_contents_->GetView()->GetContentNativeView(),
                     NULL);

  MenuItemSelected(selection);
#endif
  DestroyMenu(menu);
}

void ShellWebContentsViewDelegate::MenuItemSelected(int selection) {
  switch (selection) {
    case ShellContextMenuItemCutId:
      web_contents_->GetRenderViewHost()->Cut();
      break;
    case ShellContextMenuItemCopyId:
      web_contents_->GetRenderViewHost()->Copy();
      break;
    case ShellContextMenuItemPasteId:
      web_contents_->GetRenderViewHost()->Paste();
      break;
    case ShellContextMenuItemDeleteId:
      web_contents_->GetRenderViewHost()->Delete();
      break;
    case ShellContextMenuItemOpenLinkId: {
      ShellBrowserContext* browser_context =
          ShellContentBrowserClient::Get()->browser_context();
      Shell::CreateNewWindow(browser_context,
                             params_.link_url,
                             NULL,
                             MSG_ROUTING_NONE,
                             gfx::Size());
      break;
    }
    case ShellContextMenuItemBackId:
      web_contents_->GetController().GoToOffset(-1);
      web_contents_->GetView()->Focus();
      break;
    case ShellContextMenuItemForwardId:
      web_contents_->GetController().GoToOffset(1);
      web_contents_->GetView()->Focus();
      break;
    case ShellContextMenuItemReloadId:
      web_contents_->GetController().Reload(false);
      web_contents_->GetView()->Focus();
      break;
    case ShellContextMenuItemInspectId: {
      ShellDevToolsFrontend::Show(web_contents_);
      break;
    }
  }
}

content::WebDragDestDelegate*
ShellWebContentsViewDelegate::GetDragDestDelegate() {
  return NULL;
}

void ShellWebContentsViewDelegate::StoreFocus() {
}

void ShellWebContentsViewDelegate::RestoreFocus() {
}

bool ShellWebContentsViewDelegate::Focus() {
  return false;
}

void ShellWebContentsViewDelegate::TakeFocus(bool reverse) {
}

void ShellWebContentsViewDelegate::SizeChanged(const gfx::Size& size) {
}

}  // namespace cameo