#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>  // Include the Wire library for I2C communication
#include <EEPROM.h>
#include "DHT.h"
#include "DFRobot_OxygenSensor.h"
#include "DFRobot_AirQualitySensor.h"
#include "Seeed_BME280.h"  // Include the BME280 library
#include "GravityTDS.h"
#include "DFRobot_PH.h"

#define READ_DELAY 1000    // Define the delay between readings (in milliseconds) CHANGE TO 1 MIN

//Define variable for NRF24L01:
#define CE_PIN 49   
#define CSN_PIN 48  

RF24 radio(CE_PIN, CSN_PIN);
const uint64_t pipe = 0xE8E8F0F0E1LL;

// Define variables for DHT11:
#define DHT11_PIN_0 39
#define DHT11_PIN_1 38
#define DHT11_PIN_2 37

#define DHTTYPE DHT11

DHT dht_0(DHT11_PIN_0, DHTTYPE);
DHT dht_1(DHT11_PIN_1, DHTTYPE);
DHT dht_2(DHT11_PIN_2, DHTTYPE);

#define PH_PIN_0 A8
#define PH_PIN_1 A9
float voltage, ph_value;
DFRobot_PH PH_0;
DFRobot_PH PH_1;

// Define variables for HC-204:
int heightInCm;
long duration;

#define TRIG_PIN_0 46
#define ECHO_PIN_0 47

#define TRIG_PIN_1 44
#define ECHO_PIN_1 45

#define TRIG_PIN_2 42
#define ECHO_PIN_2 43

#define TRIG_PIN_3 40
#define ECHO_PIN_3 41

// Define values for Oxygen Sensor:
#define COLLECT_NUMBER 10  // collect number, the collection range is 1-100.
#define Oxygen_IICAddress_0 ADDRESS_2
#define Oxygen_IICAddress_1 ADDRESS_3
#define Oxygen_IICAddress_2 ADDRESS_0

DFRobot_OxygenSensor Oxygen_0;
DFRobot_OxygenSensor Oxygen_1;
DFRobot_OxygenSensor Oxygen_2;

// Define values for I2C Mulltiplexer:
#define MUX_ADDR 0x70
// Define values for Particle Count Sensor: 
#define PARTICLE_SENSOR_ADDR 0x19
DFRobot_AirQualitySensor ParticleSensors[3] = {
  DFRobot_AirQualitySensor(&Wire, PARTICLE_SENSOR_ADDR),
  DFRobot_AirQualitySensor(&Wire, PARTICLE_SENSOR_ADDR),
  DFRobot_AirQualitySensor(&Wire, PARTICLE_SENSOR_ADDR)
};

// Function for I2C Multiplexer channel switching
void channelSwitch(uint8_t bus){
  Wire.beginTransmission(MUX_ADDR);  
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
}

// Define values for Air Quality Sensor:
#define AIR_SENSOR_PIN_0 A4  // Define the pin connected to the sensor data pin
#define AIR_SENSOR_PIN_1 A5
#define AIR_SENSOR_PIN_2 A6

// Define values for Turbidity Sensor:
#define TURB_PIN_0 A2
#define TURB_PIN_1 A3

// Define values for TDS Sensor:
#define TdsSensorPin_0 A0
#define TdsSensorPin_1 A1

GravityTDS gravityTds_0;
GravityTDS gravityTds_1;
 
float temperature = 25,tdsValue_0 = 0, tdsValue_1 = 0;

// Define values for CO2 Sensor:
/************************Hardware Related Macros************************************/
#define MG_PIN_0 (A15)  
#define MG_PIN_1 (A14)  
#define MG_PIN_2 (A13)  
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


/**
 Arrays for sensor values
**/
int TDS[2];
int tankHeight[4];
float turbidity[2];
float pH[2];
int airQuality[3];
int CO2Conc[3];
float temp[3];
float hum[3];
float particle[3];
float OxConc[3];

