/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */

#include <ESP8266WiFi.h>
#include "DHT.h"
#define DHTPIN 4 
#define DHTTYPE DHT11   // DHT 11 
 // Initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHTPIN, DHTTYPE);
//edit to home wifi setting **************************
const char* ssid     = "chuenhome";
const char* password = "churit888";
 
// edit to digital ocean host ***********************
const char* host = "188.166.178.62"; 

int value = 0;
float rh=0.0;
float c=0.0;
float f=0.0;
float hif;  
float hic;
bool error =0;
void setup() {
  Serial.begin(9600);
  delay(10);
  dht.begin();
  
 
  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
  


void readfromsensor(){ 
// Wait a few seconds between measurements.
  delay(2000);
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
   rh = dht.readHumidity();
  // Read temperature as Celsius
   c = dht.readTemperature();
  // Read temperature as Fahrenheit
   f = dht.readTemperature(true);
 
  if (isnan(rh) || isnan(c) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
       error = 1;
  return ;
  } else{
  hif = dht.computeHeatIndex(f, rh);
  hic = dht.computeHeatIndex(c, rh, false);
 
  
  }

 }
void MinuteDelay(int number){
  for (int i = 0; i < number; ++i){
      
      // use 6 10 second delays for each minute // with a yield before each
      for(int j=0; j<6 ;j++){
        Serial.print("*");
        // yield();
        delay(1000);  // 1 second delay
      }
   }
   Serial.println();
}
void loop() {
  
  error = 0;
  readfromsensor();
  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 4500;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  } 
  // We now create a URI for the request
 // edit path of the server here *******************************************
  String url = "/sensor/";
  String data;
  String tmp;
  if (error){
    data = "?c=0&f=0&rh=0&hic=0&hif=0";
  }else{
    data = "?c=";
    tmp = String(c, 2);
    data += tmp;
    data += "&f=";
    tmp = String(f, 2);
    data += tmp;
    data += "&rh=";
    tmp = String(rh, 2);
    data += tmp;
    data += "&hic=";
    tmp = String(hic, 2);
    data += tmp;
    data += "&hif=";
    tmp = String(hif, 2);
    data += tmp;  
  }
   url += data;
   Serial.print("Requesting URL: ");
   Serial.println(url);
  
  // This will send the request to the server
  if (client.connect(host, httpPort)) {
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" + 
                   "Connection: close\r\n\r\n");
  }
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  
  // Read all the lines of the reply from server and print them to Serial
  if(client.available()){
    String line = client.readStringUntil('\r');
    if(line.equals("HTTP/1.1 200 OK")){
      Serial.println("upload OK");
      }else{
      Serial.println("upload Error");
      }
  }
   
  MinuteDelay(10);
}

