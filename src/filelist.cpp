/**
 * File list library.
 * Allow to handle lists stored in a file.
 * 
 * The file must have this template :
 * ssid1 pass1
 * ssid2 pass2
 * [...]
 * ssidn passn
 */

#include <filelist.h>
#include <string>
#include <LittleFS.h>


void startFile(File list) {
    list.seek(0, SeekSet);
}


bool next(File list, wifi_credentials_t &creds) {
    if(!list.available())
        return false;
    creds.ssid = list.readStringUntil(' ');
    creds.pass = list.readStringUntil('\n');
    return true;
}


bool add(File list, wifi_credentials_t creds) {
    wifi_credentials_t cur;
    startFile(list);
    while(next(list, cur)) {
        if(!creds.ssid.compareTo(cur.ssid) && !creds.pass.compareTo(cur.pass)) {
            return false;
        }
    }
    list.print(creds.ssid);
    list.print(" ");
    list.print(creds.pass);
    list.print("\n");
    return true;
}
