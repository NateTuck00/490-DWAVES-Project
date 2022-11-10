/* to do:
1. User Interface code
2. ajax and javascript for updating the page without reloading
3. State of environmental controls for UI, and manual override
4. Do some clean-up
*/

#include <vector>
#include <ESP8266WiFi.h>
#include "ESPAsyncWebServer.h"
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "Zanshin_BME680.h"

String systemTemperature;
String systemConcentration;
String systemHumidity;

float temperature_v[5];
float concentration_v[5];
float humidity_v[5];

String stationIP = "192.168.1.22"; // ip of the station server

int i = 3000000; // iterate in loop() // basically a timer // initially set high so that get request in loop is made first
String controlsState; // global variable for state of the environtal controls

/*************************
|      Sensor Setup      |
*************************/
// source: https://github.com/Zanduino/BME680/blob/master/examples/I2CDemo/I2CDemo.ino

BME680_Class BME680; // BME680 Object

static int32_t  temp, humidity, pressure, gas; // readings from BME680

float altitude(const int32_t press, const float seaLevel = 1015);
float altitude(const int32_t press, const float seaLevel) {
  /*!
  @brief     This converts a pressure measurement into a height in meters
  @details   The corrected sea-level pressure can be passed into the function if it is known,
             otherwise the standard atmospheric pressure of 1013.25hPa is used (see
             https://en.wikipedia.org/wiki/Atmospheric_pressure) for details.
  @param[in] press    Pressure reading from BME680
  @param[in] seaLevel Sea-Level pressure in millibars
  @return    floating point altitude in meters.
  */
  static float Altitude;
  Altitude =
      44330.0 * (1.0 - pow(((float)press / 100.0) / seaLevel, 0.1903));  // Convert into meters
  return (Altitude);
}  // of method altitude()


String readConcentration() { // returns alcohol concentration as a string
  BME680.getSensorData(temp, humidity, pressure, gas);
  char buf[16];
  sprintf(buf, "%d.%d", (int8_t)(gas / 100), (uint8_t)(gas % 100));
  return buf;
}

String readTemperature() { // returns the temperature as a string
  BME680.getSensorData(temp, humidity, pressure, gas);
  char buf[16];
  sprintf(buf, "%d.%d", (int8_t)(temp / 100), (uint8_t)(temp % 100));
  return buf;
}

String readHumidity() { // returns humidity as a string
  BME680.getSensorData(temp, humidity, pressure, gas);
  char buf[16];
  sprintf(buf, "%d.%d", (int8_t)(humidity / 1000), (uint8_t)(humidity % 1000));
  return buf;
}

/*************************
|     User Interface     |
*************************/

String UI() {/* Create webpage for UI here and return as a string */
char body[1024];
sprintf(body, "<html>"
"<head>"
"<title>ESP8266 Page</title> "
"<script src="https://cdn.anychart.com/releases/8.0.1/js/anychart-core.min.js"></script>"
"<script src="https://cdn.anychart.com/releases/8.0.1/js/anychart-core.min.js"></script>"
"<body style=\"background-color:#ffffff;\"></body>"
"<meta http-equiv=\"refresh\" content=\"3\">"
"<style>"
"h1 {text-align:center; color: #000000 }"
"td {font-size: 40px; padding-top: 30px; color: #000000} "
".temp {font-size:40px; color: #bd0e0e;}"
".conc {font-size:40px; color: #1dd39c;}"
".hum {font-size:40px; color: #e88712;}"
".overide {font-size:40px; color: #000000;}"
"</style>"
"</head>"
"<body>"
"<h1>D.W.A.V.E.S. User Interface</h1>"
"<div id='div1'>"
"<table>"
"<tr>"
"<td>Temperature</td><td class='temp'>%s</td>"
"</tr>"
"<tr> <td> Alcohol Concentration</td><td class='conc'> %s </td> </tr>" 
"<tr> <td> Humidity</td><td class='hum'>%s</td> </tr>" 
"<td><a href="http://192.168.1.22/manOverride"><img src="power.png" alt="Power Button" width="120" height="100"></a> </td>" 
"<tr><td>System State</td><td class='conc'>%s</td></tr> "
"</tr>"
"</div>" 
"</body>"
"</html>", systemTemperature, systemConcentration, systemHumidity,controlsState);
return body;
}

