#include <SPI.h>
#include <LoRa.h>

#define SS 16
#define RST 14
#define DIO0 26
#define TRIGPIN 5
#define ECHOPIN 18

byte LocalAddress = 0x03;  // Address of this device (Sensor Node 2)
byte Destination = 0x01;   // Destination address (Master Node)

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(TRIGPIN, OUTPUT);
  pinMode(ECHOPIN, INPUT);

  Serial.println("LoRa Sender Node 2");

  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

long readUltrasonicDistance() {
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN, LOW);
  long duration = pulseIn(ECHOPIN, HIGH);
  long distance = (duration / 2) / 29.1;
  return distance;
}

void loop() {
  long distance = readUltrasonicDistance();
  String message = "D:" + String(distance);

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
