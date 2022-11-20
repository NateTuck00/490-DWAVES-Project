
#include <SPI.h>
#include <SD.h >  // These two are for the SD card
File myFile;
String dataString = ""; 



void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);

   if (!SD.begin(53)) {

    Serial.println(F("SD Initialization failed!"));
    //while(1);// do nothing. system needs reset

  }//endif
  Serial.println (F("SD Initialization done."));


  myFile = SD.open ("test.txt", FILE_WRITE);//Open the file. Note that you can only open one file at a time. You must close this one before opening the next.


  if (myFile) { //if the file has opened correctly
      myFile.println("test write");         // This writes into the actual file
    myFile.close();
  }


}//end void setup


float temperature;
float concentration;
float humidity;

void loop() {
  // put your main code here, to run repeatedly:
  while (Serial.available()==0);

  while (Serial.available()) {

  float dataIn1 = Serial.parseFloat();

  Serial.println(dataIn1,6);

  if(dataIn1 == 123.456008){
    dataIn1 = Serial.parseFloat();  

  }//endif

  else if(dataIn1 == 456.789031){


    dataIn1 = Serial.parseFloat();
    

  }//endif 

  else{
    temperature = dataIn1;   
    concentration = Serial.parseFloat();  
    humidity = Serial.parseFloat(); 
    float waste = Serial.parseFloat();


  myFile = SD.open ("test.txt", FILE_WRITE);//Open the file. Note that you can only open one file at a time. You must close this one before opening the next.
  if (myFile) { //if the file has opened correctly
    myFile.print("Temperature: ");         // This writes into the actual file
    myFile.println(temperature); 
    myFile.print("Concentration: ");
    myFile.println(concentration); 
    myFile.print("Humidity: ");
    myFile.println(humidity);

    myFile.close();
  }//end if myfile


  }//end else
  }//end while

}//end void loop





