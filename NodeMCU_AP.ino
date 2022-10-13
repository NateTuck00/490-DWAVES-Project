/* to do:
1. figure out a way to get the IP address for the gateway NodeMCU here
2. User Interface code
*/

#include <ESP8266WiFi.h>
#include "ESPAsyncWebServer.h"
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

String readConcentration() { // returns alcohol concentration as a string
  return "0.02%";
}

String readTemperature() { // returns the temperature as a string
  return "20 F";
}

String readHumidity() { // returns humidity as a string
  return "30 humidities";
}

String UI(String gatewayIP) {/* Create webpage for UI here and return as a string */
  WiFiClient client;
  HTTPClient http;
  String controls;
  
  conPath = gatewayIP + "/controls";
  http.begin(client, conPath.c_str());
  http.GET(); // this will send the environmental controls status request to the station NodeMCU
  controls = http.getString();
}

String manOverride(String gatewayIP) {/* Sends override to other server, then returns a confirmation or a failure notice to the UI */
  WiFiClient client;
  HTTPClient http;
  
  conPath = gatewayIP + "/manOverride";
  http.begin(client, conPath.c_str());
  http.GET(); // this will send the manual override to the station NodeMCU

  return http.getString(); // confirmation or error for manual override will be returned to the UI
}

AsyncWebServer sensorServer(80); // sensor server on port 80 (HTTP)


void setup() {
  Serial.begin(9600);

  /*************************
  | Setup for WiFi Network |
  *************************/
  
  Serial.print("Starting WiFi network...");
  const char* ssid     = "Eni";
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

  sensorServer.on("/UI", HTTP_GET, [](AsyncWebServerRequest *request){ // web request for the UI display // the object name should potentially just be "/"
  request->send_P(200, "text/plain", UI(gatewayIP).c_str());
  });

  sensorServer.on("/UI/override", HTTP_GET, [](AsyncWebServerRequest *request){ // web request for the manual override
  request->send_P(200, "text/plain", manOverride(gatewayIP).c_str());
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
