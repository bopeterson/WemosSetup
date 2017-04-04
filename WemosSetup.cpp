#include "Arduino.h"
#include "WemosSetup.h"

ESP8266WebServer WemosSetup::server=ESP8266WebServer(80);

char ap_ssid[] = "configure";
char ap_pass[] = "configurenow";


bool WemosSetup::runAccessPoint=false;
bool WemosSetup::showFailureOnWeb=false;
bool WemosSetup::showSuccessOnWeb=false;
bool WemosSetup::webServerRunning=false;
bool WemosSetup::accessPointStarted=false;
bool WemosSetup::stationStarted=false;
bool WemosSetup::tryingToConnect=false;

char WemosSetup::html[]="";
const char WemosSetup::htmlstart[] = "<!doctype html>\r\n<html><head><meta charset='UTF-8'><style>body {font-family:sans-serif}</style><title>Connect</title></head><body onload=\"";
const char WemosSetup::htmlmid[] = "\">";
const char WemosSetup::htmlend[] = "</body></html>";

String WemosSetup::networks;

char WemosSetup::WiFiSSID[WFS_MAXSSIDLENGTH];
char WemosSetup::WiFiPSK[WFS_MAXPASSKEYLENGTH];

WiFiMode WemosSetup::wifimode;
byte WemosSetup::ledStatus;

const byte WemosSetup::ON = LOW;
const byte WemosSetup::OFF = HIGH;

unsigned long WemosSetup::timeToChangeToSTA = 0; //when is it time time to change to STA (station) in ms. Zero means never

int WemosSetup::_led_pin;

WemosSetup::WemosSetup() {
    wifimode = WIFI_STA;
}

void WemosSetup::begin(WiFiMode startmode, unsigned long activeTime, int led_pin) {
    /*
    default values defined in header file:
    WiFiMode startmode = WIFI_STA
    activeTime = 0
    led_pin = -1
    
    startmode can be WIFI_STA or WIFI_AP_STA
  
    if startmode is WIFI_STA and no connection can be made, it changes to AP mode
    
    activeTime is how long in ms it stays in AP mode before switching to STA. 
  
    activeTime = 0 means:
    - stay in access point mode until successfull connection is made if 
    startmode is WIFI_AP_STA

    - stay forever in station mode if startmode is WIFI_STA
    
    led_pin is pin number of status led. -1 means don't use led. 
    
    */
    _led_pin=led_pin;
    if (_led_pin != -1) {
        pinMode(_led_pin, OUTPUT);
    }
    ledStatus = ON;
    ledWrite(ledStatus);
    
    if (startmode == WIFI_AP_STA) {
        startAP_STA(activeTime);
    } else if (startmode == WIFI_STA) {
        startSTA(activeTime);
    } else {
        startSTA(activeTime); //start in station mode if no valid mode is given
    }
}

bool WemosSetup::connected() {
    return (WiFi.status() == WL_CONNECTED);
}

void WemosSetup::mode(WiFiMode mode) {
    //note: changing to AP or AP_STA won't work if startAP hasn't run before
    //same might apply when changing to STA
    
    //xxx replace this by switch to sta and switch to ap sta
    wfs_debugprint("changing mode to ");
    wfs_debugprintln(mode);
    wifimode = mode;
    WiFi.mode(wifimode);
}

void WemosSetup::switchToSTA() {
    // xxxxxxxx jfr m timetochangetosta vad som görs då
    if (!stationStarted) {
        //xxx test that this works
        /*
        right now it drops connection if you start in sta, switch to ap_sta and back to sta
        hopefully fixed now
        
        */
        startSTA(0);
    } else {
        wifimode = WIFI_STA;
        WiFi.mode(wifimode);        
    }
    timeToChangeToSTA = 0;
}

void WemosSetup::switchToAP_STA() {
    if (!accessPointStarted) {
        //xxx make sure to keep connected. now it disconnects station
        //hopefully fixed now
        startAP_STA(0);
    } else {
        wifimode = WIFI_AP_STA;
        WiFi.mode(wifimode);        
    }
    timeToChangeToSTA = 0;
}


