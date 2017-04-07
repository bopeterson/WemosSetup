/*
This demo shows how to start in station mode (WIFI_STA) and connect to last ssid.
It will change to access point mode if connection fails, and stay in 
access point mode for 120 seconds. 

When in access point mode, it is possible for a computer or phone to connect to the network "configure",
browse to 192.168.4.1 and enter ssid and password for a new wifi network.

- If no connection attempt is made, it will return to station mode, and try again to connect to last ssid
- If a successful connection attempt is made, it will return to station mode, connected to the new network
- If the connection attempt is not successful, it will stay in access point mode. 

A button connected to pin D6 will toggle between station mode and access point mode. This requires the
WemosButton library as well. 

The built in led on the wemos will signal wifi status:

- OFF with short blink every 10th second: connected
- ON  with short blink every 10th second: not connected
- Blinking 5 blinks per second: ESP in access point (configuration) mode. 
  Network ssid and password can be configured through web browser
- Blinking a little slower: ESP is trying to connect
 */

#include "WemosSetup.h"
#include "WemosButton.h"

WemosSetup wifisetup;
WemosButton d6;

const unsigned long periodicRate = 15*1000; //do periodic task in main loop every 15 seconds
unsigned long periodicLast = 0;

void setup() {
  d6.begin(D6); //initalize the button connected to D6
  Serial.begin(115200); //needed if WFS_DEBUG is true in WemosSetup.h
  //start in station mode and try to connect to last ssid,
  //switch to AP mode and stay there in 120 s if it couldn't connect,
  //and use the builtin led to signal status:
  wifisetup.begin(WIFI_STA,120,BUILTIN_LED); 

  //initalize timer for periodic task
  periodicLast = millis() - periodicRate;
}

void loop() {
  //needed in the loop to handle wifi mode changes and other tasks
  wifisetup.inLoop();
  
  //button toggles between station and access point mode
  if (d6.readButton()) {
    wifisetup.toggleAccessPoint();
  }
  
  unsigned long justNow = millis();
  if (periodicLast + periodicRate <= justNow) {
    periodicLast = justNow;
    //put any code here that should run periodically
    wifisetup.printInfo(); //WFS_DEBUG to be true in WemosSetup.h for this to work
  }
}

