#include "DHT.h"
#include <Wire.h>
#include <SPI.h>
#include <OneWire.h>
#include <SDS011.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#define uS_TO_S_FACTOR 1000000 
#define TIME_TO_SLEEP  600 
#define DHTPIN 4   
#define DHTTYPE DHT11 

float p10, p25, temp, hum;
int err;

HardwareSerial hw_port(2);
SDS011 my_sds;
DHT dht(DHTPIN, DHTTYPE);

//Wifi creds
const char* ssid = "";
const char* password =  "";

//server config
const char* host = "";
const String uri = "";
const String auth_token = "";
const char* root_ca= \
"-----BEGIN CERTIFICATE-----\n" \

"-----END CERTIFICATE-----\n";

void gatherData();
void postData();

void setup() {
  delay(1000);
  Serial.begin(115200);
  
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " 
  + String(TIME_TO_SLEEP) + " Seconds");
  
  my_sds.begin(&hw_port);
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
  if(WiFi.status() == WL_CONNECTED){   //Check WiFi connection status

  WiFiClientSecure client;
  client.setCACert(root_ca);

  if (!client.connect(host, 443)) {
    Serial.println("Connection failed.");
    return;
  }
    
  String body = 
  "{\"pm25\":\"" + String(p25) + 
  "\",\"pm10\":\"" + String(p10) + 
  "\",\"temp\":\"" + String(temp) +
  "\",\"hum\":\"" + String(hum) +
  "\"}";
      
  String postRequest = 
  "POST " + uri + " HTTP/1.1\r\n" + 
  "Host: " + host + "\r\n" + 
  "Content-Type: application/json\r\n" +
  "Content-Length: " + body.length() + "\r\n" +
  "x-auth-token: " + auth_token + "\r\n" +
  "\r\n" + body;

  Serial.println("\n********");
  Serial.println(postRequest);
  client.print(postRequest);
  client.stop(); 
  Serial.println();
  Serial.println("***DONE***");

  } else {
    Serial.println("Error in WiFi connection");  
  }
}

void loop() {
  Serial.println("Should never be reached");
}