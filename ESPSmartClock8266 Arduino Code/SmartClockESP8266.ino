
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <time.h>
#include "DHT.h"
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

// An IR detector/demodulator is connected to GPIO pin 5 (D1 on a NodeMCU
// board).
// Note: GPIO 16 won't work on the ESP8266 as it does not have interrupts.
const uint16_t kRecvPin = 5;
unsigned long key_value = 0;
IRrecv irrecv(kRecvPin);

decode_results results;
int pinCS = 2;
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays   = 1;
char time_value[20];

// LED Matrix Pin -> ESP8266 Pin
// Vcc            -> 3v  (3V on NodeMCU 3V3 on WEMOS)
// Gnd            -> Gnd (G on NodeMCU)
// DIN            -> D7  (Same Pin for WEMOS)
// CS             -> D4  (Same Pin for WEMOS)
// CLK            -> D5  (Same Pin for WEMOS)

//Day Names
String weekDays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

int wait = 70; // In milliseconds

int spacer = 1;
int width  = 5 + spacer; // The font width is 5 pixels

int m;

#define DHTPIN 0          // D3

#define DHTTYPE DHT22     // DHT 11

DHT dht(DHTPIN, DHTTYPE);

String t, h;


void setup() {
  Serial.begin(9600);
  dht.begin();
  irrecv.enableIRIn();  // Start the receiver
  while (!Serial)  // Wait for the serial connection to be establised.
    delay(50);
  Serial.println();
  Serial.print("IRrecvDemo is now running and waiting for IR message on Pin ");
  Serial.println(kRecvPin);
  //INSERT YOUR SSID AND PASSWORD HERE

  WiFi.begin("sejuk", "@nova1980");

  //CHANGE THE POOL WITH YOUR CITY. SEARCH AT https://www.ntppool.org/zone/@

  configTime(0 * 3600, 0, "2.id.pool.ntp.org");

  setenv("TZ", "GMT-7", 7);

  matrix.setIntensity(0); // Use a value between 0 and 15 for brightness
  matrix.setRotation(0, 1);    // The first display is position upside down
  matrix.setRotation(1, 1);    // The first display is position upside down
  matrix.setRotation(2, 1);    // The first display is position upside down
  matrix.setRotation(3, 1);    // The first display is position upside down
  matrix.fillScreen(LOW);
  matrix.write();

  while ( WiFi.status() != WL_CONNECTED ) {
    matrix.drawChar(2, 0, 'W', HIGH, LOW, 1); // H
    matrix.drawChar(8, 0, 'I', HIGH, LOW, 1); // HH
    matrix.drawChar(14, 0, '-', HIGH, LOW, 1); // HH:
    matrix.drawChar(20, 0, 'F', HIGH, LOW, 1); // HH:M
    matrix.drawChar(26, 0, 'I', HIGH, LOW, 1); // HH:MM
    matrix.write(); // Send bitmap to display
    delay(250);
    matrix.fillScreen(LOW);
    matrix.write();
    delay(250);
  }
}

void loop() {
  m = map(analogRead(0), 0, 1024, 0, 12);
  matrix.setIntensity(m);
  matrix.fillScreen(LOW);
  time_t now = time(nullptr);
  String time = String(ctime(&now));
  time.trim();
  //Serial.println(time);
  time.substring(11, 19).toCharArray(time_value, 10);
  matrix.drawChar(2, 0, time_value[0], HIGH, LOW, 1); // H
  matrix.drawChar(8, 0, time_value[1], HIGH, LOW, 1); // HH
  matrix.drawChar(14, 0, time_value[2], HIGH, LOW, 1); // HH:
  matrix.drawChar(20, 0, time_value[3], HIGH, LOW, 1); // HH:M
  matrix.drawChar(26, 0, time_value[4], HIGH, LOW, 1); // HH:MM
  matrix.write(); // Send bitmap to display

 // delay(30000);

  matrix.fillScreen(LOW);
  h = (String)(int)dht.readHumidity();
  t = (String)(int)dht.readTemperature();
  display_message(t + "C " + h + "%");
}
void display_message(String message) {
  if (irrecv.decode(&results)) {
    serialPrintUint64(results.value, HEX);
    Serial.println("");

    switch (results.value) {
      case 0xFFA25D:
        for ( int i = 0 ; i < width * message.length() + matrix.width() - spacer; i++ ) {
          //matrix.fillScreen(LOW);
          int letter = i / width;
          int x = (matrix.width() - 1) - i % width;
          int y = (matrix.height() - 8) / 2; // center the text vertically
          while ( x + width - spacer >= 0 && letter >= 0 ) {
            if ( letter < message.length() ) {
              matrix.drawChar(x, y, message[letter], HIGH, LOW, 1); // HIGH LOW means foreground ON, background off, reverse to invert the image
            }
            letter--;
            x -= width;
          }
          matrix.write(); // Send bitmap to display
          delay(wait / 2);
        }
        break;
    }
    key_value = results.value;
    irrecv.resume();
  }
  delay(100);
}
