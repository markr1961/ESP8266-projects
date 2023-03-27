#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include "C:\git-projects\my_pw.h"
// char ssid[]="SARAVANA-ACT";  // replace with your ssid & pass
// char pass[]="str5412stk4141";

ESP8266WiFiMulti wifiMulti;

void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.print("Connecting to: ");
    Serial.println(ssid);
    delay(500);
    WiFi.disconnect();
    WiFi.begin(ssid, pass);
    // alternatively to search for multiple connections:
    // add any potential WiFi SSID's and passwords that your project may encounter
    // wifiMulti.addAP("Enter_the_first_Wifi_name_here", "Enter_the_first_Wifi_pass_here");
    // wifiMulti.addAP("Enter_the_second_Wifi_name_here", "Enter_the_second_Wifi_pass_here");
    // wifiMulti.addAP("Enter_the_..._Wifi_name_here", "...");
    // while(wifiMulti.run() != WL_CONNECTED)

    // a loop that will print dots every half a second while the WiFi is NOT connected (WL_CONNECTED is like an attribute of the Wifi library kinda)
    while(WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("SSID ");
    Serial.print(WiFi.SSID());
    Serial.println(" successfully connected.");

    Serial.print("IP Address allotted to NodeMcu ESP: ");
    Serial.println(WiFi.localIP());

    Serial.print("MAC Address of ESP: ");
    Serial.println(WiFi.macAddress());
    Serial.println("WiFi Diag:");
    WiFi.printDiag(Serial);
}

void loop()
{

}
