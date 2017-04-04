
/**
   The MySensors Arduino library handles the wireless radio link and protocol
   between your home built sensors/actuators and HA controller of choice.
   The sensors forms a self healing radio network with optional repeaters. Each
   repeater and gateway builds a routing tables in EEPROM which keeps track of the
   network topology allowing messages to be routed to nodes.

   Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
   Copyright (C) 2013-2015 Sensnology AB
   Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors

   Documentation: http://www.mysensors.org
   Support Forum: http://forum.mysensors.org

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   version 2 as published by the Free Software Foundation.

 *******************************
   DESCRIPTION

   Dust Sensor for Shinyei ppd42ns

   1 : COMMON(GND)
   2 : OUTPUT(P2)
   3 : INPUT(5VDC 90mA)
   4 : OUTPUT(P1)
   5 : INPUT(T1)･･･FOR THRESHOLD FOR [P2] - not used
 *  * Dust Sensor for SAMYOUNG dms501

   5 : COMMON(GND)
   4 : OUTPUT(P2)
   3 : INPUT(5VDC 90mA)
   2 : OUTPUT(P1)
   1 : INPUT(T1)･･･FOR THRESHOLD FOR [P2] - not used

   http://www.seeedstudio.com/wiki/images/4/4c/Grove_-_Dust_sensor.pdf

     connect the sensor as follows :
      Pin 4/2SY of dust sensor PM1      -> Digital 7
      Pin 3 of dust sensor          -> +5V
      Pin 2/4SY of dust sensor PM25    -> Digital 8
      Pin 1/5 of dust sensor          -> Ground
  Contributor: epierre and alexsh1
**/

#include <Wire.h>
#include <Arduino.h>
#include <SoftwareSerial.h>

#include <HTS221.h>
// https://www3.epa.gov/airquality/particlepollution/2012/decfsstandards.pdf
static struct aqi {
  float clow;
  float chigh;
  int llow;
  int lhigh;
} aqi[] = {
  {0.0,    12.4,   0, 50},
  {12.1,   35.4,  51, 100},
  {35.5,   55.4, 101, 150},
  {55.5,  150.4, 151, 200},
  {150.5, 250.4, 201, 300},
  {250.5, 350.4, 301, 350},
  {350.5, 500.4, 401, 500},
};

// Define a static node address, remove if you want auto address assignment
#define NODE_ADDRESS_DUST   11

#define DUST_2_PM25  5 //pm2.5 
#define DUST_4_PM10  6 //pm10

int temp = 273.5 + 30; //external temperature, if you can replace this with a DHT11 or better


unsigned long stabilized_ms = 30000; // Sleep time between reads (in milliseconds)
//VARIABLES
int val = 0;           // variable to store the value coming from the sensor
long concentration = 0;
double ratio = 0.0;
unsigned long duration;
unsigned long starttime;
unsigned long endtime;
unsigned long sampletime_ms = 30000;
unsigned long lowpulseoccupancy = 0;

unsigned long lowpulseoccupancy10 = 0;
long concentration10 = 0;
double ratio10 = 0.0;
unsigned long duration10;

unsigned long lowpulseoccupancy25 = 0;
long concentration25 = 0;
double ratio25 = 0.0;
unsigned long duration25;

long concentrationPM10 = 0;
double concentrationPM10_ugm3;
double ppmvPM10 = 0.0;
long concentrationPM25 = 0;
double concentrationPM25_ugm3 = 0.0;
double ppmvPM25 = 0.0;
int aqiPM10 = 0;
int aqiPM25 = 0;

double humidity = 0;
double temperature = 0;

//*-- Software Serial
//
const byte _rxpin = 2; // Wire this to Tx Pin of ESP8266
const byte _txpin = 3; // Wire this to Rx Pin of ESP8266

// replace with your channel's thingspeak API key
String apiKey = "XPZJK6VHBZVRELF2";
String Channel = "237784";

SoftwareSerial esp8266 (_rxpin, _txpin);
//*-- IoT Information
//#define SSID "ING-TARN"
//#define PASS "0805957990"

//************************************ Wireless Access Point Setting **********************//
#define SSID "CS_TEACH2"
#define PASS "c$1029^_^"
//************************************ Wireless Access Point Setting **********************//


