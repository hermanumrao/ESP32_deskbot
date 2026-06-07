#include <WiFi.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <Wire.h>

// Sensor Libraries
#include "SparkFun_ISM330DHCX.h"
#include "SparkFun_MMC5983MA_Arduino_Library.h"
#include <SparkFun_VL53L5CX_Library.h>

// WebUI Header
#include "index.h"

// --- MOTOR PINS ---
const int M1A = 25; // Motor 1 (Left) PWM/DIR
const int M1B = 26; // Motor 1
const int M2A = 27; // Motor 2 (Right) PWM/DIR
const int M2B = 14; // Motor 2

// --- WIFI & NETWORKING ---
const char* ssid = "miku";
const char* password = "12345678";

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
WiFiUDP udp;
const int udpPort = 8888;
IPAddress broadcastIP(255, 255, 255, 255);

// --- SENSORS ---
SparkFun_ISM330DHCX myISM;
SFE_MMC5983MA myMag;
SparkFun_VL53L5CX myImager;
VL53L5CX_ResultsData measurementData;

sfe_ism_data_t accelData;
sfe_ism_data_t gyroData;

int imageWidth = 8;
unsigned long lastFuzzyAction = 0;
unsigned long lastWsDataSend = 0;
String lastImuPacket = "";
String lastLidarPacket = "";

enum ControlMode { RC_MODE, FUZZY_MODE };
ControlMode currentMode = RC_MODE;

// --- MOTOR FUNCTION ---
void setMotors(int leftSpeed, int rightSpeed) {
  // Constrain speeds
  leftSpeed = constrain(leftSpeed, -255, 255);
  rightSpeed = constrain(rightSpeed, -255, 255);

  if (leftSpeed > 0) {
    analogWrite(M1A, leftSpeed);
    analogWrite(M1B, 0);
  } else {
    analogWrite(M1A, 0);
    analogWrite(M1B, -leftSpeed);
  }

  if (rightSpeed > 0) {
    analogWrite(M2A, rightSpeed);
    analogWrite(M2B, 0);
  } else {
    analogWrite(M2A, 0);
    analogWrite(M2B, -rightSpeed);
  }
}

// --- WEBSOCKET EVENT ---
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    String msg = (char*)payload;
    if (msg == "RC") {
      currentMode = RC_MODE;
      setMotors(0, 0);
      Serial.println("Mode: RC");
    } else if (msg == "FUZZY") {
      currentMode = FUZZY_MODE;
      setMotors(0, 0);
      Serial.println("Mode: FUZZY");
    } else if (msg.startsWith("J_")) {
      if (currentMode == RC_MODE) {
        int firstUnder = msg.indexOf('_');
        int secondUnder = msg.indexOf('_', firstUnder + 1);
        if (firstUnder != -1 && secondUnder != -1) {
          int jX = msg.substring(firstUnder + 1, secondUnder).toInt();
          int jY = msg.substring(secondUnder + 1).toInt();
          
          // jX, jY: -100 to 100
          // Mapping logic: arcade drive
          float scale = 255.0 / 100.0;
          int drive = jY * scale;  // Forward/back
          int steer = jX * scale;  // Left/right map
          
          int speedL = drive + steer;
          int speedR = drive - steer;
          
          setMotors(speedL, speedR);
        }
      }
    }
  }
}

// --- SETUP ---
void setup() {
  Serial.begin(115200);

  // Motor Init
  pinMode(M1A, OUTPUT); pinMode(M1B, OUTPUT);
  pinMode(M2A, OUTPUT); pinMode(M2B, OUTPUT);
  setMotors(0, 0);

  // Connect WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500); Serial.print("."); attempts++;
  }
  Serial.println("\nWiFi connected! IP:");
  Serial.println(WiFi.localIP());
  
  // Wait a bit to let subnet populate if needed
  broadcastIP = WiFi.broadcastIP();
  
  // Setup Server
  server.on("/", []() {
    server.send(200, "text/html", index_html);
  });
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  // Initialize Sensors
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH); // Enable LiDAR LPn
  delay(10);             // Let it power up

  Wire.begin(21, 22); // Explicitly use 21/22 for standard ESP32 WROOM
  Wire.setClock(400000);

  Serial.println("Starting IMU/MAG/LIDAR...");
  
  if (!myISM.begin()) Serial.println("ISM330DHCX not detected!");
  else {
    myISM.deviceReset();
    while(!myISM.getDeviceReset()) delay(1);
    myISM.setDeviceConfig();
    myISM.setBlockDataUpdate();
    myISM.setAccelDataRate(ISM_XL_ODR_104Hz);
    myISM.setAccelFullScale(ISM_4g);
    myISM.setGyroDataRate(ISM_GY_ODR_104Hz);
    myISM.setGyroFullScale(ISM_500dps);
    myISM.setAccelFilterLP2();
    myISM.setAccelSlopeFilter(ISM_LP_ODR_DIV_100);
    myISM.setGyroFilterLP1();
    myISM.setGyroLP1Bandwidth(ISM_MEDIUM);
  }

  if (!myMag.begin()) Serial.println("MMC5983MA not detected!");
  else myMag.softReset();

  if (!myImager.begin()) Serial.println("VL53L5CX not detected!");
  else {
    myImager.setResolution(8*8);
    myImager.startRanging();
  }
}

