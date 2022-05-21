#include "Fader.h"
#include <Arduino.h>
#include "Mixer.h"

Fader::Fader(int muxChannelNumber, int muxS0, int muxS1, int muxS2, int muxS3, int muxSig, Mixer *mixer, int bus, int channel) {
    this->muxChannelNumber = muxChannelNumber;
    this->muxS0   = muxS0;
    this->muxS1   = muxS1;
    this->muxS2   = muxS2;
    this->muxS3   = muxS3;
    this->muxSig  = muxSig;
    this->mixer   = mixer;
    this->bus     = bus;
    this->channel = channel;
}

float Fader::getMixerValue() {
    int muxValue = readMux(muxChannelNumber);

    float currentValue = 0;
    currentValue = muxValue;
    currentValue = currentValue / 1024.0;
    currentValue = round(currentValue * 100) / 100;

    return currentValue;
}

int Fader::readMux(int channel) {
    int controlPin[] = {muxS0, muxS1, muxS2, muxS3};

    int muxChannel[16][4]={
        {0,0,0,0}, //channel 0
        {1,0,0,0}, //channel 1
        {0,1,0,0}, //channel 2
        {1,1,0,0}, //channel 3
        {0,0,1,0}, //channel 4
        {1,0,1,0}, //channel 5
        {0,1,1,0}, //channel 6
        {1,1,1,0}, //channel 7
        {0,0,0,1}, //channel 8
        {1,0,0,1}, //channel 9
        {0,1,0,1}, //channel 10
        {1,1,0,1}, //channel 11
        {0,0,1,1}, //channel 12
        {1,0,1,1}, //channel 13
        {0,1,1,1}, //channel 14
        {1,1,1,1}  //channel 15
    };

    // loop through the 4 sig
    for(int i = 0; i < 4; i ++){
        digitalWrite(controlPin[i], muxChannel[channel][i]);
    }

    // read the value at the SIG pin
    int val = analogRead(muxSig);

    // return the value
    return val;
}

void Fader::run() {

    float val = getMixerValue();

    // make sure there's a sizable enough difference to warrant a change
    if (val != lastSentMixerValue && (abs(val - lastSentMixerValue) > 0.02)) {
        Serial.print("New fader value...");
        Serial.print(val);
        Serial.println();
        lastSentMixerValue = val;
        hasMessageToSend = true;
    }

    // If we have a message to send and enough time has elapsed, send the data
    if(hasMessageToSend && ((unsigned long)(millis() - timeLastMessageSent) > SEND_INTERVAL)){
        timeLastMessageSent = millis();
        hasMessageToSend = false;
        Serial.print("Sending ");
        Serial.print(lastSentMixerValue);
        Serial.println(" to the mixer.");

        if (bus == 0) { // Treat as the front of house
            if (channel == 0) { // Treat as the Master Fader
                mixer->setMainMasterVolume(lastSentMixerValue);
            } else if (channel > 0 && channel <= 16) {
                mixer->setMainChannelVolume(channel, lastSentMixerValue);
            }
        } else if (bus > 0 && bus <= 6) {
            if (channel == 0) { // Treat as the Master Fader
                mixer->setBusMasterVolume(bus, lastSentMixerValue);
            } else if (channel > 0 && channel <= 16) {
                mixer->setBusChannelVolume(bus, channel, lastSentMixerValue);
            }
        }
        delay(SEND_INTERVAL);
    }
}
