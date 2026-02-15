#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <WiFi.h>
#include <HTTPClient.h>

// --- USER CONFIGURATION ---
const char* ssid = "YOUR_WIFI_NAME";        // <--- ENTER WIFI NAME
const char* password = "YOUR_WIFI_PASSWORD"; // <--- ENTER WIFI PASS
String webhookURL = "https://hook.eu2.make.com/";

// --- PINS ---
const uint16_t kRecvPin = 4; // GPIO 4 (D2)
const int ledPin = 2;        // Built-in LED

// --- IR SETUP ---
IRrecv irrecv(kRecvPin);
decode_results results;

// --- WATCHDOG & LOGIC ---
unsigned long lastSignalTime = 0;
const long signalTimeout = 400; // 400ms (Generous buffer to ignore the "pulsing" gaps)

// The "State Flag" - prevents spamming the link
bool alarmTriggered = false; 

void setup() {
  Serial.begin(115200);
  
  // 1. Setup Pins
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // 2. Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  // 3. Start IR Receiver
  irrecv.enableIRIn();
  Serial.println("System Armed. Waiting for IR Heartbeat...");
  
  // Initialize timer
  lastSignalTime = millis(); 
}

void loop() {
  // --- PART A: LISTEN FOR HEARTBEAT ---
  if (irrecv.decode(&results)) {
    // Check for our Emitter (0x11111111) or Repeat Code (0xFFFFFFFF)
    if (results.value == 0x11111111 || results.value == 0xFFFFFFFF) {
      lastSignalTime = millis(); // Reset the "Bomb Timer"
    }
    irrecv.resume(); // Ready for next packet
  }

  // --- PART B: CHECK TIMER (THE LOGIC) ---
  // If too much time passed since last heartbeat...
  if (millis() - lastSignalTime > signalTimeout) {
    
    // BEAM IS BROKEN!
    // Only fire if we haven't fired already (One-Shot Logic)
    if (!alarmTriggered) {
      Serial.println("ALARM! Beam Broken - Sending Webhook...");
      digitalWrite(ledPin, HIGH); // Turn on LED
      
      // FIRE THE WEBHOOK
      if(WiFi.status() == WL_CONNECTED){
        HTTPClient http;
        http.begin(webhookURL);
        int httpResponseCode = http.GET(); // Send the request
        
        if (httpResponseCode > 0) {
          Serial.print("Webhook Sent! Response: ");
          Serial.println(httpResponseCode);
        } else {
          Serial.print("Error on sending POST: ");
          Serial.println(httpResponseCode);
        }
        http.end();
      }
      
      // Lock the alarm so we don't send it again until reset
      alarmTriggered = true;
    }

  } else {
    // BEAM IS SAFE
    if (alarmTriggered) {
      Serial.println("Beam Restored. System Reset.");
      digitalWrite(ledPin, LOW); // Turn off LED
      alarmTriggered = false;    // Reset the flag, ready for next intruder
    }
  }
  
  delay(10); // Stability delay
}