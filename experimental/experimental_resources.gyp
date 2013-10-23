{
  'targets': [
    {
      'target_name': 'xwalk_experimental_resources',
      'type': 'none',
      'dependencies': [
        'generate_xwalk_experimental_resources',
      ],
      'variables': {
        'grit_out_dir': '<(SHARED_INTERMEDIATE_DIR)/xwalk',
      },
      'includes': [ '../../build/grit_target.gypi' ],
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)',
          'files': [
            '<(SHARED_INTERMEDIATE_DIR)/xwalk/xwalk_experimental_resources.pak'
          ],
        },
      ],
    },
    {
      'target_name': 'generate_xwalk_experimental_resources',
      'type': 'none',
      'variables': {
        'grit_out_dir': '<(SHARED_INTERMEDIATE_DIR)/xwalk',
      },
      'actions': [
        {
          'action_name': 'xwalk_experimental_resources',
          'variables': {
            'grit_resource_ids': '../resources/resource_ids',
            'grit_grd_file': 'experimental_resources.grd',
          },
          'includes': [ '../../build/grit_action.gypi' ],
        },
      ],
    },
  ],
}
