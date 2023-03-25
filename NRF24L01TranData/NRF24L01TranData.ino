#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Define Trig and Echo pin for HC-204:
#define trigPin 2
#define echoPin 3

// Define variables for HC-204:
long duration;
int distanceInCm;

// Define CE and CSN pin for NRF24L01
#define CEpin 4
#define CSNpin 5

//NRF radio setup
RF24 radio(CEpin, CSNpin);
const uint64_t pipe = 0xE8E8F0F0E1LL;

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  radio.begin();
  radio.setChannel(1);
  radio.openWritingPipe(pipe);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}

void loop() {
  distanceInCm = getWaterHeight();
  Serial.println(distanceInCm);
  // bool success = sendData(distanceInCm);
  // if (success) {
  //   Serial.print("Data sent: ");
  //   Serial.println(distanceInCm);
  // } else {
  //   Serial.println("Transmission failed :(");
  // }
}

bool sendData(int data){
  delay(1000);
  return radio.write(&data, sizeof(data));
}

int getWaterHeight(){
  // Clear the trigPin by setting it LOW:
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);

  // Trigger the sensor by setting the trigPin high for 10 microseconds:
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the echoPin, pulseIn() returns the duration (length of the pulse) in microseconds:
  duration = pulseIn(echoPin, HIGH);
  // Calculate the distance:
  distanceInCm = duration * 0.034 / 2;
  
  delay(50);
  return distanceInCm;
}