void WemosSetup::startSTA(unsigned long activeTime) {
    //activeTime is how long it stays in AP mode in ms if it swithces to AP after unsuccessful connection attempt. 0 means stay forever in station mode
    
    //start in station mode and try to connect to last known ssid
    wifimode = WIFI_STA;
    WiFi.mode(wifimode);
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(); //reconnectes to previous SSID and PASSKEY if it has been in AP-mode
    }
    if (activeTime!=0) {
        delay(7000);
        if (WiFi.status() == WL_CONNECTED) {
            wfs_debugprint("Successfully connected to ");
            wfs_debugprintln(WiFi.SSID());
            ledStatus = OFF;
            ledWrite(ledStatus);
        } else {
            wfs_debugprintln("!!! Could not connect to ");
            wfs_debugprintln(WiFi.SSID());
            ledStatus = ON;
            ledWrite(ledStatus);
            startAP_STA(activeTime); //start accesspoint for activeTime minutes if it could not connect
        }
    }
    stationStarted = true;
}

void WemosSetup::startAP_STA(unsigned long activeTime) {
    //activeTime is how long it stays in AP mode in ms before switching to STA. 0 means stay in AP forever 
    
    //remember old ssid and psk for STATION as they might be cleared by WiFi.disconnect() or WiFi.scanNetworks
    WiFi.SSID().toCharArray(WiFiSSID, WFS_MAXSSIDLENGTH);
    WiFi.psk().toCharArray(WiFiPSK, WFS_MAXPASSKEYLENGTH);
    
    delay(500); //these delays seems to prevent occasional crashes when scanning an already running access point
    wifimode = WIFI_AP_STA; //used to be WIFI_AP but sometimes computer lost connection when changing form WIFI_AP to WIFI_AP_STA
    WiFi.mode(wifimode);
    delay(200); //these delays seems to prevent occasional crashes when changing to access point. Maybe not needed after updated ESP8266WiFi library
    //WiFi.disconnect(); //it should not try to connect to previous networks. or maybe it should? xxx But what if it starts in AP_STA, and there is no wifi.begin? consequneces?
        
    networks="<option value=''>enter ssid above or select here</option>";
    wfs_debugprintln("start scan");
    int n=WiFi.scanNetworks();
    wfs_debugprintln("scanning complete");
        
    if (n>0) {
        for (int i = 0; i < n; ++i) {
            if (networks.length()+17+WiFi.SSID(i).length()<WFS_MAXNETWORKCHLENGTH) {
                //if more networks than what fits in networksch char array are found, skip them
                networks=networks+"<option>"+WiFi.SSID(i)+"</option>";
            }
        }
    }

    if (activeTime==0) {
        timeToChangeToSTA = 0;
    } else {
        timeToChangeToSTA = millis()+activeTime*1000;
    }

    if (!accessPointStarted) {
        IPAddress stationip(192, 168, 4, 1);//this is the default but added manually just to be sure it doesn't change in the future
        IPAddress NMask(255, 255, 255, 0);
        WiFi.softAPConfig(stationip, stationip, NMask);    
        WiFi.softAP(ap_ssid,ap_pass);
        IPAddress apip = WiFi.softAPIP();
        wfs_debugprint("Starting access point mode with IP address ");
        wfs_debugprintln(apip);
        wfs_debugprint("Connect to ");
        wfs_debugprint("configure");
        wfs_debugprint(" network and browse to ");
        wfs_debugprintln(apip);
        accessPointStarted = true;
    }

    if (!webServerRunning) {
        startWebServer();
    }    
}

void WemosSetup::handleStatus() {
    wfs_debugprintln("handleStatus");
    
    //we have the following possibilites:
    //1) this page is called after successful connection
    //2) this page is called after failed connection
    //3) this page is called during connection-but this is usually not handled
    //4) this page is called before connection attemp-redirect to form

    char onload[WFS_MAXONLOADLENGTH];
    char body[WFS_MAXBODYLENGTH];

    sprintf(onload,""); //not used now, keep for potential future use

    if (showSuccessOnWeb) {
        sprintf(body, "Successfully connected to %s", WiFiSSID);
        //xxx maybe you can stop the setInterval now with some added <script>. but interval is in window.parent
    } else if (showFailureOnWeb) {
        sprintf(body, "Could not connect to %s. Maybe wrong ssid or password.  <a target='_parent' href='/'>Try again</a>", WiFiSSID);
    } else if (tryingToConnect) {
        //show a "progress bar" of dots
        char n[5];
        if (server.hasArg("n")) {
          server.arg("n").toCharArray(n, 5);
        } else {
          n[0]='3';n[1]=0; //xxx not the best way, change later
        }
        sprintf(body,"<script>document.write(new Array(1+%s).join('.'))</script>",n);
    } else {
        sprintf(body, "<script>window.parent.location.href='/';</script>");
    }
  
    sprintf(html, "%s%s%s%s%s", htmlstart, onload, htmlmid, body, htmlend);
    server.send(200, "text/html", html);
}

