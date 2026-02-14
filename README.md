# Smart Entry & Desk Ecosystem: A Latency-Optimized IoT System

![System Banner](assets/system_architecture.jpg)
*(Note: Replace this with your actual architecture diagram or a photo of the finished setup)*

### 🚀 Project Overview
This project is a distributed IoT system designed to automate room lighting based on physical presence, synchronized with a desk environment. Unlike standard motion sensors, this system utilizes a **custom IR "Heartbeat" protocol** to reject ambient light interference and implements a **non-blocking architecture** to ensure instant physical response times (<50ms) even during heavy Wi-Fi activity.

**Status:** ✅ Active / Stable v2.0

### ⚙️ Key Features
* **Anti-Jitter Algorithm:** Implemented a 250ms "Watchdog Timer" software filter to distinguish between solid physical obstructions and packet loss/flicker.
* **Hardware-Level PWM:** Offloaded 38kHz IR signal generation to the ESP8266 hardware timer, decoupling signal stability from CPU load (Wi-Fi/Blynk tasks).
* **Over-The-Air (OTA) Updates:** Engineered a wireless update pipeline to bypass physical hardware limitations (broken USB data lines) and enable remote maintenance.
* **Dual-Control Logic:** "2-Way Switch" architecture allowing simultaneous control via physical wall buttons and the Blynk IoT cloud.

### 🔧 Engineering Journey & Technical Challenges

#### 1. The Sensor Evolution (PIR vs. Active IR)
The project began as a feasibility study to understand relay logic and pinout configurations, with the initial intention of using Passive Infrared (PIR) motion sensors. However, early testing revealed significant limitations with standard PIR sensors for this specific use case (latency and broad detection zones). 

I pivoted to an **Active IR Tripwire** mechanism, initially testing with standard IR photodiodes.
* **Problem:** Standard photodiodes were hypersensitive to ambient noise (sunlight, heat sources), causing constant false triggers.
* **Solution:** I upgraded to **TSOP-style receivers** (VS1838B) which contain internal gain control and band-pass filters.

#### 2. The "Continuous Signal" Fallacy
A major technical hurdle involved the behavior of the TSOP receiver.
* **The Bug:** I initially drove the IR Emitter with a continuous DC signal (Always ON), expecting a constant HIGH/LOW state. The system failed to trigger reliably for over 5 hours of debugging.
* **The Discovery:** I realized that TSOP receivers are designed for *burst* communication (like TV remotes) and will actively filter out continuous signals as "interference" after a few seconds.
* **The Fix:** I re-engineered the emitter logic to generate **modulated 38kHz packets**. By pulsing the emitter at the carrier frequency native to the receiver, I established a stable link that only breaks upon physical obstruction.

#### 3. Signal Debouncing & Human Patterns
Reliable detection required software-level filtering to match human movement patterns.
* **The "Double Trigger" Issue:** A person walking through the beam has two legs, which caused the sensor to register two distinct "breaks" (Trigger ON -> OFF -> ON -> OFF) within milliseconds, toggling the light incorrectly.
* **The Optimization:** I implemented a **non-blocking cooldown timer (200ms - 500ms)**. This window ignores the micro-gap between walking legs, treating the entire passage as a single valid event.

#### 4. Hardware Integration
* **High-Voltage Isolation:** The system controls a custom-modified extension cord via a 5V Relay Module.
* **Soldering & Safety:** Extensive soldering was required to bridge the logic-level ESP32/ESP8266 controllers with mains-voltage appliances, ensuring safe and robust connections for the "Smart Extension" hub.

![Relay Wiring](assets/relay_wiring_internals.jpg)
*(Note: Upload a photo of your relay soldering/wiring to your assets folder)*

### 🛠️ Hardware Architecture
* **Door Controller (Receiver):** ESP32 DevKit V1 + VS1838B IR Receiver + 5V Relay
* **Desk Controller (Emitter):** NodeMCU (ESP8266) + IR LED (940nm) + WS2812B LED Strip
* **Protocol:** 38kHz NEC Carrier Wave (Continuous Beam)

### 🔌 Circuit Diagram
![Circuit Diagram](schematics/wiring_diagram.png)
*(Note: Upload your wiring diagram to the schematics folder)*

> The system uses a transistor-buffered relay circuit to protect the ESP32 GPIO pins from inductive flyback voltage.

### 💻 Software Logic
The core challenge was preventing "blocking code" (Wi-Fi connection attempts) from freezing the sensor loop.

**The Solution:**
1.  **Boot Priority:** The system initializes physical sensors immediately (0ms delay), moving Wi-Fi connection to a background non-blocking task.
2.  **Signal Filtering:**
    ```cpp
    // Simplified logic for noise rejection
    if (signal_lost_duration > 250ms) {
       trigger_lights(); // Only act on confirmed presence
    }
    ```

### 📱 IoT Integration (Blynk)
* **V0:** Main Light Toggle (Synced with Relay)
* **V1:** System Lock (Disables sensor for "Party Mode")
* **Automation:** When Door Light turns ON -> Desk Underglow turns ON (Cloud Sync).

---
### 📜 License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

*Created by Aoun Raza*
