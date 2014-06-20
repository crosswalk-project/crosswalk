// Copyright 2014 Intel Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal;

/**
 * Provide XWalk Core internal resources to XWalk Core layer.
 * The main purpose is to dynamically set the value of following
 * resources ids when building xwalk core as a library project.
 * In such case, the ant build system won't generate R.java
 * for package org.xwalk.core.internal. However, the code inside
 * org.xwalk.core.internal uses org.xwalk.core.internal.R.
 */

public final class R {
    public static final class attr {
        public static int select_dialog_multichoice;
        public static int select_dialog_singlechoice;
    }
    public static final class color {
        public static int autofill_dark_divider_color;
        public static int autofill_divider_color;
        public static int color_picker_background_color;
        public static int color_picker_border_color;
    }
    public static final class dimen {
        public static int autofill_text_divider_height;
        public static int autofill_text_height;
        public static int color_button_height;
        public static int color_picker_gradient_margin;
        public static int config_min_scaling_span;
        public static int config_min_scaling_touch_major;
        public static int link_preview_overlay_radius;
    }
    public static final class drawable {
        public static int autofill_popup_background;
        public static int autofill_popup_background_down;
        public static int autofill_popup_background_up;
        public static int bubble;
        public static int bubble_arrow_up;
        public static int color_button_background;
        public static int color_picker_advanced_select_handle;
        public static int color_picker_border;
        public static int ic_menu_search_holo_light;
        public static int ic_menu_share_holo_light;
        public static int ondemand_overlay;
        public static int pageinfo_warning_major;
    }
    public static final class id {
        public static int ampm;
        public static int arrow_image;
        public static int autofill_label;
        public static int autofill_menu_text;
        public static int autofill_popup_window;
        public static int autofill_sublabel;
        public static int color_button_swatch;
        public static int color_picker_advanced;
        public static int color_picker_simple;
        public static int date_picker;
        public static int date_time_suggestion;
        public static int date_time_suggestion_label;
        public static int date_time_suggestion_value;
        public static int gradient;
        public static int gradient_border;
        public static int hour;
        public static int icon_view;
        public static int main_text;
        public static int milli;
        public static int minute;
        public static int more_colors_button;
        public static int more_colors_button_border;
        public static int pickers;
        public static int position_in_year;
        public static int second;
        public static int second_colon;
        public static int second_dot;
        public static int seek_bar;
        public static int select_action_menu_copy;
        public static int select_action_menu_cut;
        public static int select_action_menu_paste;
        public static int select_action_menu_select_all;
        public static int select_action_menu_share;
        public static int select_action_menu_web_search;
        public static int selected_color_view;
        public static int selected_color_view_border;
        public static int sub_text;
        public static int text;
        public static int text_wrapper;
        public static int time_picker;
        public static int title;
        public static int top_view;
        public static int year;
    }
    public static final class layout {
        public static int autofill_text;
        public static int color_picker_advanced_component;
        public static int color_picker_dialog_content;
        public static int color_picker_dialog_title;
        public static int date_time_picker_dialog;
        public static int date_time_suggestion;
        public static int multi_field_time_picker_dialog;
        public static int two_field_date_picker;
        public static int validation_message_bubble;
    }
    public static final class menu {
        public static int select_action_menu;
    }
    public static final class mipmap {
        public static int app_icon;
    }
    public static final class string {
        public static int accessibility_content_view;
        public static int accessibility_date_picker_month;
        public static int accessibility_date_picker_week;
        public static int accessibility_date_picker_year;
        public static int accessibility_datetime_picker_date;
        public static int accessibility_datetime_picker_time;
        public static int accessibility_time_picker_ampm;
        public static int accessibility_time_picker_hour;
        public static int accessibility_time_picker_milli;
        public static int accessibility_time_picker_minute;
        public static int accessibility_time_picker_second;
        public static int actionbar_share;
        public static int actionbar_web_search;
        public static int color_picker_button_black;
        public static int color_picker_button_blue;
        public static int color_picker_button_cancel;
        public static int color_picker_button_cyan;
        public static int color_picker_button_green;
        public static int color_picker_button_magenta;
        public static int color_picker_button_more;
        public static int color_picker_button_red;
        public static int color_picker_button_set;
        public static int color_picker_button_white;
        public static int color_picker_button_yellow;
        public static int color_picker_dialog_title;
        public static int color_picker_hue;
        public static int color_picker_saturation;
        public static int color_picker_value;
        public static int copy_to_clipboard_failure_message;
        public static int cpu_arch_mismatch_message;
        public static int cpu_arch_mismatch_title;
        public static int date_picker_dialog_clear;
        public static int date_picker_dialog_other_button_label;
        public static int date_picker_dialog_set;
        public static int date_picker_dialog_title;
        public static int date_time_picker_dialog_title;
        public static int download_already_exists_toast;
        public static int download_failed_toast;
        public static int download_finished_toast;
        public static int download_no_permission_toast;
        public static int download_start_toast;
        public static int goto_store_button_label;
        public static int http_auth_log_in;
        public static int http_auth_password;
        public static int http_auth_title;
        public static int http_auth_user_name;
        public static int js_alert_title;
        public static int js_confirm_title;
        public static int js_prompt_title;
        public static int low_memory_error;
        public static int media_player_error_button;
        public static int media_player_error_text_invalid_progressive_playback;
        public static int media_player_error_text_unknown;
        public static int media_player_error_title;
        public static int media_player_loading_video;
        public static int month_picker_dialog_title;
        public static int opening_file_error;
        public static int profiler_error_toast;
        public static int profiler_no_storage_toast;
        public static int profiler_started_toast;
        public static int profiler_stopped_toast;
        public static int report_feedback_button_label;
        public static int ssl_alert_title;
        public static int time_picker_dialog_am;
        public static int time_picker_dialog_hour_minute_separator;
        public static int time_picker_dialog_minute_second_separator;
        public static int time_picker_dialog_pm;
        public static int time_picker_dialog_second_subsecond_separator;
        public static int time_picker_dialog_title;
        public static int week_picker_dialog_title;
    }
    public static final class style {
        public static int AutofillPopupWindow;
        public static int SelectPopupDialog;
    }
}
