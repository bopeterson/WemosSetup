# WiFiSetup Arduino ESP8266 Library

This library lets you use a web browser to configure and connect ESP8266 to a wifi network.
It has been tested with [Wemos D1 boards](https://www.wemos.cc), but should work
with other ESP8266-based boards.

## Installation

Put the WiFiSetup folder and add to the Arduino/Libraries folder and include the library in your Arduino sketches with `#include "WiFiSetup.h"`. You also need to install [Arduino core for ESP8266 WiFi chip](https://github.com/esp8266/Arduino).

## Connecting to access point
 
1. The built in led will blinks with 5 blinks per second to signal that the ESP is a wifi access point with ssid="configure"
2. Connect a computer or other device to the wifi network "configure", and password "configurenow"
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
#include "WemosSetup.h"
WemosSetup wifisetup;

void setup() {
    //Starts in access point mode for configuration.
    //Switches to station mode after 120 s if no connection
    //attempt is made, and tries to connect to last ssid.
    wifisetup.begin(WIFI_AP_STA,120,BUILTIN_LED);

    //You can also start in station mode. It switches to AP_STA mode for
    //configuration if it can't connect to last ssid, then back to
    //station mode if no connection attempt was made
    //wifisetup.begin(WIFI_STA,120,BUILTIN_LED);
    

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

## Thanks

Much help and inspiration found here:

- <https://gist.github.com/dogrocker/f998dde4dbac923c47c1> - very basic connection manager that lacks some vital parts  
- <https://github.com/tzapu/WiFiManager> ESP8266 WiFi Connection manager with web captive portal. Works well but I didn't want the captive portal
- <https://github.com/buxtronix/arduino/tree/master/esp8266-ledclock> - An esp based TNP clock that also includes a nice connection manager. My favourite connection manager (except my own, maybe) 
