Milestone 1(a)

//**************************************************************************
// variable values for operation
//**************************************************************************  
Heart Execution Frequency (Hz): 100Hz

Pacemaker Execution Frequency (Hz): 50Hz

PQRST wave time intervals (seconds), amplitudes (milli Volts):
 P wave: 0.11 seconds, 0.2mV
 Q wave: 0.02 seconds, -0.2mV
 R wave: 0.07 seconds, 0.5mV
 S wave: 0.02 seconds, -0.2mV
 T wave: 0.3 seconds, 0.2mV

URL (beats per minute): 60 bpm

LRL (beats per minute): 50 bpm

Refractory Period (seconds): 0.5 seconds

HRL (beats per minute): 40 bpm

//************************************************************************

Pacemaker implementation

Using FreeRTOS we've split the functionality of the pacemaker into two threads; one which reads values from the heart 



