// copied pretty much verbatim from https://www.instructables.com/Pull-and-Ambiently-Display-Data-From-Any-Website-O/
//
// yeah, thingspeak.com is now a MathLab only tool and requires a university log-in.
// meaning the core http capability is gone. Time to start fresh ...
//
// The http libraries have changed ... _a_lot _ so as written the example didn't work.
//
// fixed the basic library issues, but it still fundamentally doesn't work,
// maybe because of the host and/or api call to it. Or the key is no longer valid. 
// Who TF knows. Parking this for another day.... - MAR 01/30/2023.

// Loading in the libraries that are used.
#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
//#include <ESP8266WiFiMulti.h>
// invoking the ESP8266WiFiMulti library.
//ESP8266WiFiMulti wifiMulti;
  #include <ESP8266HTTPClient.h>
#endif  // ESP8266

#include "WiFiClient.h"

#include "C:\git-projects\my_pw.h"
// char ssid[]="SARAVANA-ACT";  // replace with your ssid & pass
// char pass[]="str5412stk4141";

// naming the output pins on the NodeMcu to be used for the 7 segment display. Letters a through g correspond (by convention)
// to each of the 8 segments of the number display.
const int a = D1;
const int b = D2;
const int c = D3;
const int d = D4;
const int e = D6;
const int f = D7;
const int g = D8;

// This is the start of the setup loop. Things that are within the setup loop run just once
void setup()
{
  pinMode(LED_BUILTIN,OUTPUT);
	digitalWrite(LED_BUILTIN,HIGH);
  // To view the serial monitor, go ctrl+shift+L, or Tools->Serial Monitor
  Serial.begin(115200);
  // wait half a second
  delay(500);
  // print Connecting... to the serial monitor
  Serial.print("Connecting...");
  delay(500);

  // initializing each of the digital pins as outputs, remembering that each letter corresponds to a pin as defined at the top of this code
  // pinMode(a, OUTPUT);
  // pinMode(b, OUTPUT);
  // pinMode(c, OUTPUT);
  // pinMode(d, OUTPUT);
  // pinMode(e, OUTPUT);
  // pinMode(f, OUTPUT);
  // pinMode(g, OUTPUT);

  // disconnect any current WiFi connection... for good measure!
  WiFi.disconnect();

  WiFi.begin(ssid, pass);
  // a loop that will print dots every half a second while the WiFi is NOT connected (WL_CONNECTED is like an attribute of the Wifi library kinda)
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println(".");
  }
  // the end of the loop. This will be reached when WL_CONNECTED becomes true, i.e. when the board has established a connection with the internet
  // in the example, LOTS of info was printed out here including signal strength and IP

  // Print out the Wifi name that was connected to, and give u a little validation
  Serial.print("WiFi SSID name:");
  Serial.print (WiFi.SSID());
  Serial.println(". Successfully connected!!!");

  uint32_t chip_id = system_get_chip_id();
  Serial.printf("chip ID returns: %d.", chip_id);

} // This is the end of the setup loop

WiFiClient client;
// shortening HTTPClient to http...
HTTPClient http;

// This is the loop that DOES the stuff. It uses a function that is defined outside of this loop, at the very bottom of the code, for displaying on the 7 segment
// This loop goes forever and ever and ever and.....
void loop()
{
  // This is my thingspeak API... U gotta put YOUR proper stuff in here, but the only thing that'll change is the 16 digit api key
  if (http.begin(client, "api.thingspeak.com/apps/thinghttp/send_request?api_key=AJM48KKS5IVA3P8M"))
    // Do a GET (request)!
  {
    int httpCode = http.GET();

    // httpCode will be negative if there's a problem... it'll be displayed (see the "else" statement below) if there's an error
    if(httpCode > 0)
    {
      // If a file was found at the server (i.e. it's all g)...
      if(httpCode == HTTP_CODE_OK)
      {

        // THIS "youvalue" is the string from the thinghttp URL, IT IS THE VARIABLE (As a string, i.e. text) for you to use as you see fit. The rest of this is just
        // about displaying this value as a number, on a specific display
        String yourvalue = http.getString();

        // printing this value to the serial monitor
        Serial.println(yourvalue);
        // taking the percent symbol off the string, by taking a substring from the first to the second-from-last character. To get rid of a $ sign for example,
        // you could enter 1 instead of 0 and yourvalue.length() for that other argument.
        String trimmed_value = yourvalue.substring(0, yourvalue.length() - 1);
        //convert this string (text) to an integer. You may NOT want an integer if it isn't a whole number
        int num_payload = trimmed_value.toInt();
        //initializing dig_1 and dig_2 of a 2 digit percentage before the if loop. This is because we want to have the variables changed outside of the scope of the
        //if statement, so it can be used as an input into the 7 segment display function
        int dig_1;
        int dig_2;

        //if the number is less than 10 (i.e. single digit)
        if(num_payload < 10)
        {
          //the first digit is 0
          dig_1 = 0;
          //the second digit is the number
          dig_2 = num_payload;
        }
        else
        {
          // if the number is double digits...
          // the second (ones) number is the number "modulo" 10. This means to divide a numer and keep only the remainder (if you're my friend, this also shows up
          // lots of times in the calculating-thing-day-of-the-week-from-any-date formula!!! Awe!)
          dig_2 = num_payload % 10;
          // for the first number (the tens), you divide by 10 to move the digit you're interested in to the "ones" spot, and do the same as above.
          // in this way, moving the desired digit to the ones spot and using the modulo operator can be used to extract whatever number you want
          dig_1 = (num_payload / 10) % 10;
        }

        // display the number, and each digit to the serial monitor, to check that everything worked!
        Serial.println(num_payload);
        Serial.println(dig_1);
        Serial.println(dig_2);

        // // display digit 1, then 2, then nothing 3 with 2, then 3 seconds in between each display. Scroll to the bottom to see how numbers are displayed,
        // // through the function "sevenseg()". Note that this repeats 3 times, by just copy pasting the code, and then at the end of the loop it pauses 10 seconds
        // sevenseg(dig_1);
        // delay(2000);
        // sevenseg(dig_2);
        // delay(3000);
        // // note that the number 69 is a funny joke #, and is just a # that's impossible to have for a single digit. Again, see the function below
        // sevenseg(69);
        // delay(3000);
      }
    }
    else
    {
      // Seeee, this is the "else" statement mentioned above, which lets you know if the HTTP get request failed, and prints out the error code
      Serial.printf("[HTTP] GET... failed, error: %s", http.errorToString(httpCode).c_str());
    }
    // If it's not already clear, I don't really understand HTTP stuff, but I guess this is like a formal goodbye, or as I like to think of it, a bye-bye handshake
    // between u and the internet world. It's kind of like closing a file? Who cares!
    http.end();

    // Wait 10 seconds and repeat this loop over again!
    for (int n =0; n < 5; n++)
    { // slow blink means it's working.
      delay(1000);
      digitalWrite(LED_BUILTIN,LOW);
      delay(1000);
      digitalWrite(LED_BUILTIN,HIGH);
    }
  }
  else  // connection failed, fast blink
  {
    Serial.printf("[HTTP] connection failed\n");
    delay(250);
    digitalWrite(LED_BUILTIN,LOW);
    delay(250);
    digitalWrite(LED_BUILTIN,HIGH);
  }

}
// THIS is the end of the "doing stuff loop... it will repeat forever and ever as long as there's power and there aren't any errors


