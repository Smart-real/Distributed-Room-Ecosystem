/* FINAL HYBRID CONTROLLER (IR BLASTER + MOTION SENSOR)
   - Device: ESP8266 (NodeMCU/Wemos)
   - Job 1: Control IR LEDs via Blynk
   - Job 2: Read PIR Sensor and sync with ESP32 via Cloud
*/

// --- 1. CREDENTIALS ---
#define BLYNK_TEMPLATE_ID "TMPL_YOUR_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_DEVICE_NAME"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

// --- 2. LIBRARIES ---
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

// --- 3. WIFI SETTINGS ---
char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

// --- 4. HARDWARE PINOUT ---
const uint16_t kIrLed = 4;  // D2 (GPIO 4) -> IR Transmitter
const int kStatusLed = 2;   // D4 (GPIO 2) -> Built-in Blue LED
const int pirPin = 5;       // D1 (GPIO 5) -> PIR Motion Sensor OUT

IRsend irsend(kIrLed);

// --- 5. STATE VARIABLES ---
bool lastMotionState = false;

// --- 6. NEC HEX CODES ---
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
  Serial.begin(9600);
  
  // Initialize Pins
  pinMode(kStatusLed, OUTPUT);
  pinMode(pirPin, INPUT); // NEW: Configure PIR pin
  
  digitalWrite(kStatusLed, HIGH); // LED OFF initially
  irsend.begin();

  Serial.println("\n--- BOOTING SYSTEM ---");
  Serial.println("Connecting to Blynk...");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  Serial.println("WiFi Connected! Status LED ON.");
  digitalWrite(kStatusLed, LOW); 

  // --- STARTUP SEQUENCE ---
  Serial.println("Sequence: Turning LEDs ON...");
  irsend.sendNEC(HEX_ON, 32);
  delay(100); 
  // You can remove the 3s delay/purple sequence if you want faster startup
}

// --- BLYNK IR CONTROLS (Existing) ---
BLYNK_WRITE(V0) {
  if (param.asInt() == 1) irsend.sendNEC(HEX_ON, 32);
  else irsend.sendNEC(HEX_OFF, 32);
}

BLYNK_WRITE(V1) {
  if (param.asInt() == 1) irsend.sendNEC(HEX_BRIGHT_UP, 32);
  else irsend.sendNEC(HEX_BRIGHT_DOWN, 32);
}

BLYNK_WRITE(V2) {
  switch (param.asInt()) {
    case 1: irsend.sendNEC(HEX_RED, 32); break;
    case 2: irsend.sendNEC(HEX_GREEN, 32); break;
    case 3: irsend.sendNEC(HEX_BLUE, 32); break;
    case 4: irsend.sendNEC(HEX_WHITE, 32); break;
  }
}

BLYNK_WRITE(V3) {
  switch (param.asInt()) {
    case 1: irsend.sendNEC(HEX_PURPLE, 32); break;
    case 2: irsend.sendNEC(HEX_ORANGE, 32); break;
    case 3: irsend.sendNEC(HEX_CYAN, 32); break;
    case 4: irsend.sendNEC(HEX_YELLOW, 32); break;
  }
}

BLYNK_WRITE(V4) {
  switch (param.asInt()) {
    case 1: irsend.sendNEC(HEX_FLASH, 32); break;
    case 2: irsend.sendNEC(HEX_STROBE, 32); break;
    case 3: irsend.sendNEC(HEX_FADE, 32); break;
    case 4: irsend.sendNEC(HEX_SMOOTH, 32); break;
  }
}

// --- MAIN LOOP ---
void loop() {
  Blynk.run();

  // --- NEW: READ PIR SENSOR ---
  bool currentMotion = digitalRead(pirPin);

  // Only send data if the state has changed (Prevents flooding Blynk)
  if (currentMotion != lastMotionState) {
    
    // Send to Virtual Pin V5 (ESP32 will listen to this!)
    // 1 = Motion Detected, 0 = All Clear
    Blynk.virtualWrite(V5, currentMotion ? 1 : 0);
    
    // Optional: Visual debugging in Serial Monitor
    if (currentMotion) {
      Serial.println("Motion START -> Sent '1' to Cloud");
      // Optional: Turn IR LEDs RED on motion? 
      // irsend.sendNEC(HEX_RED, 32); 
    } else {
      Serial.println("Motion STOP -> Sent '0' to Cloud");
    }

    lastMotionState = currentMotion;
  }
  
  // Small delay for stability
  delay(100); 
}