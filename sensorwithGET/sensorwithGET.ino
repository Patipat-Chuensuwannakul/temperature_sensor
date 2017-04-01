#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "DHT.h"
#define DHTPIN 4 
#define DHTTYPE DHT11   // DHT 11 
StaticJsonBuffer<200> jsonBuffer;
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
    while (!Serial) {
    // wait serial port initialization
    }   
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
void readfromsensor(){
    String PostData="{\"sensor_name\":\"sensor_a\",";
      PostData=PostData+"\"sensor_type\":\"temperature_sensor\",";
    int j=0;
//    StaticJsonBuffer<200> jsonBuffer;
//    JsonObject& json = jsonBuffer.createObject();

      delay(5000);

       rh = dht.readHumidity();
       PostData=PostData+"\"rh";
       PostData=PostData+"\":";
       PostData=String(PostData + rh);
       PostData=PostData+",";

       c = dht.readTemperature();
       PostData=PostData+"\"c";
       PostData=PostData+"\":";
       PostData=String(PostData + c);
       PostData=PostData+",";

       f = dht.readTemperature(true);
       PostData=PostData+"\"f";
       PostData=PostData+"\":";
       PostData=String(PostData + f);
       PostData=PostData+",";

      if (isnan(rh) || isnan(c) || isnan(f)) {
          Serial.println("Failed to read from DHT sensor!");
           error = 1;
      return ;
      } else{
        hif = dht.computeHeatIndex(f, rh);

        PostData=PostData+"\"hif";
        PostData=PostData+"\":";
        PostData=String(PostData + hif);
        PostData=PostData+",";
        hic = dht.computeHeatIndex(c, rh, false);

        PostData=PostData+"\"hic";
        PostData=PostData+"\":";
        PostData=String(PostData + hic);
        PostData=PostData+"}";
        Serial.println(PostData);
        delay(5000);
        
      }
                // Print values.
//          Serial.print("json : ");
//          Serial.println(json);
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
//          Serial.print("Json Object : ");
//          json.printTo(Serial);
          Serial.println(" ");
//        Serial.println(" ");
//        if (json.containsKey("temperature_snesor") && json.containsKey("humidity") && json.containsKey("celsius") && json.containsKey("fahrenheit") && json.containsKey("HeatIndex_fahrenheit") && json.containsKey("HeatIndex_celsius")){
//             // root["extra"] is valid
//             Serial.println("All data in JSON is present");
//        }

 ////////////////////////////////////Connect server////////////////////////////////////
    WiFiClient client;
    const char* host = "188.166.178.62";
    const int httpPort = 80;
        if (client.connect(host, httpPort)) {
          Serial.println("connected");
          client.println("POST / HTTP/1.1");
          client.print("Host: ");
          client.println(host);
          client.println("User-Agent: Arduino/1.0");
          client.println("Connection: close");
          client.println("Content-Type: application/x-www-form-urlencoded;");
          client.print("Content-Length: ");
          client.println(PostData.length());
          client.println(PostData);
        } else {
          Serial.println("connection failed");
        }

        unsigned long timeout = millis();
          while (client.available() == 0) {
            if (millis() - timeout > 5000) {
              Serial.println(">>> Client Timeout !");
              client.stop();
              return;
            }
          }
          if(client.available()){
            String line = client.readStringUntil('\r');
            if(line.equals("HTTP/1.1 200 OK")){
              Serial.println("upload OK");
              }else{
              Serial.println("upload Error");
              }
          }
 ////////////////////////////////////Connect server////////////////////////////////////
 }
void loop() {
  MinuteDelay(10);
  Serial.println(" ");
  readfromsensor();
}
