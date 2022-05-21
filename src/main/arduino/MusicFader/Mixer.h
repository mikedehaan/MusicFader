#ifndef MIXER_H_
#define MIXER_H_

#include <WiFiUdp.h>
#include <OSCMessage.h>

#define UDP_PORT 10024
#define HEARTBEAT_INTERVAL 5000 // Send heartbeat every 5 seconds.

class Mixer {

    private:

        WiFiUDP udp;
        IPAddress mixerAddress;
        IPAddress broadcastIp;
        bool findRequestSent;
        unsigned long timeLastMessageSent;
        unsigned long timeLastHeartbeat;        

        void sendOSCMessage(OSCMessage &msg);
        String pad(int channelNumber);
        String noPad(int channelNumber);
        void updateMessageSentTime();

    public:
        bool connected;

        Mixer(WiFiUDP udp, IPAddress broadcastIp);

        void useMixer(IPAddress mixerAddress);

        void run();
        void findMixers();
        void receiveData();
        void sendHeartbeat();
        bool wasFindRequestSent();

        void setMainChannelVolume(int channelNumber, float volumeLevel);
        void setMainChannelMute(int channelNumber, bool mute);
        void setMainMasterVolume(float volumeLevel);
        void setMainMasterMute(bool mute);

        void setBusChannelVolume(int busNumber, int channelNumber, float volumeLevel);
        void setBusMasterVolume(int busNumber, float volumeLevel);
        void setBusMasterMute(int busNumber, bool mute);
};

/**
 * OSC Events
 */
extern void xinfo(OSCMessage &msg, int unknown);

/* 
 * Define the global 'mixer' variable
 * 
 * This needs to be global so we can respond to OSC events.
 * This is a slight drawback to the way the framework was written.
 */
extern Mixer *mixer;

#endif
