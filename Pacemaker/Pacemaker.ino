#include <FreeRTOS_SAMD21.h>
#include <time.h>
#include <string>
#include "pacemaker.h"

using namespace std;

//**************************************************************************
// global variables for RTOS
//**************************************************************************
TaskHandle_t Handle_readTask;
TaskHandle_t Handle_sendTask;

void TaskReadHeart(void*);
void TaskSendPace(void*);

//**************************************************************************
// global variables for pacemaker operation
//**************************************************************************
int lastPaceValue = 0;
int currentPaceValue = 0;


//**************************************************************************
// Can use these function for RTOS delays
// Takes into account processor speed
// Use these instead of delay(...) in rtos tasks
//**************************************************************************
void myDelay(double us){
  vTaskDelay( us / portTICK_PERIOD_US ); 
}

//**************************************************************************
// implement tasks
//**************************************************************************
void TaskReadHeart(void* pvParameters){
  const double lowerBound = 1/URL;
  double upperBound = 1/LRL;
  
  char lastWave='T';
  double beatStartTime;

  Serial.println("Thread TaskReadHeart: Started");
  while(true){
    double currentTime = clock()/CLOCKS_PER_SEC;
    static double lastRTime = currentTime;

    // No R wave was detected until upper bound
    if(currentTime - lastRTime >= upperBound){
        currentPaceValue = 1;
        Serial.println("Exceeded upper bound");
    }
    
    // detect wave signal from heart
    if(Serial1.available()>=0){  // if there is any byte available to be read on UART1 buffer
      String signal = Serial1.readStringUntil('\n'); // read amplitude signal from RandomHeart
      Serial.print("Signal received from heart: ");
      Serial.println(signal);

      // change data type for string operation
      string amplitudeValue = signal.c_str(); 

      if(stoi(amplitudeValue) == P_AMP && lastWave == 'T'){
        beatStartTime = currentTime;
        lastWave='P';
        Serial.println("P wave detected");
      }
      else if(stoi(amplitudeValue) == Q_AMP && lastWave == 'P'){
        lastWave='Q';
        Serial.println("Q wave detected");
      }
      // Detecting R wave
      else if(stoi(amplitudeValue) == R_AMP){
        // anomalous R wave after PQRST
        if(currentTime - beatStartTime <= 1/REFRACTORY_PERIOD){ 
          Serial.println("ignoring anomalous R Wave");
          continue; //ignore
        }
        // detected before lower
        if(currentTime - lastRTime <= lowerBound){
          // measure R-R interval from new wave and maintain pace=0
          lastRTime = currentTime;
          Serial.println("R wave detected before lower bound");
        }

        // if natural R-wave is detected : enable hysteresis pacing
        lastPaceValue == 0 ? upperBound = 1/HRL : 1/LRL;
        lastWave='R';
        Serial.println("R wave detected");
      }
      else if(stoi(amplitudeValue) == S_AMP && lastWave == 'R'){
        lastWave = 'S';
        Serial.println("S wave detected");
      } 
      else if(stoi(amplitudeValue) == T_AMP && lastWave=='S'){
        lastWave = 'T';
        Serial.println("T wave detected");
      }
    }
  }
}

void TaskSendPace(void* pvParameters){
  Serial.println("Thread TaskSendPace: Started");
  while(true){
    // Delay for pace frequency
    myDelay(1 / PACE_FREQUENCY);

    // Send to heart
    Serial1.println(currentPaceValue);

    Serial.print("Sending pace: ");
    Serial.println(currentPaceValue);

    lastPaceValue = currentPaceValue;
    currentPaceValue = 0;  // set to default
  }
}


void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);  // initialize UART with baud rate of 9600
  delay(1000); // prevents usb driver crash on startup, do not omit this
  
  // wait for serial port to connect
  while(!Serial){}
  while(!Serial1){}

  // sets the serial port to print errors to when the rtos crashes
  vSetErrorSerial(&Serial);

  // Create the threads that will be managed by the rtos
  // Sets the stack size and priority of each task
  // Also initializes a handler pointer to each task, which are important to communicate with and retrieve info from tasks
  
  xTaskCreate(
    TaskReadHeart, //task pointer
    "readheart",  //task name-for humans
    128, //stack size
    NULL, //parameter   
    1, //priority
    &Handle_readTask //task handle
  );
/*
  xTaskCreate(
    TaskSendPace, //task pointer
    "sendpace",  //task name-for humans
    128, //stack size
    NULL, //parameter   
    2, //priority
    &Handle_sendTask //task handle
  );
*/
  // Start the RTOS
  vTaskStartScheduler();

  // error scheduler failed to start
  // should never get here
  while(1)
  {
	  Serial.println("Scheduler Failed! \n");
	  Serial.flush();
	  delay(1000);
  }
}


void loop() {

}

