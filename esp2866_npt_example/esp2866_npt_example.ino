/*

  Udp NTP Client

  Get the time from a Network Time Protocol (NTP) time server
  Demonstrates use of UDP sendPacket and ReceivePacket
  For more on NTP time servers and the messages needed to communicate with them,
  see http://en.wikipedia.org/wiki/Network_Time_Protocol

  created 4 Sep 2010
  by Michael Margolis
  modified 9 Apr 2012
  by Tom Igoe
  updated for the ESP8266 12 Apr 2015
  by Ivan Grokhotkov

  modified for Heltec WiFi Kit-8 27 March 2023
  by Mark Rosenau

*/

#include "my_wifi.hpp"  // *ssid, *pass

#ifdef WIFI_Kit_8
const char* boardType = "Wifi Kit8 defined.";
#else
const char* boardType = "not WifiKit8";
#endif

#define LINE_1  0
#define LINE_2  8
#define LINE_3  16
#define LINE_4  24

#define TIME_LINE 12
#define DOT_LINE  20

#define FONT_WIDTH 8

#include "heltec.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

//const char * ssid = STASSID; // your network SSID (name)
//const char * pass = STAPSK;  // your network password

/* Don't hardwire the IP address or we won't get the benefits of the pool.
    Lookup the IP address for the host name instead */
//IPAddress timeServerIP(129, 6, 15, 28); // time.nist.gov NTP server
//const char* ntpServerName = "time.nist.gov";
//IPAddress timeServerIP(10,21,8,31); // austxdc1.hi.hubbell-ad.com NTP server address
//const char* ntpServerName = "austxdc1.hi.hubbell-ad.com";

#define  LOCAL_OFFSET 5 // CDT = 5, CST = 6

typedef struct WifiInfo
{
    char* ssid;
    char* pass;
    char* ntpServerName;
} wifi_info_st;

wifi_info_st KnownId[] =
{
    {"01234567890123456789012345678901", "0123456789012345678901234567890123456789012345678901234567890123", "time.nist.gov"},
    {"hubbellwifi",     "Hubbell1905Incorporated", "austxdc1.hi.hubbell-ad.com"},
};

unsigned int localPort = 2390;      // local port to listen for UDP packets
const int NTP_PACKET_SIZE = 48;     // NTP time stamp is in the first 48 bytes of the message
byte    packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
WiFiUDP udp;    // A UDP instance to let us send and receive packets over UDP

#define numWifiEntries sizeof(KnownId)/sizeof(wifi_info_st)

IPAddress localIP, timeServerIP;  //
int     wifiInstance;

void setup()
{
    bool result = false;
    Serial.begin(115200);
    Serial.println();
    Serial.println();
    Serial.println(boardType);
    // Serial.println(cpuName);
    delay(1000);

    //pinMode(LED,OUTPUT);
    Heltec.begin(true /*DisplayEnable Enable*/, true /*Serial Enable*/);
    Heltec.display->clear();

    Serial.print("display width: ");
    Serial.println(Heltec.display->width());
    Serial.print("display height: ");
    Serial.println(Heltec.display->height());

    // Serial.printf("KnownId ssid 0 is %s, size %d\n", KnownId[0].ssid, strlen(KnownId[0].ssid));
    // Serial.printf("KnownId pass 1 is %s, size %d\n", KnownId[0].pass, strlen(KnownId[0].pass));
    Serial.printf("assigning to actual...\n");
    KnownId[0].ssid = (char*) ssid;
    KnownId[0].pass = (char*) password;
    // Serial.printf("KnownId ssid 0 is %s, size %d\n", KnownId[0].ssid, strlen(KnownId[0].ssid));
    // Serial.printf("KnownId pass 0 is %s, size %d\n", KnownId[0].pass, strlen(KnownId[0].pass));

    // there is no point in continuing if the connection fails:
    while(1)
    {
        // walk through known networks:
        for (int i = 0; i < sizeof(KnownId)/sizeof(wifi_info_st); i++)
        {
            // We start by connecting to a WiFi network
            if (wifiConnect(i))
            {
                //connected
                Serial.println("Starting UDP");
                udp.begin(localPort);
                // Serial.print("Local port: ");
                // Serial.println(udp.localPort());

                delay(500);
                return; // exits setup()
            }
        }
        // no network found
        Serial.println("no network found.");

        Heltec.display->clear();
        // Heltec.display->display();
        Heltec.display->drawString(0, LINE_1, "no network found");
        Heltec.display->display();
        for (int i=0 ; i< 20; i++)
        {
            Heltec.display->drawString(i*3, LINE_2, ".");
            Heltec.display->display();
            delay(100);
        }
    }

}

