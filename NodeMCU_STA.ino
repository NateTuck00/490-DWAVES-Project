#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "ESPAsyncWebServer.h"

AsyncWebServer gatewayServer(80); // gateway server on port 80 // make post requests to this server for manual override

void setup() {
  const char* ssid = "DWAVES";
  const char* password = "123456789";
  
  WiFi.begin(ssid, password); // connect to WiFi network

  while (WiFi.status() != WL_CONNECTED) { // wait until connection happens
    delay(500);
    Serial.print(".");
  }



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