// This is a function that is used above, and takes integers as an input... 0 through 9 to be specific, and then 69 if you want it blank.
// you can look at a chart online to see how each of the letters (corresponding to each of theoutput pins) being high and low will result in each of
// the numbers. For example 0 just has segment g, the mid-line, off. 1 has the two, right vertical lines on and the rest off. Very straightforward!
void sevenseg(int x)
{
  switch (x)
  {
  case 0:
  //display 0
  digitalWrite(a, HIGH);
  digitalWrite(b, HIGH);
  digitalWrite(c, HIGH);
  digitalWrite(d, HIGH);
  digitalWrite(e, HIGH);
  digitalWrite(f, HIGH);
  digitalWrite(g, LOW);
  break;
  case 1:
    // display 1
    digitalWrite(a, LOW);
    digitalWrite(b, HIGH);
    digitalWrite(c, HIGH);
    digitalWrite(d, LOW);
    digitalWrite(e, LOW);
    digitalWrite(f, LOW);
    digitalWrite(g, LOW);
    break;
  case 2:
    // display 2
    digitalWrite(a, HIGH);
    digitalWrite(b, HIGH);
    digitalWrite(c, LOW);
    digitalWrite(d, HIGH);
    digitalWrite(e, HIGH);
    digitalWrite(f, LOW);
    digitalWrite(g, HIGH);
    break;
  case 3:
    // display 3
    digitalWrite(a, HIGH);
    digitalWrite(b, HIGH);
    digitalWrite(c, HIGH);
    digitalWrite(d, HIGH);
    digitalWrite(e, LOW);
    digitalWrite(f, LOW);
    digitalWrite(g, HIGH);
    break;
  case 4:
    // display 4
    digitalWrite(a, LOW);
    digitalWrite(b, HIGH);
    digitalWrite(c, HIGH);
    digitalWrite(d, LOW);
    digitalWrite(e, LOW);
    digitalWrite(f, HIGH);
    digitalWrite(g, HIGH);
    break;
  case 5:
    // display 5
    digitalWrite(a, HIGH);
    digitalWrite(b, LOW);
    digitalWrite(c, HIGH);
    digitalWrite(d, HIGH);
    digitalWrite(e, LOW);
    digitalWrite(f, HIGH);
    digitalWrite(g, HIGH);
    break;
  case 6:
    // display 6
    digitalWrite(a, HIGH);
    digitalWrite(b, LOW);
    digitalWrite(c, HIGH);
    digitalWrite(d, HIGH);
    digitalWrite(e, HIGH);
    digitalWrite(f, HIGH);
    digitalWrite(g, HIGH);
    break;
  case 7:
    // display 7
    digitalWrite(a, HIGH);
    digitalWrite(b, HIGH);
    digitalWrite(c, HIGH);
    digitalWrite(d, LOW);
    digitalWrite(e, LOW);
    digitalWrite(f, LOW);
    digitalWrite(g, LOW);
    break;
  case 8:
    // display 8
    digitalWrite(a, HIGH);
    digitalWrite(b, HIGH);
    digitalWrite(c, HIGH);
    digitalWrite(d, HIGH);
    digitalWrite(e, HIGH);
    digitalWrite(f, HIGH);
    digitalWrite(g, HIGH);
    break;
  case 9:
    // display 9
    digitalWrite(a, HIGH);
    digitalWrite(b, HIGH);
    digitalWrite(c, HIGH);
    digitalWrite(d, HIGH);
    digitalWrite(e, LOW);
    digitalWrite(f, HIGH);
    digitalWrite(g, HIGH);
    break;
  default:
    Serial.print("WTF?");
    Serial.print(x);
    Serial.println(".");
    // fall thru...
  case 69:
    // display off
    digitalWrite(a, LOW);
    digitalWrite(b, LOW);
    digitalWrite(c, LOW);
    digitalWrite(d, LOW);
    digitalWrite(e, LOW);
    digitalWrite(f, LOW);
    digitalWrite(g, LOW);
    break;
  }
}
