{
  'targets': [
    {
      'target_name': 'xwalk_app_runtime_client_java',
      'type': 'none',
      'variables': {
        'java_in_dir': 'app/android/runtime_client',
      },
      'includes': ['../build/java.gypi'],
    },
    {
      'target_name': 'xwalk_app_runtime_activity_java',
      'type': 'none',
      'dependencies': [
        'xwalk_app_runtime_client_java',
      ],
      'variables': {
        'java_in_dir': 'app/android/runtime_activity',
      },
      'includes': ['../build/java.gypi'],
    },
  ],
}
