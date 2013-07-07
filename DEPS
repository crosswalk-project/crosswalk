vars = {
}

deps = {
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
