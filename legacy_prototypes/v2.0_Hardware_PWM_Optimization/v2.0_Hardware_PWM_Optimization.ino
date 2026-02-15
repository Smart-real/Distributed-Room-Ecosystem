/* FINAL MASTER V2: WIRELESS UPLOAD + HARDWARE PWM FIX
   - D2: Remote for Table (IR Emitter)
   - D1: Tripwire Emitter (Hardware 38kHz - Non-Blocking)
   - D5: Tripwire Receiver (Optional local check)
   - FEATURES: Blynk + OTA + Anti-Glitch Beam
*/

#define BLYNK_TEMPLATE_ID "TMPL_YOUR_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_DEVICE_NAME"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

// --- HARDWARE PINOUT ---
const uint16_t remotePin = 4;   // D2 -> For Table LED Strip
const int tripwirePin = 5;      // D1 -> For Door Tripwire Beam
const int recvPin = 14;         // D5 -> Tripwire Receiver
const int statusLed = 2;        // D4 -> Built-in Blue LED

// --- OBJECTS ---
IRsend irRemote(remotePin);     

// --- HEX CODES (RGB Strip) ---
#define HEX_ON          0xF7C03F
#define HEX_OFF         0xF740BF
#define HEX_BRIGHT_UP   0xF700FF
#define HEX_BRIGHT_DOWN 0xF7807F
#define HEX_RED         0xF720DF
#define HEX_GREEN       0xF7A05F
#define HEX_BLUE        0xF7609F
#define HEX_WHITE       0xF7E01F
#define HEX_PURPLE      0xF748B7 
#define HEX_ORANGE      0xF710EF
#define HEX_CYAN        0xF7B04F
#define HEX_YELLOW      0xF708F7
#define HEX_FLASH       0xF728D7
#define HEX_STROBE      0xF7A857
#define HEX_FADE        0xF76897
#define HEX_SMOOTH      0xF7E817

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- BOOTING OTA MASTER (HW PWM) ---");

  // 1. Setup Pins
  pinMode(statusLed, OUTPUT);
  pinMode(recvPin, INPUT);
  digitalWrite(statusLed, HIGH); // Off initially
  
  // 2. SETUP TRIPWIRE (THE FIX)
  // We use pure Hardware PWM. It runs in the background forever.
  // No "micros()" code in loop needed.
  pinMode(tripwirePin, OUTPUT);
  analogWriteFreq(38000); // Set Hardware Timer to 38kHz
  analogWriteRange(1023); // Standard Range
  analogWrite(tripwirePin, 512); // Turn ON @ 50% Duty Cycle (Continuous)
  
  // 3. Setup Remote
  irRemote.begin();

  // 4. Start WiFi & Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  // 5. SETUP OTA
  ArduinoOTA.setHostname("SmartESP8266"); 
  
  ArduinoOTA.onStart([]() {
    Serial.println("Start updating");
    // Turn off Beam during update to save power/safety
    analogWrite(tripwirePin, 0); 
  });
  
  ArduinoOTA.onEnd([]() { Serial.println("\nEnd"); });
  ArduinoOTA.onError([](ota_error_t error) { Serial.printf("Error[%u]\n", error); });
  
  ArduinoOTA.begin();
  Serial.println("OTA Ready");
  
  // 6. Startup Sequence (Test Table Remote)
  irRemote.sendNEC(HEX_ON, 32);
  delay(150); 
  irRemote.sendNEC(HEX_PURPLE, 32);
  digitalWrite(statusLed, LOW); // LED ON = Ready
}

// --- BLYNK CALLBACKS ---
BLYNK_WRITE(V0) { if(param.asInt()) irRemote.sendNEC(HEX_ON, 32); else irRemote.sendNEC(HEX_OFF, 32); }
BLYNK_WRITE(V1) { if(param.asInt()) irRemote.sendNEC(HEX_BRIGHT_UP, 32); else irRemote.sendNEC(HEX_BRIGHT_DOWN, 32); }
BLYNK_WRITE(V2) {
  switch (param.asInt()) {
    case 1: irRemote.sendNEC(HEX_RED, 32); break;
    case 2: irRemote.sendNEC(HEX_GREEN, 32); break;
    case 3: irRemote.sendNEC(HEX_BLUE, 32); break;
    case 4: irRemote.sendNEC(HEX_WHITE, 32); break;
  }
}
BLYNK_WRITE(V3) {
  switch (param.asInt()) {
    case 1: irRemote.sendNEC(HEX_PURPLE, 32); break;
    case 2: irRemote.sendNEC(HEX_ORANGE, 32); break;
    case 3: irRemote.sendNEC(HEX_CYAN, 32); break;
    case 4: irRemote.sendNEC(HEX_YELLOW, 32); break;
  }
}
BLYNK_WRITE(V4) {
  switch (param.asInt()) {
    case 1: irRemote.sendNEC(HEX_FLASH, 32); break;
    case 2: irRemote.sendNEC(HEX_STROBE, 32); break;
    case 3: irRemote.sendNEC(HEX_FADE, 32); break;
    case 4: irRemote.sendNEC(HEX_SMOOTH, 32); break;
  }
}

void loop() {
  // 1. LISTEN FOR UPDATES (Keep this!)
  ArduinoOTA.handle();
  
  // 2. HANDLE WIFI
  Blynk.run();

  // 3. TRIPWIRE HANDLING (DELETED)
  // We removed the manual pulse code. 
  // The 'analogWrite' in setup() keeps the beam running perfectly 
  // even while Blynk is processing colors or Wi-Fi data.
}