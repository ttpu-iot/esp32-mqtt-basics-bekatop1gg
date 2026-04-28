// - RED LED - D26
// - Green LED - D27
// - Blue LED - D14
// - Yellow LED - D12
// - Button - D25
// - Light sensor - D33

/**************************************
 * LAB 3 - EXERCISE 2
 * MQTT Subscribe + LED Control
 **************************************/

#include "Arduino.h"
#include "WiFi.h"
#include <ArduinoJson.h>
#include "PubSubClient.h"

// WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// MQTT
const char* mqtt_server = "mqtt.iotserver.uz";
const char* mqtt_user = "userTTPU";
const char* mqtt_pass = "mqttpass";
const char* client_id = "bekzod_lab3";

// Topics
const char* topic_red    = "ttpu/iot/bekzod/led/red";
const char* topic_green  = "ttpu/iot/bekzod/led/green";
const char* topic_blue   = "ttpu/iot/bekzod/led/blue";
const char* topic_yellow = "ttpu/iot/bekzod/led/yellow";

// Pins
#define RED_LED 26
#define GREEN_LED 27
#define BLUE_LED 14
#define YELLOW_LED 12

WiFiClient espClient;
PubSubClient client(espClient);


/*************************
 * WiFi
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
 * MQTT reconnect
 */
void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect(client_id, mqtt_user, mqtt_pass)) {
      Serial.println("connected!");

      // Subscribe to all LED topics
      client.subscribe(topic_red);
      client.subscribe(topic_green);
      client.subscribe(topic_blue);
      client.subscribe(topic_yellow);
      Serial.println("Subscribed to LED topics");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying...");
      delay(1000);
    }
  }
}


/*************************
 * MQTT Callback
 */
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("[MQTT] Received on ");
  Serial.print(topic);
  Serial.print(": ");

  // Print received message
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Parse JSON directly from payload + length (handles trailing garbage)
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload, length);

  if (error) {
    Serial.print("[ERROR] Invalid JSON: ");
    Serial.println(error.c_str());
    return;
  }

  const char* state = doc["state"];
  if (state == nullptr) {
    Serial.println("[ERROR] No 'state' field");
    return;
  }

  int pin = -1;
  String ledName = "";

  if (String(topic) == topic_red) {
    pin = RED_LED;
    ledName = "Red";
  } else if (String(topic) == topic_green) {
    pin = GREEN_LED;
    ledName = "Green";
  } else if (String(topic) == topic_blue) {
    pin = BLUE_LED;
    ledName = "Blue";
  } else if (String(topic) == topic_yellow) {
    pin = YELLOW_LED;
    ledName = "Yellow";
  }

  if (pin != -1) {
    if (strcmp(state, "ON") == 0) {
      digitalWrite(pin, HIGH);
    } else if (strcmp(state, "OFF") == 0) {
      digitalWrite(pin, LOW);
    }

    Serial.print("[LED] ");
    Serial.print(ledName);
    Serial.print(" -> ");
    Serial.println(state);
  }
}

/*************************
 * SETUP
 */
void setup() {
  Serial.begin(115200);

  // LED setup
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}


/*************************
 * LOOP
 */
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}