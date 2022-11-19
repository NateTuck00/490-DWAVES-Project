
#include <Servo.h>
#include <SPI.h>
#include <SD.h >  // These two are for the SD card
File myFile;
String dataString = ""; 

Servo myservo;
word VentPin = 45;
bool substate= false;




void setup() {

  pinMode(VentPin, OUTPUT);
  pwm25kHzBegin();
  myservo.attach(2);  // attaches the servo on GIO2 to the servo object

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


}//end setup




float temperature;
float concentration;
float humidity;
bool manualoverride=false;
volatile int pos=55;
volatile int oldpos=55;
volatile int temppos;

void loop() {

  // put your main code here, to run repeatedly:
//while (no input )
//wait
//after input is received we log the alcohol value and react to it
// we also have a case for a shutoff toggle message and return on or off
//could return controls status on off 
//
while (Serial.available()==0);

while (Serial.available()) {
  substate=false;


  float dataIn1 = Serial.parseFloat();

  Serial.println(dataIn1,6);

  if(dataIn1 == 123.456008){

    Serial.println("Manual Override Activated (Toggle)");
    substate = true;
    dataIn1 = Serial.parseFloat();

    manualoverride = !manualoverride;

  }//endif override

  if(dataIn1 == 456.789031){

    Serial.println("Request to return controls");
    substate = true;
    // need to write back the controls in serial

    if((concentration <= 80)|| (manualoverride== true)){
      Serial.write("Off");
    }

    if(concentration > 80){
      Serial.write("On");
    }

    dataIn1 = Serial.parseFloat();
    

  }//endif 

  //temperature then concentration then humidity
  if (substate==false){
    temperature = dataIn1;
    Serial.print("Temperature: ");
    Serial.println(temperature);

    concentration = Serial.parseFloat();
    Serial.print("Concentration: ");
    Serial.println(concentration);


    humidity = Serial.parseFloat();
    Serial.print("Humidity: ");
    Serial.println(humidity);

    float waste = Serial.parseFloat();
    Serial.print("Waste: ");
    Serial.println(waste);

  
  myFile = SD.open ("test.txt", FILE_WRITE);//Open the file. Note that you can only open one file at a time. You must close this one before opening the next.
  if (myFile) { //if the file has opened correctly
    myFile.print("Temperature: ");         // This writes into the actual file
    myFile.println(temperature); 
    myFile.print("Concentration: ");
    myFile.println(concentration); 
    myFile.print("Humidity: ");
    myFile.println(humidity);

    myFile.close();
  }


  }//endif substate is false
//Serial.print("%s");
}//end while serial availabl


//Now we react to the concentration and activate certain angles and fan speeds
//servo goes from 55-155 
//fan goes from 0-320

/*
State 1: Fans off louvers off
*/

if((concentration < 80)|| (manualoverride== true)){
  
  //Timer1.setPwmDuty(9, 0);//fan control
  pwmDuty(0);
  pos=55;

  Serial.println("State 1");
}

/*
State 2: Fans low louvers low
*/

else if(concentration < 200){
  
  pwmDuty(160);
  pos=80;
 
  Serial.println("State 2");
}

/*
State3: Fans medium low louvers medium low
*/

else if(concentration < 400){
  
  pwmDuty(320);
  pos=105;
  
  
  Serial.println("State 3");
}

/*
State4: Fans medium high louvers medium high
*/

else if(concentration < 800){
  
  pwmDuty(480);
  pos=130;
  
  Serial.println("State 4");
}

/*
State5: Wide open
*/

else if(concentration >= 800){
  
  pwmDuty(639);
  pos=155;
  
  Serial.println("State 5");
}


if(oldpos > pos){
  Serial.println("oldpos > pos");
  
  for (temppos = 155; temppos >= pos; temppos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(temppos);              // tell servo to go to position in variable 'temppos'
    delay(30);                       // waits 30 ms for the servo to reach the position
  }
  
 

}//if oldpos >

else if(oldpos < pos){
  Serial.println("oldpos < pos");
 
  
  for (temppos = 55; temppos <= pos; temppos += 1) { // goes from 180 degrees to 0 degrees
    myservo.write(temppos);              // tell servo to go to position in variable 'temppos'
    delay(30);                       // waits 30 ms for the servo to reach the position
  }
  
  
}//end if oldpos<

else if(oldpos == pos){
  Serial.println("oldpos = pos");
  
  myservo.write(pos);              // tell servo to go to position in variable 'temppos'
  delay(30);
   
  
}//end if oldpos<

oldpos=pos;




}//end void loop


void pwm25kHzBegin() {
  TCCR5A = B00100011; // Fast PWM
  TCCR5B = B11001; // Prescaler = 1
  OCR5A = 639; // TOP
  //pinMode(44, OUTPUT);
  //pinMode(45, OUTPUT);      //we only use 45
  //pinMode(46, OUTPUT);
}

void pwmDuty(byte ocrb) {
  OCR5A = ocrb;                             // PWM Width (duty)
}
