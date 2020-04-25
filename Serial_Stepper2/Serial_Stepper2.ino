#include <pt.h>

// YAW Gear ratio: 4:1
// PITCH Gear ratio: 39.0625:1
// Stepper1 Default: 800
// Stepper2 Default: 7812

#define STEPPER1ROTATIONSTEPS 25600
#define STEPPER2ROTATIONSTEPS 250000
#define STEPPER1SPEED 100
#define STEPPER2SPEED 200

float deg;

const byte stepPin1 = 7; 
const byte dirPin1 = 6; 
const byte enPin1 = 5;

const byte stepPin2 = 4; 
const byte dirPin2 = 3; 
const byte enPin2 = 2;

// Microsteps
const byte microSteps = 32;
const float stepper1Gearing = 4;
const float stepper2Gearing = 39.0625;

static struct pt pt0, pt1, pt2;

struct Motion {
  float yaw;
  float pitch;
};

class StepperMotor {
  byte stepPin;
  byte dirPin;
  byte enPin;

  uint32_t stepNum = 0;
  double timePerStep;

  pt *thread;
  unsigned long timer;
  uint32_t count;
  bool complete = true;
  
  public:
    StepperMotor(byte stp, byte dir, byte en, struct pt *pt){
      this->stepPin = stp;
      this->dirPin = dir;
      this->enPin = en;
      this->thread = pt;
      pinMode(this->stepPin,OUTPUT); 
      pinMode(this->dirPin,OUTPUT);
      pinMode(this->enPin,OUTPUT);
      digitalWrite(enPin,LOW);
    }
    
  void Move(uint32_t stepNum, uint32_t rpm, bool dir){
    if (dir){
      digitalWrite(this->dirPin,HIGH);
    }
    else {
      digitalWrite(this->dirPin,LOW);
    }
    this->stepNum = stepNum;
    this->timePerStep = 150000.0/(rpm*microSteps);

    this->count = 0;
    this->complete = false;
    MoveLoop();
  }
  
  int MoveLoop(){
    PT_BEGIN(thread);
    for(this->count; this->count < stepNum; this->count++) {
      this->timer = micros();
      digitalWrite(this->stepPin,HIGH);
      PT_WAIT_UNTIL(thread, micros() - this->timer > timePerStep);
      digitalWrite(this->stepPin,LOW); 
      this->timer = micros();
      PT_WAIT_UNTIL(thread, micros() - this->timer > timePerStep);
    }
    PT_END(thread);
  }

  void CheckLoop(){
    if (count >= stepNum){
      this->complete = true;
      //Serial.println(String(this->complete));
    }
    MoveLoop();
  }

  bool CheckState(){
    // Checks whether the loop is complete
    return complete;
  }
      
};

StepperMotor stepper1 = StepperMotor(stepPin1,dirPin1,enPin1,&pt1);
StepperMotor stepper2 = StepperMotor(stepPin2,dirPin2,enPin2,&pt2);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(19200);

  PT_INIT(&pt0);
  PT_INIT(&pt1);
  PT_INIT(&pt2);
}

void loop() {
  commands(&pt0);
  stepper1.CheckLoop();
  stepper2.CheckLoop();
}

uint32_t commands(struct pt *pt){
  PT_BEGIN(pt);
  while(1){
  Motion motion;
  /** 
   *  This while loop reads the data on the Serial input
   *  There are 2 components, the command and the value
   *  If there is a space detected, then the loop will write to value
   */
   
//  while (Serial.available() > 0){
//    char c = (char)Serial.read();
//    if (c == ' '){
//      commandRead = false;
//    }
//    if (commandRead){
//      command += c;
//    } else if (c != " "){
//      value += c;
//    }
//    // Delay is needed or else the serial data would not work properly
//    delay(5);
//  }
  motion = readValue();
  //motion.yaw = readValue();
  //Serial.println(motion.yaw);
  //motion.pitch = readValue();
  //Serial.println(motion.pitch);

  deg = (motion.yaw/360) * STEPPER1ROTATIONSTEPS;
  if (deg < 0){
    stepper1.Move((uint32_t)abs(deg),STEPPER1SPEED,false);
  } else {
    stepper1.Move((uint32_t)deg,STEPPER1SPEED,true);
  }

  deg = (motion.pitch/360) * STEPPER2ROTATIONSTEPS;
  if (deg < 0){
    stepper2.Move((uint32_t)abs(deg),STEPPER2SPEED,false);
  } else {
    stepper2.Move((uint32_t)deg,STEPPER2SPEED,true);
  }
  
  PT_WAIT_UNTIL(pt,stepper1.CheckState() && stepper2.CheckState());
  Serial.println("OK:" + String(motion.yaw) + "," + String(motion.pitch));
  }
  PT_END(pt);
}

Motion readValue(){
  bool readFirstValue = true;
  String value = "";
  float value1;
  float value2;
  while (true){
    if (Serial.available() > 0){
      char c = (char)Serial.read();
      if (c == ','){
        if (readFirstValue) {
          value1 = value.toFloat();
          readFirstValue = false;
          value = "";
        } else {
          value2 = value.toFloat();
          return (Motion){value1,value2};
        }
      } else {
        value += c;
      }
    }
   }
}
