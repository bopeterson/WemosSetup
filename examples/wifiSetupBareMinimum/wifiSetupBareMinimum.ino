#include "WemosSetup.h"
WemosSetup wifisetup;

void setup() {
    //Starts in access point mode for configuration.
    //Switches to station mode after 120 s if no connection
    //attempt is made, and tries to connect to last ssid.
    wifisetup.begin(WIFI_AP,120);

    //additional code here
}

void loop() {
    wifisetup.inLoop();
    //aditional code here
}