#define IP "184.106.153.149" // ThingSpeak IP Address: 184.106.153.149


bool DEBUG = 1;
// k 1 for short, 10 for long
void bb(int n, int k) {
  for (int i = 0; i < n; i++) {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(k * 100);                     // wait for a second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(k * 100);
  }
  // wait for a second
}

void setup()
{



  Wire.begin();
  pinMode(13, OUTPUT);
  smeHumidity.begin();
  pinMode(DUST_2_PM25, INPUT);
  pinMode(DUST_4_PM10, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(_rxpin, INPUT);
  pinMode(_txpin, OUTPUT);
  if (DEBUG) {
    Serial.begin(9600);
    Serial.println("starting");
  }
  esp8266.begin(9600);
  delay(3000);

  bb(5, 1);

     sendData("AT+RST\r\n", 10000, DEBUG); // reset module
     sendData("AT+CWMODE=1\r\n", 2000, DEBUG); // configure as station
 
  bb(1, 10);

}

void loop()
{
  bb(10, 1);
  sendData("AT+RST\r\n", 10000, DEBUG); // reset module
  connectWiFi();
   

  updateHT();
  temp = 273.5 + temperature;

  bool done = getBothPM();

  if (concentrationPM10 > 0) {
    concentrationPM10_ugm3 = conversion10(concentrationPM10);
    //ppmv=mg/m3 * (0.08205*Tmp)/Molecular_mass
    //0.08205   = Universal gas constant in atm·m3/(kmol·K)
    ppmvPM10 = ((concentrationPM10_ugm3) * ((0.08205 * temp) / 28.97));
    aqiPM10 = ugm32aqi(concentrationPM10_ugm3);
  }
  if (concentrationPM25 > 0) {
    concentrationPM25_ugm3 = conversion25(concentrationPM25);
    //ppmv=mg/m3 * (0.08205*Tmp)/Molecular_mass
    //0.08205   = Universal gas constant in atm·m3/(kmol·K)
    ppmvPM25 = ((concentrationPM25_ugm3) * ((0.08205 * temp) / 28.97));
    aqiPM25 = ugm32aqi(concentrationPM25_ugm3);
  }
//************************************ Code for Upload Data and Print **********************//
// on production, set DEBUG =0

  print_serial_data();
  upload_data_via_esp_8266(); 

  //AT+CWQAP - Disconnect from AP
  sendData("AT+CWQAP\r\n", 2000, DEBUG); // disconnect from AP

  bb(10, 1);
  second_sleep(5);




}

//************************************ Code for Upload Data and Print **********************//
// on production, set DEBUG =0

void upload_data_via_esp_8266()
{
  String strT =  String(temperature, 2);
  String strRH =  String(humidity, 2);
  String strPM25ugm3 =  String(concentrationPM25_ugm3, 2);
  String strPM10ugm3 =  String(concentrationPM10_ugm3, 2);
  String strPM25ppmv =  String(ppmvPM25, 2);
  String strPM10ppmv =  String(ppmvPM10, 2);
  String strPM25AQI =  String(aqiPM25);
  String strPM10AQI =  String(aqiPM10);

// When change server, edit only IP and GET message
// GET Message Example to upload to api.thingspeak.com
// GET /update?api_key=XPZJK6VHBZVRELF2&field1=31.55&field2=52.00&field4=37.09&field6=32.04&field8=104&field3=20.80&field5=17.97&field7=69


  // TCP connection
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com
  cmd += "\",80\r\n";

  sendData(cmd, 10000, DEBUG); // setup TCP connection

  // prepare GET string
  
  String getStr = "GET /update?api_key=";
  getStr += apiKey;
  getStr += "&field1=";
  getStr += String(strT);
  getStr += "&field2=";
  getStr += String(strRH);

  if (concentrationPM10 > 0) {
    getStr += "&field4=";
    getStr += String(strPM10ugm3);
    getStr += "&field6=";
    getStr += String(strPM10ppmv);
    getStr += "&field8=";
    getStr += String(strPM10AQI);
  }

  if (concentrationPM25 > 0) {
    getStr += "&field3=";
    getStr += String(strPM25ugm3);
    getStr += "&field5=";
    getStr += String(strPM25ppmv);
    getStr += "&field7=";
    getStr += String(strPM25AQI);
  }
  getStr += "\r\n\r\n";

// *******************WARNING: Do not edit after this line ***************//

  // send data length
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  cmd += "\r\n";
  if (DEBUG) {
    Serial.println(getStr.length());
  }
  esp8266.print(cmd);
  esp8266.flush();

  unsigned long start;
  start = millis();
  bool found;
  while (millis() - start < 5000) {
    if (esp8266.find(">") == true )
    {
      found = true;
      break;
    }
  }
  if (found)

    sendData(getStr, 15000, DEBUG);

  else
  {
    sendData("AT+CIPCLOSE\r\n", 5000, DEBUG);
    esp8266.println("AT+CIPCLOSE");

  }





}

//conversion from pcs/0.01cf to ug/m3
//based on https://github.com/intel-iot-devkit/upm/pull/409
//Conversion to mass concentration was perform under strong assumption:
//• All particles are spherical, with a density of 1.65E12 μg/m3 [,]
//• The radius of a particle in the channel <2.5 μm is 0.44 μm [,]
//• The radius of a particle in the channel > 2.5 is 2.60 μm [,***]

double conversion25(long concentrationPM25) {
  double pi = 3.14159;
  double density = 1.65 * pow (10, 12);
  double r25 = 0.44 * pow (10, -6);
  double vol25 = (4 / 3) * pi * pow (r25, 3);
  double mass25 = density * vol25;
  double K = 3531.5;
  return (concentrationPM25) * K * mass25;
}

double conversion10(long concentrationPM10) {
  double pi = 3.14159;
  double density = 1.65 * pow (10, 12);
  double r10 = 0.8 * pow (10, -6);
  double vol10 = (4 / 3) * pi * pow (r10, 3);
  double mass10 = density * vol10;
  double K = 3531.5;
  return (concentrationPM10) * K * mass10;
}

void updateHT() {
  double data = 0;
  humidity = smeHumidity.readHumidity();
  temperature = smeHumidity.readTemperature();
}


// Guidelines for the Reporting of Daily Air Quality – the Air Quality Index (AQI)
// https://www3.epa.gov/ttn/oarpg/t1/memoranda/rg701.pdf
//
// Revised air quality standards for particle pollution and updates to the air quality index (aqi)
// https://www3.epa.gov/airquality/particlepollution/2012/decfsstandards.pdf
//
// calculate AQI (Air Quality Index) based on μg/m3 concentration

int ugm32aqi (double ugm3)
{
  int i;

  for (i = 0; i < 7; i++) {
    if (ugm3 >= aqi[i].clow &&
        ugm3 <= aqi[i].chigh) {
      // Ip =  [(Ihi-Ilow)/(BPhi-BPlow)] (Cp-BPlow)+Ilow,
      return ((aqi[i].lhigh - aqi[i].llow) / (aqi[i].chigh - aqi[i].clow)) *
             (ugm3 - aqi[i].clow) + aqi[i].llow;
    }
  }
}


void minute_sleep(int minute) {
  for (int i = 0; i < minute; i++) {
    delay(60000);
    if (DEBUG) {
      Serial.print(".");
    }
  }
  if (DEBUG) {
    Serial.print("\n");
  }

}
void second_sleep(int minute) {
  for (int i = 0; i < minute; i++) {
    for (int j = 0; j < 60; j++) {
      delay(1000);
      if (DEBUG) {
        Serial.print(".");
      }
    }
  }
  if (DEBUG) {
    Serial.print("\n");
  }

}

void print_serial_data() {

  Serial.print("Humidity   : ");
  Serial.print(humidity);
  Serial.print(" % ");


  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" celsius");

  if (concentrationPM10 > 0) {
    Serial.print("Vout4_PM10: ");
    Serial.print(concentrationPM10);
    Serial.print(" pcs/0.01cf");

    Serial.print(" =  ");
    Serial.print(concentrationPM10_ugm3);
    Serial.print(" ug/m3 ");


    Serial.print(ppmvPM10);
    Serial.print(" ppmv    ");
    Serial.print("AQI = ");
    Serial.println(aqiPM10);
  } else {
    Serial.println("Vout#4: N/A ");

  }
  if (concentrationPM25 > 0) {
    Serial.print("Vout#2_PM2.5: ");
    Serial.print(concentrationPM25);
    Serial.print(" pcs/0.01cf");

    Serial.print(" =  ");
    Serial.print(concentrationPM25_ugm3);
    Serial.print(" ug/m3 ");


    Serial.print(ppmvPM25);
    Serial.print(" ppmv    ");
    Serial.print("AQI = ");
    Serial.println(aqiPM25);
  } else {
    Serial.println("Vout#2: N/A ");
  }


}

