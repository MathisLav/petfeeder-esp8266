/*

  POC
  Petfeeder main code.

*/

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ESPAsyncWebServer.h>
#include <petfeeder.h>
#include <processors.h>
#include <filelist.h>
#include <LittleFS.h>


void setup() {
    Serial.begin(115200);
    if(!LittleFS.begin()) {
        eprintln("Failed to mount the filesystem.");
        soft_reset(true);
    }

    while(!WiFi.softAP("Petfeeder")) { delay(1000); }

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", String(), false, indexStateTemplate);
    });
    server.on("/bootstrap.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/bootstrap.css", "text/css");
    });
    server.on("/delcreds", HTTP_GET, [](AsyncWebServerRequest *request) {
        File wifiCreds = LittleFS.open("wifi.txt", "w");
        wifiCreds.close();
        request->send(200, "text/html", "All WiFi credentials were deleted.<br /><a href='/'>Get back to home page.</a>");
        WiFi.disconnect();
        dprintln("WiFi credentials deleted. The device is still connected to the internet, until you reboot it.");
    });
    server.on("/connect", HTTP_POST, [](AsyncWebServerRequest *request) {
        if(!request->hasArg("ssid") || !request->hasArg("password")) {
            request->send_P(200, "text/html", "No credentials provided...");
            return;
        }

        String ssid = request->arg("ssid");
        String pass = request->arg("password");
        dprint("New SSID: ");
        dprintln(ssid);

        wifi_credentials_t creds = {ssid, pass};
        storeCredentials(creds);
        wifiMulti.addAP(ssid.c_str(), pass.c_str());

        request->send(LittleFS, "connect.html", "text/html");
    });

#if DEBUG
    /* Page used to debug LittleFS stuffs */
    server.on("/test", HTTP_GET, [](AsyncWebServerRequest *request) {
        dprintln("test!");
        request->send(LittleFS, "/test.txt", "text/html");
    });
#endif // DEBUG

    server.begin();
    delay(500);

    dprintln("Hotspot available. Configure the WiFi of the module by connecting your PC to the access point `Petfeeder`.");
    dprint("Then, type `");
    dprint(WiFi.softAPIP());
    dprintln("` in your address bar.");

    /* Setting up motor */
    pinMode(Pin1, OUTPUT);
    pinMode(Pin2, OUTPUT);
    pinMode(Pin3, OUTPUT);
    pinMode(Pin4, OUTPUT);

    registerStoredCredentials();
}


void loop() {
    if(wifiMulti.run() == WL_CONNECTED) {
        updateTime();
        getUpdates();
    }
    debugInfo();
    handleFeedEvents();
    delay(GLOBAL_DELAY);
}


void storeCredentials(wifi_credentials_t creds) {
    File wifiConf = LittleFS.open("wifi.txt", "r+");
    if(!wifiConf) {
        dprintln("Cannot store WiFi configuration.");
        return;
    }
    add(wifiConf, creds);
    wifiConf.close();
}


void registerStoredCredentials() {
    wifi_credentials_t creds;
    File wifiConf = LittleFS.open("wifi.txt", "r");
    dprintln("Stored credentials:");
    while(next(wifiConf, creds)) {
        wifiMulti.addAP(creds.ssid.c_str(), creds.pass.c_str());
        dprint("   ");
        dprintln(creds.ssid);
    }
    wifiConf.close();
}


void getUpdates() {
    if(WiFi.status() == WL_CONNECTED) {
        WiFiClientSecure client;
        client.setInsecure();

        if (!client.connect(host, 443)) {
            Serial.println("Connection failed");
            return;
        }

        client.print(String("GET ") + uri + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: ESP8266\r\n" +
               "Connection: close\r\n\r\n");

        while(client.connected()) {
            String line = client.readStringUntil('\n');
            if (line == "\r")
                break;
        }

        String payload = client.readStringUntil('\n');
        DynamicJsonDocument json(2048);
        deserializeJson(json, payload);

        const unsigned int version = json["data"]["__v"].as<unsigned int>();
        if(config.version < version) {
            config.version = version;

            if(config.feed_on)
                free(config.feed_on);
            if(config.fed)
                free(config.fed);

            const JsonArray feedarr = json["data"]["feed_on"];
            unsigned int size = 0;
            for(auto arr __attribute__ ((unused)) : feedarr)
                size++;
            dprint("Size: ");
            dprint(size);
            dprint("   -   ");
            unsigned int *feed_on = (unsigned int*)calloc(size, sizeof(unsigned int));
            for(unsigned int i = 0; i<size; i++) {
                feed_on[i] = feedarr[i].as<int>();
            }

            config.size = size;
            config.feed_on = feed_on;
            config.fed = (bool*)calloc(size, 1);

            // Ignoring old feed requests of the day
            if(timeInited) {
                for(unsigned int i = 0; i<config.size; i++) {
                    config.fed[i] = getMinTimestamp() >= config.feed_on[i];
                }
            }

            feedNowRequest = json["data"]["feed_now"].as<bool>();

            dprint("Udpate: [");
            for(unsigned int i = 0; i<config.size; i++) {
                dprint(config.feed_on[i]);
                dprint(", ");
            }
            dprint(feedNowRequest ? "Feed Now" : "No Feed Now Request");
            dprintln("]");
        }
    }
}