void WemosSetup::handleRoot() {
    wfs_debugprintln("handleroot");
    char onload[WFS_MAXONLOADLENGTH];
    char networkch[WFS_MAXNETWORKCHLENGTH]; 
    char body[WFS_MAXBODYLENGTH];

    sprintf(onload,"");

    bool initiateConnection=false;
    if (!server.hasArg("ssid") && WiFi.status() != WL_CONNECTED) {
        networks.toCharArray(networkch,WFS_MAXNETWORKCHLENGTH);
        //note that html element arguments may be double qouted, single quoted or unquoted. Unquoted is usually not recommended but used here to save memory
        sprintf(body, "<p>Not connected</p><form method=post action='/'><input type=text name=ssid value='%s' size=32 id=s1 onchange='document.getElementById(\"s2\").value=this.value'> SSID (network name)<br><select name=s2 id=s2 onchange='document.getElementById(\"s1\").value=this.value'>%s</select><br><input type=password name=pass size=32> PASSWORD<br><input type=submit value=connect></form>", WiFiSSID,networkch);
    } else if (!server.hasArg("ssid")) {
        networks.toCharArray(networkch,WFS_MAXNETWORKCHLENGTH);
        sprintf(body,"<p>Connected to %s</p><form method=post action='/'><input type=text name=ssid value='%s' size=32 id=s1 onchange='document.getElementById(\"s2\").value=this.value'> SSID (network name)<br><select name=s2 id=s2 onchange='document.getElementById(\"s1\").value=this.value'>%s</select><br><input type=password name=pass size=32> PASSWORD<br><input type=submit value=connect></form>",WiFiSSID,WiFiSSID,networkch);
    } else { //server has arg ssid
        showFailureOnWeb = false;
        showSuccessOnWeb = false;
        initiateConnection = true;
        server.arg("ssid").toCharArray(WiFiSSID, WFS_MAXSSIDLENGTH);
        wfs_debugprintln("has arg ssid");
        if (server.hasArg("pass")) {
            server.arg("pass").toCharArray(WiFiPSK, WFS_MAXPASSKEYLENGTH);
        } else {
            sprintf(WiFiPSK,"");
        }
        sprintf(onload,"var n=0;setInterval(function() {n++;document.getElementById('st').src='st?r='+Math.random()+'&n='+n},2000)");
        sprintf(body,"<p>Connecting to %s</p><iframe frameborder='0'  id='st' width='480' height='240' src=''></iframe>",WiFiSSID);
    }

    //Don't change the content on any of these variables without checking their size limits!
    sprintf(html, "%s%s%s%s%s", htmlstart, onload, htmlmid, body, htmlend);
    server.send(200, "text/html", html);
    if (initiateConnection) {
        if (connectWiFi()) {
            wfs_debugprintln("Connection successful");
            showSuccessOnWeb = true;
            ledStatus = OFF;
            ledWrite(ledStatus);
        } else { 
            wfs_debugprintln("Connection failed");
            ledStatus = ON;
            ledWrite(ledStatus);
            startAP_STA(0);
            showFailureOnWeb = true;
        }
    }
}

bool WemosSetup::connectWiFi() {
    delay(400);
    //xxx stopWebServer();
    WiFi.disconnect(); //it sometimes crashes if you don't disconnect before new connection
    delay(100);

    tryingToConnect = true;
    bool timeout = false;

    //wifimode = WIFI_AP_STA; should already be in AP_STA, but used to be AP 
    //WiFi.mode(wifimode);
    
    //The connection sometimes fails if the previous ssid is the same as the new ssid.
    //A temporary connection attempt to a non existing network seems to fix this.
    WiFi.begin("hopefullynonexistingssid", "");
    delay(500);
    wfs_debugprint(F("Connecting to "));
    wfs_debugprintln(WiFiSSID);
    WiFi.begin(WiFiSSID, WiFiPSK);
    // Use the WiFi.status() function to check if the ESP8266
    // is connected to a WiFi network. Timeout after one minute.
    unsigned long timelimit = 60000;
    unsigned long starttimer = millis();
    int delayTime=200;
    int period = 600; //blink period and check if connected period. should be multiple of delayTime
    bool connected=false;
    unsigned long lastCheck=0;
    while (!connected && !timeout) {
        unsigned long loopStart = millis();
        if (lastCheck + period <= loopStart) {
            lastCheck = loopStart;
            wfs_debugprint(".");
            // Blink the LED
            ledStatus = !ledStatus;
            ledWrite(ledStatus); // Write LED high/low 
        }
        //xxx
        if (wifimode != WIFI_STA && webServerRunning) { 
          server.handleClient();
        }
        delay(delayTime); //a delay is needed when connecting and doing other stuff, otherwise it will crash

        if ((loopStart - starttimer) > timelimit) {
            wfs_debugprintln("");
            wfs_debugprint("timed out");
            timeout = true;
        }
        connected = (WiFi.status() == WL_CONNECTED);
        
    }
    if (!timeout) {
        timeToChangeToSTA = millis() + 2*60000; //keep ap running for 120 s after connection
    }
    wfs_debugprintln("");
    tryingToConnect=false;
    //startWebServer(); xxx
    return !timeout;
}

