#include "Fader.h"
#include <Arduino.h>
#include "Mixer.h"
#include "Global.h"

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

    Serial.println("Fader configured to use mux:");
    Serial.print("S0: ");
    Serial.println(muxS0);
    Serial.print("S1: ");
    Serial.println(muxS1);
    Serial.print("S2: ");
    Serial.println(muxS2);
    Serial.print("S3: ");
    Serial.println(muxS3);

    Serial.print("Fader Channel: ");
    Serial.println(muxChannelNumber);
}

int Fader::getRawValue() {

    // read the value at the SIG pin
    #ifdef USE_MUX
    int val = readMux(muxChannelNumber);
    #else
    int val = analogRead(muxSig);
    #endif

    return val;
}

float Fader::convertToMixerValue(int rawValue) {
    float currentValue = 0;
    currentValue = (float)rawValue;
    currentValue = currentValue / 1024.000;
    currentValue = round(currentValue * 1000) / 1000;

    return currentValue;
}

int Fader::readMux(int channelToRead) {
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
        digitalWrite(controlPin[i], muxChannel[channelToRead][i]);
    }

    // read the value at the SIG pin
    int val = analogRead(muxSig);

    // return the value
    return val;
}

void Fader::run() {

    int val = getRawValue();

    // make sure there's a sizable enough difference to warrant a change
    if (val != lastSentMixerValue && (abs(val - lastSentMixerValue) > 20)) {
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

        float convertedMixerValue = convertToMixerValue(lastSentMixerValue);

        Serial.print("Sending ");
        Serial.print(convertedMixerValue, 5);
        Serial.println(" to the mixer.");

        if (bus == 0) { // Treat as the front of house
            if (channel == 0) { // Treat as the Master Fader
                mixer->setMainMasterVolume(convertedMixerValue);
            } else if (channel > 0 && channel <= 16) {
                mixer->setMainChannelVolume(channel, convertedMixerValue);
            }
        } else if (bus > 0 && bus <= 6) {
            if (channel == 0) { // Treat as the Master Fader
                mixer->setBusMasterVolume(bus, convertedMixerValue);
            } else if (channel > 0 && channel <= 16) {
                mixer->setBusChannelVolume(bus, channel, convertedMixerValue);
            }
        }
        delay(SEND_INTERVAL);
    }
}