void handleFeedEvents() {
    static bool reinited = false;
    if(!reinited && getHours() < 12) {
        memset(config.fed, 0, config.size * sizeof(bool));
        reinited = true;
    }
    if(reinited && getHours() > 12) {
        reinited = false;
    }

    for(unsigned int i = 0; i < config.size; i++) {
        if(!config.fed[i] && getMinTimestamp() >= config.feed_on[i]) {
            feed();
            config.fed[i] = true;
        }
    }

    if(feedNowRequest) {
        feedNowRequest = false;
        feed();
    }
}


void feed() {
    if(timeInited) {
        dprintln("---- Feeding! ----");
        for(int j=0; j<500; j++) {
            for(int i=0; i<8; i++) {
                digitalWrite(Pin1, pole1[i]);
                delay(1);
                digitalWrite(Pin2, pole2[i]);
                delay(1);
                digitalWrite(Pin3, pole3[i]);
                delay(1);
                digitalWrite(Pin4, pole4[i]);
                delay(1);
            }
        }
    }
}


void displayTime() {
    if(getHours() < 10)
        dprint("0");
    dprint(getHours());
    dprint(":");
    if(getMinutes() < 10)
        dprint("0");
    dprint(getMinutes());
    dprint(":");
    if(getSeconds() < 10)
        dprint("0");
    dprint(getSeconds());
}


void updateTime() {
    static int rrate = REFRESH_RATE;
    if(rrate++ < REFRESH_RATE)
        return;

    if(WiFi.status() == WL_CONNECTED) {
        IPAddress timeServerIP;                 // time.nist.gov NTP server address
        byte packetBuffer[NTP_PACKET_SIZE];    // buffer to hold incoming and outgoing packets
        WiFiUDP udp;                            // A UDP instance to let us send and receive packets over UDP

        udp.begin(localPort);
        WiFi.hostByName(ntpServerName, timeServerIP);
        sendNTPpacket(timeServerIP, packetBuffer, udp);  // send an NTP packet to a time server
        int retries = 0;
        while(!udp.parsePacket()) {
            delay(400);
            retries++;
            if(retries == MAX_RETRIES) {
                eprintln("Failed to update time...");
                return;
            }
        }

        udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
        unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
        unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
        unsigned long secs = (highWord << 16 | lowWord) + fuseau;

        initTimestamp = secs - getBootTime();
        dprintln("Time synced");

        // Ignoring old feed requests of the day
        if(!timeInited && config.version != 0) { // if config has been set before time
            for(unsigned int i = 0; i<config.size; i++) {
                config.fed[i] = getMinTimestamp() >= config.feed_on[i];
            }
        }

        timeInited = true;
        rrate = 0;
    }
}


void debugInfo() {
    dprintln();
    if(WiFi.status() != WL_CONNECTED)
        dprintln("Searching for connection...");
    if(timeInited) {
        dprint("Current time: ");
        displayTime();
        dprintln();
    }
    if(config.size == 0) {
        dprintln("No config found.");
    } else {
        dprint("-");
        for(unsigned int i=0; i<config.size; i++) {
            dprint("--------");
        }
        dprint("\n| ");
        for(unsigned int i=0; i<config.size; i++) {
            const unsigned int ev = config.feed_on[i];
            if(ev/60 < 10)
                dprint("0");
            dprint(ev/60);
            dprint(":");
            if(ev%60 < 10)
                dprint("0");
            dprint(ev%60);
            dprint(" | ");
        }
        dprintln();
        dprint("-");
        for(unsigned int i=0; i<config.size; i++) {
            dprint("--------");
        }
        dprintln();
        dprint("|");
        for(unsigned int i=0; i<config.size; i++) {
            dprint(config.fed[i] ? "   *   |" : "       |");
        }
        dprint("\n-");
        for(unsigned int i=0; i<config.size; i++) {
            dprint("--------");
        }
        dprintln();
    }
}


/*
 * Send an NTP request to the time server at the given address
 */
void sendNTPpacket(IPAddress& address, byte *packetBuffer, WiFiUDP udp) {
    memset(packetBuffer, 0, NTP_PACKET_SIZE);

    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12]  = 49;
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;

    udp.beginPacket(address, 123); //NTP requests are to port 123
    udp.write(packetBuffer, NTP_PACKET_SIZE);
    udp.endPacket();
}


unsigned long getHours() {
    return ((initTimestamp + getBootTime()) % 86400L) / 3600;
}

unsigned long getMinutes() {
    return ((initTimestamp + getBootTime()) % 3600) / 60;
}

unsigned long getSeconds() {
    return (initTimestamp + getBootTime()) % 60;
}

unsigned int getMinTimestamp() {
    return ((initTimestamp + getBootTime()) % 86400L) / 60;
}

unsigned long getBootTime() {
    time_t bootTime;
    time(&bootTime);
    return bootTime;
}

void soft_reset(bool error) {
    if(error)
        eprint("An error occurred. ");
    eprintln("The device will restart.");
    delay(3000);
    ESP.restart();
    while(1) {}
}
