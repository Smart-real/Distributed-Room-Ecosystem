/* FINAL ESP32: "SIMPLE LOGIC" + SMART FEATURES
   - Logic: Door Closed = Connected (LOW). Door Open = Broken (HIGH).
   - Trigger: Acts ONLY when the door first opens.
   - Network: Non-Blocking (Instant response).
*/

// --- BLYNK CONFIGURATION (UPDATED) ---
#define BLYNK_TEMPLATE_ID "TMPL_YOUR_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_DEVICE_NAME"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ArduinoOTA.h>

char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

// --- PINS ---
const int SENSOR_PIN = 4;   // VS1838B
const int RELAY_PIN = 26;   // Transistor Base
const int BTN_PIN = 27;     // Lock Button
const int LED_PIN = 2;      // Debug LED

// --- LOGIC SETTINGS ---
// Matches your "Near Perfect" test:
const int BEAM_BROKEN_STATE = HIGH;      // Door Open
const int BEAM_CONNECTED_STATE = LOW;    // Door Closed

// --- VARIABLES ---
bool isLightOn = false;
bool isSystemLocked = false;
int lastSensorState;
int lastBtnState = HIGH;

// --- TIMERS ---
unsigned long lastBtnTime = 0;
unsigned long lastTriggerTime = 0;

void setup() {
  Serial.begin(115200);

  // 1. HARDWARE INIT (Instant)
  pinMode(SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  // Default State: OFF
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  // Snapshot Reality (Prevent startup trigger)
  lastSensorState = digitalRead(SENSOR_PIN);
  
  if (lastSensorState == BEAM_CONNECTED_STATE) {
     Serial.println("--- SYSTEM ARMED (Door Closed) ---");
  } else {
     Serial.println("--- SYSTEM STANDBY (Door Open) ---");
  }

  // 2. NETWORK INIT (Background)
  // We start the connection but DO NOT wait.
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Blynk.config(BLYNK_AUTH_TOKEN);
  
  // OTA Setup (Updated Name)
  ArduinoOTA.setHostname("SmartESP32");
  ArduinoOTA.begin();
}

void loop() {
  // --- BACKGROUND NETWORK TASKS ---
  // Only talk to internet if connected.
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.run();
    ArduinoOTA.handle();
  }

  unsigned long currentMillis = millis();

  // ==========================================
  // 1. LOCK BUTTON LOGIC
  // ==========================================
  int currentBtnState = digitalRead(BTN_PIN);

  if (currentBtnState == LOW && lastBtnState == HIGH) {
    if (currentMillis - lastBtnTime > 250) {
      // Toggle Lock Mode
      isSystemLocked = !isSystemLocked;
      
      // Visual Feedback (Quick Flash)
      digitalWrite(LED_PIN, HIGH); delay(100); digitalWrite(LED_PIN, LOW);
      
      // Update App
      if (WiFi.status() == WL_CONNECTED) {
        Blynk.virtualWrite(V1, isSystemLocked ? 1 : 0);
      }
      lastBtnTime = currentMillis;
    }
  }
  lastBtnState = currentBtnState;

  // ==========================================
  // 2. DOOR SENSOR LOGIC (The "Perfect" Logic)
  // ==========================================
  if (!isSystemLocked) {
    int currentSensorState = digitalRead(SENSOR_PIN);

    // EDGE DETECTION: 
    // Trigger ONLY when we go from CONNECTED -> BROKEN
    if (currentSensorState == BEAM_BROKEN_STATE && lastSensorState == BEAM_CONNECTED_STATE) {
      
      // COOLDOWN: 1 Second (Prevents double-toggling while walking through)
      if (currentMillis - lastTriggerTime > 1000) {
        
        Serial.println(">> DOOR OPEN DETECTED! Toggling Light.");
        toggleLight();
        
        lastTriggerTime = currentMillis;
      }
    }
    
    // UPDATE MEMORY
    // We always track the state so we are ready for the next move.
    lastSensorState = currentSensorState;
  }
}

// --- HELPER FUNCTIONS ---
void toggleLight() {
  isLightOn = !isLightOn;
  updateHardware();
}

void updateHardware() {
  // Physical
  digitalWrite(RELAY_PIN, isLightOn ? HIGH : LOW);
  digitalWrite(LED_PIN, isLightOn ? HIGH : LOW);
  
  // Cloud Sync
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.virtualWrite(V0, isLightOn ? 1 : 0);
  }
}

// --- APP CONTROL ---
// V0: Main Light Switch
BLYNK_WRITE(V0) {
  isLightOn = param.asInt();
  updateHardware();
}

// V1: System Lock Switch
BLYNK_WRITE(V1) {
  isSystemLocked = param.asInt();
}