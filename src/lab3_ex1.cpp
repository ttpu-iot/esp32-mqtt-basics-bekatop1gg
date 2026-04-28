// - RED LED - D26
// - Green LED - D27
// - Blue LED - D14
// - Yellow LED - D12

// - Button (Active high) - D25
// - Light sensor (analog) - D33

// - LCD I2C - SDA: D21
// - LCD I2C - SCL: D22

/**************************************
 * LAB 3 - EXERCISE 1:
 * Publish sensor data every 5 seconds
 * + button events on change
 **************************************/

#include "Arduino.h"
#include "WiFi.h"
#include "PubSubClient.h"
#include <ArduinoJson.h>
#include <time.h>

// WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// MQTT
const char* mqtt_server = "mqtt.iotserver.uz";
const char* mqtt_user = "userTTPU";
const char* mqtt_pass = "mqttpass";
const char* client_id = "bekzod_lab3";

// Topics
const char* topic_light = "ttpu/iot/bekzod/sensors/light";
const char* topic_button = "ttpu/iot/bekzod/events/button";

// Pins
#define RED_LED 26
#define GREEN_LED 27
#define BLUE_LED 14
#define YELLOW_LED 12
#define BUTTON_PIN 25
#define LIGHT_PIN 33

WiFiClient espClient;
PubSubClient client(espClient);

// Timing
unsigned long lastPublish = 0;

// Button debounce
int lastButtonState = LOW;
int buttonState = LOW;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

/*************************
 * WiFi Connection
 */
void setup_wifi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

/*************************
 * MQTT Connection
 */
void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");

    if (client.connect(client_id, mqtt_user, mqtt_pass)) {
      Serial.println("connected!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying...");
      delay(1000);
    }
  }
}

/*************************
 * SETUP
 */
void setup() {
  Serial.begin(115200);

  // Pins
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  // Turn off LEDs
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);

  setup_wifi();

  // Time (for timestamp)
  configTime(0, 0, "pool.ntp.org");

  client.setServer(mqtt_server, 1883);
}

/*************************
 * LOOP
 */
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // ===== Publish Light every 5 seconds =====
  if (millis() - lastPublish > 5000) {
    lastPublish = millis();

    int lightValue = analogRead(LIGHT_PIN);
    time_t now = time(nullptr);

    JsonDocument doc;
    doc["light"] = lightValue;
    doc["timestamp"] = now;

    char buffer[128];
    serializeJson(doc, buffer);

    client.publish(topic_light, buffer);

    Serial.print("[PUBLISH] ");
    Serial.print(topic_light);
    Serial.print(" -> ");
    Serial.println(buffer);
  }

  // ===== Button Handling =====
  int reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {

    if (reading != buttonState) {
      buttonState = reading;

      JsonDocument doc;
      doc["event"] = buttonState ? "PRESSED" : "RELEASED";
      doc["timestamp"] = time(nullptr);

      char buffer[128];
      serializeJson(doc, buffer);

      client.publish(topic_button, buffer);

      Serial.print("[BUTTON] ");
      Serial.print(topic_button);
      Serial.print(" -> ");
      Serial.println(buffer);
    }
  }

  lastButtonState = reading;
}