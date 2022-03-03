/*
  This cpp contains a PID implementation for temperature using P and I
  The output is scaled 0-255 with the ends capped for pwm safety bounds. 
  The cumulative error bounds can be adjusted to change the curve and windup of the PID
*/

#include "Arduino.h"
#include "Temp_PID.h"

double tempPID::computePID(tempPID tp) {
  static double currentTime = millis();
  static double elapsedTime = (double)(currentTime - tp.dbl_previousTime);


  double error = dbl_setPoint - dbl_temp_inp; // error here means how far off our temp_input temp is from where the gauge is set
  dbl_cumulativeError += error * elapsedTime;//We clamp how much this can affect our output below to prevent windup 

  if(abs(dbl_cumulativeError*c_ki)>40){

    if(dbl_cumulativeError > (10/c_ki)){
      dbl_cumulativeError=(160/c_ki); 
    }//endif positive cumulative error

    if(dbl_cumulativeError < (-70/c_ki)){
      dbl_cumulativeError=(-50/c_ki);                 // from -70 to -50
    }//endif negative cumulative error
    
  }//endif cumulating error either direction
  
  double out = c_kp * error + c_ki * dbl_cumulativeError;//output determined by I and P. 
  dbl_previousTime = currentTime;

  //check output bounds
    if(out > 229){
      out=229;
    }//endif
    
    if(out < 26){
      out=26; 
    }//endif out of bounds

  return out;
}// end 
