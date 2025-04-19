#include <Servo.h>
#include <Arduino.h>
#include "HC08Bridge.h"
#include "time.h"

// comment to turn off
// #define DEBUG_USS

// comms
#define MAX_MSG_SIZE 64

#define PWMA 53 //pwmA pin
#define STBY 47
#define AIN1 49
#define AIN2 51
#define SPEEDLIMIT 255 // full range 0-255
#define defaultSpeed 150


// STRICT SERVO RANGES
// Steering Servo Range: 0 - 65
// Ultrasonic Servo Range: 50 - 110
#define ussServoPin 52
#define steerServoPin 50
#define minUSSAngle 10
#define maxUSSAngle 160
#define minSteerAngle 0
#define maxSteerAngle 60
#define midSteer int((minSteerAngle + maxSteerAngle) / 2)
#define midUSS int((minUSSAngle + maxUSSAngle) / 2)
#define ussServoRng maxUSSAngle - minUSSAngle
#define ussServoRes 10
#define COLLISIONDIST 20

#define soundSpeed 34 // 34cm/s
#define echoTimeout 38 // timesout after 38 ms
// formula: distance = (soundSpeed * echoRiseTime) / 2

#define echoPin 23
#define trigPin 25

Servo steeringServo;
Servo ultrasonicServo;


// motors
int prevSpeed = 0;
int prevDir = 1;

// Ultrasonic sensor (USS)
volatile bool echo; // set to true when echo pin is high, false when its not
volatile bool trig;
volatile unsigned long echoStartTime;
volatile unsigned long echoEndTime;
volatile bool COLLISIONLOCK;
int ussServoAngle;
int servoDir;
int steerServoAngle;
int count = 0;

uint32_t prevTime;
uint8_t gearState;
uint8_t gasState;
uint8_t brakeState;
uint8_t steerState;


HC08Bridge btBridge(&Serial1, &SerialUSB);
// ----------------------------------------------------------------------------
// ------------------------------------ UTIL FUNCS --------------------------------------

void msgToCmd(char*msg, char*cmd, int size) {
  // int size = strlen(msg);
  for (int i=0; i<size; i++) {
    cmd[i] = msg[i];
  }
  
}

int transmitMSG(char* msg, USARTClass *srl, Serial_ *debug) {
  // char tMsg[MAX_MSG_SIZE] = {};
  char resp[MAX_MSG_SIZE] = {};
  // charArrCpy(msg, tMsg);
  // memcpy(tMsg, msg, strlen(msg));
  debug->print("\nSending: ");
  debug->write(msg, strlen(msg));
  int sent = sendMSG(msg, srl);
  debug->print("\nBytes Sent: " + String(sent));
  delay(200);
  int recv = recvMSG(resp, 32, srl);
  debug->print("\nBytes Recv: " + String(recv));
  if (recv > 0) {
     debug->print("\nResponse: \n");
    debug->write(resp, strlen(resp));
  }
  
  debug->write("\n", 1);
  return recv;
}

int sendMSG(char* msg, USARTClass *srl) {
  int size = strlen(msg);
  char cmd[size];
  msgToCmd(msg,cmd,size);
  int sent = srl->write(cmd, size);
  return sent;
}

int recvMSG(char* resp, int expSize, USARTClass *srl) {
  // return srl->readBytes(resp, expSize);
  char *tPtr = resp;
  int recv = 0;
  while (srl->available() && recv < int(MAX_MSG_SIZE)) {
    recv += srl->readBytes(tPtr, 1);
    tPtr += sizeof(char);
  }
  return recv;
}


int transmitMSG(char* msg, Serial_* srl) {
  // char tMsg[MAX_MSG_SIZE] = {};
  char resp[MAX_MSG_SIZE] = {};
  // charArrCpy(msg, tMsg);
  // memcpy(tMsg, msg, strlen(msg));
  srl->print("\nSending: ");
  srl->write(msg, strlen(msg));
  int sent = sendMSG(msg, srl);
  srl->print("\nBytes Sent: " + String(sent));
  delay(200);
  int recv = recvMSG(resp, 32, srl);
  srl->print("\nBytes Recv: " + String(recv));
  if (recv > 0) {
     srl->print("\nResponse: \n");
    srl->write(resp, strlen(resp));
  }
  
  srl->write("\n", 1);
  return recv;
}

