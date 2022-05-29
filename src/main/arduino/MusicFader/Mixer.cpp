#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include "Mixer.h"

Mixer::Mixer(WiFiUDP udp, IPAddress broadcastIp) {
    this->udp = udp;
    this->broadcastIp = broadcastIp;
    this->connected = false;
}

void Mixer::findMixers() {
    Serial.print("Sending data...");
    Serial.println();
    
    OSCMessage msg("/xinfo");
    udp.beginPacket(broadcastIp, UDP_PORT);
    msg.send(udp);
    udp.endPacket();
    msg.empty();
    
    Serial.print("Data sent...");
    Serial.println();  

    findRequestSent = true;
}

void Mixer::useMixer(IPAddress mixerAddress) {
    this->mixerAddress = mixerAddress;
}

void Mixer::sendHeartbeat() {
  Serial.print("Heartbeat sent...");
  Serial.println();
  
  OSCMessage msg("/xinfo");
  sendOSCMessage(msg);
  delay(200);
}

bool Mixer::wasFindRequestSent() {
    return findRequestSent;
}

void Mixer::setMainChannelVolume(int channelNumber, float volumeLevel) {
    String key("/ch/" + pad(channelNumber) + "/mix/fader");
    OSCMessage msg(key.c_str());
    msg.add(volumeLevel);
    sendOSCMessage(msg);
    updateMessageSentTime();
}

void Mixer::setMainChannelMute(int channelNumber, bool mute) {
    String key("/ch/" + pad(channelNumber) + "/mix/on");
    OSCMessage msg(key.c_str());
    msg.add((mute ? 0 : 1));
    sendOSCMessage(msg);
    updateMessageSentTime();
}

void Mixer::setMainMasterVolume(float volumeLevel) {
    OSCMessage msg("/lr/mix/fader"); // Main FOH LR Fader
    msg.add(volumeLevel);
    sendOSCMessage(msg);
    updateMessageSentTime();
}

void Mixer::setMainMasterMute(bool mute) {
    String key("/lr/mix/on");
    OSCMessage msg(key.c_str());
    msg.add((mute ? 0 : 1));
    sendOSCMessage(msg);
    updateMessageSentTime();
}

void Mixer::setBusChannelVolume(int busNumber, int channelNumber, float volumeLevel) {
    String key("/ch/" + pad(channelNumber) + "/mix/" + pad(busNumber) + "/level");
    OSCMessage msg(key.c_str());
    msg.add(volumeLevel);
    sendOSCMessage(msg);
    updateMessageSentTime();
}

void Mixer::setBusMasterVolume(int busNumber, float volumeLevel) {
    String key("/bus/" + noPad(busNumber) + "/mix/fader");
    OSCMessage msg(key.c_str());
    msg.add(volumeLevel);
    sendOSCMessage(msg);
    updateMessageSentTime();
}

void Mixer::setBusMasterMute(int busNumber, bool mute) {
    String key("/bus/" + noPad(busNumber) + "/mix/on");
    OSCMessage msg(key.c_str());
    msg.add((mute ? 0 : 1));
    sendOSCMessage(msg);
    updateMessageSentTime();
}

void Mixer::receiveData() {
    OSCBundle bundleIN;
    int size;

    if((size = this->udp.parsePacket())>0) {
        Serial.print("Data received...");
        Serial.println();

        while(size--) {
            bundleIN.fill(this->udp.read());
        }

        //if(!bundleIN.hasError()) { // for some reason, the message is flagged as bad, but parses perfectly anyway.
        bundleIN.route("/xinfo", xinfo);
    }
}

void Mixer::sendOSCMessage(OSCMessage &msg) {
    udp.beginPacket(mixerAddress, UDP_PORT);
    msg.send(udp);
    udp.endPacket();
    msg.empty();  
}

String Mixer::pad(int channelNumber) {
  String key;
  if (channelNumber < 10) {
    key += "0";
  }

  key += channelNumber;

  return key;
}

String Mixer::noPad(int channelNumber) {
  String key;
  key += channelNumber;
  return key;
}

void Mixer::updateMessageSentTime() {
    timeLastMessageSent = millis();
    timeLastHeartbeat = millis();  
}

void Mixer::run() {
    // If a message hasn't been sent in a while, send a heartbeat to keep the connection active.
    if ((unsigned long)(millis() - timeLastHeartbeat) > HEARTBEAT_INTERVAL) {
        timeLastHeartbeat = millis();
        sendHeartbeat();
    }
}

void xinfo(OSCMessage &msg, int unknown) {
    char address[50];
    char mixerName[50];
    char model[50];
    char firmware[50];
    msg.getString(0, address, 50);
    msg.getString(1, mixerName, 50);
    msg.getString(2, model, 50);
    msg.getString(3, firmware, 50);

    Serial.print("[");
    Serial.print(address);
    Serial.print("]");

    Serial.print("[");
    Serial.print(mixerName);
    Serial.print("]");

    Serial.print("[");
    Serial.print(model);
    Serial.print("]");

    Serial.print("[");
    Serial.print(firmware);
    Serial.print("]");
    Serial.println("");

    IPAddress mixerAddress;
    mixerAddress.fromString(address);

    mixer->useMixer(mixerAddress);
    mixer->connected = true;
}

/*
 * Define the global 'mixer' variable
 *
 * This needs to be global so we can respond to OSC events.
 * This is a slight drawback to the way the framework was written.
 */
Mixer *mixer;
