## Milestone 1(a)

------------------------- Instructions for running code -------------------------

Wire together two Arduinos as shown in class. Preferably use two computers, as the RandomHeart and Pacemaker each print out separate information to the
Serial Monitor, and it can be helpful to see them side by side. For each code file, just compile and upload them. The program may take a little time to start,
but it will eventually start running.

------------------------- Variable values for operation-------------------------

Heart Execution Frequency (Hz): 100Hz

Pacemaker Execution Frequency (Hz): 50Hz

PQRST wave time intervals (seconds), amplitudes (milli Volts):
-P wave: 0.11 seconds, 0.2mV
-Q wave: 0.02 seconds, -0.2mV
-R wave: 0.07 seconds, 0.5mV
-S wave: 0.02 seconds, -0.2mV
-T wave: 0.3 seconds, 0.2mV

URL (beats per minute): 60 bpm

LRL (beats per minute): 50 bpm

Refractory Period (seconds): 0.6 seconds

HRL (beats per minute): 40 bpm


------------------------- Details for RandomHeart -------------------------

The RandomHeart module simulates random heart beating. It goes through the entire PQRST cycle, sending the voltage of each of these waves to the
Pacemaker at the start of the wave. After the PQRST cycle is done, it sends a voltage of 0 to signify the heartbeat has finished. The time between
beats is random, taking on any value between the lower bound (1/URL) and the hysteresis bound (1/HRL). There is also a 33% chance that after any PQRST
cycle, there will be an anomolous R wave. These specifications are for the natural heart beat. The RandomHeart has a frequency of 100 Hz, computing the current
wave of execution at each cycle. The Pacemaker has a frequency of 50 Hz, sending a signal to the RandomHeart for each of its cycles. The RandomHeart reads the
signal from the Pacemaker every other cycle, since its frequency is twice as big and we don't want it reading one Pacemaker signal twice. If it receives a '0',
the RandomHeart beats normally. If it receives a '1', the cycle is reset, and the moment it receives the '1' it starts the beat at the P wave. 


------------------------- Details for Pacemaker -------------------------

The Pacemaker module reads the voltage from the RandomHeart and sends an appropriate pace signal from its calculations. The voltage signals are distinguished by 
the amplitude we've set for each wave and the order it came in, e.g, T is a 0.2mV wave that comes after S. The default pace value is '0' for a normal R wave arriving
between the lower bound and upper bound. However, if the R-R interval goes beyond the upper bound, the pace is set to '1' in order to signal a restart of the 
RandomHeart. In calculating whether the R-R interval lies within the normal range, two factors are further considered. An additional R wave detected within the 
refractory period is ignored, not impacting the interval. If a natural R-wave is detected, the lower rate limit (LRL) is lowered to the hysteresis limit (HRL), extending
the period for R sensing.
The functionality of the Pacemaker is split into two threads using the FreeRTOS library. Thread "TaskReadHeart" reads voltage signals from the heart and calculates 
pace values from them, while Thread "TaskSendPace" sends the calculated pace value to the RandomHeart. "TaskSendPace" has a higher priority over "TaskReadHeart".
It interrupts "TaskReadHeart" in the frequency of the Pacemaker, allowing the paces to be sent in a regular manner.


## Milestone 1(b)
------------------------- Details for Pacemaker -------------------------
We had some trouble sending messages to the MQTT broker inside a task implemented with the FreeRTOS library and decided to use the TaskScheduler library instead.
There are a total of three threads on the pacemaker. The first two "t1", "t2" being the threads implemented on Milestone 1(a) to read heartsignals and send the calculated 
pace signals. 
/* explanation about message receiving should be added */ 
The new third thread "t3" communicates with the MQTT broker. It creates a JSON string with the current heartbeat and pace signal value and sends it to the broker in  
the topic "v1/devices/me/telemetry".
The three threads are scheduled in a priority order of "t3" > "t2" > "t1".  


