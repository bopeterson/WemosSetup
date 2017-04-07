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
