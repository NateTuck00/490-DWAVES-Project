/* to do:
1. Do header processing from serial communications from the control unit
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "ESPAsyncWebServer.h"
#include <WiFiClient.h>
#include "Zanshin_BME680.h"

bool communicating = false; // flag that shows if something is being written to the serial port
String systemTemperature;
String systemConcentration;
String systemHumidity;
String controlsState;

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

String largest(String String1, String String2) { // converts strings to floats and returns the largest one
  float Float1 = String1.toFloat();
  float Float2 = String2.toFloat();

  if (Float1 >= Float2) 
    return String(Float1);
  else if (Float1 < Float2)
    return String(Float2);
}

/*************************
|      Server Setup      |
*************************/

AsyncWebServer stationServer(80); // gateway server on port 80

String manOverride() {/* do manual override and return status of manual override */
  while (communicating) {/* wait while another write is happening */}
  communicating = true; // set communicating flag to true
  //Serial.write("content: manual override\r\n\r\n");
  //while(Serial.available())
  //  return(Serial.readString()); // wait for control unit to send back a status for the override
    // responses from control unit will need headers to make sure the correct message is processed
    //return(Serial.read()); // alternative
  return "toggle controls";
}

String controls() { /* return state of environmental controls */
  while (communicating) {/* wait for a communication to finish*/}
  communicating = true;
  //Serial.write("content: return controls status\r\n\r\n");
  //while(Serial.available())
  //  controlsState = Serial.readString();
  //  return(controlsState); // wait for control unit to send back a status for the override
    // responses from control unit will need headers to make sure the correct message is processed
    //return(Serial.read()); // alternative
  return "system is ...";
}

String accessPointIP = "http://192.168.1.184";

IPAddress staticIP(192,168,1,22);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

void setup() {

  /*************************
  |      Sensor Setup      |
  *************************/

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
  |      Server Setup      |
  *************************/

  const char* ssid = "Eni";
  const char* password = "123456789";
  
  WiFi.config(staticIP, gateway, subnet);
  WiFi.begin(ssid, password); // connect to WiFi network

  while (WiFi.status() != WL_CONNECTED) { // wait until connection happens
    delay(500);
    Serial.print(".");
  }
  
  stationServer.on("/controls", HTTP_GET, [](AsyncWebServerRequest *request){ // requests the state of the controls from the arduino
  request->send_P(200, "text/plain", controls().c_str());
  });

  stationServer.on("/manoverride", HTTP_GET, [](AsyncWebServerRequest *request){ // web request for humidity
  controlsState = manOverride();
  request->redirect((accessPointIP + "/UI").c_str());
  });

  stationServer.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){ // web request for humidity
  request->send_P(200, "text/plain", systemTemperature.c_str());
  });

  stationServer.on("/concentration", HTTP_GET, [](AsyncWebServerRequest *request){ // web request for humidity
  request->send_P(200, "text/plain", systemConcentration.c_str());
  });

  stationServer.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){ // web request for humidity
  request->send_P(200, "text/plain", systemHumidity.c_str());
  });

  stationServer.on("/controlsState", HTTP_GET, [](AsyncWebServerRequest *request) {
  request->send_P(200, "text/plain", controlsState.c_str());    
  });

  stationServer.begin(); // begin the server


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
  String conPath = accessPointIP + "/concentration";
  String tempPath = accessPointIP + "/temperature";
  String humPath = accessPointIP + "/humidity";

  WiFiClient client;
  HTTPClient http;

  http.begin(client, conPath.c_str());
  http.GET();
  String AP_reading = http.getString();
  String STA_reading = readConcentration();
  systemConcentration = largest(AP_reading, STA_reading);


  http.begin(client, tempPath.c_str());
  http.GET();
  AP_reading = http.getString();
  STA_reading = readTemperature();
  systemTemperature = largest(AP_reading, STA_reading);


  http.begin(client, humPath.c_str());
  http.GET();
  AP_reading = http.getString();
  STA_reading = readHumidity();  
  systemHumidity = largest(AP_reading, STA_reading);
  

  char sensorReadings[128];
  sprintf(sensorReadings, 
  "content: sensor values\r\n"
  "temperature:%s,concentration:%s,humidity:%s\r\n\r\n", systemTemperature, systemConcentration, systemHumidity);
  while (communicating) {/* wait while someone else is communicating */}
  communicating = true; // communicating flag is set to true so no one else can write to the serial line
  Serial.write(sensorReadings); // write sensor readings to serial port
  communicating = false; // flag is set back to false when done


  delay(5000);
}
