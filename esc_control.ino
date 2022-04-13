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
volatile unsigned long lastQueued; // the last time that an action was queued

void setup() {
  ESC.attach(ESC_PIN);
  ESC.writeMicroseconds(MIN_THROTTLE); // always start without the motor moving
  pinMode(BUTTON, INPUT);
  pinMode(SWITCH, INPUT);
  Serial.begin(9600);

  // problem with below: user presses and holds, after this the button may bounce
  //attachInterrupt(digitalPinToInterrupt(BUTTON), queueAct, RISING); // queue a button press when the button is pressed

  // a more complicated solution: remember when the button is pressed, then upon release if a button press is queued, the release queues an action
  // this allows us to ignore long button holds; i.e. you press a button, but change your mind, and keeping it pressed for some amount of time cancels the action
  //attachInterrupt(digitalPinToInterrupt(BUTTON), queueKeyDown, RISING); // queue a button press when the button is pressed

  // perhaps, focusing on when the user releases the button will fix the issue above?
  attachInterrupt(digitalPinToInterrupt(BUTTON), queueAct, FALLING); // queue a button press when the button is pressed
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

void twoStepElectropolish(int first_speed, int second_speed, int ramp_time) {
  // runs high speed electropolishing until an act is queued, then runs low speed electropolishing
  //const int maxEpSig = 1040;
  electroPolish(first_speed);
  ramp(ramp_time, second_speed);
  electroPolish(second_speed);
}

void loop() {
  //Serial.println("looping");
  if (act_queued) {
    clearAct();
    if (digitalRead(SWITCH) == HIGH) {
      electroPolish(electropolish);
    }
    else {
      //spinCoat(4000, spincoat, 4000, 4000);      
      twoStepElectropolish(1040, electropolish, 1000);
    }
    ESC.writeMicroseconds(MIN_THROTTLE); // set the speed to 0 after finishing routine
  }
}

void queueAct() {
  // prevent multiple queueings in a short period of time (due to button bouncing)
  noInterrupts();
  if (millis() - lastQueued > 100) {
    act_queued = true;
    lastQueued = millis();
  }
  interrupts();
}

void clearAct() {
  act_queued = false;
}
