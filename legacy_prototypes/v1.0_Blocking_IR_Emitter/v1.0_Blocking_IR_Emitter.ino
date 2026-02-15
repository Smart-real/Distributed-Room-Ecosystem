#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

const uint16_t kIrLed = 5; // GPIO 5 (D1)
IRsend irsend(kIrLed);

void setup() {
  irsend.begin();
  Serial.begin(115200);
}

void loop() {
  // Send the "Heartbeat" code
  // We use a specific hex code 0x11111111 just as an identifier
  irsend.sendNEC(0x11111111);
  
  // Wait 60ms. 
  // NEC takes about 70ms to transmit, so total cycle is ~130ms.
  // This checks the beam ~7 times a second.
  delay(60); 
}