/*

  Udp NTP Client

  Get the time from a Network Time Protocol (NTP) time server
  Demonstrates use of UDP sendPacket and ReceivePacket
  For more on NTP time servers and the messages needed to communicate with them,
  see http://en.wikipedia.org/wiki/Network_Time_Protocol

  created 4 Sep 2010
  by Michael Margolis
  modified 9 Apr 2012
  by Tom Igoe
  updated for the ESP8266 12 Apr 2015
  by Ivan Grokhotkov

  This code is in the public domain.

*/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#ifndef STASSID
#define STASSID "matmat"
#define STAPSK  "tatayoyo"
#endif

#define MAX_RETRIES 10   // 10*200ms = 2s

const char * ssid = STASSID; // your network SSID (name)
const char * pass = STAPSK;  // your network password

const char* ntpServerName = "time.nist.gov";
IPAddress timeServerIP;                 // time.nist.gov NTP server address
unsigned int localPort = 2390;          // local port to listen for UDP packets
const int NTP_PACKET_SIZE = 48;         // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE];    //buffer to hold incoming and outgoing packets
WiFiUDP udp;                            // A UDP instance to let us send and receive packets over UDP

long initTimestamp = 0;
long timestamp;
byte hours, minutes, seconds;
int fuseau = 1*3600;

void sendNTPpacket(IPAddress& address);
void updateTime();
void displayTime();


void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println();

    // We start by connecting to a WiFi network
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    udp.begin(localPort);
    Serial.println(udp.localPort());
}


void loop() {
    updateTime();
    time_t bootTime;
    time(&bootTime);
    if(initTimestamp == 0) {
        initTimestamp = timestamp - bootTime;
    }

    Serial.print((initTimestamp + bootTime) - timestamp);
    Serial.print("  -  ");
    displayTime();
    delay(5000);
}


void displayTime() {
    if(hours < 10)
        Serial.print("0");
    Serial.print(hours);
    Serial.print(":");
    if(minutes < 10)
        Serial.print("0");
    Serial.print(minutes);
    Serial.print(":");
    if(seconds < 10)
        Serial.print("0");
    Serial.println(seconds);
}


void updateTime() {
    WiFi.hostByName(ntpServerName, timeServerIP);

    sendNTPpacket(timeServerIP);  // send an NTP packet to a time server
    int retries = 0;
    while(!udp.parsePacket()) {
        delay(200);
        retries++;
        if(retries == MAX_RETRIES) {
            Serial.println("Failed to update time...");
            return;
        }
    }

    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    unsigned long secs = (highWord << 16 | lowWord) + fuseau;

    timestamp = secs;
    hours = (secs % 86400L) / 3600;
    minutes = (secs % 3600) / 60;
    seconds = secs % 60;
}


/*
 * Send an NTP request to the time server at the given address
 */
void sendNTPpacket(IPAddress& address) {
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