void WemosSetup::startWebServer() {
    wfs_debugprintln("Trying to start webserver");
    if (!webServerRunning) {
      wfs_debugprintln("Starting webserver");
      server.on("/", handleRoot);
      server.on("/st", handleStatus);
      server.begin();
      webServerRunning = true;
    }
}

void WemosSetup::stopWebServer() {
  //WARNING - crash if stopping already stopped server
    wfs_debugprintln("Trying to stop webserver");
    if (webServerRunning) {
      wfs_debugprintln("Stopping webserver");
      server.close(); //stop or close? what is the difference?      
      webServerRunning = false;  
    }
  }

/*
void WemosSetup::handleClient() {
    server.handleClient(); 
}
*/

void WemosSetup::inLoop() {
    unsigned long loopStart=millis();
    //check if it is time to change from configure mode (access point) to normal mode (station)

    yield(); //xxx or delay(2);
    if (timeToChangeToSTA != 0 && timeToChangeToSTA < loopStart) {
        //change to station mode
        if (wifimode != WIFI_STA) {
            
            //xxx does not seem to be needed, might even do harm
            //WiFi.softAPdisconnect(true);
            //delay(300);
            
            wifimode = WIFI_STA;
            WiFi.mode(wifimode);
            
            if (WiFi.status() != WL_CONNECTED) {
                wfs_debugprintln("trying to reconnect to prev ssid");
                if (WiFi.SSID().length() == 0) {
                  WiFi.begin(WiFiSSID, WiFiPSK);
                } else {
                  WiFi.begin(); //reconnectes to previous SSID and PASSKEY if it has been in AP-mode
                }
            }
            wfs_debugprintln("changing to STA");
        }
        timeToChangeToSTA = 0; // Zero means don't change to STA
    }

    if (wifimode != WIFI_STA && webServerRunning) {
        //only handle web requests in access point mode and if webserver is running
        server.handleClient();
        delay(2);
    }

    if ((wifimode != WIFI_AP) && (lastCheck + checkRate <= loopStart)) {
        //turn led on or off to indicate connection (off = connected)
        lastCheck=loopStart;
        if (WiFi.status() == WL_CONNECTED) {
            ledStatus = OFF;
            ledWrite(ledStatus);
        } else {
            ledStatus = ON;
            ledWrite(ledStatus);
        }
        shortBlink();
    }

    if ((wifimode != WIFI_STA) && WiFi.status() != WL_CONNECTED && (lastApBlink + apBlinkRate <= loopStart)) {
        //blink led to indicate access point mode
        lastApBlink = loopStart;
        ledStatus = !ledStatus;
        ledWrite(ledStatus);
    }
}

void WemosSetup::toggleAccessPoint() {
    //xxx this is not safe and should be changed or deleted
    if (wifimode == WIFI_STA) {
        startAP_STA(0);
    } else {
        startSTA(0);
    }  
}

void WemosSetup::ledWrite(int status) {
    if (_led_pin != -1) {
        digitalWrite(_led_pin, status);
    }
}

void WemosSetup::shortBlink() {
    ledStatus = !ledStatus;
    ledWrite(ledStatus);
    if (ledStatus == OFF) {
        delay(30); //longer pulse needed when going ON-OFF-ON
    } else {
        delay(5); //these delays are not needed if the led is not used, but delays now and then seem to do good to the ESP. However, for time critical applications, delays may be omitted if _led_pin==-1
    }
    ledStatus = !ledStatus;
    ledWrite(ledStatus);
}

