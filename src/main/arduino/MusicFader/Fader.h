#ifndef FADER_H_
#define FADER_H_

#include "Mixer.h"

#define SEND_INTERVAL 10 // Don't send messages any faster than this value in ms

class Fader {

    private:

        //Mux control pins
        int muxS0;
        int muxS1;
        int muxS2;
        int muxS3;

        int bus;
        int channel;

        //Mux in "SIG" pin
        int muxSig;

        int lastSentMixerValue;
        unsigned long timeLastMessageSent;
        bool hasMessageToSend;

        Mixer *mixer;

        int muxChannelNumber = 0;
        int readMux(int channel);

    public:
        Fader(int muxChannelNumber, int muxS0, int muxS1, int muxS2, int muxS3, int muxSig, Mixer *mixer, int bus, int channel);
        int getRawValue();
        float convertToMixerValue(int rawValue);
        void run();
};

#endif
