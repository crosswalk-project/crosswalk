This file contains high-level info about how XwalkDriver works.

XwalkDriver is an implementation of the WebDriver standard. Part of code is
ported from XwalkDriver.

=====How To=====
For Linux xwalk:
(1) Build XwalkDriver by building the 'xwalkdriver' target and get an executable
binary in the build folder named 'xwalkdriver'.

(2) Build 'xwalk' target, install it in "/opt/crosswalk" or working path of
'xwalkdriver'.

(3) Use following python instructions to do a basic test.

$ export PYTHONPATH=<THIS_DIR>/server:<THIS_DIR>/client
$ python
>>> import server
>>> import xwalkdriver
>>> cd_server = server.Server('/path/to/xwalkdriver/executable')
>>> driver = xwalkdriver.XwalkDriver(cd_server.GetUrl())
>>> driver.Load('http://www.google.com')
>>> driver.Quit()
>>> cd_server.Kill()

For Android xwalk:

(1) Build XwalkDriver by building the 'xwalkdriver' target and get an executable
binary in the build folder named 'xwalkdriver'. Or download the binary from
    https://github.com/iKevinHan/xwalkdriver_binary 


(2) Enable remote debugging in your Android app source code, like this  

    public void onCreate(Bundle savedInstanceState) {
        ...
        setRemoteDebugging(true); // Enable remote debugging
        super.onCreate(savedInstanceState);
        ...
    }

(3) Pakage your app by execute command
    python make_apk.py --manifest=YOUR_APP_PATH/manifest.json

(4) Install your apk to device.

(5) Install Selienium package by executing command
    pip install selenium

(6) Run xwalkdriver binary.

(7) Execute following commands to test:
$ python
>>> from selenium import webdriver
>>> capabilities = {
  'xwalkOptions': {
    'androidPackage': 'YOUR_PACKAGE_NAME', 
    'androidActivity': '.YOUR_ACTIVITY_NAME',
  }
}
>>> driver = webdriver.Remote('http://localhost:9515', capabilities)      
>>> driver.execute_script("alert('aaaa')")


    
For Tizen xwalk:
It will come soon.
