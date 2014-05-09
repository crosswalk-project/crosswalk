# Copyright (c) 2013 Intel Corporation. All rights reserved.
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# WARNING: do not include this file, it is included automatically for you
# by gyp_xwalk by adding xwalk/build/common.gypi wich includes this file.
#
# It is too tempting to add the Chromium file filter rules here, but it will
# lead to some weird bugs on the buildsystem (remember it will be applyed to
# every .gyp file, including Chromium's).
{
  'target_conditions': [
    ['tizen!=1', {
      'sources/': [
        ['exclude', '_tizen(_unittest)?\\.(h|cc)$'],
        ['exclude', '(^|/)tizen/'],
      ],
    }],
  ]
}
