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

double lastR = 0;
double lastPace = 0;

int paceValue = 0;


void setup() {
  Serial1.begin(9600);  // initialize UART with baud rate of 9600
}

}

void loop() {
  paceValue = 0;

  while(Serial1.available()>=0){  // if there is any byte available to be read on UART1 buffer
    String amplitudeValue = Serial1.readStringUntil('\n'); // read amplitude from RandomHeart

    double elapsedTime = clock()/CLOCKS_PER_SEC;

    // doesn't beat until upper
    if(elapsedTime - lastR > upperBound){
      paceValue = 1;
    }

    // detect R wave
    if(amplitudeValue == R_amplitude){
      // beat before lower
      if(elapsedTime - lastR < lowerBound){
        // measure R-R interval from new wave and maintain pace=0
        
      }

      /*
        anomalous R wave after PQRST
      */
    }

    double currnetTime = clock()/CLOCKS_PER_SEC;
    if(currentTime - lastPace >= 1 / pacemakerExecutionFrequency){
      // Send pace to heart
      Serial1.println(paceValue);
      Serial.print("pace sent: ");
      Serial.print(paceValue);
      Serial.print("\n");

      lastPace = currentTime;
    }

  }

}
