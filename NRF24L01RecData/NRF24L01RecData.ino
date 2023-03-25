#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>  // Include the Wire library for I2C communication
#include "DFRobot_OxygenSensor.h"
#include "Seeed_BME280.h"  // Include the BME280 library

#define READ_DELAY 5000    // Define the delay between readings (in milliseconds)

//NRF24L01 setup and pins
#define CE_PIN 49   
#define CSN_PIN 48  

RF24 radio(CE_PIN, CSN_PIN);
const uint64_t pipe = 0xE8E8F0F0E1LL;

// Define variables for HC-204:
int heightInCm;
int tankHeight[4];
long duration;
#define TRIG_PIN_0 46
#define ECHO_PIN_0 47

#define TRIG_PIN_1 44
#define ECHO_PIN_1 45

#define TRIG_PIN_2 42
#define ECHO_PIN_2 43

#define TRIG_PIN_3 40
#define ECHO_PIN_3 41

// Define values for Oxygen Sensor
int OxConc;
#define COLLECT_NUMBER 10  // collect number, the collection range is 1-100.
#define Oxygen_IICAddress_0 ADDRESS_3
#define Oxygen_IICAddress_1 ADDRESS_2
#define Oxygen_IICAddress_2 ADDRESS_1
DFRobot_OxygenSensor Oxygen;

// Define values for Air Quality Sensor
int airQuality;
#define AIR_SENSOR_PIN_0 A0  // Define the pin connected to the sensor data pin
#define AIR_SENSOR_PIN_1 A1
#define AIR_SENSOR_PIN_2 A2

// Define values for CO2 Sensor
int CO2Conc;
/************************Hardware Related Macros************************************/
#define MG_PIN (A15)  //define which analog input channel you are going to use
#define BOOL_PIN (4)
#define DC_GAIN (8.5)  //define the DC gain of amplifier

/***********************Software Related Macros************************************/
#define READ_SAMPLE_INTERVAL (50)  //define how many samples you are going to take in normal operation
#define READ_SAMPLE_TIMES (5)      //define the time interval(in milisecond) between each samples in normal operation

/**********************Application Related Macros**********************************/
//These two values differ from sensor to sensor. user should derermine this value.
#define ZERO_POINT_VOLTAGE (0.220)  //define the output of the sensor in volts when the concentration of CO2 is 400PPM
#define REACTION_VOLTGAE (0.030)    //define the voltage drop of the sensor when move the sensor from air into 1000ppm CO2

/*****************************Globals***********************************************/
float CO2Curve[3] = { 2.602, ZERO_POINT_VOLTAGE, (REACTION_VOLTGAE / (2.602 - 3)) };

void setup() {
  Serial.begin(9600);

  Serial.println("Setting up everything.");

  //For HC-204
  Serial.println("Setting up: HC-204");
  pinMode(TRIG_PIN_0, OUTPUT);
  pinMode(ECHO_PIN_0, INPUT);
  pinMode(TRIG_PIN_1, OUTPUT);
  pinMode(ECHO_PIN_1, INPUT);
  pinMode(TRIG_PIN_2, OUTPUT);
  pinMode(ECHO_PIN_2, INPUT);
  pinMode(TRIG_PIN_3, OUTPUT);
  pinMode(ECHO_PIN_3, INPUT);
  
  //For Oxygen Sensor
  Serial.println("Setting up: Oxygen Sensor");
  while (!Oxygen.begin(Oxygen_IICAddress_0)) {
    Serial.print("I2c device 0 number error !");
    delay(1000);
  }
  
  // while (!Oxygen.begin(Oxygen_IICAddress_1)) {
  //   Serial.print("I2c device 1 number error !");
  //   delay(1000);
  // }
  // while (!Oxygen.begin(Oxygen_IICAddress_2)) {
  //   Serial.print("I2c device 2 number error !");
  //   delay(1000);
  // }
  
  //For Air Quality Sensor
  Serial.println("Setting up: AQS");
  Wire.begin();

  //For CO2 Sensor
  Serial.println("Setting up: CO2");
  pinMode(BOOL_PIN, INPUT);      //set pin to input
  digitalWrite(BOOL_PIN, HIGH);  //turn on pullup resistors

  //For NRF24L01
  Serial.println("Setting up: NRF24l01");
  radio.begin();
  radio.setChannel(1);
  radio.openReadingPipe(1, pipe);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();

  Serial.println("Setup Completed!");
}

void loop() {
  // CO2Conc = getCO2Data(); 
  // airQuality = getAirQuality();
  // OxConc = getOxygen();
  // delay(READ_DELAY);
  // // getNRFData();  
  // Serial.print(CO2Conc);
  // Serial.print(",");
  // Serial.print(airQuality);
  // Serial.print(",");
  // Serial.print(OxConc);
  // Serial.print(",");
  // Serial.print(heightInCm);
  // Serial.print("\n");
  
  // tankHeight[0] = getWaterHeight(TRIG_PIN_0, ECHO_PIN_0);
  // tankHeight[1] = getWaterHeight(TRIG_PIN_1, ECHO_PIN_1);
  // tankHeight[2] = getWaterHeight(TRIG_PIN_2, ECHO_PIN_2);
  // tankHeight[3] = getWaterHeight(TRIG_PIN_3, ECHO_PIN_3);
  
  // Serial.print(getWaterHeight(TRIG_PIN_0, ECHO_PIN_0));
  // Serial.print(", ");
  // Serial.print(getWaterHeight(TRIG_PIN_1, ECHO_PIN_1));
  // Serial.print(", ");
  // Serial.print(getWaterHeight(TRIG_PIN_2, ECHO_PIN_2));
  // Serial.print(", ");
  // Serial.println(getWaterHeight(TRIG_PIN_3, ECHO_PIN_3));
}