void setup() {
  Wire.begin();
  Serial.begin(9600);
  Serial.println("*************");
  Serial.println("*************");
  Serial.println("*************");
  Serial.println("Setting up everything.");

  Serial.println("Setting up: Particle Count Sensors");
  for (int i = 0; i < 3; i++) {
    channelSwitch(i);

    delay(1000);

    while (!ParticleSensors[i].begin()) {
      Serial.print("No devices on channel ");
      Serial.println(i);
      delay(1000);
    }
    Serial.print("Particle Count Sensor ");
    Serial.print(i);
    Serial.println(" initialized.");
  }

  //For Oxygen Sensor
  Serial.println("Setting up: Oxygen Sensors");
  while (!Oxygen_0.begin(Oxygen_IICAddress_0)) {
      Serial.print("I2c device 0 number error !");
      delay(1000);
  }
  delay(1000);
  Serial.println("Oxygen 1 done");
  while (!Oxygen_1.begin(Oxygen_IICAddress_1)) {
      Serial.print("I2c device 1 number error !");
      delay(1000);
    }
    delay(1000);
  Serial.println("Oxygen 2 done");
  while (!Oxygen_2.begin(Oxygen_IICAddress_2)) {
      Serial.print("I2c device 2 number error !");
      delay(1000);
    }
  delay(1000);
  Serial.println("Oxygen 3 done");
    
  // For TDS 
    Serial.println("Setting up: TDS");
    gravityTds_0.setPin(TdsSensorPin_0);
    gravityTds_0.setAref(5.0);  //reference voltage on ADC, default 5.0V on Arduino UNO
    gravityTds_0.setAdcRange(1024);  //1024 for 10bit ADC;4096 for 12bit ADC
    gravityTds_0.begin();  //initialization

    gravityTds_1.setPin(TdsSensorPin_1);
    gravityTds_1.setAref(5.0);  //reference voltage on ADC, default 5.0V on Arduino UNO
    gravityTds_1.setAdcRange(1024);  //1024 for 10bit ADC;4096 for 12bit ADC
    gravityTds_1.begin();  //initialization

  // For PH 
  Serial.println("Setting up: PH");
  PH_0.begin();
  PH_1.begin();

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

  //For Temp and Humidity Sensor
  Serial.println("Setting up: DHT-11");
  dht_0.begin();
  dht_1.begin();
  dht_2.begin();
  
  //For Air Quality Sensor
  Serial.println("Setting up: AQS");
  
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
  turbidity[0] = getTurbidityData(TURB_PIN_0);
  turbidity[1] = getTurbidityData(TURB_PIN_1); //Tap water

  Serial.print(turbidity[0]);
  Serial.print(",");
  Serial.print(turbidity[1]);
  Serial.print(",");

  pH[0] = getPHData(&PH_0, PH_PIN_0);
  pH[1] = getPHData(&PH_1, PH_PIN_1);

  Serial.print(pH[0]);
  Serial.print(",");
  Serial.print(pH[1]);
  Serial.print(",");

  tankHeight[0] = getWaterHeight(TRIG_PIN_0, ECHO_PIN_0);
  delay(500);
  tankHeight[1] = getWaterHeight(TRIG_PIN_1, ECHO_PIN_1);
  delay(500);
  tankHeight[2] = getWaterHeight(TRIG_PIN_2, ECHO_PIN_2);
  delay(500);
  tankHeight[3] = getWaterHeight(TRIG_PIN_3, ECHO_PIN_3);
  delay(500);


  airQuality[0] = getAirQuality(AIR_SENSOR_PIN_0);
  delay(500);
  airQuality[1] = getAirQuality(AIR_SENSOR_PIN_1);
  delay(500);
  airQuality[2] = getAirQuality(AIR_SENSOR_PIN_2);
  delay(500);

 CO2Conc[0] = getCO2Data(MG_PIN_0); 
 delay(500);
 CO2Conc[1] = getCO2Data(MG_PIN_1); 
 delay(500);
 CO2Conc[2] = getCO2Data(MG_PIN_2);
 delay(500);

  

    temp[0] = getTempData(&dht_0);
    delay(500);
    temp[1] = getTempData(&dht_1);
    delay(500);
    temp[2] = getTempData(&dht_2);
    delay(500);


    hum[0] = getHumidityData(&dht_0);
    delay(500);
    hum[1] = getHumidityData(&dht_1);
    delay(500);
    hum[2] = getHumidityData(&dht_2);
    delay(500);
   
      // temperature = readTemperature();  //add your temperature sensor and read it
    gravityTds_0.setTemperature(temperature);  // set the temperature and execute temperature compensation
    gravityTds_0.update();  //sample and calculate
    tdsValue_0 = gravityTds_0.getTdsValue();  // then get the value
    Serial.print(tdsValue_0,0);
    Serial.print(",");
    delay(1000);


    //temperature = readTemperature();  //add your temperature sensor and read it
    gravityTds_1.setTemperature(temperature);  // set the temperature and execute temperature compensation
    gravityTds_1.update();  //sample and calculate
    tdsValue_1 = gravityTds_1.getTdsValue();  // then get the value
    Serial.print(tdsValue_1,0);
    Serial.print(",");
    delay(1000);


  //PARTICLE COUNT GOES HERE 
  for (int i = 0; i < 3; i++) {
    channelSwitch(i);
    float PM2_5 = ParticleSensors[i].gainParticleConcentration_ugm3(PARTICLE_PM2_5_STANDARD);
    float PM1_0 = ParticleSensors[i].gainParticleConcentration_ugm3(PARTICLE_PM1_0_STANDARD);
    float PM10 = ParticleSensors[i].gainParticleConcentration_ugm3(PARTICLE_PM10_STANDARD);

    Serial.print(PM2_5);
    Serial.print(",");
    Serial.print(PM1_0);
    Serial.print(",");
    Serial.print(PM10);

    // if (i < 2) {
    //   Serial.print(",");
    // } else {
    //   Serial.println();
    // }
  }
  delay(1000);

  OxConc[0] = getOxygen(&Oxygen_0);
  delay(500);
  OxConc[1] = getOxygen(&Oxygen_1);
  delay(500);
  OxConc[2] = getOxygen(&Oxygen_2);
  delay(500);

 
  Serial.print(tankHeight[0]);
 Serial.print(",");
  delay(500);
 Serial.print(tankHeight[1]);
 Serial.print(",");
   delay(500);
 Serial.print(tankHeight[2]);
 Serial.print(",");
   delay(500);
 Serial.print(tankHeight[3]);
 Serial.print(",");
  delay(500);

 Serial.print(airQuality[0]);
 Serial.print(",");
   delay(500);
 Serial.print(airQuality[1]);
 Serial.print(",");
   delay(500);
 Serial.print(airQuality[2]);
 Serial.print(",");
  delay(500);

 Serial.print(CO2Conc[0]);
 Serial.print(",");
   delay(500);
 Serial.print(CO2Conc[1]);
 Serial.print(",");
   delay(500);
 Serial.print(CO2Conc[2]);
 Serial.print(",");
   delay(500);
  

    Serial.print(temp[0]);
    Serial.print(",");
      delay(500);
    Serial.print(temp[1]);
    Serial.print(",");
      delay(500);
    Serial.print(temp[2]);
    Serial.print(",");
      delay(500);


    Serial.print(hum[0]);
    Serial.print(",");
      delay(500);
    Serial.print(hum[1]);
    Serial.print(",");
      delay(500);
    Serial.print(hum[2]);
    Serial.print(",");
      delay(500);


  Serial.print(OxConc[0]);
  Serial.print(",");
    delay(500);
  Serial.print(OxConc[1]);
  Serial.print(",");
    delay(500);
  Serial.print(OxConc[2]);
    delay(500);

  delay(READ_DELAY);
  Serial.println("");
  // SERIAL PRINT AS CSV FORMAT
}

