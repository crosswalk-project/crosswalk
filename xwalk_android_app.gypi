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
  ],
}
