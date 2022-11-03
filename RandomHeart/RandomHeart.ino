// define constants here
int heart_frequency = 100;
int pacemaker_frequency = 50;

double P_interval = 0.11;
double Q_interval = 0.02;
double R_interval = 0.07;
double S_interval = 0.02;
double T_interval = 0.3;

double P_amplitude = 0.2;
double Q_amplitude = -0.2;
double R_amplitude = 0.5;
double S_amplitude = -0.2;
double T_amplitude = 0.2;

int execution_steps[] = {P_interval*100, Q_interval*100, R_interval*100, S_interval*100, T_interval*100, 
                         R_interval*100, 99999};
double volts[] = {P_amplitude, Q_amplitude, R_amplitude, S_amplitude, T_amplitude, R_amplitude, 0};
int curr_pulse = 0;
int num_executed = 0;

int cycles = 0;

double refract_period = 0.6;

int LRL = 40;
int URL = 60;
int HRL = 30;

const long interval = 1000.0 / heart_frequency;
unsigned long previousMillis = 0;

double upper_bound =  (1.0 / LRL) * 60 * 100;
double lower_bound = (1.0 / URL) * 60 * 100;
double hysteresis_bound = (1.0 / HRL) * 60 * 100;

int period_curr = 0;
int period_end = random(lower_bound, hysteresis_bound);

// in any cycle, 1/3 chance that there is an anomolous R wave
int anomolous_R = random(0,100);
bool anomolous = (anomolous_R <= 33) ? true : false;

bool pace_handled = false;

String pace = "0";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);               // initialize serial communication at 9600 bits per second:
  Serial1.begin(9600); 
}

void loop() {
  // put your main code here, to run repeatedly:
  // send info to the pacemaker every cycle, saying which stage the heart is in
  unsigned long currentMillis = millis();
  bool remote_control = false;
  String message = "";

  if (Serial1.readStringUntil('\n').indexOf("LRL") != -1 || Serial1.readStringUntil('\n').indexOf("URL") != -1 || Serial1.readStringUntil('\n').indexOf("HRL") != -1) {
    remote_control = true;
    message = Serial1.readStringUntil('\n');
  }

  if (message.indexOf("LRL") != -1) {
    LRL = message.substring(message.length() - 3).toInt();
    upper_bound =  (1.0 / LRL) * 60 * 100;
  }
  if (message.indexOf("URL") != -1) {
    URL = message.substring(message.length() - 3).toInt();
    lower_bound = (1.0 / URL) * 60 * 100;
  }
  if (message.indexOf("HRL") != -1) {
    HRL = message.substring(message.length() - 3).toInt();
    hysteresis_bound = (1.0 / HRL) * 60 * 100;
  }

  if (currentMillis - previousMillis >= interval) {
    // save the last time a message was sent
    previousMillis = currentMillis;
    if (cycles % 2 == 0 && !remote_control) {
      pace = Serial1.readStringUntil('\n'); 
    }

    if (pace.indexOf('0') != -1 || (pace.indexOf('1') != -1 && pace_handled)) {

      //Serial.print("HERE");

        // check if this wavelet has an anomolous R pulse
        anomolous = (anomolous_R <= 33) ? true : false;

        if (period_curr == 0) {
          curr_pulse = 0;
        }

        if (num_executed == 0) {

          Serial.println(String(volts[curr_pulse],2));
          Serial1.println(String(volts[curr_pulse],2));

        }

        num_executed++;
        period_curr++;

        // if we finish one wave, go to next wave
        if (num_executed == execution_steps[curr_pulse]) {
          
          curr_pulse++;

          // if there is not an anomolous R wave, voltage is 0 since wavelet is done
          if (!anomolous && curr_pulse == 5) {
            curr_pulse = 6;
          }

          if (anomolous && curr_pulse >= 7) {
            curr_pulse = 0;
          }

          num_executed = 0;

        }

        // reset period
        if (period_curr == period_end) {
          num_executed = 0;
          curr_pulse = 0;

          anomolous_R = random(0,100);

          pace_handled = false;

          period_curr = 0;
          period_end = random(lower_bound, hysteresis_bound);
        }

    }

    // reset period, starting again from P and Q
    if (pace.indexOf('1') != -1 && !pace_handled) {
      num_executed = 0;
      curr_pulse = 0;
      period_curr = 0;
      period_end = random(lower_bound, hysteresis_bound);

      anomolous_R = random(0,100);

      pace_handled = true;

      //Serial.println("paced");

    }

    cycles++;

  }
}

