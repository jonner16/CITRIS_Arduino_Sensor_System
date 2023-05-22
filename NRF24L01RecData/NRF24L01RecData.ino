#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>  // Include the Wire library for I2C communication
#include "DHT.h"
#include "DFRobot_OxygenSensor.h"
#include "Seeed_BME280.h"  // Include the BME280 library

#define READ_DELAY 5000    // Define the delay between readings (in milliseconds)

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

// Define variables for Turbidity:
#define TURBIDITY_PIN_0 A5
#define TURBIDITY_PIN_1 A6

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
#define Oxygen_IICAddress_0 ADDRESS_3
#define Oxygen_IICAddress_1 ADDRESS_2
#define Oxygen_IICAddress_2 ADDRESS_1
DFRobot_OxygenSensor Oxygen_0;
DFRobot_OxygenSensor Oxygen_1;
DFRobot_OxygenSensor Oxygen_2;

// Define values for Air Quality Sensor:
#define AIR_SENSOR_PIN_0 A0  // Define the pin connected to the sensor data pin
#define AIR_SENSOR_PIN_1 A1
#define AIR_SENSOR_PIN_2 A2

// Define values for TDS Sensor:
#define TDS_SENSOR_PIN_0 A3
#define TDS_SENSOR_PIN_1 A4
#define VREF 5.0      // analog reference voltage(Volt) of the ADC
#define SCOUNT  30           // sum of sample point
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,tdsValue = 0,temperature = 25;

// Define values for CO2 Sensor:
/************************Hardware Related Macros************************************/
#define MG_PIN_0 (A15)  //define which analog input channel you are going to use
#define MG_PIN_1 (A14)  //define which analog input channel you are going to use
#define MG_PIN_2 (A13)  //define which analog input channel you are going to use
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
int turbidity[2];
int pH[2];
int airQuality[3];
int CO2Conc[3];
int temp[3];
int humidity[3];
int particle[3];
int OxConc[3];

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
  // Serial.println("Setting up: Oxygen Sensor");
  // while (!Oxygen_0.begin(Oxygen_IICAddress_0)) {
  //   Serial.print("I2c device 0 number error !");
  //   delay(1000);
  // }
  
  // while (!Oxygen_1.begin(Oxygen_IICAddress_1)) {
  //   Serial.print("I2c device 1 number error !");
  //   delay(1000);
  // }
  // while (!Oxygen_2.begin(Oxygen_IICAddress_2)) {
  //   Serial.print("I2c device 2 number error !");
  //   delay(1000);
  // }

  //For Temp and Humidity Sensor
    Serial.println("Setting up: DHT-11");
  dht_0.begin();
  dht_1.begin();
  dht_2.begin();

  //For Air Quality Sensor
  Serial.println("Setting up: AQS");
  Wire.begin();

  //For TDS Sensor
  Serial.println("Setting up: TDS");
  pinMode(TDS_SENSOR_PIN_0,INPUT);
  pinMode(TDS_SENSOR_PIN_1,INPUT);
  
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
  // getNRFData();  
  // Serial.print(CO2Conc);
  // Serial.print(",");
  // Serial.print(airQuality);
  // Serial.print(",");
  // Serial.print(OxConc);
  // Serial.print(",");
  // Serial.print(heightInCm);
  // Serial.print("\n");

  // TDS[0] = getTDSData(TDS_SENSOR_PIN_0);
  // TDS[1] = getTDSData(TDS_SENSOR_PIN_1);

  // tankHeight[0] = getWaterHeight(TRIG_PIN_0, ECHO_PIN_0);
  // tankHeight[1] = getWaterHeight(TRIG_PIN_1, ECHO_PIN_1);
  // tankHeight[2] = getWaterHeight(TRIG_PIN_2, ECHO_PIN_2);
  // tankHeight[3] = getWaterHeight(TRIG_PIN_3, ECHO_PIN_3);
  
  // turbidity[0] = getTurbidityData(TURBIDITY_PIN_0);
  // turbidity[1] = getTurbidityData(TURBIDITY_PIN_1);

  // PH GOES HERE

  // airQuality[0] = getAirQuality(AIR_SENSOR_PIN_0);
  // airQuality[1] = getAirQuality(AIR_SENSOR_PIN_1);
  // airQuality[2] = getAirQuality(AIR_SENSOR_PIN_2);

  // CO2Conc[0] = getCO2Data(MG_PIN_0); 
  // CO2Conc[1] = getCO2Data(MG_PIN_1); 
  // CO2Conc[2] = getCO2Data(MG_PIN_2);
  
  // temp[0] = getTempData(DHT11_PIN_0);
  // temp[1] = getTempData(DHT11_PIN_1);
  // temp[2] = getTempData(DHT11_PIN_2);

  // humidity[0] = getHumidityData(DHT11_PIN_0);
  // humidity[1] = getHumidityData(DHT11_PIN_1);
  // humidity[2] = getHumidityData(DHT11_PIN_2);

  //PARTICLE COUNT GOES HERE 

  // OxConc[0] = getOxygen(Oxygen_0);
  // OxConc[1] = getOxygen(Oxygen_1);
  // OxConc[2] = getOxygen(Oxygen_2);
  
  // MAKE A FUNCTION THAT DOES ALL THE CSV OUTPUTTING STUFF AND CALL THAT FUNCTION HERE.

  // delay(READ_DELAY);

  // Serial.print(getWaterHeight(TRIG_PIN_0, ECHO_PIN_0));
  // Serial.print(", ");
  // Serial.print(getWaterHeight(TRIG_PIN_1, ECHO_PIN_1));
  // Serial.print(", ");
  // Serial.print(getWaterHeight(TRIG_PIN_2, ECHO_PIN_2));
  // Serial.print(", ");
  // Serial.println(getWaterHeight(TRIG_PIN_3, ECHO_PIN_3));
}


