#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include "Mixer.h"
#include "Fader.h"

/* Set these to your desired credentials. */
const char *ssid     = ""; //Enter your WIFI ssid
const char *password = ""; //Enter your WIFI password

#define UDP_PORT 10024

#define MUX_S0  D0
#define MUX_S1  D1
#define MUX_S2  D2
#define MUX_S3  D3
#define MUX_SIG A0

const unsigned int inPort = 8888;

Fader *fader1;
Fader *fader2;

void muxSetup();

void oscSetup() {
    pinMode(LED_BUILTIN, OUTPUT);
    delay(1000);
    Serial.begin(115200);
    Serial.println();
    Serial.print("Configuring access point...");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Parse the localIP and change to the broadcast IP
    String ipAddressString;
    ipAddressString += WiFi.localIP().toString();

    Serial.print("NETMASK: ");
    Serial.println(WiFi.subnetMask());

    uint8_t  netAddress1 = WiFi.localIP()[0] & WiFi.subnetMask()[0];
    uint8_t  netAddress2 = WiFi.localIP()[1] & WiFi.subnetMask()[1];
    uint8_t  netAddress3 = WiFi.localIP()[2] & WiFi.subnetMask()[2];
    uint8_t  netAddress4 = WiFi.localIP()[3] & WiFi.subnetMask()[3];

    uint8_t  broadcast1 = netAddress1 | ~WiFi.subnetMask()[0];
    uint8_t  broadcast2 = netAddress2 | ~WiFi.subnetMask()[1];
    uint8_t  broadcast3 = netAddress3 | ~WiFi.subnetMask()[2];
    uint8_t  broadcast4 = netAddress4 | ~WiFi.subnetMask()[3];

    String broadcastString;
    broadcastString += broadcast1;
    broadcastString += ".";
    broadcastString += broadcast2;
    broadcastString += ".";
    broadcastString += broadcast3;
    broadcastString += ".";
    broadcastString += broadcast4;

    IPAddress broadcastIp;
    broadcastIp.fromString(broadcastString);
    Serial.print("Broadcast IP: ");
    Serial.println(broadcastIp);

    // At this point, we're on the network.
    // Now, how do we send data.....
    WiFiUDP UDP;
    UDP.begin(inPort);

    mixer = new Mixer(UDP, broadcastIp);

    // When bus is set to 0, this refers to the MainLR (FOH)
    // Otherwise, bus 1-6 refers to the 6 buses
    int bus = 1;

    // Then channel is set to 0, this refers to the master fader
    // So, bus 0 channel 0 is the MainLR master fader
    // bus 1 channel 0 will be the bus 1 master fader
    // Otherwise, channel can be from 1-16 referring to the 16 channels
    int channel = 1;

    // Still need to determine a pattern for aux
    // /rtn/aux/mix/fader
    // /rtn/aux/mix/on

    fader1 = new Fader(0, MUX_S0, MUX_S1, MUX_S2, MUX_S3, MUX_SIG, mixer, bus, channel);
    //fader2 = new Fader(1, MUX_S0, MUX_S1, MUX_S2, MUX_S3, MUX_SIG, mixer);
}

void setup() {
    oscSetup();
    muxSetup();
}

void loop() {
    if (!mixer->connected) {
        // Broadcast a request for all mixers to respond
        // Only send this once
        if (!mixer->wasFindRequestSent()) {
            mixer->findMixers();
        }
    
        // Listen for a response from the mixer.
        mixer->receiveData();
    } else {
        fader1->run();
        //fader2->run();
        mixer->run();

        mixer->receiveData();
    }
}

void muxSetup() {

    //Mux control pins
    int s0 = MUX_S0;
    int s1 = MUX_S1;
    int s2 = MUX_S2;
    int s3 = MUX_S3;

    pinMode(s0, OUTPUT);
    pinMode(s1, OUTPUT);
    pinMode(s2, OUTPUT);
    pinMode(s3, OUTPUT);

    digitalWrite(s0, LOW);
    digitalWrite(s1, LOW);
    digitalWrite(s2, LOW);
    digitalWrite(s3, LOW);
}
