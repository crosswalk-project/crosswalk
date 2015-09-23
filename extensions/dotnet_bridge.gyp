{
    # This is needed so that /RTC1 and /MTx are not added to the
    # compilation command line (it's not compatible with /clr)
    'target_defaults': {
      'variables': {
        'win_release_RuntimeChecks': '0',
        'win_debug_RuntimeChecks': '0',
        'win_release_RuntimeLibrary': '2', # 2 = /MD (nondebug DLL)
        'win_debug_RuntimeLibrary': '3',   # 3 = /MDd (debug DLL)
      },
    },
    'targets': [
      {
          'target_name': 'dotnet_bridge',
          'type': 'none',
          'dependencies': [
            'xwalk_dotnet_bridge'
          ],
      },
      {
        'target_name': 'xwalk_dotnet_bridge',
        'type': 'shared_library',
        'sources': [
          'common/win/xwalk_dotnet_bridge.cc',
          'common/win/xwalk_dotnet_bridge.h',
        ],
        'defines': [
          'DOTNET_BRIDGE_IMPLEMENTATION',
        ],
        'include_dirs': [
          '../../',
        ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'RuntimeTypeInfo': 'true',
            'CompileAsManaged':'true',
          }
        },
     },
   ],
}
