#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>

#define SS 16
#define RST 14
#define DIO0 26
#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

byte LocalAddress = 0x02;  // Address of this device (Sensor Node 1)
byte Destination = 0x01;   // Destination address (Master Node)

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Sender Node 1");
  dht.begin();

  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  String message = "T:" + String(t) + " H:" + String(h);

  LoRa.beginPacket();
  LoRa.write(Destination);
  LoRa.write(LocalAddress);
  LoRa.write(message.length());
  LoRa.print(message);
  LoRa.endPacket();

  Serial.print("Sent packet: ");
  Serial.println(message);

  delay(5000);
}
