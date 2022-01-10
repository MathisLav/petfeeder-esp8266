/*
 * @info Petfeeder
 */


#ifndef __PET_FEEDER__
#define __PET_FEEDER__

#include <filelist.h>
#include <WiFiUdp.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266WiFiMulti.h>


#define DEBUG 1
#define ERROR 1

#if DEBUG == 1
#define dprint(x) Serial.print(x)
#define dprintln(x) Serial.println(x)
#else
#define dprint(x)
#define dprintln(x)
#endif
#if ERROR == 1
#define eprint(x) Serial.print(x)
#define eprintln(x) Serial.println(x)
#else
#define eprint(x)
#define eprintln(x)
#endif


#define GLOBAL_DELAY 	10000


/**
 * WiFi Connection stuffs
 */
AsyncWebServer server(80);
ESP8266WiFiMulti wifiMulti;


/**
 * Time update stuffs
 */
#define REFRESH_TIME	60 // 60 seconds. Warning: This value is an ESTIMATION : the actual refresh time will be greater!
#define REFRESH_RATE	(int)(REFRESH_TIME/(GLOBAL_DELAY/1000))
#define MAX_RETRIES		15   // 15*200ms = 3s
const char* ntpServerName = "fr.pool.ntp.org";
const unsigned int localPort = 2390;          // local port to listen for UDP packets
const int NTP_PACKET_SIZE = 48;         // NTP time stamp is in the first 48 bytes of the message
const long fuseau = 1*3600;
unsigned long initTimestamp = 0;


/**
 * Petfeeder config stuffs
 */
typedef struct feed_config {
	unsigned int version = 0;
	unsigned int size = 0;
	unsigned int *feed_on = NULL;
	bool *fed = NULL;
} feed_config_t;

typedef enum _HTTP_STATUS_CODE {
	HTTP_OK = 200,
	HTTP_NOT_FOUND = 404
} HTTP_STATUS_CODE;


const char *host = "petfeeder-website.vercel.app";
const char *uri = "/api/config";
feed_config_t config;


/**
 * Motor stuffs
 */
const int Pin1 = D1; // IN1
const int Pin2 = D2; // IN2  
const int Pin3 = D3; // IN3
const int Pin4 = D4; // IN4

const int pole1[] ={0,0,0,0,0,1,1,1}; // magnet 1
const int pole2[] ={0,0,0,1,1,1,0,0}; // magnet 2
const int pole3[] ={0,1,1,1,0,0,0,0}; // magnet 3
const int pole4[] ={1,1,0,0,0,0,0,1}; // magnet 4


/**
 * Functions
 */
void storeCredentials(wifi_credentials_t creds);
void registerStoredCredentials();
void getUpdates();
void handleFeedEvents();
void feed();
void displayTime();
void updateTime();
void debugInfo();
void sendNTPpacket(IPAddress& address, byte *packetBuffer, WiFiUDP udp);
unsigned long getHours();
unsigned long getMinutes();
unsigned long getSeconds();
unsigned int getMinTimestamp();
unsigned long getBootTime();
void soft_panic();


#endif // __PET_FEEDER__