/*****************************  outputSerial *********************************************
Input:   all sensor values
Output:   none
Remarks:   takes all sensor info to output to serial
************************************************************************************/
int outputSerial(int TDS_1, int TDS_2, int w_height_1, int w_height_2, int w_height_3, int w_height_4, int turbidity_1, int turbidity_2, int pH_1, int pH_2, int AQS_1, int AQS_2, int AQS_3, int CO2_1, int CO2_2, int CO2_3, int temp_1, int temp_2, int temp_3, int humidity_1, int humidity_2, int humidity_3, int O2_1, int O2_2, int O2_3){

}


/*****************************  getTempData *********************************************
Input:   dht object
Output:   An integer value that represents temperature in farenheit.
Remarks:   Sensor readings may be up to 2 seconds 'old' (its a very slow sensor)
************************************************************************************/
int getTempData(DHT dht){
  // Reading temperature or humidity takes about 250 milliseconds!
  // Read temperature as Fahrenheit
  float f = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
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
int getHumidityData(DHT dht){
  // Reading temperature or humidity takes about 250 milliseconds!
  float h = dht.readHumidity();
  // Check if any reads failed and exit early (to try again).
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  return h;
  // Wait a few seconds between measurements.
  delay(2000);
}

/*****************************  getTurbidityData *********************************************
Input:   Analog Pin
Output:   An integer value that represents Turbidity as a voltage.
Remarks:   
************************************************************************************/
int getTurbidityData(uint8_t TURBIDITY_PIN){
  int sensorValue = analogRead(TURBIDITY_PIN);// read the input on analog pin 0:
  float voltage = sensorValue * (5.0 / 1024.0); // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  return voltage;
}

/*****************************  getTDSData *********************************************
Input:   Analog Pin
Output:   An integer value that represents TDS in ppm
Remarks:   Utilizes getMedianNum()
************************************************************************************/
int getTDSData(uint8_t TDS_PIN){
  static unsigned long analogSampleTimepoint = millis();
   if(millis()-analogSampleTimepoint > 40U)     //every 40 milliseconds,read the analog value from the ADC
   {
     analogSampleTimepoint = millis();
     analogBuffer[analogBufferIndex] = analogRead(TDS_PIN);    //read the analog value and store into the buffer
     analogBufferIndex++;
     if(analogBufferIndex == SCOUNT) 
         analogBufferIndex = 0;
   }   
   static unsigned long printTimepoint = millis();
   if(millis()-printTimepoint > 800U)
   {
      printTimepoint = millis();
      for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
        analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      float compensationCoefficient=1.0+0.02*(temperature-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
      float compensationVolatge=averageVoltage/compensationCoefficient;  //temperature compensation
      tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value
      //Serial.print("voltage:");
      //Serial.print(averageVoltage,2);
      //Serial.print("V   ");
      Serial.print("TDS Value:");
      Serial.print(tdsValue,0);
      Serial.println("ppm");
      return tdsValue;
   }
}

/*****************************  getMedianNum *********************************************
Input:   
Output:   
Remarks:   Helper function for getTDSData()
************************************************************************************/
int getMedianNum(int bArray[], int iFilterLen) 
{
      int bTab[iFilterLen];
      for (byte i = 0; i<iFilterLen; i++)
      bTab[i] = bArray[i];
      int i, j, bTemp;
      for (j = 0; j < iFilterLen - 1; j++) 
      {
      for (i = 0; i < iFilterLen - j - 1; i++) 
          {
        if (bTab[i] > bTab[i + 1]) 
            {
        bTemp = bTab[i];
            bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
         }
      }
      }
      if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
      else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
      return bTemp;
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
int getAirQuality(uint8_t AIR_SENSOR_PIN) {
  int airQuality = analogRead(AIR_SENSOR_PIN);  // Read the analog value from the sensor pin              // Print the value to the Serial Monitor
  return airQuality;
  // Serial.print("Air quality: ");
  // Serial.println(airQuality);
}

/*****************************  getOxygen **********************************
Input:   None
Output:   An integer which represents an analog read of air sensor
Remarks:   This is calling the getOxygenData function from the DFRobot library.
************************************************************************************/
int getOxygen(DFRobot_OxygenSensor Oxygen) {
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