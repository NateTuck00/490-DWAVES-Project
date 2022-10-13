/* to do:
1. Do header processing from serial communications from the control unit

*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "ESPAsyncWebServer.h"
#include <WiFiClient.h>

AsyncWebServer gatewayServer(80); // gateway server on port 80

String manOverride() {/* do manual override and return status of manual override */
  Serial.write("override\n");
  while(Serial.available())
    return(Serial.readString()); // wait for control unit to send back a status for the override
    // responses from control unit will need headers to make sure the correct message is processed
    //return(Serial.read()); // alternative
}

String controls() { /* return state of environmental controls */
  Serial.write("controls\n");
  while(Serial.available())
    return(Serial.readString()); // wait for control unit to send back a status for the override
    // responses from control unit will need headers to make sure the correct message is processed
    //return(Serial.read()); // alternative
}

void setup() {
  const char* ssid = "DWAVES";
  const char* password = "123456789";
  
  WiFi.begin(ssid, password); // connect to WiFi network

  while (WiFi.status() != WL_CONNECTED) { // wait until connection happens
    delay(500);
    Serial.print(".");
  }

  gatewayServer.on("/manOverride", HTTP_GET, [](AsyncWebServerRequest *request){ // web request for humidity
  request->send_P(200, "text/plain", manOverride().c_str());
  });

  gatewayServer.on("/controls", HTTP_GET, [](AsyncWebServerRequest *request){ // web request for humidity
  request->send_P(200, "text/plain", controls().c_str());
  });


  /* I don't know why I got obsessed with this */
  Serial.begin(9600);
  Serial.print(" "); Serial.print(" "); Serial.print(" "); Serial.print(" "); Serial.print(" "); Serial.println(); // clearing the junk
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);
  Serial.println();
}

void loop() {
  String IP = "http://192.168.4.1";
  String conPath = IP + "/concentration";
  String tempPath = IP + "/temperature";
  String humPath = IP + "/humidity";

  WiFiClient client;
  HTTPClient http;

  http.begin(client, conPath.c_str());
  http.GET();
  //Serial.print("Ethanol Concentration: ");
  //Serial.println(http.getString());
  Serial.write("Ethanol Concentration: ");
  Serial.write((http.getString()).c_str());
  Serial.write("\n");

  http.begin(client, tempPath.c_str());
  http.GET();
  //Serial.print("Temperature: ");
  //Serial.println(http.getString());
  Serial.write("Temperature: ");
  Serial.write((http.getString()).c_str());
  Serial.write("\n");

  http.begin(client, humPath.c_str());
  http.GET();
  //Serial.print("Humidity: ");
  //Serial.println(http.getString());
  Serial.write("Humidity: ");
  Serial.write((http.getString()).c_str());
  Serial.write("\n");
  Serial.println();

  delay(5000);
}
