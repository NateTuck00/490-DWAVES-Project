# <ESP8include266WiFi.h>
#include "ESPAsyncWebServer.h"

String readConcentration() { // returns alcohol concentration as a string
  return "0.02%";
}

String readTemperature() { // returns the temperature as a string
  return "20 F";
}

String readHumidity() { // returns humidity as a string
  return "30 humidities";
}

AsyncWebServer sensorServer(80); // sensor server on port 80 (HTTP)

void setup() {
  Serial.begin(9600);

  /*************************
  | Setup for WiFi Network |
  *************************/
  
  Serial.print("Starting WiFi network...");
  const char* ssid     = "DWAVES";
  const char* password = "123456789";

  WiFi.softAP(ssid, password); // start WiFi network
  Serial.println("Done!");

  IPAddress IP = WiFi.softAPIP(); // The IP address
  Serial.print("Access Point SSID: ");
  Serial.println(ssid);
  //Serial.print("Access Point IP address: ");
  //Serial.println(IP);

  /**************************
  | Setup for Sensor Server |
  **************************/

  Serial.print("Starting server...");
 
  sensorServer.on("/concentration", HTTP_GET, [](AsyncWebServerRequest *request){ // web request for concentration
  request->send_P(200, "text/plain", readConcentration().c_str());
  }); 

  sensorServer.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){ // web request for temperature
  request->send_P(200, "text/plain", readTemperature().c_str());
  });

  sensorServer.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){ // web request for humidity
  request->send_P(200, "text/plain", readHumidity().c_str());
  });

  sensorServer.begin();
  Serial.println("Done!");
  Serial.print("Server IP address: ");
  Serial.println(IP);
  Serial.println();

  Serial.println("URLs:");
  Serial.print("http://"); Serial.print(IP); Serial.println("/concentration");
  Serial.print("http://"); Serial.print(IP); Serial.println("/temperature");
  Serial.print("http://"); Serial.print(IP); Serial.println("/humidity");
}

void loop() {}