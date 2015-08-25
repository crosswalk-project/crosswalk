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
          'xwalk_dotnet_bridge.cc',
          'xwalk_dotnet_bridge.h',
          '../extensions/public/XW_Extension.h',
          '../extensions/public/XW_Extension_EntryPoints.h',
          '../extensions/public/XW_Extension_Permissions.h',
          '../extensions/public/XW_Extension_Runtime.h',
          '../extensions/public/XW_Extension_SyncMessage.h',
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