int sendMSG(char* msg, Serial_ *srl) {
  int size = strlen(msg);
  char cmd[size];
  msgToCmd(msg,cmd,size);
  int sent = srl->write(cmd, size);
  return sent;
}

int recvMSG(char* resp, int expSize, Serial_ *srl) {
  // return srl->readBytes(resp, expSize);
  char *tPtr = resp;
  int recv = 0;
  while (srl->available() && recv < int(MAX_MSG_SIZE)) {
    recv += srl->readBytes(tPtr, 1);
    tPtr += sizeof(char);
  }
  return recv;
}

// BLE module
// Use Serial1 fool
// MyBLEModule ble(&Serial1);
// ----------------------------------------------------------------------------
// ------------------------------------ ISR FUNCS --------------------------------------


void echoISR() {
  //int state = digitalRead(echoPin);
  if (!echo) {
    echoStartTime = millis();
    echo = true;
  } else {
    echoEndTime = millis();
  }
}

// ------------------------------------ END ISR FUNCS --------------------------------------
// ------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// ------------------------------------ INIT FUNCS --------------------------------------
void initHC08() {
  delay(200);
  // Serial1.begin(38400);
  Serial1.begin(9600);
}

void initMotor() {
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(STBY, OUTPUT);
  
  
  digitalWrite(STBY, 1);
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  analogWrite(PWMA, 0);
}

void initServos() {
  ultrasonicServo.attach(ussServoPin);
  steeringServo.attach(steerServoPin);
  steerServoAngle = midSteer;
  steeringServo.write(steerServoAngle);
  
}

void initUSS() {
  interrupts();
  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(echoPin), echoISR, CHANGE);
  ussServoAngle = midUSS;
  servoDir = 1;
}


// ------------------------------------ END INIT FUNCS --------------------------------------
// ------------------------------------------------------------------------------------------



// --------------------------------------------------------------------------------------
// ------------------------------------ TEST FUNCS --------------------------------------

void testServos() {
  steeringServo.write(minSteerAngle);
  ultrasonicServo.write(minUSSAngle);
  delay(1000);
  ultrasonicServo.write(midUSS);
  steeringServo.write(midSteer);
  delay(1000);
  ultrasonicServo.write(maxUSSAngle);
  steeringServo.write(maxSteerAngle);
  delay(1000);
  ultrasonicServo.write(midUSS);
  steeringServo.write(midSteer);
  delay(1000);
}

// void testMotor() {
//   driveMotor(PWMA, defaultSpeed);
//   delay(500);
//   driveMotor(PWMA, 0);
//   delay(500);
//   driveMotor(PWMA, -defaultSpeed);
//   delay(500);
//   driveMotor(PWMA, 0);
// }

void testUSSsystem() {
  int startTime = millis();
  while (millis() - startTime < 4000) {
    int dist = runUSS();
    #ifdef DEBUG_USS
      String echoTime = "dist: " + String(dist) + "\nussServoAngle: " + String(ussServoAngle);
      SerialUSB.println(echoTime);
    #endif
    cycleUSSservo();
    delay(1);
  }
  driveUSSservo(midUSS);
}
// ------------------------------------ END TEST FUNCS --------------------------------------
// ------------------------------------------------------------------------------------------



// --------------------------------------------------------------------------------------
// ------------------------------------ MOTOR FUNCS --------------------------------------

void driveMotor(int motorPin, int speed=0) {
  if (gearState == 0) {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
  } else {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
  }

  delay(10);

  // if (gasState && !COLLISIONLOCK) {
  if (gasState) {
    analogWrite(PWMA, defaultSpeed);
  } else {
    analogWrite(PWMA, 0);
  }
  prevTime = millis();
}

// ------------------------------------ END MOTOR FUNCS --------------------------------------
// ------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// ------------------------------------ SERVOR FUNCS --------------------------------------

void steerServo(int angle) {
  
  if (angle < minSteerAngle) {
    angle = minSteerAngle;
  } else if (angle > maxSteerAngle) {
    angle = maxSteerAngle;
  }
  steeringServo.write(angle);
}

void driveUSSservo(int angle) {
  if (angle < minUSSAngle) {
    angle = minUSSAngle;
  } else if (angle > maxUSSAngle) {
    angle = maxUSSAngle;
  }
  ultrasonicServo.write(angle);
}

