/*

This library lets you use a web browser to configure and connect ESP8266 to a wifi network
It has been tested with Wemos D1 boards (www.wemos.cc), but should work
with other ESP8266-based boards

Instructions for connecting to access point, when the ESP8266 is in 
Access Point mode (configuration mode):

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

NOTE: the led can be active or not depending on arguments to the WemosSetup::begin-method

The loop of the Arduino sketch must include the inLoop()-method:

#include "WemosSetup.h"
WemosSetup wifisetup;

void setup() {
    wifisetup.begin();
    //additional code here
}

void loop() {
    wifisetup.inLoop();
    //aditional code here
}

See more in example files
*/

#ifndef WemosSetup_h
#define WemosSetup_h

#define WFS_DEBUG true
//Serial.begin(baudrate) required in setup() of Arduino sketch if WFS_DEBUG is true

#if WFS_DEBUG
    #define wfs_debugprint(...) Serial.print(__VA_ARGS__)
    #define wfs_debugprintln(...) Serial.println(__VA_ARGS__)
#else
    #define wfs_debugprint(...)
    #define wfs_debugprintln(...)
#endif

//ssid: allow for 32 chars + null termination
//passkey: allow for 63 chars + null termination
//html: length of htmlstart+onload+htmlmid+body+htmlend (at the moment 158+128+4+1500+15 = 1745)
//networkc: truncate if there are more networks
//body: max length of networkch + 2 * max length of WiFiSSID + fixed part (at the moment 1024 + 2*32 +367 = 1455)

#define WFS_MAXSSIDLENGTH 33
#define WFS_MAXPASSKEYLENGTH 64
#define WFS_MAXNETWORKCHLENGTH 1024

#define WFS_MAXBODYLENGTH 1516
#define WFS_MAXHTMLLENGTH 1805
#define WFS_MAXONLOADLENGTH 128

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

class WemosSetup {
  public:
    WemosSetup();
    void begin(WiFiMode startmode = WIFI_STA, unsigned long activeTime = 0, int led_pin = -1);

    static void switchToSTA();
    static void switchToAP_STA();
    
    static void startSTA(unsigned long activeTime);
    static void startAP_STA(unsigned long activeTime);
    static bool connectWiFi();
    void shortBlink();
    static void startWebServer();
    static void stopWebServer();
    //xxx void handleClient();
    void printInfo();
    bool connected();
    void inLoop();
    void toggleAccessPoint();
    static void sendHtml(const char *body, const char *onload);
    static void afterConnection(const char *url);
    
    static void handleRoot();
    static void handleFrm();
    static void ledWrite(int status);

    static ESP8266WebServer server;
    static WiFiMode wifimode;

    static byte ledStatus; //LOW means ON, HIGH means OFF
    static int _led_pin;

    //some variables to keep track of current state
    static bool runAccessPoint;
    static bool showFailureOnWeb;
    static bool showSuccessOnWeb;
    static bool webServerRunning;
    static bool stationStarted;
    static bool accessPointStarted;
    static bool tryingToConnect;
    static unsigned long timeToChangeToSTAAfterConnection;
    

    static char html[WFS_MAXHTMLLENGTH];
    static char body[WFS_MAXBODYLENGTH];
    static char onload[WFS_MAXONLOADLENGTH];
    
    static char afterConnectionUrl[32];

    const static char htmlstart[];
    const static char htmlmid[];
    const static char htmlend[];

    static String networks;

    static char WiFiSSID[WFS_MAXSSIDLENGTH];
    static char WiFiPSK[WFS_MAXPASSKEYLENGTH];

    static unsigned long timeToChangeToSTA; //when is it time time to change to STA (station) in ms. Zero means never

    static const byte OFF;
    static const byte ON;



  private:
    
    const unsigned long checkRate = 10*1000; //how often wifi status is checked and led blinked
    unsigned long lastCheck = 0;

    const unsigned long apBlinkRate = 100; //blink rate when in AP (access point) mode.
    unsigned long lastApBlink = 0;


    /*
    WL_NO_SHIELD = 255,
    WL_IDLE_STATUS = 0,
    WL_NO_SSID_AVAIL = 1
    WL_SCAN_COMPLETED = 2
    WL_CONNECTED = 3
    WL_CONNECT_FAILED = 4
    WL_CONNECTION_LOST = 5
    WL_DISCONNECTED = 6
    */


};

#endif
