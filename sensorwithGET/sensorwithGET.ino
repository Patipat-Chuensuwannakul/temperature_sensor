#include <ESP8266WiFi.h>
#include "DHT.h"
#define DHTPIN 4 
#define DHTTYPE DHT11   // DHT 11 
DHT dht(DHTPIN, DHTTYPE);
int value = 0;
float rh=0.0;
float c=0.0;
float f=0.0;
float hif;  
float hic;
bool error =0;
const char* ssid     = "chuenhome";         
const char* password = "churit888";
void setup() 
{
    Serial.begin(9600);   
    delay(10);
    Serial.println();
    Serial.println();
    Serial.print("Connecting to "); 
    Serial.println(ssid);       
    WiFi.begin(ssid, password);
   
    while (WiFi.status() != WL_CONNECTED)   
    {
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
      Serial.print("rh : ");
      Serial.println(rh);
      Serial.print("c : ");
      Serial.println(c);
      Serial.print("f : ");
      Serial.println(f);
      Serial.print("hif : ");
      Serial.println(hif);
      Serial.print("hic : ");
      Serial.println(hic);
      }
 }
void loop() {
  readfromsensor();
}
