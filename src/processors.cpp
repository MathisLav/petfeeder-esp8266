/**
 * Web Processors.
 * Replace HTML templates.
 */

#include <ESP8266WiFi.h>
 
String indexStateTemplate(const String &value) {
    if(value == "STATE") {
        if(WiFi.status() == WL_CONNECTED)
            return "Connected! You can use the petfeeder.";
        else
            return "Offline. Please provide valid WiFi credentials.";
    }
    return "";
}
