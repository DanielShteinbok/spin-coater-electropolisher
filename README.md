# Arduino Spin Coater/Sample Rotator For Electropolishing
The objective of this project was to make a tool that could be used to rotate a sample for both the purposes of spin-coating and consistent electropolishing (in my case, I intend to rotate a piece of copper foil that is being electropolished to get a consistent degree of polish over the entire sample). The motor used was a 400 KV brushless DC motor typically marketed for drone applications with an appropriate BEC ESC from [RC Electric Parts](https://www.rcelectricparts.com/esc-user-guide.html#04).

Ideally, the electropolishing should be done as a continuous ~600 rpm spin; no ramp-up or ramp-down is needed. On the other hand, spin-coating should be done at 4000 rpm with a preset ramp-up, a strictly-controlled spin time, and a ramp-down.

To begin, the ESC was calibrated using an HJ Servo Consistency Tester to have a minimum throttle of 800
 microsecond pulse-position (800 us gap between pulses provided in the PPM signal) and a maximum 
throttle of 2200 microseconds. The reason for this is that this was the range of the Servo Consistency 
Tester, and I did not want the ESC to limit the throttle at all (the motor's lowest consistent speed 
and highest consistent speed were within these throttle limits). However, I discovered that the motor 
only rotates with a PPM signal of > 920 microseconds, and does not have a linear response 
after a signal of 2130 microseconds. I set these as the functional minimum and maximum signals given 
by my Arduino.

The circuit consists of an Arduino Nano, a button, and a switch (and some resistors, etc). When the 
button is pressed, the Arduino starts either the electropolishing or spin-coating routine, depending 
on the position of the switch at that moment.

In the electropolishing routine, the motor starts spinning at a set speed (currently the lowest PPM 
signal that I can give) and continues until the button is pressed again (at which point it stops and 
quits the routine). 

In the spin-coating routine, the motor ramps to the given final speed over a given ramp-up time, then
rotates at that speed for a given time, and then ramps down to a standstill in a given time.

## Routines:
Generally, I have two routines programmed into the Arduino, with the ability to select between them via the switch.
Currently, the two routines to select between are regular 850-rpm electropolishing, and two-step electropolishing: press the button to start high speend electropolishing, 
then press again to ramp down to 850 rpm over the course of a second. Press a third time to stop completely.

So far, a 100 ms delay between the button being pressed down (so as to eliminate bouncing electrodes), but a problem continues to exist if the button is held down for more than 100 ms 
(and then, upon release, the button bounces and electrodes collide again).
