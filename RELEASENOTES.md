# Crosswalk Project release notes

Note: dates indicate the day when the release was promoted to the Stable channel.

## Crosswalk 21 (Sep 8, 2016)

* Rebase to Chromium 51

### Embedding API changes

* Get the rendering backend type of XWalkView (XWalkView.getBackendType)
* Expose setSaveFormData/getSaveFormData in XWalkSettings
* Expose setDomStorageEnabled in XWalkSettings
* Expose setCacheMode API in XWalkSettings
* Expose accessing file and content schemes related APIs
* Expose set/getJavaScriptEnabled
* Expose set/getJavaScriptCanOpenWindowsAutomatically

### Notable bug fixes

* [XWALK-3889] Fail to exit fullscreen when invoking the 'leaveFullScreen' method of embeddingAPI on Android

## Crosswalk 20 (Jul 27, 2016)

* Rebase to Chromium 50
* Remove support for Android 4.0
* Android: launch external apps via custom URL schemes
* Windows: add download support
* Windows: support notifications

### Embedding API changes

* Add support for removeJavascriptInterface
* setLoadWithOverviewMode
* New APIs to find text in web page and manage the results
* New APIs to customize the dialog shown when downloading shared Crosswalk
* Add support for getContentHeight

### Notable bug fixes

* [XWALK-6630] - shouldOverrideUrlLoading blocks js execution in some cases

## Crosswalk 19 (Jun 22, 2016)

* Rebase to Chromium 49
* First stable release for Windows
* Native File System extension (xwalk.experimental.native_file_system) changes:
   * The getDirectoryList() function has been removed. The existing implementation never worked (it always returned an empty string).
   * The requestNativeFileSystem() function no longer passes a JSON string to the optional error callback it accepts. Instead, a regular Error object is passed instead.
* The sysapps_promise module has been removed. Chromium has implemented Promises natively since version 32 (released in early 2014). This means calling requireNative(‘sysapps_promise’) in JavaScript extension code will not work, and regular Promise objects should be used instead.
* Windows: support fullscreen apps
* Windows: support manifest orientation field
* Support Google API Keys for Geolocation when packaging the application.
* Allow for a separate 64-bit Crosswalk APK in Play Store
* Extension to support TouchID on Android and iOS

### Embedding API Changes

* computeHorizontalScrollRange related APIs:
   * computeHorizontalScrollRange()
   * computeHorizontalScrollOffset()
   * computeVerticalScrollRange()
   * computeVerticalScrollOffset()
   * computeVerticalScrollExtent()
   * computeScroll()
* load(String url, String content, HashMap<String, String> headers)
* clearClientCertPreferences()
* Add isFirstAttempt as part of public XWalkHttpAuthHandler API
* getPrincipals and getKeyTypes for ClientCertRequest

### Notable bug fixes

* [XWALK-4871] - 'onOverScrolled' method doesn't work in the subclass of XWalkView
* [XWALK-4894] - 'overScrollBy' method doesn't work in the subclass of XWalkView
* [XWALK-5503] - Page Visibility API visibilitychange event not triggered on Windows OS
* [XWALK-5929] - setTextZoom does not work with CSS3 columns

## Crosswalk 18 (Apr 22, 2016)

* Rebase to Chromium 48
* Modify Presentation API Implementation on Android to follow W3C Spec
* Windows: Generic mechanism to request permissions to use APIs from user
* Binary message interface support for Java extension
* Support external extensions for Crosswalk Webview on Android
* make_apk is replaced by crosswalk-app-tools
* Option to disable theme-color support on Android
* Enable access to set-cookie in Android cordova xwalk
* In-App Purchase extension with support for Google Play and Xiaomi store on Android, Apple Store on iOS
* Support RealSense Cameras on Windows

### Embedding API updates

* Access response cookies with XWalkWebResourceResponse
* getFavicon()
* webview.clearSslPreferences()

### Notable bug fixes

* [XWALK-4401] - Crosswalk crashes when running benchmark JS/DROMAEO-DOM after re-basing to M44
* [XWALK-6141] - Crosswalk 16+ stable crashes in Genymotion
* [XWALK-6482] - Message "Please repackage xxx" pops up after launching embedded arm app on x86_64 devices
* [XWALK-6631] - android project which using crosswalk run in genymotion will crash when simulator os below 6.0
* [XWALK-6746] - Cannot build Crosswalk 18 for Android with LibPhoneNumber

## Crosswalk 17 (Mar 4, 2016)

