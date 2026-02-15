// --- PIN DEFINITIONS ---
const int SENSOR_PIN = 18;  // Pin connected to TSOP Sensor
const int LED_PIN = 2;     // Pin connected to LED / Relay

// --- TUNING ---
// Your cycle is ~130ms. We use 300ms to be safe.
// If the sensor sees NO signal for 300ms, it decides the beam is broken.
const int BRIDGE_TIMEOUT = 300; 

// --- VARIABLES ---
bool isLightOn = false;        // Main light state
bool isBeamBroken = false;     // The "Filtered" beam state
bool lastBeamState = false;    // To detect the exact moment of breaking
unsigned long lastSignalTime = 0; // Timer to track the gaps

void setup() {
  Serial.begin(9600);
  pinMode(SENSOR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  digitalWrite(LED_PIN, LOW);
  Serial.println("System Ready. 300ms Bridge Filter Active.");
}

void loop() {
  int rawReading = digitalRead(SENSOR_PIN);
  unsigned long currentMillis = millis();

  // --- 1. WATCHDOG TIMER (The Filter) ---
  // Most TSOP sensors go LOW when they receive a burst of IR
  if (rawReading == LOW) {
    // We saw a signal! Reset the timer.
    lastSignalTime = currentMillis;
    isBeamBroken = false; // Beam is definitely CONNECTED right now
  } 
  else {
    // We are in a gap (or the beam is blocked).
    // Only decide it's broken if the silence lasts longer than 300ms
    if (currentMillis - lastSignalTime > BRIDGE_TIMEOUT) {
      isBeamBroken = true;
    }
  }

  // --- 2. TOGGLE LOGIC ---
  // Trigger only on the transition: Beam WAS connected, NOW it is broken.
  if (isBeamBroken == true && lastBeamState == false) {
    
    Serial.println("ACTION: Beam Broken (Confirmed) -> Toggling Light");
    
    // Toggle the light variable
    isLightOn = !isLightOn;

    // Apply to hardware
    if (isLightOn) {
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
    }
    
    // Wait a bit so we don't double-trigger while your hand is moving out
    delay(500); 
  }

  // Save the current calculated state for the next loop
  lastBeamState = isBeamBroken;
}