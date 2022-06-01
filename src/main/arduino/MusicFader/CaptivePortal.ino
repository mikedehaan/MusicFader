#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>

/* Set these to your desired softAP credentials. They are not configurable at runtime */
#ifndef APSSID
#define APSSID "MusicFader"
#define APPSK "12345678"
#endif

const char *softAP_ssid = APSSID;
const char *softAP_password = APPSK;

/* hostname for mDNS. Should work at least on windows. Try http://esp8266.local */
const char *myHostname = "musicFader";

// DNS server
const byte DNS_PORT = 53;
DNSServer dnsServer;

// Web server
ESP8266WebServer server(80);

/* Soft AP network parameters */
IPAddress apIP(172, 217, 28, 1);
IPAddress netMsk(255, 255, 255, 0);

/** Should I connect to WLAN asap? */
boolean connect;

/** Last time I tried to connect to WLAN */
unsigned long lastConnectTry = 0;

/** Current WLAN status */
unsigned int status = WL_IDLE_STATUS;

void captivePortalSetup() {
    delay(1000);
    Serial.begin(115200);
    Serial.println();
    Serial.println("Configuring access point...");
    /* You can remove the password parameter if you want the AP to be open. */
    WiFi.softAPConfig(apIP, apIP, netMsk);
    WiFi.softAP(softAP_ssid, softAP_password);
    delay(500);  // Without delay I've seen the IP address blank
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());

    /* Setup the DNS server redirecting all the domains to the apIP */
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(DNS_PORT, "*", apIP);

    /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
    server.on("/", handleWifi);
    server.on("/generate_204", handleWifi);  // Android captive portal. Maybe not needed. Might be handled by notFound handler.
    server.on("/fwlink", handleWifi);        // Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
    
    server.on("/wifi", handleWifi);
    server.on("/wifisave", handleWifiSave);
    server.onNotFound(handleNotFound);
    server.begin();  // Web server start
    Serial.println("HTTP server started");
    loadCredentials();           // Load WLAN credentials from network
    connect = strlen(eepromSavedData.ssid) > 0;  // Request WLAN connect if there is a SSID
}

void connectWifi() {
    Serial.println("Connecting as wifi client...");
    WiFi.disconnect();
    WiFi.begin(eepromSavedData.ssid, eepromSavedData.password);
    int connRes = WiFi.waitForConnectResult();
    Serial.print("connRes: ");
    Serial.println(connRes);
}

void captivePortalLoop() {

    // Do work:
    // DNS
    dnsServer.processNextRequest();
    // HTTP
    server.handleClient();
}
