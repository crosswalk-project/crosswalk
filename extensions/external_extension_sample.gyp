{
  'targets': [
  {
    'target_name': 'external_extension_sample',
    'type': 'loadable_module',
    'include_dirs': [
      '../..',
    ],
    'sources': [
      'test/external_extension_sample.c',
    ]
  },
  {
    'target_name': 'echo_extension',
    'type': 'loadable_module',
    'include_dirs': [
      '../..',
    ],
    'sources': [
      'test/echo_extension.c',
    ]
  },

  ],
}
