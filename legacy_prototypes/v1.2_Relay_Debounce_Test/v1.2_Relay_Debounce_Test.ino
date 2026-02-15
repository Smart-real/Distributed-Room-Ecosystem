// --- PIN DEFINITIONS ---
const int SENSOR_PIN = 4;  // The pin connected to your Beam Sensor (Receiver)
const int LED_PIN = 2;     // Built-in Blue LED on ESP32

// --- CONFIGURATION ---
// VS1838B outputs LOW when it sees the 38kHz signal.
// Therefore, when the beam is BROKEN, the signal stops and it goes HIGH.
const int BEAM_BROKEN_STATE = HIGH; 

// --- VARIABLES ---
bool isLightOn = false;       // Tracks if the light is currently ON or OFF
int lastSensorState;          // Tracks the previous reading of the sensor

void setup() {
  Serial.begin(115200);       // Changed to 115200 for ESP32 standard
  
  pinMode(SENSOR_PIN, INPUT); // Set sensor pin as Input
  pinMode(LED_PIN, OUTPUT);   // Set LED pin as Output
  
  // Initialize the light to OFF
  digitalWrite(LED_PIN, LOW);
  
  // Read the initial state so we don't false trigger at startup
  lastSensorState = digitalRead(SENSOR_PIN);
}

void loop() {
  // 1. Read the current state of the sensor
  int currentSensorState = digitalRead(SENSOR_PIN);

  // 2. Check for the "Moment" the beam breaks (Edge Detection)
  // Logic: Is it BROKEN now? AND Was it CONNECTED (not broken) before?
  if (currentSensorState == BEAM_BROKEN_STATE && lastSensorState != BEAM_BROKEN_STATE) {
    
    // 3. Toggle the Light State
    isLightOn = !isLightOn; 

    // 4. Apply the new state to the hardware
    digitalWrite(LED_PIN, isLightOn ? HIGH : LOW);
    
    if (isLightOn) {
      Serial.println("Action: Beam Broken -> LED ON");
    } else {
      Serial.println("Action: Beam Broken -> LED OFF");
    }

    // 5. Debounce (Prevent double triggers)
    delay(250); 
  }

  // 6. Save the current reading
  lastSensorState = currentSensorState;
}