* Rebased to Chromium 46
* Add ability to intercept touch events on the XWalkView
* Initial Support Presentation API on Windows
* Support security APIs for Crosswalk
* set background color in Cordova-plugin-xwalkview
* XWalkRuntimeLib.apk can be downloaded and installed silently in shared-mode
* WiFi Direct API for Android
* Implement support for .NET extensions
* Remove support for Cordova Android 3.x

### Embedding API updates

* XWalkView.getUserAgentString
* XWalkSettings.setSupportZoom
* onJsAlert/onJsConfirm/onJsPrompt
* XWalkView.clearFormData
* XWalkSettings.setTextZoom/ XWalkSettings.getTextZoom
* captureBitmapAsync

### Notable bug fixes

* [XWALK-3112] - View.scrollTo() and other functions do not work as expected
* [XWALK-4045] - Duplicate history per URL visit
* [XWALK-4255] - XWalk crashes upon back nav if intercepted request has not finished loading
* [XWALK-4514] - "A network change was detected" when create 5 xwalkviews which load different pages
* [XWALK-5621] - shouldInterceptLoadRequest is unreliable with iframes
* [XWALK-6178] - A network change was detected

## Crosswalk 16 (Jan 11, 2016)

* Update to Chromium 45, which includes new JavaScript language features, an improved audio experience on Android, and a large number of minor API improvements and deprecations. Read the [announcement](http://blog.chromium.org/2015/07/chrome-45-beta-new-es2015-features.html) for more details.
* Crosswalk can now play back Widevine DRM-protected content ([XWALK-5030](https://crosswalk-project.org/jira/browse/XWALK-5030))
* A new xwalk_view.background_color field can be added to the manifest.json, to specify the background color of the XWalkView
* When onReceivedLoadError occurs, a Toast notification is displayed to the user instead of a dialog (as the user cannot do anything to respond to the error)
* Presentation API on Android follows an updated [W3C Spec](http://www.w3.org/TR/2015/WD-presentation-api-20150701/).
* External extensions have an additional binary messaging interface for better performance when large amounts of data need to be passed to and from the extension.
* The signature of the downloaded Crosswalk APK is verified when starting the application in shared mode.
* Android support libraries (e.g. support-v4, support-v7 etc) are no longer bundled by Crosswalk and should be included explicitly if required.

## Crosswalk 15 (Oct 26, 2015)

* Rebased to Chromium 44
* Reintroduced support for Android 4.0 which was dropped in Chromium 44
* Preserve local storage when upgrading an existing application from platform webview to Crosswalk
* Support for the `theme-color` meta tag

### Embedding API updates

* XWalkView now uses [`SurfaceView`](http://developer.android.com/reference/android/view/SurfaceView.html) as the default rendering backend. This solves several visual glitches, for example when opening/closing the virtual keyboard, and gives better performance, however it doesn’t support animating the view. If you need to animate the XWalkView, select [`TextureView`](http://developer.android.com/reference/android/view/TextureView.html) as the rendering backend. To change the view type to TextureView, use `XWalkPreferences.setValue(XWalkPreferences.ANIMATABLE_XWALK_VIEW, true)`
* New methods to programmatically zoom the view: [`XWalkView.zoomOut`](https://crosswalk-project.org/jira/browse/XWALK-4171),[`XWalkView.zoomIn`](https://crosswalk-project.org/jira/browse/XWALK-4170), [`XWalkView.zoomBy`](https://crosswalk-project.org/jira/browse/XWALK-4169)
* Override the default DownloadListener with [`XWalkView.setDownloadListener`](https://crosswalk-project.org/jira/browse/XWALK-3958)
* New helper class [`XWalkInitializer`](https://crosswalk-project.org/jira/browse/XWALK-3971) to initialize XWalkView asynchronously

### Extension framework

JavaScript stub auto-generation for Java extensions on Android [[XWALK-3969](https://crosswalk-project.org/jira/browse/XWALK-3969)]

### Size optimizations

The following features were ported from the Crosswalk Lite branch to support better compression of the APK:

* Enable ProGuard for Crosswalk to compress APK size [[XWALK-3854](https://crosswalk-project.org/jira/browse/XWALK-3854)]
* LZMA support [[XWALK-3810](https://crosswalk-project.org/jira/browse/XWALK-3810)]
* Ability to customize the dialog shown when uncompressing Crosswalk [[XWALK-3475](https://crosswalk-project.org/jira/browse/XWALK-3475)]

## Crosswalk 14 (Jul 3, 2015)

* First release [available](https://play.google.com/store/apps/details?id=org.xwalk.core&hl=en) in the Google Play Store. Applications can be configured to download it on demand
* Rebased to Chromium 43
* New WebCL API for devices that support OpenCL
* SIMD support for code written in asm.js.

### Embedding API Updates

The Crosswalk XwalkView class has a new method to control whether its surface is placed on top of its window. Note that this only works when XWalkPreferences.ANIMATABLE_XWALK_VIEW is false.

```public void setZOrderOnTop (boolean onTop);```

### Notable bug fixes

The --disk-cache-size command line switch now behaves as expected (bug XWALK-3821)

## Crosswalk 13 (May 11, 2015)

* Rebased to Chromium 42
* First version available via the Cordova Crosswalk Webview plugin
* 64-bit support
* Enable shared mode in xwalk core library and related signature verification
* Expose API to specify xwalk_hosts programatically

### Bug Fixes

* [XWALK-1621] - The webkitSpeechRecognition method is not defined when running “WebAPI/webspeech” on Android OS
* [XWALK-1899] - One XWalkView cannot be displayed on top of another XWalkView
* [XWALK-2132] - XWalk Cordova doesn't seem to set window.navigator.language - always en-US
* [XWALK-2352] - Some methods in XWalkView and XWalkUIClient interface do not support when running “WebAPI/Embeddingapi” on Android OS
* [XWALK-2511] - screen.orientation.lock() fails to return Promise when running “WebAPI/Screenorientation” on Android OS
* [XWALK-2643] - getUserMedia Get low FPS in IA devices
* [XWALK-3308] - transparent webview appears white, fixed in chromium
* [XWALK-3736] - Failed to execute 'start' on 'SpeechRecognition'
* [XWALK-3897] - Fail to load the external link when clicking the link with '_blank' target on Android

## Crosswalk 12 (Mar 17, 2015)

* Rebased to Chromium 41, which brings new ES6 template literals and new debugging features for CSS Animation and Service Worker.

## Embedding API updates

### XWalkView

Set the User Agent string of the application:

    void setUserAgentString(String userAgent)

Use `XWalkView.setBackgroundColor(0)` to make the XWalkView transparent ([XWALK-3308](https://crosswalk-project.org/jira/browse/XWALK-3308))

### XWalkUIClient

Notify the host application of a console message.:

    void onConsoleMessage(XWalkView view, String message, int lineNumber, String sourceId, ConsoleMessageType messageType)


## SIMD updates

The [SIMD.js API](https://github.com/johnmccutchan/ecmascript_simd/) now implements load and store of data types. These APIs allow developers to load or store all or partial elements of SIMD data from or to variable typed arrays and are important for several use cases:


Adds "ByScalar" suffix to shift operations according to the latest [SIMD.js spec](https://github.com/johnmccutchan/ecmascript_simd/). It clarifies that their shift count is a scalar value, and to make namespace room for adding vector-vector shifts in the future.

    SIMD.int32x4.shiftLeftByScalar
    SIMD.int32x4.shiftRightLogicalByScalar
    SIMD.int32x4.shiftRightArithmeticByScalar

SIMD.js introduces new swizzle and shuffle APIs to rearrange lanes in SIMD data types. They include:

    SIMD.float32x4.swizzle
    SIMD.float32x4.shuffle
    SIMD.float64x2.swizzle
    SIMD.float64x2.shuffle
    SIMD.int32x4.swizzle
    SIMD.int32x4.shuffle

## Crosswalk 11 (Feb 26, 2015)

* Rebased to Chromium 40, which introduces the [Service Worker API](http://www.w3.org/TR/service-workers/).

## SIMD updates

The [SIMD.js API](https://github.com/johnmccutchan/ecmascript_simd/) now implements load and store of data types. These APIs allow developers to load or store all or partial elements of SIMD data from or to variable typed arrays and are important for several use cases:

1. load/store SIMD data from/to non 16 bytes aligned memory.
1. load/store 1, 2 or 3 float32/int32 packed data structure from/to memory to/from SIMD types.
1. enable Emscripten (https://github.com/kripken/emscripten) generated SIMD.js code.

APIs include:

	SIMD.float32x4.load
	SIMD.float32x4.loadX
	SIMD.float32x4.loadXY
	SIMD.float32x4.loadXYZ
	SIMD.float32x4.store
	SIMD.float32x4.storeX
	SIMD.float32x4.storeXY
	SIMD.float32x4.storeXYZ
	SIMD.float64x2.load
	SIMD.float64x2.loadX
	SIMD.float64x2.store
	SIMD.float64x2.storeX
	SIMD.int32x4.load
	SIMD.int32x4.loadX
	SIMD.int32x4.loadXY
	SIMD.int32x4.loadXYZ
	SIMD.int32x4.store
	SIMD.int32x4.storeX
	SIMD.int32x4.storeXY
	SIMD.int32x4.storeXYZ

## Crosswalk 10 (Dec 31, 2014)

* Updates the SSL implementation to BoringSSL. This solves an issue with the Google Play Store warning about a security vulnerability in the version of OpenSSL previously used by Crosswalk ([XWALK-3217](https://crosswalk-project.org/jira/browse/XWALK-3217))
* Due to some changes to support multiple profiles, upgrading an application to Crosswalk 9 was causing local storage data to disappear. With Crosswalk 10 the data is migrated correctly ([XWALK-3252](https://crosswalk-project.org/jira/browse/XWALK-3252))
* Rebased to Chromium 39
* Updated Embedding API (v4), adding the following methods:
   * [getRemoteDebuggingUrl](https://crosswalk-project.org/jira/browse/XWALK-2763)
   * [onReceivedSslError](https://crosswalk-project.org/jira/browse/XWALK-2762)
   * [onCreateWindowRequested](https://crosswalk-project.org/jira/browse/XWALK-2374)
   * [onIconAvailable and onReceivedIcon](https://crosswalk-project.org/jira/browse/XWALK-2373)
* Support for Cordova 3.6.3
* Crosswalk AAR package in Stable and Beta channels for Maven/Gradle integration ([documentation](https://crosswalk-project.org/documentation/android/embedding_crosswalk/crosswalk_aar.html))
* Remote debugging is disabled by default and needs to be enabled with `--enable-remote-debugging`

## Crosswalk 9 (Nov 25, 2014)

* Rebased to Chromium 38, bringing:
   * `<picture>` Element
   * File constructor
   * JS iterators (ES6)
   * Map (ES6)
   * Math functions (ES6)
   * Screen Orientation API
   * Set (ES6)
   * Symbols (ES6)
   * Unscopables (ES6)
  * image-rendering: pixelated

### Crosswalk on Android

* [XWALK-1638] - Enable shared mode for Crosswalk core.
* [XWALK-1914] - Implement the changes in SIMD.JS according to API spec change
* [XWALK-1930] - Add Crosswalk library to the Maven repository
* [XWALK-1947] - Packaging enhancement for external extensions  (extension_manager.py )
* [XWALK-2376] - Embedding API: Preference value
* [XWALK-2378] - Add a new onNewIntent method that users can override to handle a new Intent
* [XWALK-2391] - Crosswalk script make_apk.py is runnable from any location
* [XWALK-2394] - Temporary build directories are in system "temp" location
* [Embedding API version 3.0](https://crosswalk-project.org/apis/embeddingapidocs_v3/index.html)

### Notable Bug Fixes

* [XWALK-1664] - Unable to save the image in Capture component on Cordova 3.5.0
* [XWALK-2150] - Fail to switch to Full Screen mode when tapping "Request" button in Webapi Usecase/Full Screen Test
* [XWALK-2154] - Unable to get cpu/storage/memory/display/avcodecs info of the device and system in Webapi/DeviceCapabilities on Android
* [XWALK-2188] - [Android] XWalkView will display a short white screen when press home button
* [XWALK-2417] - CordovaCrossWalk - onPause javascript code not executed until onresume
* [XWALK-2432] - [Android] Crash on http://www.w3schools.com/html/tryit.asp?filename=tryhtml5_video_all
* [XWALK-2462] - [REG] Fail to run Cordova remote debug testing in chrome browser
* [XWALK-2051] - screen.orientation.type is undefined when running “W3C/ScreenOrientation” on Android OS
* [XWALK-1321] - No words show in the text area when running “Usecase/WebSpeech” on Android OS
* [XWALK-2229] - White Flash between application transitions

## Crosswalk 8 (Sep 30, 2014)

* Rebased to Chromium 37 which brings:
   * &#60;dialog&#62; element
   * CSS Shapes Module Level 1
   * Navigator.hardwareConcurrency
   * NavigatorLanguage: navigator.languages and languagechange event
   * Subpixels font scaling
   * [Web Crypto API] (http://www.w3.org/TR/WebCryptoAPI/)

* Initial support of http://w3c.github.io/manifest/

### Android

* Support for Copy/Paste
* [W3C Gamepad API] (http://www.w3.org/TR/gamepad/) (on devices that support it in the [MotionEvent API](http://developer.android.com/reference/android/view/MotionEvent.html) - so far tested on Nexus 7 and Nexus 5 with Android 4.4.2)
* Pluggable extensions for embedded Crosswalk
* [Embedding API v2.1] (https://crosswalk-project.org/apis/embeddingapidocs_v2/reference/org/xwalk/core/package-summary.html)
* ~30% performance improvement when transferring large blocks of data with extensions

### Notable bug fixes

* [XWALK-1037] - Open web pages in default browser instead of in-app browsing
* [XWALK-1348] - Fail to get correct value of “client.statusText” when running “WebAPI/xmlhttprequest” on Android OS
* [XWALK-1408] - Resource timing data contains negative values when running “Use Case/ResourceTiming” on Android OS
* [XWALK-1833] - [REG]Crosswalk crashed when running “WebAPI/getUserMedia” component on Android OS
* [XWALK-1931] - [REG] Webapp 100% crashes on Android Crosswalk if no SD card on test device
* [XWALK-1952] - Native filesystem API initialize crash for cordova
* [XWALK-1970] - XWalkView shows a white background momentarily as the app starts and stops from the task switcher
* [XWALK-2010] - Hook the native destroying into GC
* [XWALK-2053] - "version" field in manifest should not be mandatory and should be renamed to "xwalk_version"
* [XWALK-2059] - The scroll experience is poor when scrolling a rich and long page
* [XWALK-2135] - [Android] make apk doesn't take alias password as argument
* [XWALK-2292] - App goes into full screen mode after switching from Android task manager

## Crosswalk 7 (Aug 1, 2014)

* Rebased to Chromium 36 which brings:
   * CSS touch-action
   * CSS will-change
   * HTML imports
   * Object.observe()
   * Unprefixed CSS Transforms
   * Web Animations JavaScript API [element.animate()]
* Enabled Native Client (NaCL) on Tizen and Linux for 32 and 64-bit
* Capability to access file system directly on Android and Tizen/Linux

### Android

* Support for Cordova 3.5.0
* [API for In-App Purchases](https://github.com/crosswalk-project/crosswalk-android-extensions/wiki/A-Possible-IAP-Web-IDL)
* Ability to open a file with third party app based on the MIME type
* Only letters, digits, " " and "_" are allowed for the application's name in the manifest
* Use TextureView and SurfaceView for embedding API

### Notable bug fixes

* [XWALK-978] - Pixel game got blurred on crosswalk
* [XWALK-1207], [XWALK-1209] - Exception thrown when minifying CSS and JS files packer tool minify css files with compressor option
* [XWALK-1245] - Font rendering is weird in Crosswalk 5
* [XWALK-1367] - Unable to launch application in fullscreen mode
* [XWALK-1466] - make_apk_test fails when out/ has both ARM and x86 builds
* [XWALK-1476] - Crosswalk crashes if the application name has a space in it
* [XWALK-1538] - Power consumption of HTML5 video has increased ~14% on Android/Geek
* [XWALK-1608] - Can not re-enter fullscreen mode again when playing embeded video
* [XWALK-1616] - Launching Crosswalk-Cordova ARM app on IA device doesn't report Architecture mismatch error
* [XWALK-1618] - Apk will crash when set ‘undefined’ arch option
* [XWALK-1623] - Launch screen display is delayed in Crosswalk 6
* [XWALK-1627] - Javascript's native confirm box locks the app
* [XWALK-1633] - Always get IntentReceiverLeaked message when exiting app using Presentation API
* [XWALK-1635] - White screen displayed when no 'image' provided in Launch Screen
* [XWALK-1639] - XWalk will crash when exit.
* [XWALK-1653] - Timer can not resumed when resuming back
* [XWALK-1695] - onRsourceLoadStarted callback must be posted to UI thread to run
* [XWALK-1702] - Blank browser after loading URL until it is resized
* [XWALK-1711] - "space" in application name is replaced with "underscore"
* [XWALK-1713] - Python 3 deprecated sys.maxint
* [XWALK-1760] - Get a half black and white screen before going to the game when loading the splashscreen
* [XWALK-1780] - onPageStarted will be fired when iframe URL changes.
* [XWALK-1815] - XWalkView evaluateJavascript API should allow null resultCallback to align with Android WebView
* [XWALK-1859] - Crosswalk Cordova should allow universal access from file URLs for Android 4.0.x
* [XWALK-1863] - Start app and quickly press back button, the app will crash
* [XWALK-1882] - XWalkViewClient.onPageStarted not being called
* [XWALK-1908] - Cache mode does not work.
* [XWALK-1963] - Webapp 100% crashes on Android Crosswalk if no SD card on test device

## Crosswalk 6 (Jun 25, 2014)

* Rebased to Chromium 35

### Android

* First version of [public API for embedded usage (XWalkView)] (https://crosswalk-project.org/#documentation/apis/embedding_api)
* [Build Crosswalk applications with Eclipse] (https://crosswalk-project.org/#wiki/Crosswalk-Developer-Tools-Eclipse-plugin)
* Packaging tool generates APKs for both ARM and x86 processor architectures
* Support for Cordova 3.4.0
* Full support for ["Launch Screen" specification] (https://crosswalk-project.org/#wiki/Launch-Screen)
* [Pass Chromium command line options via configuration file] (https://crosswalk-project.org/#wiki/Use-Chromium-command-lines-in-your-apps-on-Android)
* [Option to prevent screen from switching off] (https://crosswalk-project.org/#wiki/Try-Crosswalk)
* [Option to ignore GPU blacklist] (https://crosswalk-project.org/#wiki/Use-Chromium-command-lines-in-your-apps-on-Android)

### Notable bug fixes

* [XWALK-817] - TCPSocket does not support addressReuse/bufferedAmount /noDelay attributes on Android OS
* [XWALK-822] - Handle security exception in Android messaging api
* [XWALK-985] - White screen when package web page named as "http.html" with --manifest
* [XWALK-1200] - Fullscreen immersive mode doesn't work by swiping up from bottom on Android OS
* [XWALK-1210] - Application with space in its name cannot be launched
* [XWALK-1212] - The webapp can be created successfully when set incorrect value of app-local-path option
* [XWALK-1250] - d8 crashes when constructing Float32x4Array or Int32x4Array without --simd-object flag
* [XWALK-1274] - There is an empty alert dialog popped up caused by network error when opening a remote site
* [XWALK-1460] -  Packaging tool enable set long string in package option
* [XWALK-1463] - The space is not replaced with '_' when the Manifest name is tail with space
* [XWALK-1582] - space is replaced with '_' when name contain space

## Crosswalk 5 (Apr 20, 2014)

Based on Chromium 34

* Use SIMD (Single Instruction, Multiple Data) in JavaScript with the [SIMD4JS API] (https://github.com/johnmccutchan/ecmascript_simd) (_Note: SIMD operations are only accelerated on x86 architecture at present_)
* Support for declaring a default [Content Security Policy] (https://dvcs.w3.org/hg/content-security-policy/raw-file/tip/csp-specification.dev.html) in the manifest file
* New download folder and package structure to support multiple architectures in one packaging tool
* New --compressor option in make_apk to minify web app resources (JS, CSS) during packaging
* Rebased to Chromium 34, which brings:
   * Unprefixed Web Audio support
   * Responsive images
   * [Vibration API] (http://www.w3.org/TR/vibration/)

### Android

* Android 4.4+ now uses [Immersive Mode](https://developer.android.com/about/versions/kitkat.html#44-immersive) when in fullscreen
* Integration with system notifications
* Initial support for declaring a [launch screen](https://crosswalk-project.org/#wiki/Launch-Screen) in the manifest file
* New downloadable archive containing all the tools needed to package Cordova applications with Crosswalk (see the [documentation](https://crosswalk-project.org/#wiki/crosswalk-cordova-android))

##Notable Bugs Fixed

* Failed to get correct result of "background-position" when running tct-backgrounds-css3-tests on Android OS [XWALK-470]
* setWebContentsDebuggingEnabled API missing in XWalk [XWALK-753]
* packaging tool does not work with minimal manifest [XWALK-909]
* make_apk.py --enable-remote-debugging not working [XWALK-933]
* Crosswalk doesn't honor Chrome GPU blacklist [XWALK-934]
* window.close doesn't work on Android OS [XWALK-939]
* Unable to get the "messaging" permission on Android OS [XWALK-940]
* Volume is too low when playing audio via WebAudio API [XWALK-980]
* XWalkView shouldInterceptRequest never called [XWALK-996]
* Remote debugging not enabled for debug build of Crosswalk Cordova application [XWALK-1039]
* Capture API test in cordova_mob_spec crashes after recording the audio [XWALK-1116]

##Known issues

* Music does not seamlessly loop [XWALK-1125]
* Setting gain to 0 does not mute [XWALK-1126]
* Web app cannot detect the right motion when device orientation change [XWALK-1264]
* Full screen immersive mode doesn't work by swiping up from bottom [XWALK-1200]
* Fail to get correct value of CSS3 multicolumn column-rule-width [XWALK-1093]
* W3C Notification onclick event callback is not working on Cordova packaged app [XWALK-1080]
* Can't save content to a file by using W3C file API [XWALK-1079]

## Crosswalk 4 (Mar 28, 2014)

Based on Chromium 32

### Runtime

#### Android

* Support app:// scheme for web apps
* Add "crosswalk" in User Agent
* Bridge crosswalk manifest permission to Android Manifest in packaging tool
* Apps can be moved to SD Card

#### Tizen (note: Crosswalk 4 on Tizen is only available in the Beta channel)

* "Run As Service" - Allow to run Crosswalk as a central daemon process with additional render/extension process for each running application
* Extension Process SMACK support for Tizen 2.x
* Support for Tizen legacy application packages (wgt)
* Initial support for Tizen IVI
* Update an already installed web application with xwalk --install
* Support App.runtime APIs (onLaunched, onRestarted, on Suspend, onInstalled)
* Extension namespace is read-only

### API

* WebAudio
* WebRTC
* Screen orientation
* app: URI
* HTML5 fullscreen API support
* WebGL 1.0
* Cross-Origin Resource Sharing
* iframe
* Server-Sent Events
* Navigation Timing
* Resource Timing
* User Timing
* High Resolution Time

#### Android

* Support for Cordova 3.3.0
* Speech Recognition API


## Crosswalk 3 (Jan 24, 2014)

Based on Chromium 32

### Runtime

#### Common

* Non-persistent "main" document
* Basic application event API support
* [Manifest format] (https://crosswalk-project.org/#wiki/Crosswalk-manifest)

#### Android Runtime

* Embedded mode enabled by default
* Support for 'mailto:' scheme
* Support for 'geo:0,0?q=' scheme
* Support for 'content://' scheme
* Cookie support
* Enable HTTP authentication
* Support manifest.json when running web apps
* onTitleChanged callback added in XWalkViewTestBase
* Back key bahavior for XWalkView

#### Tizen Runtime

* Rotation support
* Runtime flags to enable/disable experimental features

#### Extensions framework

* Extension Process Crash Handling
* Android permission support

### API

#### Common

* [Device Capabilities API] (http://www.w3.org/2012/sysapps/device-capabilities/)
* [DeviceOrientation event API] (http://www.w3.org/TR/orientation-event/)
* [Raw Sockets] (http://www.w3.org/TR/raw-sockets/)
* [Media Capture and Streams] (http://www.w3.org/TR/mediacapture-streams/)
* HTML5 Audio/Video
* [Page Visibility API] (http://www.w3.org/TR/page-visibility/)

#### Android API

* Phonegap: Accelerometer
* Phonegap: Camera
* Phonegap: Capture
* Phonegap: Compass
* Phonegap: Connection
* Phonegap: Contacts
* Phonegap: Device
* Phonegap: Events
* Phonegap: File
* Phonegap: Geolocation
* Phonegap: Globalization
* Phonegap: InAppBrowser
* Phonegap: Media
* Phonegap: Notification
* Phonegap: Splashscreen
* Phonegap: Storage
* Presentation API for WiDi Support
* Contacts Manager API

#### Tizen Device API

* Message Port API
* Notification API
* Filesystem API
* Bookmark API

## Crosswalk 2 (Dec 20, 2013)

Based on Chromium 31

### Runtime

* Initial Digital Signature in XPK Package Header
* Web Manifest
* Application runtime APIs (getMainDocument, getManifest)

### Android

* WebRTC support
* Support version, description, permissions in packaging and manifest parsing tools
* Support the properties in manifest.json
* Media Capture and Streams
* DeviceOrientation event API

### Tizen

* --install, --uninstall and --list-apps command line options
* Bluetooth API

### Extensions framework

* Extension support for Android (except permissions for external extensions). See [documentation] (https://github.com/crosswalk-project/crosswalk-website/wiki/Writing-a-Crosswalk-Java-Extension-on-Android).
* Improved robustness: better browser and unit tests coverage, fixed race conditions and tons of internal fixes.
* Improved internal extensions execution model: now external extensions run in the main thread.
* Only load Extensions on demand, exactly on the first time they are used on a frame.
* Extensions writers can make use of JS helpers and small libraries by using the "requireNative()" function and the v8tools module.
* Create one Extension Process per Render Process. Now each WebApp will have its own pair of Renderer+Extensions Processes.

## Crosswalk 1 (Nov 11, 2013)

Based on Chromium 29
Supports Android 4.0 and later, Tizen 2.x mobile
Create and install packaged applications (.apk on Android, .xpk on Tizen)
Initial manifest parsing
Launch app in fullscreen mode
Persistent event page
Command line package installation
Launch installed applications from home screen
Web application integrated with task manager
Viewport meta tag handling across all Crosswalk configurations
Sample web applications to demonstrate HTML5 features and performance
(Android) Run a simple web application without manifest support
(Android) Package HTML/JS/CSS files into web app APKs
(Android) Initial web app APK template
(Android) Android API for debugging over USB
(Tizen) First release of Crosswalk RPM for Tizen with Aura support
(Tizen) First release of extension system - developers can create their own API extensions and install them as separate packages
(Tizen) Support for HW keys (menu&back)
(Tizen) Basic virtual keyboard support
(Extensions) Run extensions in separated extension process, also support running them in-process
(Extensions) Support require() primitive in JS API extensions
(Extensions) Allows providing convenience and native functions when needed
(Extensions) Basic support for dependency tracking, focusing on Tizen Device APIs case
(Extensions) Provide bindings for code running in renderer process and tools for generating bindings.

### APIs

* [app: URI] (http://www.w3.org/2012/sysapps/app-uri/) (_Tizen only_)
* [Fullscreen] (http://fullscreen.spec.whatwg.org/)
* [Touch Events] (https://dvcs.w3.org/hg/webevents/raw-file/v1/touchevents.html)
* [Typed Array 1.0] (http://www.khronos.org/registry/typedarray/specs/latest/)
* [Media Queries Level 3] (http://w3c-test.org/csswg/mediaqueries3/)
* [Scalable Vector Graphics (SVG) 1.1] (http://www.w3.org/TR/SVG11/)
* [HTML Canvas 2D Context] (http://www.w3.org/TR/2dcontext/)
* [Online State] (http://www.w3.org/html/wg/drafts/html/CR/browsers.html#browser-state)
* [XMLHttpRequest] (http://www.w3.org/TR/XMLHttpRequest/)
* [WebSocket] (http://www.w3.org/TR/websockets/)
* [HTML5 web Messaging] (http://www.w3.org/TR/webmessaging/)
* [HTML5 Date and Time state of input element] (http://www.w3.org/TR/html5/forms.html#date-and-time-state-(type=datetime)) (_Android only_)
*  [HTML5 telephone, email and URL state of input element] (http://www.w3.org/TR/html5/forms.html#telephone-state-(type=tel)) (_Android only_)
* [Indexed DB] (https://dvcs.w3.org/hg/IndexedDB/raw-file/default/Overview.html)
* [Web Storage] (http://www.w3.org/TR/webstorage/)
* [File API] (http://dev.w3.org/2006/webapi/FileAPI/)
* [File API: Directories and System] (http://dev.w3.org/2009/dap/file-system/file-dir-sys.html)
* [File API: Writer] (http://dev.w3.org/2009/dap/file-system/file-writer.html)
* [Web SQL] (http://www.w3.org/TR/webdatabase/)
* [CSS Color Module Level 3] (http://www.w3.org/TR/css3-color/)
* [Selectors Level 1] (http://www.w3.org/TR/selectors-api/)
* [Selectors Level 2] (http://www.w3.org/TR/selectors-api2/)
* [CSS Backgrounds and Borders Level 3] (http://www.w3.org/TR/css3-background/)
* [CSS Multi-column Layout] (http://www.w3.org/TR/css3-multicol/)
* [CSS Flexible Box Layout ] (http://www.w3.org/TR/css3-flexbox/)
* [CSS Text Decoration Level 3] (http://www.w3.org/TR/css-text-decor-3/)
* [CSS Animations] (http://www.w3.org/TR/css3-animations/)
* [CSS Fonts Level 3] (http://www.w3.org/TR/css3-webfonts/)
* [CSS Transforms] (http://www.w3.org/TR/css3-transforms/)
* [CSS Transitions] (http://www.w3.org/TR/css3-transitions/)
* [Navigation Timing] (http://www.w3.org/TR/navigation-timing/)
* [Resource Timing] (http://www.w3.org/TR/resource-timing/)
* [Page Visibility] (http://www.w3.org/TR/page-visibility/)
* [Web Workers] (http://www.w3.org/TR/workers/)
