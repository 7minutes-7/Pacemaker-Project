#include <time.h>

const int pacemakerExecutionFrequency = 50;

const double refractoryPeriod = 0.5;

const int url = 60;
const int lrl = 40;
const int hrl = 30;

const double lowerBound = 1/url;
double upperBound = 1/lrl;

const String P_amplitude = "0.2";
const String Q_amplitude = "-0.2";
const String R_amplitude = "0.5";
const String S_amplitude = "-0.2";
const String T_amplitude = "0.2";

int lastPaceValue = 0;
int currentPaceValue = 0;

bool lastWasT = true;
double lastWaveTime;

void setup() {
  Serial1.begin(9600);  // initialize UART with baud rate of 9600
}


void loop() {
  static double lastPaceTime = clock()/CLOCKS_PER_SEC;
  static double lastR = clock()/CLOCKS_PER_SEC;

  double currentTime = clock()/CLOCKS_PER_SEC;
  currentPaceValue = 0;

  // No R wave was detected until upper bound
  if(currentTime - lastR >= upperBound){
      currentPaceValue = 1;
  }
  
  // detect wave signal from heart
  if(Serial1.available()>=0){  // if there is any byte available to be read on UART1 buffer
    String amplitudeValue = Serial1.readStringUntil('\n'); // read amplitude from RandomHeart

    // Detecting R wave
    if(amplitudeValue == R_amplitude){
      // natural R-wave is detected : enable hysteresis pacing
      if(lastPaceValue == 0 && !lastWasT){
        upperBound = 1/hrl;
      }
      else if(lastPaceValue == 1){
        upperBound = 1/lrl;
      }

      // detected before lower
      if(currentTime - lastR <= lowerBound){
        // measure R-R interval from new wave and maintain pace=0
        lastR = currentTime;
      }

      // anomalous R wave after PQRST
      if(lastWasT && currentTime - lastWaveTime <= 1/refractoryPeriod){
        // ignore
      }

      lastWaveTime = currentTime;
      lastWasT = false;
    }
    else if(amplitudeValue == T_amplitude) lastWasT = true;
    else lastWasT = false;
  }

  // Send pace signal to the heart
  if(currentTime - lastPaceTime >= 1 / pacemakerExecutionFrequency){
    Serial1.println(currentPaceValue);
    Serial.print("pace sent: ");
    Serial.print(currentPaceValue);
    Serial.print("\n");

    lastPaceTime = currentTime;
    lastPaceValue = currentPaceValue;
  }
}
