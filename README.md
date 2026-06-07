# ESP32 DeskBot 🤖

A tiny, autonomous desk robot built in ~15 hours over two days. It moves around your desk, avoids obstacles with a Time-of-Flight sensor, streams video from an onboard camera, and can be controlled from your phone via a locally hosted web server — or remotely via a cloud-connected app.

---

## Features

- **Obstacle Avoidance (Fuzzy Mode)** — 8×8 ToF depth map drives a fuzzy logic controller that steers around objects autonomously
- **RC Mode** — Joystick control from any phone browser on the same Wi-Fi network
- **Real-time Sensor Broadcast** — IMU, magnetometer, and LiDAR data broadcast over UDP at full sensor rate
- **WebSocket Live Dashboard** — Sensor data pushed to the web UI at 10 Hz for live visualization
- **9-DOF IMU + Compass** — ISM330DHCX (accel/gyro) + MMC5983MA (magnetometer) with computed heading
- **Compact Form Factor** — Designed to live on your desk without getting in the way (much)


---

## Hardware

| Component | Description |
|---|---|
| **MCU** | ESP32 (WROOM) |
| **ToF Sensor** | VL53L5CX | 8×8 depth map, obstacle detection |
| **Camera** | ESP32 cam module |
| **IMU** | SparkFun ISM330DHCX | Accelerometer + Gyroscope |
| **Magnetometer** | SparkFun MMC5983MA | Compass heading |
| **Motor Driver** | Cytron MDD3A motor driver |
| **Motor** | N20 motors x2 |

---

## Communication & Data Broadcasting

The DeskBot runs three parallel communication channels simultaneously:

### 1. HTTP Web Server — Port 80
Serves the built-in control UI (`index.h`) to any browser on the local network. No app install needed — just open `http://<IP_ADDRESS>` on your phone.

### 2. WebSocket Server — Port 81
Bidirectional real-time channel between the web UI and the ESP32.

**Incoming commands (phone → bot):**

| Message | Action |
|---|---|
| `RC` | Switch to manual joystick control |
| `FUZZY` | Switch to autonomous obstacle avoidance |
| `J_<x>_<y>` | Joystick values (−100 to 100 each), arcade-drive mixed to left/right motor speeds |

**Outgoing data (bot → phone):**

Sensor packets are broadcast to all connected WebSocket clients at **10 Hz**:

- `IMU,ax,ay,az,gx,gy,gz,mx,my,mz,heading` — accel (g), gyro (dps), mag (Gauss), compass heading (°)
- `LIDAR,d0,d1,...,d63` — 64 distance values (mm) from the 8×8 ToF grid

### 3. UDP Broadcast — Port 8888
Every sensor reading is also broadcast as a UDP datagram to `255.255.255.255:8888` (subnet broadcast), allowing any application on the local network — or a cloud relay — to receive raw sensor data without needing a WebSocket connection.

- **IMU packet** — sent on every IMU data-ready interrupt (~104 Hz)
- **LiDAR packet** — sent on every ranging cycle

This is what the cloud app listens to for remote monitoring and control.

```
┌─────────────────────────────────────────────────────┐
│                      ESP32                          │
│                                                     │
│  Sensors          Processing          Network       │
│  ─────────        ──────────          ───────       │
│  VL53L5CX  ──►  Fuzzy Logic  ──►  WebSocket :81    │
│  ISM330DHCX ──► Motor Control ──►  HTTP     :80    │
│  MMC5983MA  ──► Heading Calc  ──►  UDP Bcast :8888 │
│                                                     │
└─────────────────────────────────────────────────────┘
         ▲ WebSocket commands (RC / FUZZY / J_x_y)
         ▼ UDP datagrams + WebSocket pushes (IMU, LIDAR)

    Phone Browser          Cloud App
    (same Wi-Fi)        (UDP listener)
```


---


## Modes of Operation

### RC Mode
Joystick X/Y values sent from the web UI via WebSocket are arcade-drive mixed into independent left/right motor speeds:
```
drive = Y × 2.55    (forward/back)
steer = X × 2.55    (left/right)
leftSpeed  = drive + steer
rightSpeed = drive - steer
```

### Fuzzy Mode
The center 4×4 cells of the 8×8 ToF grid are averaged to estimate forward clearance. A simple fuzzy rule set drives behavior:

| Distance (front avg) | Action |
|---|---|
| `< 300 mm` | Stop and turn randomly left or right |
| `≥ 300 mm` | Drive forward with small random jitter |

Actions are gated to fire at most every 500 ms to prevent oscillation.

---

## Getting Started

### Prerequisites

- [Arduino IDE](https://www.arduino.cc/en/software) or [PlatformIO](https://platformio.org/)
- ESP32 board support package
- Libraries:
  - `SparkFun VL53L5CX`
  - `SparkFun ISM330DHCX`
  - `SparkFun MMC5983MA`
  - `WebSockets` (Markus Sattler)
  - `WebServer` (built-in ESP32)

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

## Project Notes

This was a **two-day, ~15-hour build** focused on getting a fully sensor-equipped, wirelessly controlled robot up and running fast. The VL53L5CX is notably capable — its 8×8 depth grid gives the fuzzy controller spatial context rather than just a single ping distance, making avoidance behavior more robust.

**Potential next steps:**
- SLAM using the full ToF depth map
- Swarm coordination over UDP broadcast
- PID heading lock using the magnetometer
- Cloud relay server for true remote control (not just monitoring)
- Battery life optimization

---

## Author

**Herman Umrao** — [@hermanumrao](https://github.com/hermanumrao)


