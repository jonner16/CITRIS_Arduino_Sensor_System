#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CEpin 7 //D7
#define CSNpin 8 //D8

RF24 radio(CEpin, CSNpin);
const uint64_t pipe = 0xE8E8F0F0E1LL;
int distanceInCm = -1;

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.setChannel(1);
  radio.openReadingPipe(1, pipe);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    radio.read(&distanceInCm, sizeof(distanceInCm));
    if (distanceInCm != -1) {
      Serial.println("Reception successful!");
      Serial.print("Distance received: ");
      Serial.println(distanceInCm);
      distanceInCm = -1;
    } else {
      Serial.println("Reception failed :(");
    }
  }
}