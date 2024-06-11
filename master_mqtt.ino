#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define SS 16
#define RST 14
#define DIO0 26
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define WIFI_SSID "eedlab"
#define WIFI_PASSWORD "eedlab@1"
#define MQTT_SERVER "broker.hivemq.com"
#define MQTT_PORT 1883
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

WiFiClient espClient;
PubSubClient client(espClient);

byte LocalAddress = 0x01; // Address of this device (Master Node)
String Incoming = "";

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
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
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Receiver");

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.display();
  delay(2000);
  display.clearDisplay();

  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  setup_wifi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);
}


void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    byte recipient = LoRa.read();
    byte sender = LoRa.read();
    byte incomingLength = LoRa.read();

    Incoming = "";
    while (LoRa.available()) {
      Incoming += (char)LoRa.read();
    }

    if (recipient != LocalAddress) return;

    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, Incoming);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    Serial.print("Received packet: '");
    Serial.print(Incoming);
    Serial.print("' from 0x");
    Serial.println(sender, HEX);

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);

    if (sender == 0x02) {
      display.print("Node 1 (DHT11): ");
      display.print("Temp: ");
      display.print(doc["temperature"].as<float>());
      display.print("C ");
      display.print("Hum: ");
      display.print(doc["humidity"].as<float>());
      display.println("%");

      if (client.connected()) {
        char jsonBuffer[512];
        serializeJson(doc, jsonBuffer);
        client.publish("sensor/node1", jsonBuffer);
      }
    } else if (sender == 0x03) {
      display.print("Node 2 (Ultrasonic): ");
      display.print("Dist: ");
      display.print(doc["distance"].as<long>());
      display.println("cm");

      if (client.connected()) {
        char jsonBuffer[512];
        serializeJson(doc, jsonBuffer);
        client.publish("sensor/node2", jsonBuffer);
        Serial.print("ok\n");
      }
    }

    display.display();
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
