#include <Arduino_LSM6DS3.h>   // Built-in IMU
#include <TinyGPSPlus.h>       // GPS library
#include <WiFiNINA.h>          // Wi-Fi (built into Nano 33 IoT)
#include <PubSubClient.h>      // MQTT
#include "wiring_private.h"    // For second serial port (SERCOM5)

// -------------------- GPS Setup (pins D10/D11) --------------------
Uart GpsSerial(&sercom5, 10, 11, SERCOM_RX_PAD_3, UART_TX_PAD_0);
TinyGPSPlus gps;

// -------------------- Wi-Fi & MQTT Setup --------------------
const char* ssid = "wifi name";          
const char* password = "wifi pass";  

const char* mqttServer = "RPI IP Address"; 
const int   mqttPort   = 1883;
const char* mqttTopic  = "accidents/alerts/bike_001";

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

// -------------------- Crash Detection Thresholds --------------------
const float ACCEL_G_THRESHOLD = 2.5;
const float GYRO_DPS_THRESHOLD = 120.0;

// -------------------- Accident Alert Function --------------------
void handle_accident_alert() {
  Serial.println("Accident detected! Preparing to send alert...");

  // Get GPS fix
  if (!gps.location.isValid()) {
    Serial.println("WARNING: No valid GPS fix. Using last known location.");
  }

  float lat = gps.location.lat();
  float lng = gps.location.lng();

  // Ensure Wi-Fi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to Wi-Fi...");
    WiFi.begin(ssid, password);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
      delay(500);
      Serial.print(".");
    }
    Serial.println(WiFi.status() == WL_CONNECTED ? " Connected!" : " Failed!");
  }

  // Connect to MQTT broker
  Serial.print("Connecting to MQTT...");
  if (!mqtt.connected()) {
    if (mqtt.connect("BikeDetector_001")) Serial.println(" Connected!");
    else {
      Serial.println(" Failed!");
      return;
    }
  }

  // Create payload
  String payload = "{\"device_id\":\"BikeDetector_001\",";
  payload += "\"latitude\":" + String(lat, 6) + ",";
  payload += "\"longitude\":" + String(lng, 6) + ",";
  payload += "\"severity\":\"High\"}";

  // Publish to MQTT
  Serial.print("Publishing to MQTT: ");
  if (mqtt.publish(mqttTopic, payload.c_str())) Serial.println("Success!");
  else Serial.println("Failed to publish!");

  mqtt.disconnect();
  Serial.println("Alert sent and MQTT disconnected.");
}

// -------------------- Setup --------------------
void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Starting setup...");

  // IMU initialization
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }
  Serial.println("IMU ready.");

  // GPS initialization (D10/D11)
  pinPeripheral(10, PIO_SERCOM_ALT);
  pinPeripheral(11, PIO_SERCOM_ALT);
  GpsSerial.begin(9600);
  Serial.println("GPS started on pins 10/11.");

  // Wi-Fi setup
  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.status() == WL_CONNECTED ? " Connected!" : " Failed!");

  // MQTT setup
  mqtt.setServer(mqttServer, mqttPort);
  Serial.println("Setup complete. Monitoring...");
}

// -------------------- Loop --------------------
void loop() {
  // Process GPS data
  while (GpsSerial.available()) {
    gps.encode(GpsSerial.read());
  }

  // IMU data
  float x, y, z;
  float gx, gy, gz;

  if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
    IMU.readAcceleration(x, y, z);
    IMU.readGyroscope(gx, gy, gz);

    // Calculate total acceleration and rotation magnitudes
    float total_acc = sqrt(x * x + y * y + z * z);
    float total_rot = sqrt(gx * gx + gy * gy + gz * gz);

    // Detect a crash event
    if (total_acc > ACCEL_G_THRESHOLD && total_rot > GYRO_DPS_THRESHOLD) {
      Serial.println("\n*** CRASH DETECTED! ***");
      handle_accident_alert();
      delay(15000); // prevent multiple triggers
    }
  }

  delay(10); // small delay to stabilize readings
}

