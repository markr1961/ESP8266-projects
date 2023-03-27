/*
    This sketch demonstrates how to scan WiFi networks.
    The API is almost the same as with the WiFi Shield library,
    the most obvious difference being the different file you need to include:
*/
#include "ESP8266WiFi.h"

#include <U8g2lib.h>

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 16, /* clock=*/ 5, /* data=*/ 4);

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

void printMacAddress() {
  // the MAC address of your Wifi shield
  byte mac[6];

  // print your MAC address:
  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);

  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
  u8g2.drawStr(0,10,"MAC:"); // write something to the internal memory

  u8g2.write(mac,6);
  u8g2.sendBuffer();          // transfer internal memory to the display

  // Wait a bit after MAC is displayed
  delay(5000);
}

void setup() {
  Serial.begin(115200);

  u8g2.begin();

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println("");
  printMacAddress();
  
  Serial.println("Setup done");
}

void loop() {
  char m_str[100];

  Serial.println("scan start");
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
  u8g2.drawStr(0,10,"scan start");    // write something to the internal memory
  u8g2.sendBuffer();          // transfer internal memory to the display

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  
  u8g2.clearBuffer();                   // clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr);   // choose a suitable font
  u8g2.drawStr(0,10,"scan done, ");	// write something to the internal memory
  
  if (n == 0) {
    Serial.println("no networks found");
    u8g2.drawStr(0,20,"no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    u8g2.drawStr(0,10,);  // write something to the internal memory
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
//      sprintf(m_str, "SSID %s\n", WiFi.SSID(i));    /* convert m to a string with two digits */
//      u8g2.drawStr(0,(i*10)+20,m_str);  // write the SSID
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");

  u8g2.sendBuffer();          // transfer internal memory to the display

  // Wait a bit before scanning again
  delay(5000);
}
