#include <TM1637.h>
#include <Stepper.h>
#include <Servo.h>

Servo Servomotor;

#define Servopin 3

#define stepsPerRevolution 2048

Stepper myStepper(stepsPerRevolution, 8, 10, 9, 11);

const int CLK = 13;
const int DIO = 12;
TM1637 tm1637(CLK, DIO);

#define ps 23   // maximum parking spaces

byte fps = ps; // free parking spaces

int nv1;
int nv2;
int nv3;

bool etage = 0; // 0 = ground floor, 1 = upper floor

void setup() {
  Servomotor.attach(Servopin);

  // Test barrier movement
  Servomotor.write(90);
  delay(1000);
  Servomotor.write(180);
  delay(1000);

  // Read baseline values from light sensors
  nv1 = analogRead(A1);
  nv2 = analogRead(A2);
  nv3 = analogRead(A3);

  pinMode(5, INPUT_PULLUP); // elevator home switch
  pinMode(6, INPUT_PULLUP); // floor 1 button
  pinMode(7, INPUT_PULLUP); // floor 2 button

  myStepper.setSpeed(5);

  // Move elevator to home position
  while (digitalRead(5)) {
    myStepper.step(stepsPerRevolution / 4);
  }

  delay(500);
  myStepper.setSpeed(10);

  // Initialize display
  tm1637.init();
  tm1637.displayNum(8888);
  tm1637.set(4);
  delay(2000);

  // Display test text (scrolling)
  tm1637.displayStr((char *)"DISPLAY-TEST", 500);
  delay(2000);

  // Show sensor calibration values
  tm1637.displayNum(nv1);
  delay(3000);
  tm1637.displayNum(nv2);
  delay(3000);
  tm1637.displayNum(nv3);
  delay(3000);

  // Check if environment is bright enough
  if (nv1 > 70 && nv2 > 70 && nv3 > 70) {
    tm1637.displayStr((char *)"OK");
  } else {
    tm1637.displayStr((char *)"TO_DARK", 750);
  }

  delay(2000);

  // Show initial free parking spaces
  tm1637.displayNum(fps);
  delay(2000);
}

void loop() {

  // ---------------- Barrier control ----------------
  // Car enters parking
  if (analogRead(A1) < nv1 - 50 && fps > 0) {

    Servomotor.write(90); // open barrier

    // Wait until car passes first sensor
    while (analogRead(A1) < nv1 - 30);

    // Wait until car reaches second sensor
    while (analogRead(A2) > nv2 - 50);

    delay(20);

    // Wait until car leaves second sensor
    while (analogRead(A2) < nv2 - 30);

    Servomotor.write(180); // close barrier

    // Decrease free spaces (safe)
    if (fps > 0) fps--;

    tm1637.displayNum(fps);

    if (fps == 0) {
      tm1637.displayStr((char *)"FULL");
    }
  }

  // ---------------- Exit detection ----------------
  // Car leaves parking
  if (analogRead(A3) < nv3 - 50 && fps < ps) {

    // Wait until car passes sensor
    while (analogRead(A3) < nv3 - 30);

    // Increase free spaces (safe)
    if (fps < ps) fps++;

    tm1637.displayNum(fps);
  }

  // ---------------- Elevator control ----------------

  // Move to ground floor
  if (digitalRead(6) == LOW && etage == 1) {
    myStepper.step((int)(stepsPerRevolution * 2.88)); // FIX: cast to int
    etage = 0;
  }

  // Move to upper floor
  if (digitalRead(7) == LOW && etage == 0) {
    myStepper.step((int)(-stepsPerRevolution * 2.88)); // FIX: cast to int
    etage = 1;
  }
}