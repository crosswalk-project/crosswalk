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
    # Generate .gclient-xwalk for Crosswalk's dependencies.
    "name": "generate-gclient-xwalk",
    "pattern": ".",
    "action": ["python", "src/xwalk/tools/generate_gclient-xwalk.py"],
  },
  {
    # Fetch Crosswalk dependencies.
    "name": "fetch-deps",
    "pattern": ".",
    "action": ["python", "src/xwalk/tools/fetch_deps.py", "-v"],
  },
  {
    # A change to a .gyp, .gypi, or to GYP itself should run the generator.
    "name": "gyp-xwalk",
    "pattern": ".",
    "action": ["python", "src/xwalk/gyp_xwalk"],
  }
]
