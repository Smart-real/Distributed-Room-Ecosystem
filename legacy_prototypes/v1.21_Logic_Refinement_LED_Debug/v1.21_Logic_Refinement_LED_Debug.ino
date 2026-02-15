// --- PIN DEFINITIONS ---
const int SENSOR_PIN = 4;   // IR Receiver (VS1838B)
const int RELAY_PIN = 26;   // Relay Module
const int LED_PIN = 2;      // Built-in Blue LED (Debug)

// --- CONFIGURATION ---
// VS1838B goes HIGH when beam is broken (Active LOW sensor)
const int BEAM_BROKEN_STATE = HIGH; 

// --- RELAY & LED SETTINGS ---
// Most Relays are Active LOW (LOW = ON)
// Built-in LEDs are often Active HIGH (HIGH = ON) on ESP32
const int RELAY_ON = LOW;   
const int RELAY_OFF = HIGH; 

const int LED_ON = HIGH;
const int LED_OFF = LOW;

// --- VARIABLES ---
bool isLightOn = false;       // Logic State
int lastSensorState;          // Memory

void setup() {
  Serial.begin(115200);
  
  pinMode(SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  // Start Everything OFF
  digitalWrite(RELAY_PIN, RELAY_OFF);
  digitalWrite(LED_PIN, LED_OFF);
  
  // Read initial state
  lastSensorState = digitalRead(SENSOR_PIN);
  
  Serial.println("--- SYSTEM READY: Relay + Debug LED ---");
}

void loop() {
  int currentSensorState = digitalRead(SENSOR_PIN);

  // --- EDGE DETECTION ---
  // If Beam is BROKEN now, but was CONNECTED before...
  if (currentSensorState == BEAM_BROKEN_STATE && lastSensorState != BEAM_BROKEN_STATE) {
    
    // 1. Toggle Logic Variable
    isLightOn = !isLightOn; 

    // 2. Update Hardware
    if (isLightOn) {
      digitalWrite(RELAY_PIN, RELAY_ON);  // Click Relay ON
      digitalWrite(LED_PIN, LED_ON);      // Turn Blue LED ON
      Serial.println("Action: Beam Broken -> ON");
    } else {
      digitalWrite(RELAY_PIN, RELAY_OFF); // Click Relay OFF
      digitalWrite(LED_PIN, LED_OFF);     // Turn Blue LED OFF
      Serial.println("Action: Beam Broken -> OFF");
    }

    // 3. Debounce (Prevent double-clicks)
    delay(250); 
  }

  lastSensorState = currentSensorState;
}
