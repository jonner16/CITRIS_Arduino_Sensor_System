#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>              // Include the Wire library for I2C communication
#include "DFRobot_OxygenSensor.h"
#include "Seeed_BME280.h"      // Include the BME280 library

//NRF24L01 setup and pins
#define CEpin 7 //D7
#define CSNpin 8 //D8

RF24 radio(CEpin, CSNpin);
const uint64_t pipe = 0xE8E8F0F0E1LL;

// Define variables for HC-204:
int distanceInCm;
long duration;
#define TRIG_PIN 2
#define ECHO_PIN 3

// Define values for Oxygen Sensor
int OxConc;
#define COLLECT_NUMBER    10             // collect number, the collection range is 1-100.
#define Oxygen_IICAddress ADDRESS_3
DFRobot_OxygenSensor Oxygen;

// Define values for Air Quality Sensor
int airQuality;
#define AIR_SENSOR_PIN A0      // Define the pin connected to the sensor data pin
#define READ_DELAY 5000        // Define the delay between readings (in milliseconds)

// Define values for CO2 Sensor
int CO2Info[2];
/************************Hardware Related Macros************************************/
#define         MG_PIN                       (A1)     //define which analog input channel you are going to use
#define         BOOL_PIN                     (2)    
#define         DC_GAIN                      (8.5)   //define the DC gain of amplifier

/***********************Software Related Macros************************************/
#define         READ_SAMPLE_INTERVAL         (50)    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (5)     //define the time interval(in milisecond) between each samples in normal operation

/**********************Application Related Macros**********************************/
//These two values differ from sensor to sensor. user should derermine this value.
#define         ZERO_POINT_VOLTAGE           (0.220) //define the output of the sensor in volts when the concentration of CO2 is 400PPM
#define         REACTION_VOLTGAE             (0.030) //define the voltage drop of the sensor when move the sensor from air into 1000ppm CO2

/*****************************Globals***********************************************/
float           CO2Curve[3]  =  {2.602,ZERO_POINT_VOLTAGE,(REACTION_VOLTGAE/(2.602-3))};

void setup() {
  Serial.begin(9600);

  Serial.println("Setting up everything.");

  //For HC-204 
  Serial.println("Setting up: HC-204");
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  //For Oxygen Sensor
  Serial.println("Setting up: Oxygen Sensor");
  while(!Oxygen.begin(Oxygen_IICAddress)) {
    Serial.print("I2c device number error !");
    delay(1000);
  }

  //For Air Quality Sensor  
  Serial.println("Setting up: AQS");  
  Wire.begin(); 
  
  //For CO2 Sensor
  Serial.println("Setting up: CO2");
  pinMode(BOOL_PIN, INPUT);                        //set pin to input
  digitalWrite(BOOL_PIN, HIGH);                    //turn on pullup resistors

  Serial.println("Setting up: NRF24l01");
  //For NRF24L01
  radio.begin();
  radio.setChannel(1);
  radio.openReadingPipe(1, pipe);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();

  Serial.println("Setup Completed!");
}

void loop() {
  CO2Info = getCO2Data();     // [0] - volts,  [1] - percentage
  airQuality = getAirQualityData(); 
  OxConc = getOxygenData();
  distanceInCm = getWaterHeight();
  // getNRFData();
}

int getCO2Data(){
    int percentage;
    float volts;

    volts = getMG(MG_PIN);

    percentage = getMGPercentage(volts, CO2Curve);
    delay(500);
    return percentage;
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
float getMG(int mg_pin){
    int i;
    float v=0;

    for (i=0;i<READ_SAMPLE_TIMES;i++) {
        v += analogRead(mg_pin);
        delay(READ_SAMPLE_INTERVAL);
    }
    v = (v/READ_SAMPLE_TIMES) *5/1024 ;
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
int  getMGPercentage(float volts, float *pcurve){
   if ((volts/DC_GAIN )>=ZERO_POINT_VOLTAGE) {
      return -1;
   } else {
      return pow(10, ((volts/DC_GAIN)-pcurve[1])/pcurve[2]+pcurve[0]);
   }
}

/*****************************  getAirQualityData **********************************
Input:   None
Output:  analog read of air sensor
Remarks: This is a simple analog reading.
************************************************************************************/
int getAirQualityData(){
  int airQuality = analogRead(AIR_SENSOR_PIN);   // Read the analog value from the sensor pin              // Print the value to the Serial Monitor
  delay(READ_DELAY); 
  return airQuality;
  // Serial.print("Air quality: ");
  // Serial.println(airQuality);      
}

int getOxygenData(){
  float oxygenData = Oxygen.getOxygenData(COLLECT_NUMBER);
  delay(1000);
  return oxygenData;
  // Serial.print(" Oxygen concentration is ");
  // Serial.print(oxygenData);
  // Serial.println(" %vol");
}

int getWaterHeight(){
  // Clear the TRIG_PIN by setting it LOW:
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(5);

  // Trigger the sensor by setting the TRIG_PIN high for 10 microseconds:
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read the ECHO_PIN, pulseIn() returns the duration (length of the pulse) in microseconds:
  duration = pulseIn(ECHO_PIN, HIGH);

  // Calculate the distance:
  distanceInCm = duration * 0.034 / 2;
  delay(50);
  return distanceInCm;
}

void getNRFData(){
    if (radio.available()) {
    radio.read(&distanceInCm, sizeof(distanceInCm));
    if (distanceInCm != -1) {
      Serial.println("Reception successful!");
      Serial.print("Distance received: ");
      Serial.println(distanceInCm);
      // distanceInCm = -1;  //DEBUG STUFF
    } else {
      Serial.println("Reception failed :(");
    }
  }
}