# Crosswalk's dependencies, which either override the ones that come from
# Chromium upstream or are not present there.
#
# Add repositories and git revisions to |vars| and use them in |deps|.
vars = {
  'crosswalk_git': 'https://github.com/crosswalk-project',
  '01org_git': 'https://github.com/01org',

  'chromium_crosswalk_rev': '6488cb72d9cc33832af16d65d75f16e06f0bdd88',
  'blink_crosswalk_rev': '99b0aa2fd873af570eb84cbbd342a343ce8bfd7f',
  'v8_crosswalk_rev': '535cd006e5174ff00fd7b745a581980b1d371a9f',
  'ozone_wayland_rev': '5047b6ea8843bfd439a9f5adf2e270cd6fa6db7c',
}

# gclient's DEPS parsing logic is too tangled with the rest of the code, so we
# cannot easily use keywords such as Var() here.
deps = {
  'src':
    vars['crosswalk_git'] + '/chromium-crosswalk.git@' + vars['chromium_crosswalk_rev'],
  'src/third_party/WebKit':
    vars['crosswalk_git'] + '/blink-crosswalk.git@' + vars['blink_crosswalk_rev'],
  'src/v8':
    vars['crosswalk_git'] + '/v8-crosswalk.git@' + vars['v8_crosswalk_rev'],
  'src/ozone':
    vars['01org_git'] + '/ozone-wayland.git@' + vars['ozone_wayland_rev'],
}

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
