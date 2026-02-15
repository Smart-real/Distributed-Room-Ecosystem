/* FINAL "DOOR LOGIC" MASTER
   - Setup: Door Closed = Beam CONNECTED. Door Open = Beam BROKEN.
   - Action: Opening the door toggles the light.
   - Constraint: If door stays open, sensor is blind (use button to toggle).
*/

// --- PINS ---
const int SENSOR_PIN = 4;
const int RELAY_PIN = 26;   // Transistor -> Relay
const int BTN_PIN = 27;     // Manual Button
const int LED_PIN = 2;      // Debug LED

// --- LOGIC ---
const int BEAM_BROKEN_STATE = HIGH;      // Signal stops when blocked
const int BEAM_CONNECTED_STATE = LOW;    // Signal active when connected

// --- VARIABLES ---
bool isLightOn = false;
bool isSystemLocked = false;
int lastSensorState;
int lastBtnState = HIGH;
unsigned long lastBtnTime = 0;
unsigned long lastTriggerTime = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("--- DOOR SYSTEM READY ---");

  pinMode(SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  // Snapshot Initial State
  lastSensorState = digitalRead(SENSOR_PIN);
  if(lastSensorState == BEAM_CONNECTED_STATE) Serial.println("Status: DOOR CLOSED (Armed)");
  else Serial.println("Status: DOOR OPEN (Beam Blocked)");
}

void loop() {
  unsigned long currentMillis = millis();

  // ==========================================
  // 1. MANUAL BUTTON (To fix the "Blind Spot")
  // ==========================================
  int currentBtnState = digitalRead(BTN_PIN);
  
  if (currentBtnState == LOW && lastBtnState == HIGH) {
    if (currentMillis - lastBtnTime > 250) {
      // Button acts as a direct toggle for the light
      Serial.println(">> BUTTON PRESS: Manual Toggle");
      toggleLight(); 
      lastBtnTime = currentMillis;
    }
  }
  lastBtnState = currentBtnState;

  // ==========================================
  // 2. DOOR SENSOR LOGIC
  // ==========================================
  if (!isSystemLocked) {
    int currentSensorState = digitalRead(SENSOR_PIN);

    // TRIGGER CONDITION:
    // We only act when the beam *changes* from CONNECTED to BROKEN.
    // This happens exactly when you crack the door open.
    if (currentSensorState == BEAM_BROKEN_STATE && lastSensorState == BEAM_CONNECTED_STATE) {
      
      if (currentMillis - lastTriggerTime > 1000) {
        Serial.println(">> DOOR OPENED! Toggling Light.");
        toggleLight();
        lastTriggerTime = currentMillis;
      }
    }
    
    // MEMORY UPDATE
    // If the door stays open, 'lastSensorState' becomes BROKEN.
    // The loop waits until it becomes CONNECTED (Closed) again to re-arm.
    lastSensorState = currentSensorState;
  }
}

void toggleLight() {
  isLightOn = !isLightOn;
  digitalWrite(RELAY_PIN, isLightOn ? HIGH : LOW);
  digitalWrite(LED_PIN, isLightOn ? HIGH : LOW); // Sync LED
}