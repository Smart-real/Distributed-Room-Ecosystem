/* FINAL LED CONTROLLER 
   - CONNECTS to your new Blynk Template
   - Startup: ON -> Wait 3s -> PURPLE
   - Controls: Power, Brightness, Colors, Modes
*/

// --- 1. NEW CREDENTIALS (FROM YOUR QUICKSTART) ---
#define BLYNK_TEMPLATE_ID "TMPL_YOUR_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_DEVICE_NAME" // You can rename this in the web dashboard later
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
// D2 on NodeMCU is GPIO 4
const uint16_t kIrLed = 4; 
// D4 on NodeMCU is GPIO 2 (Built-in Blue LED)
const int kStatusLed = 2;   

IRsend irsend(kIrLed);

// --- 5. NEC HEX CODES (24-KEY REMOTE) ---
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
  // Debug console at 9600 for stability
  Serial.begin(9600);
  
  // Initialize Pins
  pinMode(kStatusLed, OUTPUT);
  digitalWrite(kStatusLed, HIGH); // Start with Blue LED OFF
  irsend.begin();

  Serial.println("\n--- BOOTING SYSTEM ---");
  Serial.println("Connecting to Blynk...");

  // Connect using the NEW credentials
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  // If we pass this line, we are connected!
  Serial.println("WiFi Connected! Status LED ON.");
  digitalWrite(kStatusLed, LOW); // Turn ON Blue LED to show connection

  // --- STARTUP SEQUENCE ---
  Serial.println("Sequence: Turning ON...");
  irsend.sendNEC(HEX_ON, 32);
  
  Serial.println("Sequence: Waiting 3 seconds...");
  delay(3000);
  
  Serial.println("Sequence: Setting PURPLE...");
  irsend.sendNEC(HEX_PURPLE, 32);
}

// --- BLYNK CONTROLS ---

// V0: Power Button
BLYNK_WRITE(V0) {
  int val = param.asInt();
  if (val == 1) {
    irsend.sendNEC(HEX_ON, 32);
    Serial.println("Blynk: ON");
  } else {
    irsend.sendNEC(HEX_OFF, 32);
    Serial.println("Blynk: OFF");
  }
}

// V1: Brightness (1=UP, 0=DOWN)
BLYNK_WRITE(V1) {
  if (param.asInt() == 1) irsend.sendNEC(HEX_BRIGHT_UP, 32);
  else irsend.sendNEC(HEX_BRIGHT_DOWN, 32);
}

// V2: Main Colors (1=Red, 2=Green, 3=Blue, 4=White)
BLYNK_WRITE(V2) {
  switch (param.asInt()) {
    case 1: irsend.sendNEC(HEX_RED, 32); break;
    case 2: irsend.sendNEC(HEX_GREEN, 32); break;
    case 3: irsend.sendNEC(HEX_BLUE, 32); break;
    case 4: irsend.sendNEC(HEX_WHITE, 32); break;
  }
}

// V3: Custom Colors (1=Purple, 2=Orange, 3=Cyan, 4=Yellow)
BLYNK_WRITE(V3) {
  switch (param.asInt()) {
    case 1: irsend.sendNEC(HEX_PURPLE, 32); break;
    case 2: irsend.sendNEC(HEX_ORANGE, 32); break;
    case 3: irsend.sendNEC(HEX_CYAN, 32); break;
    case 4: irsend.sendNEC(HEX_YELLOW, 32); break;
  }
}

// V4: Modes (1=Flash, 2=Strobe, 3=Fade, 4=Smooth)
BLYNK_WRITE(V4) {
  switch (param.asInt()) {
    case 1: irsend.sendNEC(HEX_FLASH, 32); break;
    case 2: irsend.sendNEC(HEX_STROBE, 32); break;
    case 3: irsend.sendNEC(HEX_FADE, 32); break;
    case 4: irsend.sendNEC(HEX_SMOOTH, 32); break;
  }
}

void loop() {
  Blynk.run();
}