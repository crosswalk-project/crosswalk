// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/browser/ui/native_app_window_mac.h"

#include "base/mac/mac_util.h"
#include "base/strings/sys_string_conversions.h"
#include "base/values.h"
#include "xwalk/runtime/browser/runtime.h"
#include "xwalk/runtime/common/xwalk_notification_types.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#import "ui/base/cocoa/underlay_opengl_hosting_window.h"

@interface XWalkWindowDelegate : NSObject<NSWindowDelegate>
@end

@implementation XWalkWindowDelegate
- (BOOL)windowShouldClose:(id)target {
  [target autorelease];
  [self release];

  return YES;
}
@end

@interface XWalkWindow : UnderlayOpenGLHostingWindow {
}
@end

@implementation XWalkWindow {
}
@end

namespace xwalk {

NativeAppWindowMac::NativeAppWindowMac(
    const NativeAppWindow::CreateParams& create_params)
  : web_contents_(create_params.web_contents),
    is_fullscreen_(false),
    minimum_size_(create_params.minimum_size),
    maximum_size_(create_params.maximum_size),
    resizable_(create_params.resizable),
    attention_request_id_(0) {
  gfx::Size size = create_params.bounds.size();

  NSRect main_screen_rect = [[[NSScreen screens] objectAtIndex:0] frame];
  NSRect bounds = NSMakeRect(
      (NSWidth(main_screen_rect) - size.width()) / 2,
      (NSHeight(main_screen_rect) - size.height()) / 2,
      size.width(),
      size.height());
  NSUInteger style_mask = NSTitledWindowMask | NSClosableWindowMask |
                          NSMiniaturizableWindowMask | NSResizableWindowMask;
  window_ = [[XWalkWindow alloc]
             initWithContentRect:bounds
             styleMask:style_mask
             backing:NSBackingStoreBuffered
             defer:NO];
  [window_ setDelegate: [XWalkWindowDelegate alloc]];

  gfx::NativeView view = web_contents_->GetView()->GetNativeView();
  [view setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
  [view setFrame:[[window_ contentView] bounds]];
  [[window_ contentView] addSubview:view];

  // This shows the full-screen icon on the top right corner in Lion
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
  [window_ setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
#endif

  if (create_params.state == ui::SHOW_STATE_FULLSCREEN)
    SetFullscreen(true);
}

NativeAppWindowMac::~NativeAppWindowMac() {
}

gfx::NativeWindow NativeAppWindowMac::GetNativeWindow() const {
  return window_;
}

void NativeAppWindowMac::UpdateIcon(const gfx::Image&) {
}

void NativeAppWindowMac::UpdateTitle(const base::string16& title) {
  [window_ setTitle:base::SysUTF16ToNSString(title)];
}

gfx::Rect NativeAppWindowMac::GetRestoredBounds() const {
  NSRect bounds = [window_ frame];
  return gfx::Rect(NSMinX(bounds), NSMinY(bounds), NSWidth(bounds), NSHeight(bounds));
}

gfx::Rect NativeAppWindowMac::GetBounds() const {
  NSRect bounds = [window_ frame];
  return gfx::Rect(NSMinX(bounds), NSMinY(bounds), NSWidth(bounds), NSHeight(bounds));
}

void NativeAppWindowMac::SetBounds(const gfx::Rect& bounds) {
  NSRect rect = NSMakeRect(bounds.x(), bounds.y(), bounds.width(), bounds.height());
  [window_ setFrame:rect display:NO];
}

void NativeAppWindowMac::Focus() {
  [window_ makeKeyAndOrderFront:nil];
}

void NativeAppWindowMac::Show() {
  [window_ makeKeyAndOrderFront:nil];
}

void NativeAppWindowMac::Hide() {
  [window_ orderOut:nil];
}

void NativeAppWindowMac::Maximize() {
  [window_ zoom:nil];
}

void NativeAppWindowMac::Minimize() {
  [window_ miniaturize:nil];
}

void NativeAppWindowMac::SetFullscreen(bool fullscreen) {
  if (is_fullscreen_ == fullscreen)
    return;
  DCHECK(base::mac::IsOSLionOrLater());

  is_fullscreen_ = fullscreen;
  [window_ toggleFullScreen:nil];

  content::NotificationService::current()->Notify(
      xwalk::NOTIFICATION_FULLSCREEN_CHANGED,
      content::Source<NativeAppWindow>(this),
      content::NotificationService::NoDetails());
}

void NativeAppWindowMac::Restore() {
  [window_ deminiaturize:nil];
}

void NativeAppWindowMac::FlashFrame(bool flash) {
  if (flash) {
    attention_request_id_ = [NSApp requestUserAttention:NSInformationalRequest];
  } else {
    [NSApp cancelUserAttentionRequest:attention_request_id_];
    attention_request_id_ = 0;
  }
}

void NativeAppWindowMac::Close() {
  [window_ performClose:nil];
}

bool NativeAppWindowMac::IsActive() const {
  // FIXME: This requires listening for NSApplicationDidBecomeActiveNotification
  // Currently, IsActive() is unused so leave this broken for now.
  return [window_ isKeyWindow];
}

bool NativeAppWindowMac::IsMaximized() const {
  return [window_ isZoomed];
}

bool NativeAppWindowMac::IsMinimized() const {
  return [window_ isMiniaturized];
}

bool NativeAppWindowMac::IsFullscreen() const {
  return is_fullscreen_;
}

// static
NativeAppWindow* NativeAppWindow::Create(
    const NativeAppWindow::CreateParams& create_params) {
  return new NativeAppWindowMac(create_params);
}

void NativeAppWindow::Initialize() {
}

}  // namespace xwalk
