#!/usr/bin/env python

import os

if __name__ == '__main__':
  for dirpath, dirs, files in os.walk(".", topdown=False):
    for name in files:
      try:
        os.utime(os.path.join(dirpath, name),(1260572400, 1260572400))
      except Exception:
        pass