// --- LOOP ---
void loop() {
  webSocket.loop();
  server.handleClient();
  
  // 1. Check and broadcast IMU data
  if (myISM.checkStatus()) {
    myISM.getAccel(&accelData);
    myISM.getGyro(&gyroData);
    
    // Check Mag
    uint32_t mx = myMag.getMeasurementX();
    uint32_t my = myMag.getMeasurementY();
    uint32_t mz = myMag.getMeasurementZ();
    float magX = ((float)mx - 131072.0) / 131072.0 * 8.0;
    float magY = ((float)my - 131072.0) / 131072.0 * 8.0;
    float magZ = ((float)mz - 131072.0) / 131072.0 * 8.0;
    
    float heading = atan2(magY, magX) * 180.0 / PI;
    if (heading < 0) heading += 360;
    
    char msg[256];
    snprintf(msg, sizeof(msg), "IMU,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.3f,%.3f,%.3f,%.1f",
             accelData.xData, accelData.yData, accelData.zData,
             gyroData.xData, gyroData.yData, gyroData.zData,
             magX, magY, magZ, heading);
             
    lastImuPacket = String(msg);

    udp.beginPacket(broadcastIP, udpPort);
    udp.print(msg);
    udp.endPacket();
  }

  // 2. Check and broadcast LiDAR data + Fuzzy Logic
  if (myImager.isDataReady()) {
    if (myImager.getRangingData(&measurementData)) {
      
      // Build UDP Packet
      String lidarPacket = "LIDAR,";
      int middleAvg = 0;
      int midCount = 0;
      
      for(int y=0; y < 8; y++) {
        for(int x=0; x < 8; x++) {
          int distance = measurementData.distance_mm[y*8 + x];
          lidarPacket += String(distance) + ",";
          
          // Fuzzy Logic: look at middle cells (e.g. x=3,4 and y=3,4)
          // Adjust based on sensor orientation, assuming roughly center for obstacle detection.
          if ((x == 3 || x == 4) && (y >= 2 && y <= 5)) { 
            middleAvg += distance;
            midCount++;
          }
        }
      }
      lastLidarPacket = lidarPacket;
      
      udp.beginPacket(broadcastIP, udpPort);
      udp.print(lidarPacket);
      udp.endPacket();
      
      // Execute Fuzzy Logic if active
      if (currentMode == FUZZY_MODE) {
        if (midCount > 0) {
          middleAvg /= midCount; // average distance in front
          
          // Basic obstacle avoidance
          if (middleAvg < 300) {
            // Obstacle too close! Turn sharply or back up
            if (millis() - lastFuzzyAction > 500) {
              int r = random(2);
              if(r == 0) setMotors(-150, 150); // Turn left
              else setMotors(150, -150); // Turn right
              lastFuzzyAction = millis();
            }
          } else if (millis() - lastFuzzyAction > 500) {
            // Path is clear, go forward randomly adjusting
            int jitter = random(-30, 30);
            setMotors(120 + jitter, 120 - jitter);
          }
        }
      }
    }
  }

  // 3. Send WebSockets Updates @ 10Hz
  if (millis() - lastWsDataSend > 100) {
    lastWsDataSend = millis();
    if (lastImuPacket.length() > 0) webSocket.broadcastTXT(lastImuPacket);
    if (lastLidarPacket.length() > 0) webSocket.broadcastTXT(lastLidarPacket);
  }
}
