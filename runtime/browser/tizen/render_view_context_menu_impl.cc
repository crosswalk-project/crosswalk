// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/tizen/render_view_context_menu_impl.h"

#include "base/strings/utf_string_conversions.h"
#include "components/renderer_context_menu/views/toolkit_delegate_views.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/web_contents.h"
#include "third_party/WebKit/public/web/WebContextMenuData.h"

namespace xwalk {
using blink::WebContextMenuData;

namespace {

enum {
  // Nativation
  CMD_BACK = 0,
  CMD_FORWARD,
  CMD_RELOAD,

  // Link
  CMD_OPEN_LINK_NEW_ACTIVITY,

  // Edit
  CMD_UNDO,
  CMD_REDO,
  CMD_CUT,
  CMD_COPY,
  CMD_PASTE,
  CMD_PASTE_AND_MATCH_STYLE,
  CMD_DELETE,
  CMD_SELECT_ALL,
  CMD_LAST,
};

// Max number of custom command ids allowd.
const int kNumCustomCommandIds = 1000;

// TODO(oshima): Move IDS for context menus to components/renderer_context_menu
// and replace hardcoded strings below.
void AppendPageItems(ui::SimpleMenuModel* menu_model) {
  menu_model->AddItem(CMD_BACK, base::ASCIIToUTF16("Back"));
  menu_model->AddItem(CMD_FORWARD, base::ASCIIToUTF16("Forward"));
  menu_model->AddItem(CMD_RELOAD, base::ASCIIToUTF16("Reload"));
  menu_model->AddSeparator(ui::NORMAL_SEPARATOR);
}

void AppendLinkItems(const content::ContextMenuParams& params,
                     ui::SimpleMenuModel* menu_model) {
  if (!params.link_url.is_empty())
    menu_model->AddItem(CMD_OPEN_LINK_NEW_ACTIVITY,
                        base::ASCIIToUTF16("Open Link"));
}

void AppendEditableItems(ui::SimpleMenuModel* menu_model) {
  menu_model->AddItem(CMD_UNDO, base::ASCIIToUTF16("Undo"));
  menu_model->AddItem(CMD_REDO, base::ASCIIToUTF16("Redo"));
  menu_model->AddSeparator(ui::NORMAL_SEPARATOR);
  menu_model->AddItem(CMD_CUT, base::ASCIIToUTF16("Cut"));
  menu_model->AddItem(CMD_COPY, base::ASCIIToUTF16("Copy"));
  menu_model->AddItem(CMD_PASTE, base::ASCIIToUTF16("Paste"));
  menu_model->AddItem(CMD_PASTE_AND_MATCH_STYLE,
                      base::ASCIIToUTF16("Paste as plain text"));
  menu_model->AddItem(CMD_DELETE, base::ASCIIToUTF16("Delete"));
  menu_model->AddSeparator(ui::NORMAL_SEPARATOR);
  menu_model->AddItem(CMD_SELECT_ALL, base::ASCIIToUTF16("Select All"));
}

}  // namespace

RenderViewContextMenuImpl::RenderViewContextMenuImpl(
    content::RenderFrameHost* render_frame_host,
    const content::ContextMenuParams& params)
    : RenderViewContextMenuBase(render_frame_host, params) {
  SetContentCustomCommandIdRange(CMD_LAST, CMD_LAST + kNumCustomCommandIds);
  // TODO(oshima): Support other types
  set_content_type(
      new ContextMenuContentType(source_web_contents_, params, true));
  set_toolkit_delegate(scoped_ptr<ToolkitDelegate>(new ToolkitDelegateViews));
}

RenderViewContextMenuImpl::~RenderViewContextMenuImpl() {
}

void RenderViewContextMenuImpl::RunMenuAt(views::Widget* parent,
                                          const gfx::Point& point,
                                          ui::MenuSourceType type) {
  static_cast<ToolkitDelegateViews*>(toolkit_delegate())
      ->RunMenuAt(parent, point, type);
}

void RenderViewContextMenuImpl::InitMenu() {
  RenderViewContextMenuBase::InitMenu();
  bool needs_separator = false;
  if (content_type_->SupportsGroup(ContextMenuContentType::ITEM_GROUP_PAGE)) {
    AppendPageItems(&menu_model_);
    needs_separator = true;
  }

  if (content_type_->SupportsGroup(ContextMenuContentType::ITEM_GROUP_LINK)) {
    if (needs_separator)
      AddSeparator();
    AppendLinkItems(params_, &menu_model_);
    needs_separator = true;
  }

  if (content_type_->SupportsGroup(
          ContextMenuContentType::ITEM_GROUP_EDITABLE)) {
    if (needs_separator)
      AddSeparator();
    AppendEditableItems(&menu_model_);
  }
}

void RenderViewContextMenuImpl::RecordShownItem(int id) {
  // TODO(oshima): Imelement UMA stats. crbug.com/401673
  NOTIMPLEMENTED();
}

void RenderViewContextMenuImpl::RecordUsedItem(int id) {
  // TODO(oshima): Imelement UMA stats. crbug.com/401673
  NOTIMPLEMENTED();
}

void RenderViewContextMenuImpl::NotifyMenuShown() {
}

void RenderViewContextMenuImpl::NotifyURLOpened(
    const GURL& url,
    content::WebContents* new_contents) {
}

bool RenderViewContextMenuImpl::GetAcceleratorForCommandId(
    int command_id,
    ui::Accelerator* accelerator) {
  NOTIMPLEMENTED();
  return false;
}

bool RenderViewContextMenuImpl::IsCommandIdChecked(int command_id) const {
  return false;
}

bool RenderViewContextMenuImpl::IsCommandIdEnabled(int command_id) const {
  {
    bool enabled = false;
    if (RenderViewContextMenuBase::IsCommandIdKnown(command_id, &enabled))
      return enabled;
  }
  switch (command_id) {
    // Navigation
    case CMD_BACK:
      return source_web_contents_->GetController().CanGoBack();
    case CMD_FORWARD:
      return source_web_contents_->GetController().CanGoForward();
    case CMD_RELOAD:
      return true;

    // Link
    case CMD_OPEN_LINK_NEW_ACTIVITY:
      return params_.link_url.is_valid();

    // Editable
    case CMD_UNDO:
      return !!(params_.edit_flags & WebContextMenuData::CanUndo);

    case CMD_REDO:
      return !!(params_.edit_flags & WebContextMenuData::CanRedo);

    case CMD_CUT:
      return !!(params_.edit_flags & WebContextMenuData::CanCut);

    case CMD_COPY:
      return !!(params_.edit_flags & WebContextMenuData::CanCopy);

    case CMD_PASTE:
    case CMD_PASTE_AND_MATCH_STYLE:
      return !!(params_.edit_flags & WebContextMenuData::CanPaste);

    case CMD_DELETE:
      return !!(params_.edit_flags & WebContextMenuData::CanDelete);

    case CMD_SELECT_ALL:
      return !!(params_.edit_flags & WebContextMenuData::CanSelectAll);
  }
  return false;
}

void RenderViewContextMenuImpl::ExecuteCommand(int command_id,
                                               int event_flags) {
  RenderViewContextMenuBase::ExecuteCommand(command_id, event_flags);
  if (command_executed_)
    return;
  command_executed_ = true;
  switch (command_id) {
    // Navigation
    case CMD_BACK:
      source_web_contents_->GetController().GoBack();
      break;
    case CMD_FORWARD:
      source_web_contents_->GetController().GoForward();
      break;
    case CMD_RELOAD:
      source_web_contents_->GetController().Reload(true);
      break;

    // Link
    case CMD_OPEN_LINK_NEW_ACTIVITY:
      OpenURL(
          params_.link_url,
          params_.frame_url.is_empty() ? params_.page_url : params_.frame_url,
          NEW_WINDOW,
          ui::PAGE_TRANSITION_LINK);
      break;

    // Editable
    case CMD_UNDO:
      source_web_contents_->Undo();
      break;

    case CMD_REDO:
      source_web_contents_->Redo();
      break;

    case CMD_CUT:
      source_web_contents_->Cut();
      break;

    case CMD_COPY:
      source_web_contents_->Copy();
      break;

    case CMD_PASTE:
      source_web_contents_->Paste();
      break;

    case CMD_PASTE_AND_MATCH_STYLE:
      source_web_contents_->PasteAndMatchStyle();
      break;

    case CMD_DELETE:
      source_web_contents_->Delete();
      break;

    case CMD_SELECT_ALL:
      source_web_contents_->SelectAll();
      break;
  }
}

void RenderViewContextMenuImpl::HandleAuthorizeAllPlugins() {
}

}  // namespace xwalk
