{
  'variables': {
    # Enable multitouch support with XInput2.1. When it is enabled, unlike
    # XInput2.2, there are no XI_TouchBegin, XI_TouchUpdate and XI_TouchEnd
    # raw touch events emitted from touch device. Instead, the touch event is
    # simulated by a normal mouse event, since X server maintains multiple
    # virtual touch screen devices as floating device, each of them can
    # simulate a touch event tracked by the device id of event source, as a
    # result, multi-touch support works with these simulated touch events
    # dispatched from floating device.
    'enable_xi21_mt%': 0,

    'tizen%': 0,
    'tizen_mobile%': 0,
  },
  'target_defaults': {
    'conditions': [
      ['enable_xi21_mt==1', {
        'defines': ['ENABLE_XI21_MT=1'],
      }],
      ['tizen==1', {
        'defines': ['OS_TIZEN=1'],
      }],
      ['tizen_mobile==1', {
        'defines': ['OS_TIZEN_MOBILE=1', 'OS_TIZEN=1'],
      }],
    ],
  },
}