/*************************
|     Server and AP      |
*************************/

AsyncWebServer sensorServer(80); // sensor server on port 80 (HTTP)

IPAddress subnet(255, 255, 255, 0);
IPAddress local_IP(192, 168, 1, 184);
IPAddress gateway(192, 168, 1, 1);

void setup() {
  // start sample code
  /*!
  @brief    Arduino method called once at startup to initialize the system
  @details  This is an Arduino IDE method which is called first upon boot or restart. It is only
            called one time and then control goes to the main "loop()" method, from which
            control never returns
  @return   void
  */
  
  Serial.begin(9600);  // Start serial port at Baud rate
  
#ifdef __AVR_ATmega32U4__      // If this is a 32U4 processor, then wait 3 seconds to init USB port
  delay(3000);
#endif
  Serial.print(F("Starting I2CDemo example program for BME680\n"));
  Serial.print(F("- Initializing BME680 sensor\n"));
  while (!BME680.begin(I2C_STANDARD_MODE)) {  // Start BME680 using I2C, use first device found
    Serial.print(F("-  Unable to find BME680. Trying again in 5 seconds.\n"));
    delay(5000);
  }  // of loop until device is located
  Serial.print(F("- Setting 16x oversampling for all sensors\n"));
  BME680.setOversampling(TemperatureSensor, Oversample16);  // Use enumerated type values
  BME680.setOversampling(HumiditySensor, Oversample16);     // Use enumerated type values
  BME680.setOversampling(PressureSensor, Oversample16);     // Use enumerated type values
  Serial.print(F("- Setting IIR filter to a value of 4 samples\n"));
  BME680.setIIRFilter(IIR4);  // Use enumerated type values
  Serial.print(F("- Setting gas measurement to 320\xC2\xB0\x43 for 150ms\n"));  // "�C" symbols
  BME680.setGas(320, 150);  // 320�c for 150 milliseconds
  // end sample code

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

  /* These sensorServer block below deal with the UI */

  sensorServer.on("/UI", HTTP_GET, [](AsyncWebServerRequest *request){ // web request for the UI display // the object name should potentially just be "/"
  request->send_P(200, "text/html", UI().c_str());
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


  for (int i = 0; i < 5; ++i) { // history arrays are filled
    temperature_v[i] = 0;
    concentration_v[i] = 0;
    humidity_v[i] = 0;
  }

}

void loop() {
  if (i == 3000000) { // apparently http requests can only be made in this loop() // about once every 24 seconds
    //Serial.println("if (i == 1000000) ... ");
    WiFiClient client;
    HTTPClient http;
    String controlsPath = "http://" + stationIP + "/controls";
    http.begin(client, controlsPath.c_str());
    int getState = http.GET();
    Serial.print("http.GET() returned: "); Serial.println(getState);
    controlsState = http.getString(); // this will be the state of the controls. It's returned to the global controlState variable to be used in the UI function


    String conPath = "http://" + stationIP + "/concentration";
    http.begin(client, conPath.c_str());
    getState = http.GET();
    Serial.print("http.GET() returned: "); Serial.println(getState);
    systemConcentration = http.getString();
    

    String tempPath = "http://" + stationIP + "/temperature";
    http.begin(client, tempPath.c_str());
    getState = http.GET();
    Serial.print("http.GET() returned: "); Serial.println(getState);
    systemTemperature = http.getString(); 


    String humPath = "http://" + stationIP + "/humidity";
    http.begin(client, humPath.c_str());
    getState = http.GET();
    Serial.print("http.GET() returned: "); Serial.println(getState);
    systemHumidity = http.getString();

    for (int i = 1; i < 5; ++i) {
      temperature_v[i-1] = temperature_v[i];
      concentration_v[i-1] = concentration_v[i];
      humidity_v[i-1] = humidity_v[i];
    }
    temperature_v[4] = systemTemperature.toFloat();
    concentration_v[4] = systemConcentration.toFloat();
    humidity_v[4] = systemHumidity.toFloat();


    i = 0; // i is set back to 0 so the "timer" restarts
  }
  i = i + 1; // i is iterated
}
