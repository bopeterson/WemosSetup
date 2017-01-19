# WiFiSetup Arduino ESP8266 Library

This library lets you use a web browser to configure and connect ESP8266 to a wifi network.
It has been tested with [Wemos D1 boards](https://www.wemos.cc), but should work
with other ESP8266-based boards.

## Instalation

Put the WiFiSetup folder and add to the Arduino/Libraries folder and include the library in your Arduino sketches with `#include "WiFiSetup.h"`.

## Connecting to access point
 
1. The built in led will blinks with 5 blinks per second to signal that the ESP is a wifi access point with ssid="configure"
2. Connect a computer or other device to the wifi network "configure", no password needed
3. Open a browser on the device and go to http://192.168.4.1
4. Enter ssid and password for the network the ESP should connect to. 
5. The led blinks a little slower while it is trying to connect
6. A messege should appear if you succeeded or not.
7. If you succeeded, the ESP leaves configure state and is now connected
8. If you did not succeed you can try again

The built in led on the wemos will signal wifi status:

- OFF with short blink every 10th second: connected
- ON  with short blink every 10th second: not connected
- Blinking 5 blinks per second: ESP in access point (configuration) mode. 
  Network ssid and password can be configured through web browser
- Blinking a little slower: ESP is trying to connect

NOTE: the led can be active or not depending on arguments to the WiFiSetup::begin-method

The loop of the Arduino sketch should include the inLoop()-method:

```cpp
#include "WiFiSetup.h"
WiFiSetup wifisetup;

void setup() {
    wifisetup.begin();
    //additional code here
}

void loop() {
    wifisetup.inLoop();
    //aditional code here
}
```

See more in example files

## Todo

- more examples
- add password protection in AP mode

## Thanks

Much help and inspiration found here:

- <https://gist.github.com/dogrocker/f998dde4dbac923c47c1> - very basic connection manager that lacks some vital parts  
- <https://github.com/tzapu/WiFiManager> ESP8266 WiFi Connection manager with web captive portal. Works well but I didn't want the captive portal
