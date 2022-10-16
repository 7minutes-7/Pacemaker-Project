------------------------- Instructions for running code -------------------------
Wire together two Arduinos as shown in class. Preferably use two computers, as the RandomHeart and Pacemaker each print out separate information to the
Serial Monitor, and it can be helpful to see them side by side. For each code file, just compile and upload them. The program may take a little time to start,
but it will eventually start running.

------------------------- Details for RandomHeart -------------------------

The RandomHeart module simulates random heart beating. It goes through the entire PQRST cycle, sending the voltage of each of these waves to the
Pacemaker at the start of the wave. After the PQRST cycle is done, it sends a voltage of 0 to signify the heartbeat has finished. The time between
beats is random, taking on any value between the lower bound (1/URL) and the hysteresis bound (1/HRL). There is also a 33% chance that after any PQRST
cycle, there will be an anomolous R wave. These specifications are for the natural heart beat. The RandomHeart has a frequency of 100 Hz, computing the current
wave of execution at each cycle. The Pacemaker has a frequency of 50 Hz, sending a signal to the RandomHeart for each of its cycles. The RandomHeart reads the
signal from the Pacemaker every other cycle, since its frequency is twice as hig and we don't want it reading one Pacemaker signal twice. If it receives a '0',
the RandomHeart beats normally. If it receives a '1', the cycle is reset, and the moment it receives the '1' it starts the beat at the P wave. 


------------------------- Details for Pacemaker -------------------------