/*****************************  getOxygen **********************************
Input:   None
Output:   An integer which represents an analog read of air sensor
Remarks:   This is calling the getOxygenData function from the DFRobot library.
************************************************************************************/
float getOxygen(DFRobot_OxygenSensor* Oxygen) {
  float oxygenData = Oxygen->getOxygenData(COLLECT_NUMBER);
  delay(1000);
  return oxygenData;
}


/*****************************  getTempData *********************************************
Input:   dht object
Output:   An integer value that represents temperature in farenheit.
Remarks:   Sensor readings may be up to 2 seconds 'old' (its a very slow sensor)
************************************************************************************/
float getTempData(DHT* dht){
  // Reading temperature or humidity takes about 250 milliseconds!
  // Read temperature as Fahrenheit
  float f = dht->readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(f)) {
    return -1; //ERROR
  }
  return f;
  // Wait a few seconds between measurements.
  delay(2000);
}

/*****************************  getHumidityData *********************************************
Input:   dht object
Output:   An integer value that represents humidity as a percentage.
Remarks:   Sensor readings may be up to 2 seconds 'old' (its a very slow sensor)
************************************************************************************/
float getHumidityData(DHT* dht){
  // Reading temperature or humidity takes about 250 milliseconds!
  float h = dht->readHumidity();
  // Check if any reads failed and exit early (to try again).
  if (isnan(h)) {
    return -1; //ERROR 
  }

  delay(2000);
  return h;
  // Wait a few seconds between measurements.

}

/*****************************  getPHData *********************************************
Input:   DFRobot_PH pointer
Output:   
Remarks:   
************************************************************************************/
float getPHData(DFRobot_PH* ph, uint8_t ph_pin){
  voltage = analogRead(ph_pin)/1024.0*5000;
  ph_value = ph->readPH(voltage,temperature);
  delay(1000);
  return ph_value;
}


/*****************************  getTurbidityData *********************************************
Input:   Analog Pin
Output:   An integer value that represents Turbidity as a voltage.
Remarks:   
************************************************************************************/
float getTurbidityData(uint8_t analogPin){
  int sensorValue = analogRead(analogPin);// read the input on analog pin 0:
  float voltage = abs((sensorValue * (5.0 / 1024.0)) - 5.0); // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  // Serial.println(voltage); // print out the value you read:
  return voltage;
  delay(500);
}


/*****************************  getCO2Data *********************************************
Input:   None
Output:   An integer value that represents C02 in ppm
Remarks:   Utilizes getMG() and getMGPercentage() 
************************************************************************************/
int getCO2Data(uint8_t MG_PIN) {
  int percentage;
  float volts;

  volts = getMG(MG_PIN);
  percentage = getMGPercentage(volts, CO2Curve);
  // delay(500);
  return (percentage == -1) ? 400 : percentage; // if == -1 that means less than 400 ppm
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
int getAirQuality(uint8_t AIR_SENSOR_PIN) {
  int airQuality = analogRead(AIR_SENSOR_PIN);  // Read the analog value from the sensor pin        
  return airQuality;
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
