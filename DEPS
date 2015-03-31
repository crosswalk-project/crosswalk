include_rules = [
  "+content/public",

  "+crypto",
  "+net",
  "+sandbox",
  "+skia",
  "+ui",
  "+v8",
  "+webkit",

  # Allow inclusion of third-party code.
  "+third_party/skia",
  "+third_party/WebKit/public/platform",
  "+third_party/WebKit/public/web",

  # Files generated during Crosswalk build.
  "+grit/xwalk_resources.h",
]

vars = {
}

deps = {
}

hooks = [
  {
    # Remove separate src/third_party/speex checkout from M39 on.
    "name": "fix-speex-checkout",
    "pattern": ".",
    "action": ["python", "src/xwalk/tools/fix-speex-checkout.py"],
  },
  {
    # Generate .gclient-xwalk for Crosswalk's dependencies.
    "name": "generate-gclient-xwalk",
    "pattern": ".",
    "action": ["python", "src/xwalk/tools/generate_gclient-xwalk.py"],
  },
  {
    # Fetch and prepare SevenZip LZMA SDK
    "name": "download-lzma-sdk",
    "pattern": ".",
    "action": ["python", "src/xwalk/third_party/lzma_sdk/download_lzma_sdk.py"],
  },
  {
    # Fetch Crosswalk dependencies.
    "name": "fetch-deps",
    "pattern": ".",
    "action": ["python", "src/xwalk/tools/fetch_deps.py", "-v"],
  },
  {
    # Make src/third_party/android_tools/android_tools.gyp shut up and not exit
    # if the Google Play Services library is not installed (after
    # https://chromium-review.googlesource.com/#/c/247861). We do not use it on
    # Crosswalk, and installing it is a manual process that involves accepting
    # an EULA.
    "name": "empty-google-play-services_lib",
    "pattern": ".",
    "action": ["python", "src/xwalk/tools/empty_google_play_services_lib.py"],
  },
  {
    # A change to a .gyp, .gypi, or to GYP itself should run the generator.
    "name": "gyp-xwalk",
    "pattern": ".",
    "action": ["python", "src/xwalk/gyp_xwalk"],
  }
]
