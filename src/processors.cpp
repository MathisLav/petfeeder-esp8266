/**
 * Web Processors.
 * Replace HTML templates.
 */

#include <ESP8266WiFi.h>
 
String indexStateTemplate(const String &value) {
    int n = WiFi.scanComplete();
    if(value == "STATE") {
        if(WiFi.status() == WL_CONNECTED)
            return "Connected! You can use the petfeeder.";
        else
            return "Offline. Please provide valid WiFi credentials.";
    } else if (value == "SSIDS") {
        String networks = "";
        for (int i = 0; i < n - 1; i++) {
            networks += WiFi.SSID(i) + ",";
        }
        networks += WiFi.SSID(n-1);
        return networks;
    }

    return "";
}