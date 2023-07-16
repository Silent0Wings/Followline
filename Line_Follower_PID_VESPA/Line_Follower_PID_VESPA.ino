/****************************************************************
* Folow Line
* CESAR School
* 
* Line Follower PID with the microcontroller Vespa 
* from RoboCore and the Pololu's QTR-8RC sensor
****************************************************************/

#include <RoboCore_Vespa.h>  //Library for the Vespa microcontroller
#include <QTRSensors.h>      //Library for the QTR-8A or the QTR-8RC

VespaMotors motor;
QTRSensors qtr;

#define DEBUG

// Set button and led pins
const uint8_t PIN_BUTTON = 35;
const uint8_t PIN_LED = 15;

//Setup of the module of sensors:
const uint8_t SensorCount = 8;
uint16_t sensorValues[SensorCount];

//Stop sensor variable
bool robotRun = false;

//Marker sensor variables
int markerCount = 10;
int markerCountNow = 0;
const int PIN_MARKER_SENSOR = 36;
bool findLine = false;

// Limit value of the margin of error
const int MARGIN_ERROR = 20;

//------------------PID Control-------------------
float P = 0, I = 0, D = 0, PID = 0, error = 0, lastError = 0;

float Kp = 0.25;
float Ki = 0.0001;
float Kd = 3;

byte maxSpeed = 100;
int lSpeed, rSpeed;

const bool LINE_BLACK = false;
//-------------------------------------------------

void setup() {
  qtr.setTypeRC();  //For QTR-8RC      Sensor pins:
  qtr.setSensorPins((const uint8_t[]){ 17, 16, 18, 5, 23, 19, 22, 21 }, SensorCount);

  pinMode(PIN_BUTTON, INPUT);
  pinMode(PIN_LED, OUTPUT);

#ifdef DEBUG
  Serial.begin(115200);
  delay(100);
#endif

  // Calibration
  Serial.println("Calibration start...");
  digitalWrite(PIN_LED, HIGH);
  while (digitalRead(PIN_BUTTON) == HIGH) {  // Calibrates until the button is pressed
    qtr.calibrate();
  }
  Serial.println("Calibration finished...");
  digitalWrite(PIN_LED, LOW);

  for (uint8_t i = 0; i < SensorCount; i++) {
    Serial.print(qtr.calibrationOn.minimum[i]);
    Serial.print(' ');
  }
  Serial.println();

  // print the calibration maximum values measured when emitters were on
  for (uint8_t i = 0; i < SensorCount; i++) {
    Serial.print(qtr.calibrationOn.maximum[i]);
    Serial.print(' ');
  }
  Serial.println();

  //marker setup
  pinMode(PIN_MARKER_SENSOR, INPUT);

  delay(2000);  // Start loop after 2 seconds
}

void loop() {
  //Read sensors
  error = map(readSensors(), 0, 7000, -1000, 1000);

  //Calculate PID
  P = error;
  I = I + error;
  D = error - lastError;
  PID = (Kp * P) + (Ki * I) + (Kd * D);
  lastError = error;

  //Control Motors
  lSpeed = maxSpeed + PID;
  rSpeed = maxSpeed - PID;

  lSpeed = constrain(lSpeed, -maxSpeed, maxSpeed);
  rSpeed = constrain(rSpeed, -maxSpeed, maxSpeed);

  //If the error value is less than MARGIN_ERROR, move on
  if (error >= -MARGIN_ERROR && error <= MARGIN_ERROR) {
    motor.turn(maxSpeed, maxSpeed);
  } else {
    motor.turn(lSpeed, rSpeed);
  }

  //Count the markers and stop the robot when reach a certain number
  if (markerChecker()) {
    digitalWrite(PIN_LED, HIGH);
    motor.stop();
  }
}

int readSensors() {
  if (LINE_BLACK) {
    return qtr.readLineBlack(sensorValues);
  } else {
    return qtr.readLineWhite(sensorValues);
  }
}

/**
  Counts the number of crossed lines.

  @return `true` if the limit number of rows was detected.
*/
bool markerChecker() {
  if (analogRead(PIN_MARKER_SENSOR) < 2000 && findLine == false) {
    findLine = true;
  }

  if (analogRead(PIN_MARKER_SENSOR) >= 2000 && findLine == true) {
    findLine = false;
    markerCountNow++;
  }

  if (markerCountNow >= markerCount) {
    return true;
  }

  return false;
}
