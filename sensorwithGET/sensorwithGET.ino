#include <ESP8266WiFi.h>
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
void loop() {}
