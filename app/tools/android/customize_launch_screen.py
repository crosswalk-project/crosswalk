#!/usr/bin/env python

# Copyright (c) 2014 Intel Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import shutil
import sys

from manifest_json_parser import ManifestJsonParser

def CopyToPathWithName(root, name, final_path, rename):
  if name == '':
    return False
  origin_path = os.path.join(root, name)
  if not os.path.exists(origin_path):
    print ('Error: \'' + origin_path + '\' not found.' )
    sys.exit(6)
  if not os.path.exists(final_path):
    os.makedirs(final_path)
  # Get the extension.
  # Need to take care of special case, such as 'img.9.png'
  name_components = name.split('.')
  name_components[0] = rename
  new_name = '.'.join(name_components)
  final_path_with_name = os.path.join(final_path, new_name)
  shutil.copyfile(origin_path, final_path_with_name)
  return True


def CopyDrawables(image_dict, orientation, sanitized_name, name, app_root):
  drawable = os.path.join(sanitized_name, 'res', 'drawable')
  if orientation == 'landscape':
    drawable = drawable + '-land'
  elif orientation == 'portrait':
    drawable = drawable + '-port'
  drawable_ldpi = drawable + '-ldpi'
  drawable_mdpi = drawable + '-mdpi'
  drawable_hdpi = drawable + '-hdpi'
  drawable_xhdpi = drawable + '-xhdpi'

  image_075x = image_dict.get('0.75x', '')
  image_1x = image_dict.get('1x', '')
  image_15x = image_dict.get('1.5x', '')
  image_2x = image_dict.get('2x', '')

  # Copy all supported images: 0.75x, 1x, 1.5x, 2x.
  has_image = False
  if image_075x:
    if CopyToPathWithName(app_root, image_075x, drawable_ldpi, name):
      has_image = True
  if image_1x:
    if CopyToPathWithName(app_root, image_1x, drawable_mdpi, name):
      has_image = True
  if image_15x:
    if CopyToPathWithName(app_root, image_15x, drawable_hdpi, name):
      has_image = True
  if image_2x:
    if CopyToPathWithName(app_root, image_2x, drawable_xhdpi, name):
      has_image = True

  # If no supported images found, find the closest one as 1x.
  if not has_image:
    closest = ''
    delta = sys.maxsize
    for(k, v) in image_dict.items():
      items = k.split('x')
      if len(items) == 2:
        float_value = sys.maxsize
        try:
          float_value = float(items[0])
        except ValueError:
          continue
        if abs(float_value - 1) < delta:
          closest = v
          if CopyToPathWithName(app_root, closest, drawable_mdpi, name):
            delta = float_value


def CustomizeDrawable(image, orientation, sanitized_name, app_root, name):
  # Parse the image.
  # The format of image: 'image-1x.png [1x], image-75x.png 0.75x,
  #                       image-15x.png 1.5x, image-2x.png 2x'
  image_list = image.split(',')

  # The first image: 'image-1x.png', the density is not provided.
  image_pair_1 = image_list[0].strip()
  items = image_pair_1.split(' ')
  image_1x = ''
  if len(items) == 1:
    image_1x = items[0]
    image_list.pop(0)
  # The dictionary which contains the image pair.
  image_dict = {'1x': image_1x}

  for image_pair in image_list:
    items = image_pair.strip().split(' ')
    if len(items) >= 2:
      x = items[len(items)-1]
      image_item = items[0]
      image_dict[x] = image_item

  CopyDrawables(image_dict, orientation, sanitized_name, name, app_root)


def CustomizeForeground(image, orientation, sanitized_name, app_root):
  CustomizeDrawable(image, orientation, sanitized_name,
                    app_root, "launchscreen_img")


def CustomizeBackground(background_color,
                        background_image,
                        orientation,
                        sanitized_name,
                        app_root):
  background_path = os.path.join(sanitized_name, 'res',
                                 'drawable', 'launchscreen_bg.xml')
  if not os.path.isfile(background_path):
    print('Error: launchscreen_bg.xml is missing in the build tool.')
    sys.exit(6)

  has_background = False
  background_file = open(background_path, 'r')
  content = background_file.read()
  background_file.close()
  # Fill the background_color.
  if background_color:
    content = content.replace('#000000', background_color, 1)
    has_background = True
  # Fill the background_image.
  if background_image:
    CustomizeDrawable(background_image, orientation, sanitized_name,
                      app_root, "launchscreen_bg_img")
    # Only set Background Image once for each orientation.
    tmp = '<item>\n' \
          '  <bitmap\n' \
          '    android:src=\"@drawable/launchscreen_bg_img\"\n' \
          '    android:tileMode=\"repeat\" />\n' \
          '</item>\n'
    content = content.replace('<!-- Background Image -->', tmp, 1)
    has_background = True
  if has_background:
    background_file = file(background_path,'w')
    background_file.write(content)
    background_file.close()
  return has_background


def CustomizeByOrientation(parser, orientation, sanitized_name, app_root):
  background_color = parser.GetLaunchScreenBackgroundColor(orientation)
  background_image = parser.GetLaunchScreenBackgroundImage(orientation)
  image = parser.GetLaunchScreenImage(orientation)
  # Customize background: background_color, background_image.
  has_background = CustomizeBackground(background_color, background_image,
                                       orientation, sanitized_name, app_root)
  # Customize foreground: image.
  CustomizeForeground(image, orientation, sanitized_name, app_root)
  return has_background


def CustomizeLaunchScreen(app_manifest, sanitized_name):
  if not app_manifest:
    return False
  parser = ManifestJsonParser(os.path.expanduser(app_manifest))
  app_root = os.path.dirname(app_manifest)
  default = CustomizeByOrientation(parser, 'default',
                                   sanitized_name, app_root)
  portrait = CustomizeByOrientation(parser, 'portrait',
                                    sanitized_name, app_root)
  landscape = CustomizeByOrientation(parser, 'landscape',
                                     sanitized_name, app_root)
  return default or portrait or landscape