bool getBothPM() {

  starttime = millis();

  while (1) {
    duration10 = pulseIn(DUST_4_PM10, LOW);
    lowpulseoccupancy10 += duration10;
    duration25 = pulseIn(DUST_2_PM25, LOW);
    lowpulseoccupancy25 += duration25;
    endtime = millis();

    if ((endtime - starttime) > sampletime_ms)
    {



      //PM2.5
      if (lowpulseoccupancy25 > 0) {
        lowpulseoccupancy25 = lowpulseoccupancy25 - lowpulseoccupancy10;
        ratio25 = (lowpulseoccupancy25 - endtime + starttime) / (sampletime_ms * 10.0); // Integer percentage 0=>100
        concentrationPM25 = 1.1 * pow(ratio25, 3) - 3.8 * pow(ratio25, 2) + 520 * ratio25 + 0.62; // using spec sheet curve
      }
      //PM10
      if (lowpulseoccupancy10 > 0) {
        //
        ratio10 = (lowpulseoccupancy10 - endtime + starttime) / (sampletime_ms * 10.0); // Integer percentage 0=>100
        concentrationPM10 = 1.1 * pow(ratio10, 3) - 3.8 * pow(ratio10, 2) + 520 * ratio10 + 0.62; // using spec sheet curve
      }
      if (DEBUG) {
        Serial.print("PM10 ");
        Serial.print("  lowpulse: ");
        Serial.print(lowpulseoccupancy10);
        //  Serial.print("\n");
        Serial.print(" ratio: ");
        Serial.print(ratio10);
        //  Serial.print("\n");
        Serial.print(" Concentration: ");
        Serial.println(concentrationPM10);
        //Serial.print("\n");

        Serial.print("PM2.5 ");
        Serial.print(" lowpulse: ");
        Serial.print(lowpulseoccupancy25);
        //  Serial.print("\n");
        Serial.print(" ratio: ");
        Serial.print(ratio25);
        //  Serial.print("\n");
        Serial.print(" Concentration: ");
        Serial.println(concentrationPM25);
        //Serial.print("\n");
      }
      lowpulseoccupancy10 = 0;
      lowpulseoccupancy25 = 0;
      return 1;
    }
  }
}
boolean connectWiFi()
{
     //AT+CWQAP - Disconnect from AP
  sendData("AT+CWQAP\r\n", 2000, DEBUG); // disconnect from AP
  delay(2000);
  //Connect to Router with AT+CWJAP="SSID","Password";
  // Check if connected with AT+CWJAP?
  String cmd;
  cmd = "AT+CWJAP=\""; // Join accespoint
  cmd += SSID;
  cmd += "\",\"";
  cmd += PASS;
  cmd += "\"\r\n\"";
  sendData(cmd, 20000, DEBUG);

  cmd = "AT+CIPMUX=0\r\n";// Set Single connection
  sendData(cmd, 5000, DEBUG);

}

String sendData(String command, const int timeout, boolean debug)
{
  esp8266.flush();
  String response = "> ";
  if (debug)
  {
    Serial.println(command);
  }

  esp8266.print(command); // send the read character to the esp8266
  esp8266.flush();
  long int time = millis();

  while ( (time + timeout) > millis())
  {
    while (esp8266.available())
    {
      delay(1);
      // The esp has data so display its output to the serial window
      char c = esp8266.read(); // read the next character.
      response += c;
    }
  }

  if (debug)
  {
    Serial.println(response);
  }

  return response;
}
