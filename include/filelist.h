/*
 * @info Unique list stuffs.
 */


#ifndef __UNIQUE_LIST__
#define __UNIQUE_LIST__

#include <string>
#include <LittleFS.h>


/**
 * Struct
 */
typedef struct wifi_credentials {
    String ssid;
    String pass;
} wifi_credentials_t;


/**
 * Functions
 */
void startFile(File list);
bool next(File list, wifi_credentials_t &creds);
bool add(File list, wifi_credentials_t creds);



#endif // __UNIQUE_LIST__
