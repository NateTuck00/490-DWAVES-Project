//computePID: uses several global variables to calculate errors in temp/setTemp, and time. Could have temp timeout inside or externally
//Consts: P&I
//Inputs: input temp, previousTime, cumulativeError, setPoint
//Outputs: double out
#ifndef Temp_PID
#define Temp_PID

#include "Arduino.h"


const uint8_t c_kp = 20;
const uint8_t c_ki = 1;

class tempPID 
{
  public:
  double dbl_temp_inp; 
  double dbl_previousTime;
  double dbl_cumulativeError;
  double dbl_setPoint;

  double computePID(tempPID tp);
  
  private:
  
};


#endif
