{
  'variables': {
    # The files and directories will be added with the same names to the
    # generated zip file, with <(PRODUCT_DIR)/ stripped from the beginning.
    'directories_to_package': [
      '<(PRODUCT_DIR)/locales/xwalk',
    ],
    'files_to_package': [
      '<(PRODUCT_DIR)/VERSION',
      '<(PRODUCT_DIR)/d3dcompiler_47.dll',
      '<(PRODUCT_DIR)/icudtl.dat',
      '<(PRODUCT_DIR)/libEGL.dll',
      '<(PRODUCT_DIR)/libGLESv2.dll',
      '<(PRODUCT_DIR)/natives_blob.bin',
      '<(PRODUCT_DIR)/osmesa.dll',
      '<(PRODUCT_DIR)/snapshot_blob.bin',
      '<(PRODUCT_DIR)/xwalk.exe',
      '<(PRODUCT_DIR)/xwalk.pak',
      '<(PRODUCT_DIR)/xwalk_100_percent.pak',
      '<(PRODUCT_DIR)/xwalk_200_percent.pak',
      '<(PRODUCT_DIR)/xwalk_300_percent.pak',
      '<(PRODUCT_DIR)/xwalk_dotnet_bridge.dll',
    ],
  },
  # TODO(rakuco): This could be done earlier in the build for other targets to
  # use this file instead of the one in the source tree.
  'copies': [
    {
      'destination': '<(PRODUCT_DIR)',
      'files': [
        '<(DEPTH)/xwalk/VERSION',
      ],
    },
  ],
  'actions': [
    {
      'action_name': 'generate_crosswalk_win_zip',
      'variables': {
        'zip_script': '<(DEPTH)/xwalk/build/win/generate_crosswalk_zip.py',
        'zip_name': '<(PRODUCT_DIR)/crosswalk_win.zip',
      },
      'inputs': [
        '<(zip_script)',
        '<@(directories_to_package)',
        '<@(files_to_package)',
      ],
      'outputs': [
        '<(zip_name)',
      ],
      'action': [
        'python', '<(zip_script)',
        '--build-dir', '<(PRODUCT_DIR)',
        '--dest', '<(zip_name)',
        '--dirs', '<(directories_to_package)',
        '--files', '<(files_to_package)',
      ],
    },
  ],
}
