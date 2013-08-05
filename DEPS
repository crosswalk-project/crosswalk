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
  "+third_party/WebKit/Source/Platform/chromium",
  "+third_party/WebKit/Source/WebKit/chromium",

  # Files generated during Crosswalk build.
  "+grit/xwalk_resources.h",
]

vars = {
}

deps = {
  "src/xwalk/tizen-extensions-crosswalk":
    "ssh://git@github.com/otcshare/tizen-extensions-crosswalk.git@origin/master",
}

hooks = [
  {
    # Fetch Crosswalk dependencies.
    "pattern": ".",
    "action": ["python", "src/xwalk/tools/fetch_deps.py", "-v"],
  },
  {
    # A change to a .gyp, .gypi, or to GYP itself should run the generator.
    "pattern": ".",
    "action": ["python", "src/xwalk/gyp_xwalk"],
  }
]
