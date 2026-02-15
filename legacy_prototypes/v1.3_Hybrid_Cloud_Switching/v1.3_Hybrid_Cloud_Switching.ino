#include <WiFi.h>
#include <HTTPClient.h>

// --- WIFI SETTINGS ---
const char* ssid = "YOUR_WIFI_SSID";         // <--- ENTER WIFI NAME
const char* password = "YOUR_WIFI_PASSWORD"; // <--- ENTER WIFI PASSWORD

// --- WEBHOOK SETTINGS ---
// This link toggles the bulb (ON -> OFF, or OFF -> ON)
const char* webhook_url = "https://hook.eu2.make.com/cghkq6jx1khowjfxetmzr8oharwpclxu";

// --- PIN DEFINITIONS ---
const int SENSOR_PIN = 5;  // IR Receiver
const int LED_PIN = 2;     // Physical Relay/LED

// --- TUNING (The Logic that worked) ---
const int BRIDGE_TIMEOUT = 300; 

// --- VARIABLES ---
bool isLightOn = false;        // Local Relay State
bool isBeamBroken = false;     // Filtered Beam State
bool lastBeamState = false;    // Edge Detection
unsigned long lastSignalTime = 0; 

void setup() {
  Serial.begin(9600);
  
  pinMode(SENSOR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // --- WIFI CONNECTION ---
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  
  Serial.println("System Ready. Bridge Filter + Webhook Active.");
}

void loop() {
  int rawReading = digitalRead(SENSOR_PIN);
  unsigned long currentMillis = millis();

  // --- 1. WATCHDOG FILTER (Fixes 38kHz Gaps) ---
  if (rawReading == LOW) { // Signal Received
    lastSignalTime = currentMillis;
    isBeamBroken = false; 
  } 
  else { // No Signal
    if (currentMillis - lastSignalTime > BRIDGE_TIMEOUT) {
      isBeamBroken = true;
    }
  }

  // --- 2. TOGGLE LOGIC ---
  if (isBeamBroken == true && lastBeamState == false) {
    
    Serial.println("ACTION: Beam Broken! Toggling Systems...");

    // A. Toggle Local Relay/LED
    isLightOn = !isLightOn;
    digitalWrite(LED_PIN, isLightOn ? HIGH : LOW);

    // B. Trigger Smart Bulb Webhook
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(webhook_url);
      
      Serial.print("Sending Webhook... ");
      int httpCode = http.GET(); // Send the request
      
      if (httpCode > 0) {
        Serial.print("Success! Code: ");
        Serial.println(httpCode);
      } else {
        Serial.print("Error: ");
        Serial.println(http.errorToString(httpCode).c_str());
      }
      http.end();
    } else {
      Serial.println("WiFi Disconnected! Cannot trigger Smart Bulb.");
    }

    // C. Delay to prevent double triggers
    delay(1000); 
  }

  lastBeamState = isBeamBroken;
}