void loop()
{
    int cb, offset;
    char displayString[20];
    int count = 0;

    Heltec.display->clear();
    //get a random server from the pool
    WiFi.hostByName(KnownId[wifiInstance].ntpServerName, timeServerIP);
    Serial.print("time server IP: ");
    Serial.println(timeServerIP);

    sendNTPpacket(timeServerIP); // send an NTP packet to a time server
    Serial.print("packet sent");
    Heltec.display->drawString(0, LINE_1, "waiting on packet");

    // wait up to 500mS for a reply:
    while(!(cb=udp.parsePacket()) && count++ < 20)
    {
        Serial.print(".");
        Heltec.display->drawString(5 * sizeof("waiting on packet ") + count * 2, LINE_1, ".");
        Heltec.display->display();
        // wait to see if a reply is available
        delay(100);
    }
    // if there is a reply, parse it:
    if (cb)
    {
        Serial.println();

        Serial.print("packet received, length=");
        Serial.println(cb);
        // We've received a packet, read the data from it
        udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

        // the timestamp starts at byte 40 of the received packet and is four bytes,
        // or two words, long. First, extract the two words:

        unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
        unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
        // combine the four bytes (two words) into a long integer
        // this is NTP time (seconds since Jan 1 1900):
        unsigned long secsSince1900 = highWord << 16 | lowWord;
        Serial.print("Seconds since Jan 1 1900 = ");
        Serial.println(secsSince1900);

        // now convert NTP time into everyday time:
        Serial.print("Unix time = ");
        // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
        const unsigned long seventyYears = 2208988800UL;
        // subtract seventy years:
        unsigned long epoch = secsSince1900 - seventyYears;
        // print Unix time:
        Serial.println(epoch);

        int hour = (epoch  % 86400L) / 3600;    //(86400 equals secs per day)
        int localhr = ((epoch  % 86400L) / 3600)-LOCAL_OFFSET;  // adjust for local offset from UTC
        if (localhr<0) localhr +=24;
        int minutes = ((epoch % 3600) / 60);
        int seconds = epoch % 60;

        // print the hour, minute and second:
        Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
        Serial.print(hour); // print the hour (86400 equals secs per day)
        Serial.print(':');
        if (minutes < 10)
        {
            // In the first 10 minutes of each hour, we'll want a leading '0'
            Serial.print('0');
        }
        Serial.print(minutes); // print the minute (3600 equals secs per minute)
        Serial.print(':');
        if (seconds < 10)
        {
            // In the first 10 seconds of each minute, we'll want a leading '0'
            Serial.print('0');
        }
        Serial.println(seconds); // print the second

        // print the local time:
        Serial.print("The local time is ");       // UTC is the time at Greenwich Meridian (GMT)
        Serial.print(localhr); // print the hour
        Serial.print(':');
        if (minutes < 10)
        {
            // In the first 10 minutes of each hour, we'll want a leading '0'
            Serial.print('0');
        }
        Serial.print(minutes); // print the minute
        Serial.print(':');
        if (seconds < 10)
        {
            // In the first 10 seconds of each minute, we'll want a leading '0'
            Serial.print('0');
        }
        Serial.println(seconds); // print the second

        //display local time on OLED:
        Heltec.display->clear();
        Heltec.display->drawString(0, LINE_1, "local time is:");
        itoa(localhr,displayString,10);

//        Heltec.display->setTextSize(2);
        Heltec.display->setFont(ArialMT_Plain_16);

        if (localhr < 10)
        {
            Heltec.display->drawString(0 * FONT_WIDTH,TIME_LINE,"0");
            offset = 1;
        }
        else
            offset = 0;
        Heltec.display->drawString((0/*digits*/ + offset/*leading spaces*/) * FONT_WIDTH/*pixels*/, TIME_LINE,displayString);

        Heltec.display->drawString(3 * FONT_WIDTH, TIME_LINE,":");

        itoa(minutes,displayString,10);
        if (minutes < 10)
        {
            Heltec.display->drawString(4 * FONT_WIDTH, TIME_LINE,"0");
            offset = 1;
        }
        else
            offset = 0;
        Heltec.display->drawString((4/*digits*/ + offset/*leading spaces*/) * FONT_WIDTH /*pixels*/, TIME_LINE,displayString);

        Heltec.display->drawString(7 * FONT_WIDTH, TIME_LINE,":");

        itoa(seconds,displayString,10);
        if (seconds < 10)
        {
            Heltec.display->drawString(8 * FONT_WIDTH, TIME_LINE,"0");
            offset = 1;
        }
        else
            offset = 0;
        Heltec.display->drawString((8/*digits*/ + offset/*leading spaces*/) * FONT_WIDTH/*pixels*/, TIME_LINE,displayString);
        Heltec.display->display();

//        Heltec.display->setTextSize(1);
        Heltec.display->setFont(ArialMT_Plain_10);

    }
    else // connection failed:
    {
        Serial.println("connection lost!");

        if (!wifiConnect(wifiInstance))
        {
            Serial.println("reconnect failed");
            // re-init everything
            setup();
        }
    }
    Heltec.display->setFont(ArialMT_Plain_10);
    // wait 30 seconds before asking for the time again
    for (int i =0; i<20; i++)
    {
        delay(1500);
        Heltec.display->drawString(i*5, DOT_LINE, ".");
        Heltec.display->display();
    }
}

