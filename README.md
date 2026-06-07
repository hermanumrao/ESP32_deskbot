# ESP32 DeskBot 🤖

A tiny, autonomous desk robot built in ~15 hours over two days. It moves around your desk, avoids obstacles with a Time-of-Flight sensor, streams video from an onboard camera, and can be controlled from your phone via a locally hosted web server — or remotely via a cloud-connected app.

---

## Features

- **Obstacle Avoidance** — VL53L7CX ToF sensor detects objects and steers clear autonomously
- **Live Camera Feed** — Onboard camera streams video for monitoring or remote navigation
- **IMU Integration** — Inertial measurement unit for orientation-aware motion control
- **Web Control Interface** — Locally hosted web server lets you drive the bot from any phone browser on the same Wi-Fi network
- **Cloud App Control** — Custom application connects over the cloud for remote control from anywhere
- **Compact Form Factor** — Designed to live on your desk without getting in the way (much)

---

## Hardware

| Component | Description |
|---|---|
| **MCU** | ESP32 Devkit |
| **ToF Sensor** | VL53L7CX (obstacle detection) |
| **Camera** | ESP32 cam module |
| **IMU** | DF-robt 9-DOF IMU module |
| **Motor Driver** | Cytron MDD3A motor driver |
| **Motor** | N20 motors x2 |

---

## Getting Started

### Prerequisites

- [Arduino IDE](https://www.arduino.cc/en/software) or [PlatformIO](https://platformio.org/)
- ESP32 board support installed


### Flashing the Firmware

1. Clone the repository:
   ```bash
   git clone https://github.com/hermanumrao/ESP32_deskbot.git
   cd ESP32_deskbot
   ```

2. Open the project in Arduino IDE or PlatformIO.

3. Set your Wi-Fi credentials in the files:
   ```cpp
   #define WIFI_SSID "your_network"
   #define WIFI_PASSWORD "your_password"
   ```

4. Select your ESP32 board and flash.

### Web Control (Local)

1. Power on the DeskBot.
2. Open the Serial Monitor at `115200` baud and press Reset on the ESP32.
3. Note the IP address printed once it connects to Wi-Fi.
4. Open a browser on your phone (on the **same Wi-Fi network**) and navigate to:
   ```
   http://<IP_ADDRESS>
   ```
5. Use the on-screen controls to drive the bot.

> ⚠️ **Do NOT connect the battery and USB cable to the ESP32 at the same time.**



---

## Libraries Used

- `VL53L7CX` — ST ToF sensor driver
- `ESP32 Camera` — camera streaming
- `MPU/IMU library` — orientation sensing
- `WebServer` — local HTTP control interface

---

## Project Notes

This was a **two-day, ~15-hour build** focused on getting a functional, sensor-equipped moving robot deployed quickly. The VL53L7CX is a particularly capable ToF sensor — it outputs a full depth map rather than just a single distance reading, which opens the door to more sophisticated navigation in future iterations.

**Potential next steps:**
- Cloud based SLAM (Simultaneous Localization and Mapping)
- Swarm behavior with multiple bots
- cloud app with live camera feed
- Battery life optimization


---

## Author

**Herman Umrao** — [@hermanumrao](https://github.com/hermanumrao)