void WemosSetup::printInfo() {
    //this function is only needed for debugging/testing and can be deleted
    wfs_debugprintln(F("CURRENT STATUS"));
    wfs_debugprint(F("Running for:         "));
    unsigned long now=millis();
    unsigned long h=now/1000/3600;
    byte m=(now-h*1000*3600)/1000/60;
    byte s=(now-h*1000*3600-m*1000*60)/1000;
    wfs_debugprint(h);
    wfs_debugprint(F(":"));
    wfs_debugprint(m);
    wfs_debugprint(F(":"));
    wfs_debugprintln(s);
    wfs_debugprint(F("Change to STA in:    "));
    if (timeToChangeToSTA==0) {
        wfs_debugprintln("don't change");
    } else {
        wfs_debugprint(timeToChangeToSTA-now);
        wfs_debugprintln(" ms");
    }
    wfs_debugprint("WIFI MODE:           ");
    if (wifimode==WIFI_STA) {
        wfs_debugprintln(F("WIFI_STA"));
    } else if (wifimode==WIFI_AP) {
        wfs_debugprintln(F("WIFI_AP"));
    } else if (wifimode==WIFI_AP_STA) {
        wfs_debugprintln(F("WIFI_AP_STA"));
    } else {
        wfs_debugprintln(wifimode);
    }
    wfs_debugprint(F("runAccessPoint:      "));
    wfs_debugprintln(runAccessPoint);
    wfs_debugprint(F("showFailureOnWeb:    "));
    wfs_debugprintln(showFailureOnWeb);
    wfs_debugprint(F("webServerRunning:    "));
    wfs_debugprintln(webServerRunning);
    wfs_debugprint(F("accessPointStarted:  "));
    wfs_debugprintln(accessPointStarted);
    wfs_debugprint(F("stationStarted:      "));
    wfs_debugprintln(stationStarted);
    wfs_debugprint(F("showSuccessOnWeb:    "));
    wfs_debugprintln(showSuccessOnWeb);
    wfs_debugprint(F("WiFi Status:         "));
    switch(WiFi.status()){
        case WL_NO_SHIELD: 
        wfs_debugprintln(F("WL_NO_SHIELD"));
        break;
        case WL_IDLE_STATUS:
        wfs_debugprintln(F("WL_IDLE_STATUS"));
        break;
        case WL_NO_SSID_AVAIL:
        wfs_debugprintln(F("WL_NO_SSID_AVAIL"));
        break;
        case WL_SCAN_COMPLETED:
        wfs_debugprintln(F("WL_SCAN_COMPLETED"));
        break;
        case WL_CONNECTED:
        wfs_debugprintln(F("WL_CONNECTED"));
        break;
        case WL_CONNECT_FAILED:
        wfs_debugprintln(F("WL_CONNECT_FAILED"));
        break;
        case WL_CONNECTION_LOST:
        wfs_debugprintln(F("WL_CONNECTION_LOST"));
        break;
        case WL_DISCONNECTED:
        wfs_debugprintln(F("WL_DISCONNECTED"));
        break;
        default:
        wfs_debugprintln(WiFi.status());
    }
    long rssi = WiFi.RSSI();
    wfs_debugprint(F("RSSI:                "));
    wfs_debugprint(rssi);
    wfs_debugprintln(F(" dBm"));
    wfs_debugprint(F("SSID:                "));
    wfs_debugprintln(WiFi.SSID());
    IPAddress ip = WiFi.localIP();
    wfs_debugprint(F("Station IP Address:  "));
    wfs_debugprintln(ip);
    ip = WiFi.softAPIP();
    wfs_debugprint(F("AP IP Address:       "));
    wfs_debugprintln(ip);
    byte mac[6];
    WiFi.macAddress(mac);
    wfs_debugprint(F("MAC address:         "));
    wfs_debugprint(mac[0], HEX);
    wfs_debugprint(":");
    wfs_debugprint(mac[1], HEX);
    wfs_debugprint(":");
    wfs_debugprint(mac[2], HEX);
    wfs_debugprint(":");
    wfs_debugprint(mac[3], HEX);
    wfs_debugprint(":");
    wfs_debugprint(mac[4], HEX);
    wfs_debugprint(":");
    wfs_debugprintln(mac[5], HEX);
    wfs_debugprintln("");
}


