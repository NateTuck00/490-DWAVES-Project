/* to do:
1. Replace sensor dummy variables with real readings
2. User Interface code
*/

#include <ESP8266WiFi.h>
#include "ESPAsyncWebServer.h"
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "Zanshin_BME680.h"

String stationIP = "http://192.168.1.22"; // ip of the gateway server

int i = 3000000; // iterate in loop() // basically a timer // initially set high so that get request in loop is made first
String controlsState; // global variable for state of the environtal controls
int overrideRequest = 0; // flag is set to 1 to override the system
String manOverrideState; //not really needed... but we'll see what I do with it

String readConcentration() { // returns alcohol concentration as a string
  return "0.02%";
}

String readTemperature() { // returns the temperature as a string
  return "20 F";
}

String readHumidity() { // returns humidity as a string
  return "30 humidities";
}

String UI() {/* Create webpage for UI here and return as a string */
  return controlsState;  
}

AsyncWebServer sensorServer(80); // sensor server on port 80 (HTTP)

IPAddress subnet(255, 255, 255, 0);
IPAddress local_IP(192, 168, 1, 184);
IPAddress gateway(192, 168, 1, 1);

void setup() {
  Serial.begin(9600);

  /*************************
  | Setup for WiFi Network |
  *************************/
  
  Serial.print("Starting WiFi network...");
  const char* ssid     = "Eni";
  const char* password = "123456789";

  WiFi.softAPConfig(local_IP, gateway, subnet);
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

  WiFiClient client; // for testing
  HTTPClient http;
 
  sensorServer.on("/concentration", HTTP_GET, [](AsyncWebServerRequest *request){ // web request for concentration
  request->send_P(200, "text/plain", readConcentration().c_str());
  }); 

  sensorServer.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){ // web request for temperature
  request->send_P(200, "text/plain", readTemperature().c_str());
  });

  sensorServer.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){ // web request for humidity
  request->send_P(200, "text/plain", readHumidity().c_str());
  });

  /* These two sensorServer blocks below deal with the UI and the manual override */

  sensorServer.on("/UI", HTTP_GET, [](AsyncWebServerRequest *request){ // web request for the UI display // the object name should potentially just be "/"
  request->send_P(200, "text/plain", UI().c_str());
  });

  sensorServer.on("/override", HTTP_GET, [](AsyncWebServerRequest *request){ // web request for the manual override
  overrideRequest = 1;
  request->send_P(200, "text/plain", "Request sent");
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
  Serial.print("http://"); Serial.print(IP); Serial.println("/UI");
  Serial.print("http://"); Serial.print(IP); Serial.println("/override");

}

void loop() {
  if (i == 3000000) { // apparently http requests can only be made in this loop() // about once every 24 seconds
    //Serial.println("if (i == 1000000) ... ");
    WiFiClient client;
    HTTPClient http;
    
    String conPath = stationIP + "/controls";
    http.begin(client, conPath.c_str());
    int getState = http.GET();
    Serial.print("http.GET() returned: ");
    Serial.println(getState);

    controlsState = http.getString(); // this will be the state of the controls. It's returned to the global controlState variable to be used in the UI function
    //Serial.println(controlsState);

    i = 0; // i is set back to 0 so the "timer" restarts
  }
  i = i + 1; // i is iterated

  if (overrideRequest == 1) { // if override flag is set http request is sent to the control unit
    WiFiClient client;
    HTTPClient http;
    
    String conPath = stationIP + "/manOverride";
    http.begin(client, conPath.c_str());
    http.GET();

    manOverrideState = http.getString();
    overrideRequest = 0; // flag is unset so only one override is sent
    Serial.println("Manual Override Sent");
  }
}
