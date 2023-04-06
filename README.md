# CITRIS Project

### Contents of this repository:
* Two .ino files which enable the Arduino to take in sensor information.
* One .py file which works as a listener for the Arduino's output. 
* This code is meant to run on an Arduino Mega 2560. 
* THIS REPO WILL ALSO CONTAIN THE WEB APPLICATION.

#### Arduino Files
Much if not all of the .ino file is self commented and so once completed I will compile the in code comments into this Readme for better understanding.

#### Python File
This code works by accessing the information from the serial port that the Arduino is connected to and outputting that data into a .csv file after a specified waiting period. These .csv files are saved into a "SerialMonitorLogs" directory where the files are named with the current date (YYYY/MM/DD).

#### Web Application
The idea behind this application is that it will access the .csv files from the Python script and continuously update a GUI which displays all the sensor information. 

Figma Link: https://www.figma.com/file/nWNJIhEkAPbU1ASkIq7kTz/CITRIS-Software-Layout?node-id=0%3A1&t=cDcoa1VLyBcIUr4z-1
