# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'variables': {
    'chromium_code': 1,
  },
  'targets': [
    {
      # GN version: //ui/views/examples
      'target_name': 'browser_view_lib',
      'type': '<(component)',
      'dependencies': [
        '../base/base.gyp:base',
        '../skia/skia.gyp:skia',
        '../third_party/icu/icu.gyp:icui18n',
        '../third_party/icu/icu.gyp:icuuc',
        '../ui/base/ui_base.gyp:ui_base',
        '../ui/events/events.gyp:events',
        '../ui/gfx/gfx.gyp:gfx',
        '../ui/gfx/gfx.gyp:gfx_geometry',
        '../ui/gfx/gfx.gyp:gfx_vector_icons',
        '../ui/resources/ui_resources.gyp:ui_resources',
        '../ui/resources/ui_resources.gyp:ui_test_pak',
        '../ui/views/views.gyp:views',
      ],
      'include_dirs': [
        '..',
      ],
      'defines': [
        'GFX_VECTOR_ICONS_UNSAFE',
        'VIEWS_EXAMPLES_IMPLEMENTATION',
      ],
      'sources': [
        # Note: sources list duplicated in GN build.
        'views/browser_view.cc',
        'views/browser_view.h',
        'views_examples_export.h',
      ],
      'conditions': [
        ['OS=="win"', {
          'include_dirs': [
            '../third_party/wtl/include',
          ],
          # TODO(jschuh): crbug.com/167187 fix size_t to int truncations.
          'msvs_disabled_warnings': [ 4267, ],
        }],
        ['use_aura==1', {
          'dependencies': [
            '../ui/aura/aura.gyp:aura',
          ],
        }],
      ],
    },  # target_name: browser_view_lib
    {
      # GN version: //ui/views/examples:views_examples_exe
      'target_name': 'browser_view_exe',
      'type': 'executable',
      'dependencies': [
        '../base/base.gyp:base',
        '../base/base.gyp:base_i18n',
        '../ui/base/ui_base.gyp:ui_base',
        '../ui/compositor/compositor.gyp:compositor',
        '../ui/compositor/compositor.gyp:compositor_test_support',
        '../ui/gfx/gfx.gyp:gfx',
        '../ui/resources/ui_resources.gyp:ui_test_pak',
        '../ui/views/views.gyp:views',
        '../ui/views/views.gyp:views_test_support',
        'browser_view_lib',
      ],
      'sources': [
        # Note: sources list duplicated in GN build.
        'browser_main.cc',
      ],
      'conditions': [
        ['use_aura==1', {
          'dependencies': [
            '../ui/aura/aura.gyp:aura',
          ],
        }],
      ],
    },  # target_name: browser_view_exe    
  ],
}
