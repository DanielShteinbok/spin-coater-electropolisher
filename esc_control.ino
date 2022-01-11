#include <Servo.h>

Servo ESC;
int speed;

#define ESC_PIN 9
#define BUTTON 8
#define SWITCH 7
#define MIN_THROTTLE 920
#define MAX_THROTTLE 2130

const int electropolish = 950;
const int spincoat = 2130;

void setup() {
  ESC.attach(ESC_PIN);
  ESC.writeMicroseconds(MIN_THROTTLE); // always start without the motor moving
  pinMode(BUTTON, INPUT);
  pinMode(SWITCH, INPUT);
  Serial.begin(9600);
}

void electroPolish(int signal_us) {
  // signal_us is the pulse position, in microseconds, sent to ESC

  Serial.println("starting electropolish");
  ESC.writeMicroseconds(signal_us);
  while (not digitalRead(BUTTON) == LOW){} // wait until the button is released, so as not to start and then immediately stop
  while (not digitalRead(BUTTON) == HIGH){} // repeat until the button is pressed
  while (not digitalRead(BUTTON) == LOW){} // wait until the button is released, so as not to stop and then immediately restart
  ESC.writeMicroseconds(MIN_THROTTLE); // set the speed to 0
}

void spinCoat(int ramp_up_time, int speed_signal, int plateau_time, int ramp_down_time) {
  // ramp_up_time is time in milliseconds taken to ramp up
  // ramp_down_time is time in milliseconds taken to ramp down
  // speed_signal is the ppm position, in microseconds, sent to ESC

  Serial.println("starting spincoat");

  // ramp up
  ramp(ramp_up_time, speed_signal);
  long startTime = millis(); // the start time, used to time the plateau
  Serial.println(millis() - startTime);
  while (millis() - startTime < plateau_time) {
    Serial.print("PLATEAUING!!!");
    Serial.println(ESC.readMicroseconds()); // want to ensure that it's plateauing  
  }
  ramp(ramp_down_time, MIN_THROTTLE);
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
  } while (currentTime - startTime < ramp_time);
}

void loop() {
  //Serial.println("looping");
  if (digitalRead(BUTTON) == HIGH) {
    Serial.println("Button pressed!");
    if (digitalRead(SWITCH) == HIGH) {
      electroPolish(electropolish);
    }
    else {
      spinCoat(4000, spincoat, 4000, 4000);
    }
  }
}
