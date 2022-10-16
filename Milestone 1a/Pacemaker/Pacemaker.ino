#include <FreeRTOS_SAMD21.h>
#include <string>
#include "pacemaker.h"
using namespace std;

//**************************************************************************
// global variables for pacemaker operation
//**************************************************************************  
int currentPaceValue = 0;

//**************************************************************************
// global variables for RTOS
//**************************************************************************
TaskHandle_t Handle_readTask;
TaskHandle_t Handle_sendTask;

void TaskReadHeart(void*);
void TaskSendPace(void*);

//**************************************************************************
// Can use these function for RTOS delays
// Takes into account processor speed
// Use these instead of delay(...) in rtos tasks
//**************************************************************************
void myDelayMs(int ms)
{
  vTaskDelay( ms / portTICK_PERIOD_MS );  
}

//*****************************************************************

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

  xTaskCreate(
    TaskSendPace, //task pointer
    "sendpace",  //task name-for humans
    128, //stack size
    NULL, //parameter   
    2, //priority
    &Handle_sendTask //task handle
  );

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


//**************************************************************************
// implement tasks
//**************************************************************************
void TaskReadHeart(void* pvParameters){
  const double lowerBound = 1000.0*60/URL;
  double upperBound = 1000.0*60/LRL;

  char lastWave='T';
  double beatStartTime;

  bool isNatural = true;
  
  while(true){
    unsigned long currentTime = millis(); //time in milliseconds
    static unsigned long lastRTime = currentTime;
  
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

        // change data type from String to double
        double amplitudeValue = stod(signal.c_str()); 

        // Checking which wave we got
        if(amplitudeValue == P_AMP  && (lastWave == 'T' || lastWave == 'R')){
          beatStartTime = currentTime;
          lastWave='P';
        }
        else if(amplitudeValue == Q_AMP && lastWave == 'P'){
          lastWave='Q';
        }
        // Detecting R wave
        else if(amplitudeValue == R_AMP){
          // anomalous R wave after PQRST
          if(currentTime - beatStartTime <= 1000.0 * REFRACTORY_PERIOD && lastWave == 'T'){ 
            Serial.print("ignoring anomalous R Wave: ");
            Serial.println(1);
            continue; //ignore
          }
          
          // detected before lower
          if(currentTime - lastRTime <= lowerBound && lastWave == 'Q'){
            // measure R-R interval from new wave and maintain pace=0
            Serial.print("R wave detected before lower bound: ");
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
        else if(amplitudeValue == S_AMP && lastWave == 'R'){
          lastWave = 'S';
        } 
        else if(amplitudeValue == T_AMP && lastWave=='S'){
          lastWave = 'T';
        }
    }
  }
}


void TaskSendPace(void* pvParameters){
  long interval = 1000.0 / PACE_FREQUENCY;
  
  while(true){
    myDelayMs(interval);

    // Send pace signal to the heart
    Serial1.println(currentPaceValue);
    Serial.print("pace sent: ");
    Serial.println(currentPaceValue);
  }
}


