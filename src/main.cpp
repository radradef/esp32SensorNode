#include "DHT.h"
#include <Wire.h>
#include <SPI.h>
#include <OneWire.h>
#include <SDS011.h>
#include <WiFi.h>
#include <HTTPClient.h>

float p10, p25, temp, hum;
int err;

HardwareSerial port(2);
SDS011 my_sds;
#define uS_TO_S_FACTOR 1000000 
#define TIME_TO_SLEEP  600 

#define DHTPIN 4   
#define DHTTYPE DHT11   
DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "";
const char* password =  "";
const String serverAPI = "";
const String auth_token = "";

void gatherData();
void postData();

void setup() {
  delay(1000);
  Serial.begin(115200);
  
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
  " Seconds");
  
  my_sds.begin(&port);
  dht.begin();

  WiFi.begin(ssid, password); 
  
  while (WiFi.status() != WL_CONNECTED) { 
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the WiFi network");

  gatherData();
  postData();

  Serial.println("Going to sleep");
  esp_deep_sleep_start();
  
}

void gatherData(){

  my_sds.wakeup();
  delay(30000);
  err = my_sds.read(&p25, &p10);
	if (!err) {
		Serial.println("P2.5: " + String(p25));
		Serial.println("P10:  " + String(p10));
	}
	delay(100);
  my_sds.sleep();

  hum = dht.readHumidity();
  temp = dht.readTemperature();
  
  if (isnan(hum) || isnan(temp)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.print(F("Humidity: "));
  Serial.print(hum);
  Serial.print(F("%  Temperature: "));
  Serial.print(temp);
  Serial.print(F("°C "));
}

void postData(){
    if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
  
    HTTPClient http;   
    String json = "{\"pm25\":\"" + String(p25) + 
      "\",\"pm10\":\"" + String(p10) + 
      "\",\"temp\":\"" + String(temp) +
      "\",\"hum\":\"" + String(hum) +
      "\"}";

    http.begin(serverAPI);  
    http.addHeader("Content-Type", "application/json");
    http.addHeader("x-auth-token", auth_token);          
    Serial.println(json);
    int httpResponseCode = http.POST(json);  
  
    if(httpResponseCode>0){
  
      String response = http.getString();   
  
      Serial.println(httpResponseCode);   
      Serial.println(response);         
  
    }else{
  
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
  
    }
    http.end(); 

  }else{

    Serial.println("Error in WiFi connection");  
 }
}

void loop() {

  Serial.println("Should never be reached");
 
}