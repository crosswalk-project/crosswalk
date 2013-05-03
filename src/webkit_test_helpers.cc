// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/webkit_test_helpers.h"

#include "base/command_line.h"
#include "base/utf_string_conversions.h"
#include "content/shell/shell_switches.h"
#include "third_party/WebKit/Tools/DumpRenderTree/chromium/TestRunner/public/WebPreferences.h"
#include "webkit/glue/webpreferences.h"

using WebTestRunner::WebPreferences;

namespace content {

void ExportLayoutTestSpecificPreferences(const WebPreferences& from,
                                         webkit_glue::WebPreferences* to) {
  to->allow_universal_access_from_file_urls =
      from.allowUniversalAccessFromFileURLs;
  to->dom_paste_enabled = from.DOMPasteAllowed;
  to->javascript_can_access_clipboard = from.javaScriptCanAccessClipboard;
  to->xss_auditor_enabled = from.XSSAuditorEnabled;
  to->editing_behavior =
      static_cast<webkit_glue::WebPreferences::EditingBehavior>(
          from.editingBehavior);
  to->default_font_size = from.defaultFontSize;
  to->minimum_font_size = from.minimumFontSize;
  to->default_encoding = from.defaultTextEncodingName.utf8().data();
  to->javascript_enabled = from.javaScriptEnabled;
  to->supports_multiple_windows = from.supportsMultipleWindows;
  to->loads_images_automatically = from.loadsImagesAutomatically;
  to->plugins_enabled = from.pluginsEnabled;
  to->java_enabled = from.javaEnabled;
  to->uses_page_cache = from.usesPageCache;
  to->page_cache_supports_plugins = from.pageCacheSupportsPlugins;
  to->application_cache_enabled = from.offlineWebApplicationCacheEnabled;
  to->tabs_to_links = from.tabsToLinks;
  to->experimental_webgl_enabled = from.experimentalWebGLEnabled;
  to->css_grid_layout_enabled = from.experimentalCSSGridLayoutEnabled;
  // experimentalCSSRegionsEnabled is deprecated and ignored.
  to->hyperlink_auditing_enabled = from.hyperlinkAuditingEnabled;
  to->caret_browsing_enabled = from.caretBrowsingEnabled;
  to->allow_displaying_insecure_content = from.allowDisplayOfInsecureContent;
  to->allow_running_insecure_content = from.allowRunningOfInsecureContent;
  to->css_shaders_enabled = from.cssCustomFilterEnabled;
  to->should_respect_image_orientation = from.shouldRespectImageOrientation;
  to->asynchronous_spell_checking_enabled =
      from.asynchronousSpellCheckingEnabled;
  to->allow_file_access_from_file_urls = from.allowFileAccessFromFileURLs;
  to->author_and_user_styles_enabled = from.authorAndUserStylesEnabled;
  to->javascript_can_open_windows_automatically =
      from.javaScriptCanOpenWindowsAutomatically;
  to->user_style_sheet_location = from.userStyleSheetLocation;
  to->touch_drag_drop_enabled = from.touchDragDropEnabled;
}

// Applies settings that differ between layout tests and regular mode. Some
// of the defaults are controlled via command line flags which are
// automatically set for layout tests.
void ApplyLayoutTestDefaultPreferences(webkit_glue::WebPreferences* prefs) {
  CommandLine& command_line = *CommandLine::ForCurrentProcess();
  prefs->allow_universal_access_from_file_urls = true;
  prefs->dom_paste_enabled = true;
  prefs->javascript_can_access_clipboard = true;
  prefs->xss_auditor_enabled = false;
#if defined(OS_MACOSX)
  prefs->editing_behavior = webkit_glue::WebPreferences::EDITING_BEHAVIOR_MAC;
#else
  prefs->editing_behavior = webkit_glue::WebPreferences::EDITING_BEHAVIOR_WIN;
#endif
  prefs->java_enabled = false;
  prefs->application_cache_enabled = true;
  prefs->tabs_to_links = false;
  prefs->hyperlink_auditing_enabled = false;
  prefs->allow_displaying_insecure_content = true;
  prefs->allow_running_insecure_content = true;
  prefs->webgl_errors_to_console_enabled = false;
  string16 serif;
#if defined(OS_MACOSX)
  prefs->cursive_font_family_map[webkit_glue::WebPreferences::kCommonScript] =
      ASCIIToUTF16("Apple Chancery");
  prefs->fantasy_font_family_map[webkit_glue::WebPreferences::kCommonScript] =
      ASCIIToUTF16("Papyrus");
  serif = ASCIIToUTF16("Times");
#else
  prefs->cursive_font_family_map[webkit_glue::WebPreferences::kCommonScript] =
      ASCIIToUTF16("Comic Sans MS");
  prefs->fantasy_font_family_map[webkit_glue::WebPreferences::kCommonScript] =
      ASCIIToUTF16("Impact");
  serif = ASCIIToUTF16("times new roman");
#endif
  prefs->serif_font_family_map[webkit_glue::WebPreferences::kCommonScript] =
      serif;
  prefs->standard_font_family_map[webkit_glue::WebPreferences::kCommonScript] =
      serif;
  prefs->fixed_font_family_map[webkit_glue::WebPreferences::kCommonScript] =
      ASCIIToUTF16("Courier");
  prefs->sans_serif_font_family_map[
      webkit_glue::WebPreferences::kCommonScript] = ASCIIToUTF16("Helvetica");
  prefs->minimum_logical_font_size = 9;
  prefs->asynchronous_spell_checking_enabled = false;
  prefs->user_style_sheet_enabled = true;
  prefs->threaded_html_parser = true;
  prefs->accelerated_2d_canvas_enabled =
      command_line.HasSwitch(switches::kEnableAccelerated2DCanvas);
  prefs->accelerated_compositing_for_video_enabled = false;
  prefs->mock_scrollbars_enabled = false;
  prefs->fixed_position_creates_stacking_context = false;
  prefs->apply_page_scale_factor_in_compositor = false;
  prefs->smart_insert_delete_enabled = true;
  prefs->minimum_accelerated_2d_canvas_size = 0;
}

}  // namespace content
