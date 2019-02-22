// ************************************************************************************************************************
// *** Washing machine - Blinking led at (520ms->1.92KHz) sketch
// ***
// *** To test the circuitry I will use another NodeMCU to blink a led with the desired frequency.
// ***
// *** To detect the end of my old washing machine cycle, and then send an email and beeps. On the future may activate
// *** the washing machine with a servomotor through internet. (Create WebServer, Send eMail, Sense Light, Activate Servomotor)
// ***
// *** TTD: Check the LDR once every 5 minutes, and sleep after sending the mail to save battery. Send mail if battery is low
// *** Calculate the energy cost of sleeping and awakening inbetween sensor readings. Reinicialize variables after sending mail
// ***
// *** I will use the ESP8266 (with the Lolin NodeMCU V3)
// ***  
// *** D0 ---(Servomotor)
// *** D2 ---(Vibration sw)---|(gnd)
// *** D3 ---(10kOhm)---------|(3V)
// *** D4 ---(LED)------------|(gnd)
// *** A0 ---(LDR)------------|(3V)
// ***   \---(10KOhm)---------|(gnd)
// ***
// *** My board of NodeMCU loses the programation after a Power cycle. This is fixed connecting GPIO0(D3-FLASH) 
// *** with a 10kOhm resistor to 3V. As per https://github.com/esp8266/Arduino/blob/master/doc/boards.md#boot-messages-and-modes.
// ***
// *** Created at 08 of February 2019
// *** By Celatzur
// *** Created at 08 of February 2019
// *** By Celatzur
// *** 
// *** https://github.com/celatzur/Washing-Machine
// *** 
// *** To run this board the first time, on the Arduino IDE: 
// *** 1) File>Preferences>“Additional Boards Manager URLs:”>“http://arduino.esp8266.com/stable/package_esp8266com_index.json“
// *** 2) Tools>Board>NodeMCU 1.0 (ESP-12E Module).
// *** 3) Tools>Board>Board Manager... "esp8266 by ESP8266 Community">Install
// *** 4) Tools>Board>NodeMCU 1.0 (ESP-12E Module). Upload Speed: "115200".
// *** 
// ************************************************************************************************************************

#include <ESP8266WiFi.h>      // Downloadable from: http://arduino.esp8266.com/stable/package_esp8266com_index.json

// **** LED definitions
const int ledPin = 2;              //#define D4 2 // Same as "LED_BUILTIN", but inverted logic
int ledState = LOW;
unsigned long blink_freq_ms = 520; // My WM blinks at 0.52 blinks/s (520ms->1.92KHz). We sample at three time this frequency
unsigned long previousMillis = 0;

// ************************************************************************************************************************
// *** Setup
// ************************************************************************************************************************
void setup() {
  Serial.begin(115200);       //Baud rate for serial port communication
  delay(10);

  //Output LED
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);
  
  Serial.print("Start blinking LED");
}

// ************************************************************************************************************************
// *** Main loop: Blinks a LED in a seocndary MCU to Test
// ************************************************************************************************************************
void loop() {
//  unsigned long currentMillis = millis();
//
//  if (currentMillis - previousMillis > blink_freq_ms) {
//    previousMillis = currentMillis;
//    if (ledState == LOW)
//      ledState = HIGH;  // Turn it on
//    else
//      ledState = LOW;   // Turn it off
//    digitalWrite(ledPin, ledState);
//    }
    ledState = LOW;
    digitalWrite(ledPin, ledState);
    delay(blink_freq_ms);

    ledState = HIGH;
    digitalWrite(ledPin, ledState);
    delay(1000-blink_freq_ms);
}
