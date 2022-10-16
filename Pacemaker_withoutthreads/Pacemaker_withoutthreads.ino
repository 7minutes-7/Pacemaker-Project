#include <string>
#include "pacemaker.h"
using namespace std;

int lastPaceValue = 0;
int currentPaceValue = 0;

const double lowerBound = 1000*60/URL;
double upperBound = 1000*60/LRL;
  
char lastWave='T';
double beatStartTime;

void TaskReadHeart();
void TaskSendPace();

long interval = 1000.0 / PACE_FREQUENCY;


double lastPaceTime = 0;

void setup() {
  /*
  xTaskCreate(
    TaskReadHeart, //task pointer
    "readheart",  //task name-for humans
    128, //stack size
    NULL, //parameter   
    2, //priority
    NULL //task handle
  )

  xTaskCreate(
    TaskSendPace, //task pointer
    "sendpace",  //task name-for humans
    128, //stack size
    NULL, //parameter   
    1, //priority
    NULL //task handle
  )  
*/

  Serial.begin(9600);
  Serial1.begin(9600);  // initialize UART with baud rate of 9600
}


void loop() {
  unsigned long currentTime = millis(); //time in milliseconds
  static unsigned long lastRTime = currentTime;
  static unsigned long lastPaceTime = 0;
  currentPaceValue = 0;

  // No R wave was detected until upper bound
  if(currentTime - lastRTime >= upperBound){
    currentPaceValue = 1;
  }

  // detect wave signal from heart
  if(Serial1.available()>0){  // if there is any byte available to be read on UART1 buffer
     String signal = Serial1.readStringUntil('\n'); // read amplitude signal from RandomHeart
      Serial.print("Signal received from heart: ");
      Serial.println(signal);

      // change data type for string operation
      string amplitudeValue = signal.c_str(); 

      // Checking which wave we got
      if(stod(amplitudeValue) == P_AMP  && lastWave == 'T'){
        beatStartTime = currentTime;
        lastWave='P';
      }
      else if(stod(amplitudeValue) == Q_AMP && lastWave == 'P'){
        lastWave='Q';
      }
      // Detecting R wave
      else if(stod(amplitudeValue) == R_AMP){
        // anomalous R wave after PQRST
        if(currentTime - beatStartTime <= 1/REFRACTORY_PERIOD){ 
          Serial.println("ignoring anomalous R Wave");
          return; //ignore
        }
        // detected before lower
        if(currentTime - lastRTime <= lowerBound){
          // measure R-R interval from new wave and maintain pace=0
          Serial.println("R wave detected before lower bound");
        }
        
        // if natural R-wave is detected : enable hysteresis pacing
        lastPaceValue == 0 ? upperBound = 1000*60/HRL : 1000*60/LRL;
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
    lastPaceValue = currentPaceValue;

  }
}

