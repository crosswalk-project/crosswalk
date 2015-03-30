#!/usr/bin/env python

# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

class AppInfo:
  def __init__(self):
    self.app_root = ''
    self.app_version = '1.0.0'
    self.app_versionCode = ''
    self.fullscreen_flag = ''
    self.icon = ''
    # android_name is only composed of alphabetic characters,
    # generated from the last segment of input package name.
    # It will be used for Android project name,
    # APK file name and Activity name.
    self.android_name = 'AppTemplate'
    self.orientation = 'unspecified'
    # app_name is human readable string,
    # it will be used for the Android application name.
    self.app_name = ''
    self.package = 'org.xwalk.app.template'
    self.remote_debugging = ''
    self.use_animatable_view = ''
    self.xwalk_apk_url = ''
