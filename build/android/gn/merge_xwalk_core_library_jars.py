import os
import optparse
import sys

GYP_ANDROID_DIR = os.path.join(os.path.dirname(__file__),
                               os.pardir, os.pardir, os.pardir, os.pardir,
                               'build',
                               'android',
                               'gyp')

SRC_DIR = os.path.join(os.path.dirname(__file__),
                       os.pardir, os.pardir, os.pardir, os.pardir)

sys.path.append(GYP_ANDROID_DIR)
import jar
from util import build_utils


def merge_jars(jars, output_jar, root_out_dir):
  with build_utils.TempDir() as temp_dir:
    for jar_file in jars:
      build_utils.ExtractAll(root_out_dir+jar_file, path=temp_dir, pattern='*.class')
    jar.JarDirectory(temp_dir, output_jar)

def main():
  parser = optparse.OptionParser()
  parser.add_option('--jar_type', type='string', default='app_part')
  parser.add_option('--output_dir', type='string', default='out/Default')
  (options, _) = parser.parse_args()
  build_dir = SRC_DIR + '/' + options.output_dir
  app_part_jars = [
    "/lib.java/xwalk/runtime/android/core/xwalk_core_java.jar",
    "/lib.java/xwalk/third_party/lzma_sdk/lzma_sdk_java.jar",
  ]

  library_part_jars = [
    "/lib.java/third_party/android_protobuf/protobuf_nano_javalib.jar",
    "/lib.java/third_party/cardboard-java/cardboard-java.jar",
    "/lib.java/third_party/jsr-305/jsr_305_javalib.jar",
    "/lib.java/third_party/WebKit/public/blink_headers_java.jar",
    "/lib.java/components/navigation_interception/android/navigation_interception_java.jar",
    "/lib.java/components/web_contents_delegate_android/web_contents_delegate_android_java.jar",
    "/lib.java/ui/android/ui_java.jar",
    "/lib.java/ui/accessibility/ui_accessibility_java.jar",
    "/lib.java/base/base_java.jar",
    "/lib.java/mojo/public/java/system.jar",
    "/lib.java/mojo/public/java/bindings.jar",
    "/lib.java/mojo/android/system_java.jar",
    "/lib.java/media/base/android/media_java.jar",
    "/lib.java/media/midi/midi_java.jar",
    "/lib.java/xwalk/extensions/android/xwalk_core_extensions_java.jar",
    "/lib.java/xwalk/runtime/android/core_internal/xwalk_core_internal_java.jar",
    "/lib.java/device/battery/android/battery_monitor_android.jar",
    "/lib.java/device/battery/mojo_bindings_java.jar",
    "/lib.java/device/bluetooth/java.jar",
    "/lib.java/device/vibration/android/vibration_manager_android.jar",
    "/lib.java/device/vibration/mojo_bindings_java.jar",
    "/lib.java/device/usb/java.jar",
    "/lib.java/net/android/net_java.jar",
    "/lib.java/content/public/android/content_java.jar",
  ]

  library_part_resources_jars = [
    "/gen/xwalk/runtime/android/core_internal_empty/xwalk_core_internal_empty_embedder_apk__java__compile_java.javac.jar",
  ]

  if(options.jar_type=="app_part"):
    jars = app_part_jars
    output_jar = build_dir + "/lib.java/xwalk_core_library_java_app_part.jar"
  elif(options.jar_type=="library_part"):
    jars = library_part_jars + library_part_resources_jars
    output_jar = build_dir + "/lib.java/xwalk_core_library_java_library_part.jar"
  else:
    jars = app_part_jars + library_part_jars + library_part_resources_jars
    output_jar = build_dir + "/lib.java/xwalk_core_library_java.jar"

  merge_jars(jars, output_jar, build_dir)

if __name__ == '__main__':
  sys.exit(main())
