#include <Servo.h>

Servo ESC;
int speed;

#define ESC_PIN 9
#define BUTTON 2
#define SWITCH 7
#define MIN_THROTTLE 920
#define MAX_THROTTLE 2130

const int electropolish = 950;
const int spincoat = 1350;

volatile bool act_queued = false; // set to true when an action is cued by e.g. a button press

void setup() {
  ESC.attach(ESC_PIN);
  ESC.writeMicroseconds(MIN_THROTTLE); // always start without the motor moving
  pinMode(BUTTON, INPUT);
  pinMode(SWITCH, INPUT);
  Serial.begin(9600);

  attachInterrupt(digitalPinToInterrupt(BUTTON), queueAct, RISING); // queue a button press when the button is pressed
}

void electroPolish(int signal_us) {
  // signal_us is the pulse position, in microseconds, sent to ESC

  Serial.println("starting electropolish");
  ESC.writeMicroseconds(signal_us);
  while (not act_queued) {} // wait for an act to be queued by a button press
  clearAct();
}

void spinCoat(int ramp_up_time, int speed_signal, int plateau_time, int ramp_down_time) {
  // ramp_up_time is time in milliseconds taken to ramp up
  // ramp_down_time is time in milliseconds taken to ramp down
  // speed_signal is the ppm position, in microseconds, sent to ESC

  Serial.println("starting spincoat");
  // start ramping
  ramp(ramp_up_time, speed_signal);

  // if an act is queued, exit
  if (act_queued) {
    clearAct();
    return;
  }
  
  long startTime = millis(); // the start time, used to time the plateau
  Serial.println(millis() - startTime);
  while (millis() - startTime < plateau_time) {
    //Serial.print("PLATEAUING!!!");
    // Serial.println(ESC.readMicroseconds()); // want to ensure that it's plateauing
    
    // exit this funtion if the button is pressed
    if (act_queued) {
      clearAct();
      return;
    }
  }
  ramp(ramp_down_time, MIN_THROTTLE);
  if (act_queued) {
    clearAct();
  }
}

void ramp(int ramp_time, int final_speed) {
  // ramp_up_time is time in milliseconds taken to ramp up
  // final_speed is the signal pulse position, corresponding to the final speed after ramp up

  int startingPosition = ESC.readMicroseconds();
  long startTime = millis(); // the start time, used to time ramping
  long currentTime;
  do {
    currentTime = millis();
    ESC.write(
      map(currentTime, startTime, startTime + ramp_time, startingPosition, final_speed)
    );
    //Serial.println(ESC.readMicroseconds());

    // if an act is queued, exit. Do not clear act because the calling function may need to handle the abortion too.
    if (act_queued) {
      return;
    }
  } while (currentTime - startTime < ramp_time);
}

void loop() {
  //Serial.println("looping");
  if (act_queued) {
    clearAct();
    if (digitalRead(SWITCH) == HIGH) {
      electroPolish(electropolish);
    }
    else {
      spinCoat(4000, spincoat, 4000, 4000);
    }
    ESC.writeMicroseconds(MIN_THROTTLE); // set the speed to 0 after finishing routine
  }
}

void queueAct() {
  act_queued = true;
}

void clearAct() {
  act_queued = false;
}
