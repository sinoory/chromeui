# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
   'targets': [
      {
      # GN version: //net
      'target_name': 'net',
      'type': 'static_library',
      'sources': [
        'base/filename_util.cc',
        'base/escape.cc',
      ],
      'include_dirs':[
        '..'
      ]
      },
    ]
}
