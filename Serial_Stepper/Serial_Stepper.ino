#include <AFMotor.h>

// YAW
AF_Stepper Stepper1(200,1);

// PITCH
AF_Stepper Stepper2(200,2);

// YAW Gear ratio: 4:1
// PITCH Gear ratio: 39.0625:1
// Stepper1 Default: 800
// Stepper2 Default: 7812

#define STEPPER1ROTATIONSTEPS 1600
#define STEPPER2ROTATIONSTEPS 15625

String command;
String value;
float valueFloat;
float deg;

bool commandRead = true;

void setup() {
  Serial.begin(19200);
  Stepper1.setSpeed(100);
  Stepper2.setSpeed(200);
}

void loop() {
  // Release for low power scenarios
  Stepper1.release();
  Stepper2.release();
  command = "";
  value = "";
  commandRead = true;
  while (!Serial.available()){}

  /** 
   *  This while loop reads the data on the Serial input
   *  There are 2 components, the command and the value
   *  If there is a space detected, then the loop will write to value
   */
  while (Serial.available() > 0){
    char c = (char)Serial.read();
    if (c == ' '){
      commandRead = false;
    }
    if (commandRead){
      command += c;
    } else if (c != " "){
      value += c;
    }
    // Delay is needed or else the serial data would not work properly
    delay(5);
  }
  valueFloat = value.toFloat();
  
  if (command.length() > 0){
    if (command == "YAW"){
      deg = (valueFloat/360) * STEPPER1ROTATIONSTEPS;
      if (deg < 0){
        Stepper1.step((int)abs(deg),BACKWARD,INTERLEAVE);
      } else {
        Stepper1.step((int)deg,FORWARD,INTERLEAVE);
      }
      Serial.println("OK");
    } else if (command == "PITCH"){
      deg = (valueFloat/360) * STEPPER2ROTATIONSTEPS;
      if (deg < 0){
        Stepper2.step((int)abs(deg),FORWARD,INTERLEAVE);
      } else {
        Stepper2.step((int)deg,BACKWARD,INTERLEAVE);
      }
      Serial.println("OK");
    } else {
        Serial.println("Invalid");
    }
  }
}
