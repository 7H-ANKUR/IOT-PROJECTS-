#include <WiFi.h>
#include <DHT.h>
#include <PubSubClient.h>

// Pin definitions
#define DHTPIN 14  // DHT sensor pin
#define DHTTYPE DHT22
#define PIRPIN 33  // PIR sensor pin
#define LDRPIN 34  // LDR sensor pin (analog input)
#define LIGHT_PIN 26
#define FAN_PIN 27
#define AC_PIN 25

DHT dht(DHTPIN, DHTTYPE);

// WiFi and MQTT settings
const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqtt_server = "demo.thingsboard.io";
const char* token = "nh1WnzuKTRCjMP6gxVpf";
WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32_SmartRoom", token, NULL)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);

  pinMode(PIRPIN, INPUT);
  pinMode(LDRPIN, INPUT); // No need for analogRead initialization
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(AC_PIN, OUTPUT);

  digitalWrite(LIGHT_PIN, LOW);  // Initialize all relays as OFF
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(AC_PIN, LOW);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read sensor values
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int pirState = digitalRead(PIRPIN); // Check if motion is detected
  int ldrValue = analogRead(LDRPIN); // Measure light intensity

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(1000);
    return;
  }

  // Logic for controlling appliances
  if (pirState == HIGH) { // Motion detected
    if (ldrValue < 500) { // Low ambient light
      digitalWrite(LIGHT_PIN, HIGH); // Turn on light
    } else {
      digitalWrite(LIGHT_PIN, LOW); // Turn off light
    }
    digitalWrite(FAN_PIN, HIGH); // Turn on fan
    digitalWrite(AC_PIN, HIGH); // Turn on A.C.
  } else { // No motion
    digitalWrite(LIGHT_PIN, LOW); // Turn off light
    digitalWrite(FAN_PIN, LOW); // Turn off fan
    digitalWrite(AC_PIN, LOW); // Turn off A.C.
  }

  // Prepare payload
  String payload = "{\"temperature\":";
  payload += t;
  payload += ", \"humidity\":";
  payload += h;
  payload += ", \"pir_state\":";
  payload += pirState;
  payload += ", \"ldr_value\":";
  payload += ldrValue;
  payload += ", \"light_state\":";
  payload += digitalRead(LIGHT_PIN);
  payload += ", \"fan_state\":";
  payload += digitalRead(FAN_PIN);
  payload += ", \"ac_state\":";
  payload += digitalRead(AC_PIN);
  payload += "}";

  Serial.print("Sending payload: ");
  Serial.println(payload);

  client.publish("v1/devices/me/telemetry", payload.c_str());

  // Print status on Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" °C");
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println(" %");
  Serial.print("PIR state: ");
  Serial.println(pirState ? "Motion Detected" : "No Motion");
  Serial.print("LDR value: ");
  Serial.println(ldrValue);
  Serial.print("Light state: ");
  Serial.println(digitalRead(LIGHT_PIN) ? "ON" : "OFF");
  Serial.print("Fan state: ");
  Serial.println(digitalRead(FAN_PIN) ? "ON" : "OFF");
  Serial.print("A.C. state: ");
  Serial.println(digitalRead(AC_PIN) ? "ON" : "OFF");

  delay(1000);
}
