# Edge-Sync-Room-Ecosystem: Latency-Optimized Automation

![System Architecture](assets/system_architecture.jpg)
*(Note: Replace this with your actual architecture diagram or a photo of the finished setup)*

### 🚀 Project Overview
This project is a distributed IoT ecosystem designed to automate room entry, ambient lighting, and legacy hardware with sub-50ms latency. It orchestrates two microcontrollers (ESP32 & ESP8266) to create a **synchronized environment**—managing physical access via a custom Active IR Tripwire while simultaneously modernizing non-smart devices (RGB Strips) into a cohesive smart home node.

Unlike standard smart plugs, this system utilizes a **custom IR "Heartbeat" protocol** for noise-immune presence detection and implements a **non-blocking architecture** to ensure instant physical response times even during heavy Wi-Fi activity.

**Status:** ✅ Active / Stable v2.1

### ⚙️ Key Features
* **Dual-Node Sync:** Distributed logic between Door (Receiver) and Desk (Emitter) allows the environment to react instantly to human presence.
* **Smart Extension Hub:** Pivoted from controlling a single bulb to retrofitting a mains extension cord. This transforms the unit into a "Smart Hub," allowing modular control of multiple devices (Main Lamp, 5V RGB Drivers, Chargers) simultaneously.
* **Legacy Hardware Modernization:** Automated "State Recovery" logic overrides the chaotic default flash modes of cheap RGB strips on boot, restoring user presets (Purple/Solid) without physical remotes.
* **Hardware-Level PWM:** Offloaded 38kHz IR signal generation to the ESP8266 hardware timer, decoupling signal stability from CPU load (Wi-Fi/Blynk tasks).
* **Anti-Jitter Algorithm:** Implemented a "Smart Latch" timer to distinguish between a lingering person and a re-entry event.

---

### 🔧 Engineering Journey & Technical Challenges

#### 1. The Sensor Evolution (PIR vs. Active IR)
The project began as a feasibility study to understand relay logic, initially intending to use Passive Infrared (PIR) sensors. However, testing revealed significant latency and broad detection zones unsuitable for a precise "Tripwire."

I pivoted to an **Active IR Tripwire** mechanism:
* **Problem:** Standard photodiodes were hypersensitive to ambient noise (sunlight), causing false triggers.
* **Solution:** Upgraded to **TSOP-style receivers** (VS1838B) with internal gain control and band-pass filters.

#### 1.5 The Cloud Latency Lesson (Cloud-to-Edge Pivot)
Before committing to a hardware-based relay solution, the initial prototype relied on a "No-Code" cloud integration which served as a critical lesson in IoT bottlenecks.

* **The Architecture:** `ESP32 -> Webhooks -> Make.com -> Samsung SmartThings -> Cloud Server -> Smart Bulb`.
* **The Failure:** This convoluted data path crossed four independent servers, introducing a **3-5 second latency** between the physical beam break and light activation.
* **The Dependency Risk:** The system was entirely reliant on external uptime. If the third-party webhooks or bulb proprietary clouds were down, the physical room automation ceased to function.
* **The Solution:** I pivoted to **Edge Computing**, moving the automation logic directly onto the ESP32. By retrofitting a physical relay into the hardware stack, I bypassed all external server dependencies.
* **The Result:** Reduced trigger latency from ~5000ms to **<50ms (a 99.9% improvement)**, ensuring 100% reliability regardless of internet connectivity.



[Image of Cloud vs Edge Computing diagram for IoT]


#### 2. The "Continuous Signal" Fallacy
A major hurdle involved the TSOP receiver logic.
* **The Bug:** Driving the IR Emitter with a continuous DC signal failed because TSOP receivers are designed to filter out non-modulated signals as "interference."
* **The Fix:** Re-engineered the emitter logic to generate **modulated 38kHz packets**, establishing a stable link that only breaks upon physical obstruction.

#### 3. The "Resource War" (Tripwire vs. RGB Control)
Integrating the Desk Underglow created a critical resource conflict.
* **The Conflict:** The ESP8266 has limited hardware timers. Generating the continuous 38kHz tripwire signal (`analogWrite`) clashed with the precise timing required to send NEC IR codes (`IRremote`) to the LED strip, causing the lights to lag or fail.
* **The Optimization:** Implemented a **"Mutex" (Mutual Exclusion)** strategy. The system briefly pauses the 38kHz carrier wave (~50ms) to transmit LED color commands cleanly, then immediately resumes the tripwire. This ensures reliable lighting control without compromising security.

#### 4. Design Pivot: The "Smart Hub" Architecture
Early iterations focused on a custom enclosure for a single light bulb. I realized this lacked scalability and utility relative to the engineering effort.
* **The Pivot:** I retrofitted a standard AC extension cord by placing the Relay Module inline with the main input Live rail.
* **The Result:** A scalable IoT node that can control any plugged-in device, future-proofing the system for additional peripherals (chargers, heaters, etc.) without hardware redesigns.

![Relay Wiring](assets/relay_wiring_internals.jpg)
*(Note: Upload a photo of your relay soldering/wiring to your assets folder)*

---

### 🛠️ Hardware Architecture
* **Door Node (Receiver):**
    * **Core:** ESP32 DevKit V1
    * **Sensors:** VS1838B IR Receiver (3.3V Logic)
    * **Actuators:** 5V Relay Module (Transistor Buffered)
* **Desk Node (Emitter & Controller):**
    * **Core:** NodeMCU (ESP8266)
    * **Emitters:** 940nm High-Power IR LED (Driven via 2N2222 Transistor on 5V Rail)
    * **Legacy Control:** IR Emitters for RGB Strip control (NEC Protocol)
* **Protocol:** 38kHz Modulated Carrier Wave

### 🔌 Circuit Diagram
![Circuit Diagram](schematics/wiring_diagram.png)
*(Note: Upload your wiring diagram to the schematics folder)*

> The system uses a transistor-buffered relay circuit to protect the ESP32 GPIO pins from inductive flyback voltage.

### 📱 IoT Integration (Blynk)
* **V0:** Main Room Light (Synced with physical beam break)
* **V1:** "Party Mode" Lock (Disables automatic sensors)
* **V2:** Desk RGB Color Selector (Injects NEC Codes)
* **Automation:** When Door Light turns ON -> Desk Underglow syncs state automatically via cloud bridge.

---
### 📜 License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

*Created by Aoun Raza*
