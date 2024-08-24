/*
   Description: This program is for controlling Bug. It recieves commands from the robot remote and uses those commands to determine how to drive Bug's motors.

   Components: The required components are an Arduino Uno, Emakefun Motor Driver Shield, NRF24L01 module, 2x DC 3-6V Gearbox Motors, Ultrasonic Distance Sensor, and a 2 Cell Lipo Battery.
        All components are wired to their dedicated connections on the Emakefun Shield.
*/

#include "Emakefun_MotorDriver.h"                                                                   //Include library for Emakefun Motor Driver Shield
Emakefun_MotorDriver mMotorDriver = Emakefun_MotorDriver(0x60, MOTOR_DRIVER_BOARD_V5);
Emakefun_DCMotor *left = mMotorDriver.getMotor(M2);                                                 //Define Motors and Servos
Emakefun_DCMotor *right = mMotorDriver.getMotor(M4);

#include <RF24.h>                                                                                   //Include library for NRF24L01 Radio Transceiver
RF24 radio(10, 9);
const byte chan[6] = "00007";
byte data[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#include <NewTone.h>
int buzzer = A0;

int trig = A2;                                                                                      //Define pins for Ultrasonic Distance Sensor
int echo = A3;

long xDir = 0;                                                                                      //Variables for requested speed and directions from remote
long yDir = 0;
double spd = 0;

void setup() {
  Serial.begin(9600);

  radio.begin();                                                                                    //Begin radio communication
  radio.openReadingPipe(1, chan);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();

  mMotorDriver.begin(50);                                                                           //Initialize Motor Driver

  pinMode(trig, OUTPUT);                                                                            //Set mode of Ultrasonic Distance Sensor pins
  pinMode(echo, INPUT);

  delay(1000);
}

void loop() {
  if (radio.available()) {                                                                          
    radio.read(&data, sizeof(data));                                                                //Read in radio data if available

    if (data[4] == 0) xDir = 0;                                                                     //Steering controlled by left to right motion of right joystick
    else xDir = map(data[4], 1, 255, -255, 255);

    if (data[1] == 0) yDir = 0;                                                                     //Forward and reverse controlled by front to back motion of left joystick
    else yDir = map(data[1], 1, 255, -255, 255);

    spd = map(data[9], 0, 255, 25, 100);                                                            //Driving speed controlled by right potentiometer
    spd = spd / 100;
  }


  if (yDir == 0) {                                                                                  //Logic for driving motors from directions commanded
    if (xDir == 0) {
      left->run(BRAKE);
      right->run(BRAKE);
    }
    else if (xDir > 0) {
      left->setSpeed(xDir * spd);
      left->run(FORWARD);
      right->setSpeed(xDir * spd * 0.6);
      right->run(BACKWARD);
    }
    else {
      left->setSpeed(-xDir * spd);
      left->run(BACKWARD);
      right->setSpeed(-xDir * spd * 0.6);
      right->run(FORWARD);
    }
  }
  else if (yDir > 0) {
    if (xDir > 0) {
      left->setSpeed(yDir * spd);
      left->run(FORWARD);
      right->setSpeed((yDir - ((xDir * yDir) / 255))*spd * 0.6);
      right->run(FORWARD);
    }
    else {
      left->setSpeed((yDir + ((xDir * yDir) / 255))*spd);
      left->run(FORWARD);
      right->setSpeed(yDir * spd * 0.6);
      right->run(FORWARD);
    }
  }
  else {
    if (xDir > 0) {
      left->setSpeed(-yDir * spd);
      left->run(BACKWARD);
      right->setSpeed((-yDir + ((xDir * yDir) / 255))*spd * 0.6);
      right->run(BACKWARD);
    }
    else {
      left->setSpeed((-yDir - ((xDir * yDir) / 255))*spd);
      left->run(BACKWARD);
      right->setSpeed(-yDir * spd * 0.6);
      right->run(BACKWARD);
    }
  }
                                                                     

  if (data[6]) NewTone(buzzer, 440);                                                                //Activate horn (buzzer) if right joystick is pressed
  else noNewTone(buzzer);

  int distance = Ping();                                                                            //Perform function to get distance from Ultrasonic Distance Sensor
  Serial.println(distance);

  delay(10);
}

int Ping() {                                                                                        //Function to return distance indicated by sensor in inches
  long duration, inches;
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);                                                                         //Sends out pulse
  delayMicroseconds(5);
  digitalWrite(trig, LOW);
  duration = pulseIn(echo, HIGH);                                                                   //Measures how long it take for the pulse to return
  inches = duration / 74 / 2;                                                                       //Calculates how many inches sound would travel in this time and divides by 2 for round trip
  inches = constrain(inches, 0, 120);                                                               //Limits readouts to be from 0 to 120 inches
  return inches;                                                                                    //Returns the distance in inches
}
