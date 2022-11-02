#include <string>
#include "pacemaker.h"
using namespace std;

//**************************************************************************
// global variables for pacemaker operation
//**************************************************************************
int currentPaceValue = 0;

const double lowerBound = 1000*60/URL;
double upperBound = 1000*60/LRL;
long interval = 1000.0 / PACE_FREQUENCY;
  
char lastWave='T';
double beatStartTime;
double lastPaceTime = 0;

//*****************************************************************

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);  // initialize UART with baud rate of 9600
  delay(1000); // prevents usb driver crash on startup, do not omit this
  
  // wait for serial port to connect
  while(!Serial){}
  while(!Serial1){}

}


void loop() {
  unsigned long currentTime = millis(); //time in milliseconds
  static unsigned long lastRTime = currentTime;
  static unsigned long lastPaceTime = 0;
  static bool isNatural = true;
  currentPaceValue = 0;

  // No R wave was detected until upper bound
  if(currentTime - lastRTime >= upperBound && lastWave == 'T'){
    currentPaceValue = 1;
    isNatural = false;  // this is not a natural wave!!
  } 
  else {
    currentPaceValue = 0;
  }

  // detect wave signal from heart
  if(Serial1.available()>0){  // if there is any byte available to be read on UART1 buffer
     String signal = Serial1.readStringUntil('\n'); // read amplitude signal from RandomHeart
      Serial.print("Signal received from heart: ");
      Serial.println(signal);

      // change data type for string operation
      string amplitudeValue = signal.c_str(); 

      // Checking which wave we got
      if(stod(amplitudeValue) == P_AMP  && (lastWave == 'T'|| lastWave == 'R')){
        beatStartTime = currentTime;
        lastWave='P';
      }
      else if(stod(amplitudeValue) == Q_AMP && lastWave == 'P'){
        lastWave='Q';
      }
      // Detecting R wave
      else if(stod(amplitudeValue) == R_AMP){
        // anomalous R wave after PQRST
        if(currentTime - beatStartTime <= 1000.0/REFRACTORY_PERIOD){ 
          Serial.println("ignoring anomalous R Wave");
          Serial.println(1);
          return; //ignore
        }
        // detected before lower
        if(currentTime - lastRTime <= lowerBound){
          // measure R-R interval from new wave and maintain pace=0
          Serial.println("R wave detected before lower bound");
          Serial.println(1);
        }

        // if natural R-wave is detected : enable hysteresis pacing
        if(isNatural){
          Serial.println("natural r-wave detected");
          upperBound = 1000*60/HRL;
        }
        else{
          Serial.println("paced r-wave detected");
          upperBound = 1000*60/LRL;
          isNatural = true;
        }
        
        lastWave='R';
        lastRTime = currentTime;
      }
      else if(stod(amplitudeValue) == S_AMP && lastWave == 'R'){
        lastWave = 'S';
      } 
      else if(stod(amplitudeValue) == T_AMP && lastWave=='S'){
        lastWave = 'T';
      }
  }

  // Send pace signal to the heart
  if(currentTime - lastPaceTime >= interval){

    Serial1.println(currentPaceValue);
    Serial.print("pace sent: ");
    Serial.println(currentPaceValue);
    
    lastPaceTime = currentTime;
  }
}

