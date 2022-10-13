const int heartExecutionFrequency = 100;

const double P_interval = 0.11;
const double Q_interval = 0.02;
const double R_interval = 0.07;
const double S_interval = 0.02;
const double T_interval = 0.3;

// are distinct values necessary?
// is there another way to distingush waves?
const double P_amplitude = 0.2;
const double Q_amplitude = -0.2;
const double R_amplitude = 0.5;
const double S_amplitude = -0.2;
const double T_amplitude = 0.2;


void setup() {
  
  Serial1.begin(9600);  // initialize UART with baud rate of 9600
}

void loop() {
  while(Serial1.available()>=0){  // if there is any byte available to be read on UART1 buffer
    char paceValue = Serial1.read(); // read pace value from Pacemaker
    if(paceValue == '1'{ 
      
    })
    else if(paceValue == '0'){ //normal heart

    }
    /*
      generate different combinations of heart beats
    */
  }
}
