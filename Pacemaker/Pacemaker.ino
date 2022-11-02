#include "ChRt.h"
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <string>

#include <ArduinoMqttClient.h>
#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_AVR_UNO_WIFI_REV2)
  #include <WiFiNINA.h>
#elif defined(ARDUINO_SAMD_MKR1000)
  #include <WiFi101.h>
#elif defined(ARDUINO_ARCH_ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_NICLA_VISION) || defined(ARDUINO_ARCH_ESP32)
  #include <WiFi.h>
#endif

#include "arduino_secrets.h"
using namespace std;

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;    // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

// To connect with SSL/TLS:
// 1) Change WiFiClient to WiFiSSLClient.
// 2) Change port value from 1883 to 8883.
// 3) Change broker value to a server with a known SSL/TLS root certificate 
//    flashed in the WiFi module.

WiFiSSLClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[]    = "tb-exp.precise.seas.upenn.edu";
int        port        = 8883;
const char willTopic[] = "arduino/will";
const char Topic1[]   = "v1/devices/me/telemetry";
const char Topic2[]   = "v1/gateway/attributes";
const char attribute_topic[] = "v1/devices/me/attributes";

//**************************************************************************
// global variables for pacemaker operation
//**************************************************************************  
int currentPaceValue = 0;
string currentHeartSignal;

/* constants for pacemaker project */
const int HEART_FREQUENCY = 100;
const int PACE_FREQUENCY = 50;
const int PUBLISH_FREQUENCY = 100;

const double P_AMP = 0.2;
const double Q_AMP = -0.2;
const double R_AMP = 0.5;
const double S_AMP = -0.2;
const double T_AMP = 0.2;

int URL = 60;
int LRL = 50;
int HRL = 40;

double REFRACTORY_PERIOD = 0.6;


// Declare a semaphore with an inital counter value of zero.
SEMAPHORE_DECL(sem, 0);

//**************************************************************************
// global variables for RTOS
//**************************************************************************
static THD_WORKING_AREA(waThread1, 128);
static THD_WORKING_AREA(waThread2, 128);
static THD_WORKING_AREA(waThread3, 128);

//------------------------------------------------------------------------------
// continue setup() after chBegin().
void chSetup(){
 // Start threads.
  
  chThdCreateStatic(waThread1, sizeof(waThread1),
    NORMALPRIO + 2, TaskSendPace, NULL);
/*
  chThdCreateStatic(waThread2, sizeof(waThread2),
    NORMALPRIO + 1, TaskReadHeart, NULL);
*/
  chThdCreateStatic(waThread3, sizeof(waThread3),
    NORMALPRIO + 1, TaskMQTT, NULL);
}

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);  // initialize UART with baud rate of 9600
  delay(1000); // prevents usb driver crash on startup, do not omit this
  
  // wait for serial port to connect
  while(!Serial){}
  while(!Serial1){}


  // attempt to connect to WiFi network:
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }

  Serial.println("You're connected to the network");

  // You can provide a unique client ID, if not set the library uses Arduino-millis()
  // Each client must have a unique client ID
  // mqttClient.setId("clientId");

  // You can provide a username and password for authentication
  mqttClient.setUsernamePassword("MDzmeB6mOLuj0G0lGDZl", "");

  // By default the library connects with the "clean session" flag set,
  // you can disable this behaviour by using
  // mqttClient.setCleanSession(false);

  // set a will message, used by the broker when the connection dies unexpectedly
  // you must know the size of the message beforehand, and it must be set before connecting
  String willPayload = "oh no!";
  bool willRetain = true;
  int willQos = 1;

  mqttClient.beginWill(willTopic, willPayload.length(), willRetain, willQos);
  mqttClient.print(willPayload);
  mqttClient.endWill();

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();

  // Initialize OS and then call chSetup.
  chBegin(chSetup);
  // chBegin() resets stacks and should never return
  while(true){}
}

void loop() {
}


//**************************************************************************
// implement tasks
//**************************************************************************
static THD_FUNCTION(TaskReadHeart, arg){
  const double lowerBound = 1000.0*60/URL;
  double upperBound = 1000.0*60/LRL;

  char lastWave='T';
  double beatStartTime;

  bool isNatural = true;
  
  while(true){
    chThdYield();
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
        currentHeartSignal = signal.c_str();

        // change data type from String to double
        double amplitudeValue = stod(currentHeartSignal); 

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

static THD_FUNCTION(TaskSendPace, arg){
  long interval = 1000.0 / PACE_FREQUENCY;
  
  while(true){
    chThdSleepMilliseconds(interval);

    // Send pace signal to the heart
    Serial1.println(currentPaceValue);
    Serial.print("pace sent: ");
    Serial.println(currentPaceValue);
  }
}

static THD_FUNCTION(TaskMQTT, arg){
  long interval = 1000.0 / PUBLISH_FREQUENCY;
  Serial.println("TaskMQTT thread");
  
  while(true){
    chThdSleepMilliseconds(interval);

    mqttClient.subscribe(attribute_topic);
    String message = mqttClient.readString();

    if (message.indexOf("LRL") != -1) {
      LRL = (message.substring(message.length() - 3)).toInt();
    }
    if (message.indexOf("URL") != -1) {
      URL = message.substring(message.length() - 3).toInt();
    }
    if (message.indexOf("HRL") != -1) {
      HRL = message.substring(message.length() - 3).toInt();
    }
    if (message.indexOf("VRP") != -1) {
      REFRACTORY_PERIOD = message.substring(message.length()-3).toFloat();
    }
    
    // Send message to broker
    String payload1;
    DynamicJsonDocument doc1(1024);
    doc1["HS"] = currentHeartSignal;
    doc1["PS"] = String(currentPaceValue);
    serializeJson(doc1, payload1);  

    bool retained = false;
    int qos = 1;
    bool dup = false;

    Serial.print("Try to send message: ");
    Serial.println(payload1);
    
    Serial.println(mqttClient.connected());
    mqttClient.beginMessage(Topic1, payload1.length(), retained, qos, dup);
    mqttClient.print(payload1);
    mqttClient.endMessage();

    Serial.print("Message sent: ");
    Serial.println(payload1);
  }
}

