#!/bin/sh

# generate-flat-tree.sh

# Creates a tarball containing all directories (including other repositories
# like Blink and V8) of a given chromium+xwalk tree, similar to
# tools/export_tarball in the Chromium tree (which could also be used, but it's
# slower and always calls xz(1)).
#
# Usage: go to the top-level directory (the one above src/) and run
#  $ export XWALK_PREFIX=/root/of/xwalk/tree
#  $ gclient recurse --no-progress -j 1 path-to/generate-flat-tree.sh
#
# Your .gclient file should contain all entries needed. Something like this:
# solutions = [
#     {'custom_deps': {'src': 'ssh://git@github.com/otcshare/chromium-crosswalk.git@9341417b1591282e02a4d3da0ece84496cb6999a',
#                      'src/chrome/tools/test/reference_build/chrome_linux': None,
#                      'src/chrome/tools/test/reference_build/chrome_mac': None,
#                      'src/chrome/tools/test/reference_build/chrome_win': None,
#                      'src/chrome_frame/tools/test/reference_build/chrome_win': None,
#                      'src/content/test/data/layout_tests/LayoutTests': None,
#                      'src/third_party/WebKit': 'ssh://git@github.com/otcshare/blink-crosswalk.git@abfee977cd60a915a094f247ff81f9d17dc85efe',
#                      'src/third_party/WebKit/LayoutTests': None,
#                      'src/third_party/hunspell_dictionaries': None,
#                      'src/webkit/data/layout_tests/LayoutTests': None},
#      'name': '28.0.1500.36',
#      'url': 'http://src.chromium.org/svn/releases/28.0.1500.36'},
#
#     {"name"        : "src/xwalk",
#      "url"         : "ssh://git@github.com/otcshare/crosswalk.git@origin/master",
#      "deps_file"   : "DEPS",
#      "managed"     : True,
#      "custom_deps" : {},
#      "safesync_url": ""},
# ]
# cache_dir = None

if [ -z "${XWALK_PREFIX}" ]; then
    echo "You need to set XWALK_PREFIX to the top-level directory of your source tree (the one that contains src/)."
    exit 1
fi

# Ignore directories outside src/.
if [ "${GCLIENT_DEP_PATH#src}" = "${GCLIENT_DEP_PATH}" ]; then
    exit
fi

cd ${XWALK_PREFIX}
echo "Adding ${GCLIENT_DEP_PATH}..."

tar --append --file ${XWALK_PREFIX}/flat-xwalk-tree.tar \
    --exclude-vcs --exclude=native_client --exclude=LayoutTests \
    ${GCLIENT_DEP_PATH}
