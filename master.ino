#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SS 16
#define RST 14
#define DIO0 26
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

byte LocalAddress = 0x01; // Address of this device (Master Node)
String Incoming = "";

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
      display.println(Incoming);
    } else if (sender == 0x03) {
      display.print("Node 2 (Ultrasonic): ");
      display.println(Incoming);
    }

    display.display();
  }
}