bool wifiConnect(int inst)
{
    char  ipString[4];
    int   offset;

//    Serial.printf( "instance is %d\n", inst);
//    Serial.printf( "pointer is 0x%8X, string is %s\n", &ssid, ssid);

    Serial.print("trying connection to ");
    Serial.print(KnownId[inst].ssid);
    Heltec.display->clear();
    Heltec.display->drawString(0, LINE_1, "connecting...");
    Heltec.display->display();
    WiFi.mode(WIFI_STA);
    WiFi.begin(KnownId[inst].ssid, KnownId[inst].pass);
    // give it a second to connect
    delay(1000);

    // try up to 10x to connect:
    for (int j=0; j<10; j++)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            // save the instance that worked.
            wifiInstance = inst;
            localIP = WiFi.localIP();
            // display the setup:
            Serial.println();
            Serial.println("WiFi connected");
            Serial.print("IP address: ");
            Serial.println(localIP);

            // print OK at the end of the line:
            Heltec.display->drawString(5*sizeof("connecting..."), LINE_1, " OK ");
            Heltec.display->drawString(0,LINE_2,"IP address: ");
            for (int i=0; i < 4; i++)
            {
                itoa(localIP[i],ipString,10);
                offset = 3-(strlen(ipString));

                Heltec.display->drawString((i * 4/*digits*/ + offset/*leading spaces*/) * 5/*pixels*/, TIME_LINE, ipString);
                if (i !=3)
                    Heltec.display->drawString(((i * 4) + 3) * 5,TIME_LINE,".");
            }
            Heltec.display->display();
            delay(500);
            // since we have a connection, we're done.
            return(true);
        }
        Serial.print(".");
        delay(500);
    }
    Serial.println();
    return(false);
} // end WifiConnect()


// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress& address)
{
    Serial.println("sending NTP packet.");
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12]  = 49;
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;

    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    udp.beginPacket(address, 123); //NTP requests are to port 123
    udp.write(packetBuffer, NTP_PACKET_SIZE);
    udp.endPacket();
}