/*****************************  getCO2Data *********************************************
Input:   None
Output:   An integer value that represents C02 in ppm
Remarks:   Utilizes getMG() and getMGPercentage() 
************************************************************************************/
int getCO2Data() {
  int percentage;
  float volts;

  volts = getMG(MG_PIN);
  percentage = getMGPercentage(volts, CO2Curve);
  // delay(500);
  return (percentage == -1) ? 400 : percentage; // if == -1 that means less than 400 ppm
  // Serial.print( "SEN0159:" );
  // Serial.print(volts);
  // Serial.print( "V           " );
  // Serial.print("CO2:");
  // if (percentage == -1) {
  //     Serial.print( "<400" );
  // } else {
  //     Serial.print(percentage);
  // }

  // Serial.print( "ppm" );
  // Serial.print("\n");

  // if (digitalRead(BOOL_PIN) ){
  //     Serial.print( "=====BOOL is HIGH======" );
  // } else {
  //     Serial.print( "=====BOOL is LOW======" );
  // }

  // Serial.print("\n");
}

/*****************************  getMG *********************************************
Input:   mg_pin - analog channel
Output:  output of SEN-000007
Remarks: This function reads the output of SEN-000007
************************************************************************************/
float getMG(int mg_pin) {
  int i;
  float v = 0;
  for (i = 0; i < READ_SAMPLE_TIMES; i++) {
    v += analogRead(mg_pin);
    delay(READ_SAMPLE_INTERVAL);
  }
  v = (v / READ_SAMPLE_TIMES) * 5 / 1024;
  return v;
}

/*****************************  getMGPercentage **********************************
Input:   volts   - SEN-000007 output measured in volts
         pcurve  - pointer to the curve of the target gas
Output:  ppm of the target gas
Remarks: By using the slope and a point of the line. The x(logarithmic value of ppm)
         of the line could be derived if y(MG-811 output) is provided. As it is a
         logarithmic coordinate, power of 10 is used to convert the result to non-logarithmic
         value.
************************************************************************************/
int getMGPercentage(float volts, float *pcurve) {
  if ((volts / DC_GAIN) >= ZERO_POINT_VOLTAGE) {
    return -1;
  } else {
    return pow(10, ((volts / DC_GAIN) - pcurve[1]) / pcurve[2] + pcurve[0]);
  }
}

/*****************************  getAirQuality **********************************
Input:   None
Output:   analog read of air sensor
Remarks:   This is a simple analog reading.
************************************************************************************/
int getAirQuality() {
  int airQuality = analogRead(AIR_SENSOR_PIN_0);  // Read the analog value from the sensor pin              // Print the value to the Serial Monitor
  return airQuality;
  // Serial.print("Air quality: ");
  // Serial.println(airQuality);
}

/*****************************  getOxygen **********************************
Input:   None
Output:   An integer which represents an analog read of air sensor
Remarks:   This is calling the getOxygenData function from the DFRobot library.
************************************************************************************/
int getOxygen() {
  float oxygenData = Oxygen.getOxygenData(COLLECT_NUMBER);
  // delay(1000);
  return oxygenData;
  // Serial.print(" Oxygen concentration is ");
  // Serial.print(oxygenData);
  // Serial.println(" %vol");
}

/*****************************  getWaterHeight **********************************
Input:   None
Output:   An integer which represents the digital read + some math of the water heigh sensor (HC-204)
Remarks:   Turns a pin on and off in short intervals,
           measuring the response time of that pulse, and translates that time into a distance value in cm. 
************************************************************************************/
int getWaterHeight(uint8_t TRIG, uint8_t ECHO) {
  // Clear the TRIG_PIN by setting it LOW:
  digitalWrite(TRIG, LOW);
  delayMicroseconds(5);

  // Trigger the sensor by setting the TRIG_PIN high for 10 microseconds:
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  // Read the ECHO_PIN, pulseIn() returns the duration (length of the pulse) in microseconds:
  duration = pulseIn(ECHO, HIGH);

  // Calculate the distance:
  heightInCm = duration * 0.034 / 2;
  delay(50);
  return heightInCm;
}

/*****************************  getNRFData **********************************
Input:   None
Output:   None
Remarks:   I dont really get it tbh but this reads data from any NRF24L01
           module on the above specified radio channel.
************************************************************************************/
void getNRFData() {
  if (radio.available()) {
    radio.read(&heightInCm, sizeof(heightInCm));
    if (heightInCm != -1) {
      Serial.println("Reception successful!");
      Serial.print("Distance received: ");
      Serial.println(heightInCm);
      // heightInCm = -1;  //DEBUG STUFF
    } else {
      Serial.println("Reception failed :(");
    }
  }
}