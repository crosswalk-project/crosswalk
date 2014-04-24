# Source folder for xwalk_core_library_empty_apk
## Why it's empty
The "xwalk_core_library_empty_apk" is an apk target which
only depends on xwalk_core_java. The purpose of it is to
get all the classes compiled from the code chromium generated
for each apk.
So there is no java src here.
## Why put me here
To make git keep the folder, the src directory is needed to
build an apk.