void cycleUSSservo() {
  ussServoAngle = ussServoAngle + servoDir * ussServoRes;
  if (ussServoAngle > maxUSSAngle) {
    ussServoAngle = maxUSSAngle;
  } else if (ussServoAngle < minUSSAngle) {
    ussServoAngle = minUSSAngle;
  }
  if (ussServoAngle == minUSSAngle || ussServoAngle == maxUSSAngle) {
    servoDir *= -1;
  }
  ultrasonicServo.write(ussServoAngle);
}

// returns dist
int runUSSservo() {
  int dist = runUSS();
  cycleUSSservo();
  return dist;
}

// ------------------------------------ END SERVO FUNCS --------------------------------------
// ------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// ------------------------------------ USS FUNCS --------------------------------------

int runUSS() {
  echo = false;
  #ifdef DEBUG_USS
    String ussStr = "Starting Conditions: \nechoStartTime: " + String(echoStartTime) + "\nechoEndTime: " + String(echoEndTime);
    SerialUSB.println(ussStr);
  #endif
  digitalWrite(trigPin, 1);
  delayMicroseconds(10);
  digitalWrite(trigPin, 0);
  #ifdef DEBUG_USS
    ussStr = "Ending Conditions: \nechoStartTime: " + String(echoStartTime) + "\nechoEndTime: " + String(echoEndTime);
    SerialUSB.println(ussStr);
  #endif
  delay(10);
  echo = true;
  delay(28);
  
  int echoDelta = abs(uint32_t(echoEndTime - echoStartTime));
  int dist = soundSpeed * echoDelta / 2;

  return dist;
}

// ------------------------------------ END USS FUNCS --------------------------------------
// ------------------------------------------------------------------------------------------


// ------------------------------------------------------------------------------------------
// ------------------------------------ TEMP FUNCS --------------------------------------

// cmd structure
// char cmd[12] ={'~','|', gearState,'|', gasState, '|', brakeState, '|', encoderState, '|', '~', '\0'};
// char resp[6]; //= {~, |, K, C, |, ~}
uint8_t updateBT(bool collision) {
  char cmd[12];
  char resp[7] = {'~', '|', 'N', 'C', '|', '~', '\0'};
  uint8_t tempVals[4] = {};
  Serial1.readBytes(cmd, 12);

  SerialUSB.print(cmd);
  uint8_t valid = 0;

  if (cmd[0] == '~' && cmd[1] == '|' && cmd[9] == '|' && cmd[10] == '~') {
    valid = 1;
    tempVals[0] = uint8_t(cmd[2]-48);
    tempVals[1] = uint8_t(cmd[4]-48);
    tempVals[2] = uint8_t(cmd[6]-48);
    tempVals[3] = uint8_t(cmd[8]-65);
    for (int i=0; i<3; i++) {
      if (tempVals[i] > 1) {
        valid = 0;
      }
    }
    if (tempVals[3] > 60) {
      valid = 0;
    }
  }
  if (valid) {
    resp[2] = 'K';
    if (collision) {
      resp[3] = 'C';
    }
    gearState = tempVals[0];
    gasState = tempVals[1];
    brakeState = tempVals[2];
    steerState = minSteerAngle + tempVals[3];
  }
  char temp[5] = {tempVals[0], tempVals[1], tempVals[2], tempVals[3], '\0'};

  Serial1.write(resp);
  SerialUSB.write(resp);
  SerialUSB.print(String(temp));
  return valid;

}
void updateCarState() {
  steerServo(steerState);
  driveMotor(PWMA);
}
void setup() {
  gearState = 0; // forward
  gasState = 0;
  brakeState = 0;
  // init Serial
  SerialUSB.begin(115200);
  // while (!SerialUSB) {}
  delay(1000);

  // SerialUSB.println("Serial Ready \nPeripheral INIT...");
  // //------------
  
  initServos();
  initMotor();
  initUSS();

  
  initHC08();
  // btBridge.begin(); // for testing
  prevTime = millis();
  
  //------------
}

void loop() {
  // btBridge.update(); // for testing
  uint8_t valid = 0;
  
  int dist = 0; //runUSSservo(); // confirm nothing on USS
  COLLISIONLOCK = dist <= COLLISIONDIST;

  if (Serial1.available()) {
    valid = updateBT(false);
  }
  updateCarState();
  
  // if (valid) {
  //   updateCarState();
  //   // prevTime = millis();
  // }

  delay(1);

}
