#include <SharpIR.h>
#include <PID_v1.h>
#include <DualVNH5019MotorShield.h>
#include <EnableInterrupt.h>
#include "RobotMotor.h"
#include "RobotSensor.h"
#include <math.h>
#include <ArduinoSort.h>  // for median sensor reading funct

RobotMotor motor;

/*==
2YK Sensors range 20-150cm (Big sensor)

21YK Sensor range 10-80cm (Small Sensor)
==*/

/*
 * GP2Y0A21YK0F = short 10cm to 80cm
 * GP2Y0A2YK0F = long 20cm to 150cm
 * 
 * Physical configuration of sensors on robot:
 *       Front
 *    |  S S S  |
 *    | S     L |
 *    | S       |
 *       Back
 * There are three front facing short range, two left facing short range, one right facing long range.
 * The algorithm implemented is left hugging.
 */

// This definition of the Sharp IR sensors should be used with the EDITED library
SharpIR ir_FL(GP2Y0A21YK0F, A0);  // Front left short range sensor
SharpIR ir_FC(GP2Y0A21YK0F, A1);  // Front center short range sensor
SharpIR ir_FR(GP2Y0A21YK0F, A2);  // Front right short range sensor
SharpIR ir_LF(GP2Y0A21YK0F, A3);  // Left front short range sensor
SharpIR ir_LB(GP2Y0A21YK0F, A4);  // Left back short range sensor
SharpIR ir_R(GP2Y0A02YK0F, A5);   // Right facing long range sensor

#define FL A0
#define FC A1
#define FR A2
#define LF A3
#define LB A4
#define R A5 

// This definition of the Sharp IR sensors should be used with the DEFAULT library
//SharpIR ir_FL(SharpIR::GP2Y0A21YK0F, A0); // front left
//SharpIR ir_FC(SharpIR::GP2Y0A21YK0F, A1); // Front center
//SharpIR ir_FR(SharpIR::GP2Y0A21YK0F, A2); // Front Right
//SharpIR ir_LF(SharpIR::GP2Y0A21YK0F, A3); // Left Front
//SharpIR ir_LB(SharpIR::GP2Y0A21YK0F, A4); // Left Back
//SharpIR ir_R(SharpIR::GP2Y0A21YK0F, A5); // Right

//===================== Motor Pins =====================
const int M1A = 3;
const int M1B = 5;
const int M2A = 11;
const int M2B = 13;

DualVNH5019MotorShield md;

//Encoder rising edge tick++
void m1Change()
{
	motor.M1Change();
}

void m2Change()
{
	motor.M2Change();
}

/*
 * ==================================== Callibration functions ====================================
 */

// Aligning both front left and front right sensors, i.e. both sensors measure the same distance from the front
void calibrate_frontAngle()
{
  double leftSensor;
  double rightSensor;
  for (int i = 0; i < 1; ++i)
  {
    // align both sensors forward first, regardless whether its far or near
    leftSensor = ir_FL.getDistance();
    rightSensor =ir_FR.getDistance()+0.36;
    
    double sensorDiff = abs(leftSensor - rightSensor);

    // difference tolerance of 0.1
    if (sensorDiff <= 0.1)
      break;

    // atan returns radian. Convert rad to deg by multiplying 180/PI
    double angle = asin(sensorDiff / 16.8) * 180 / PI;

    // if left is further
    if (leftSensor > rightSensor)
    {
      // turn right
      motor.Turn(angle);

    }
    else // right is further
    {
      // turn left
      motor.Turn(-angle);
    }
  }

}

// Calibrates left of robot parallel to wall or obstacle
void calibrate_sideAngle()
{
  double leftSensor;
  double rightSensor;
  for (int i = 0; i < 1; ++i)
  {
    // align both sensors forward first, regardless whether its far or near
    leftSensor = ir_LF.getDistance();
    rightSensor = ir_LB.getDistance();



    double sensorDiff = abs(leftSensor - rightSensor);

    // difference tolerance of 0.1
    if (sensorDiff <= 0.08)
      break;

    // atan returns radian. Convert rad to deg by multiplying 180/PI
    double angle = asin(sensorDiff / 17.5) * 180 / PI;

    // if left is further
    if (leftSensor > rightSensor)
    {
      // turn right
      motor.Turn(-angle);

    }
    else // right is further
    {
      // turn left
      motor.Turn(angle);
    }
  }
}

// move forward till all front left sensor reads setPoint distance
void calibrate_forward(double setPoint = 6.5) // old setPoint = 12.3
{

  for (int i = 0; i < 2; ++i)
  {
    double frontSensor = ir_FR.getDistance()+0.2;

    /* 
     *  Distance between robot and setpoint
     *  If negative, it means that the robot is too near from the setpoint.
     *  If positive, it means that the robot is too far from the setpoint
     */
    double distance = frontSensor - setPoint;

    if (abs(distance) <= 0.05)
      break;


    if (distance > 0) // robot away from setpoint
    {
      /*
       * need to pass absolute distance, as -ve value will mess up Forward function.
       * to indicate revverse, pass true in second argument.
       */
      motor.CalibrationForward(abs(distance), false);
    }
    
    else if (distance < 0) // robot too close
    {
      motor.CalibrationForward(abs(distance), true);
    }
  }
  motor.CalibrationForward(1, false);
}

void Calibrate_Corner()
{
  motor.TurnLeft90();
  calibrate_forward();
  delay(50);
  calibrate_frontAngle();
  delay(50);
  calibrate_frontAngle();
  delay(50);
  calibrate_forward();
  delay(50);
  
  motor.TurnRight90();
  delay(50);
  calibrate_forward();
  delay(50);
  calibrate_frontAngle();
  delay(50);
  calibrate_sideAngle();
  delay(50);
  calibrate_forward();
  delay(50);
  calibrate_frontAngle();
  delay(50);
  calibrate_sideAngle();
  delay(50);
  calibrate_forward();
  delay(50);
}


// Autocalibration functions
/* 
 *  This function is to be used for INTIIAL autocallibration within 3x3 grid before callibration
 *  Delays are included to stop motors from swinging 
 */
void autoCalibrate3x3(){
   motor.TurnLeft90();
  motor.TurnLeft90();
  calibrate_forward();
  delay(50);
  calibrate_frontAngle();
  delay(50);
  calibrate_frontAngle();
  delay(50);
  calibrate_forward();
  delay(50);
  
  motor.TurnRight90();
  delay(50);
  calibrate_frontAngle();
  delay(50);
  calibrate_sideAngle();
  delay(50);
  calibrate_forward();
  delay(50);
  calibrate_frontAngle();
  delay(50);
  calibrate_sideAngle();
  delay(50);
  calibrate_forward();
  delay(50);
 
  motor.TurnRight90();
  delay(50);
  calibrate_sideAngle();
  delay(50);
  calibrate_sideAngle();
  delay(50);
  calibrate_sideAngle();
  delay(50);
  calibrate_sideAngle();
  delay(50);
  calibrate_sideAngle();
  delay(100);
}


/*
 * Sensor Functions:
 * 
 * Normalize Sensors
 * These functions return the number of grids the nearest obstacle is away from a particular sensor.
 */
// For the front facing left sensor
int normalizeFrontLeft(double frontLeftSensor)
{
  int dist = 3;

  if (frontLeftSensor > 21.2 && frontLeftSensor <= 26)
  {
    dist = 2;
  }
  else if (frontLeftSensor > 11.5 && frontLeftSensor <= 21.2)
  {
    dist = 1;
  }
  else if (frontLeftSensor <= 11.5){
    dist = 0;
  }
  return dist;
}

// For the front facing center sensor
int normalizeFrontCenter(double frontCenterSensor)
{
  int dist = 3;

  if (frontCenterSensor > 15 && frontCenterSensor <= 23.5)
  {
    dist = 2;
  }
  else if (frontCenterSensor > 7.7 && frontCenterSensor <= 15)
  {
    dist = 1;
  }
  else if (frontCenterSensor <= 7.7){
    dist = 0;
  }
  return dist;
}

// For the front facing right sensor
int normalizeFrontRight(double frontRightSensor)
{
  int dist = 3;
 
  if (frontRightSensor > 21.7 && frontRightSensor <= 28)
  {
    dist = 2;
  }
  else if (frontRightSensor > 11 && frontRightSensor <= 21.7)
  {
    dist = 1;
  }
  else if (frontRightSensor <= 11.8){
    dist = 0;
  }
  return dist;
}

// For the left facing front (right) sensor
int normalizeLeftFront(double leftFrontSensor)
{
  int dist = 3;

  if (leftFrontSensor > 21.6 && leftFrontSensor <= 26.5)
  {
    dist = 2;
  }
  else if (leftFrontSensor > 10.9 && leftFrontSensor <= 21.6)
  {
    dist = 1;
  }
  else if (leftFrontSensor <= 10.9){
    dist = 0;
  }
  
  return dist;
}

// For the left facing back (left) sensor
int normalizeLeftBack(double leftBackSensor)
{
  int dist = 3;

  if (leftBackSensor > 22.07 && leftBackSensor <= 30.68)
  {
    dist = 2;
  }
  else if (leftBackSensor > 10.77 && leftBackSensor <= 22.07)
  {
    dist = 1;
  }
  else if (leftBackSensor <= 10.77){
    dist = 0;
  }

  return dist;
}

// For the right facing long range sensor
int normalizeLong(double longSensor)
{
  int dist = 6;
  if (longSensor > 45.7 && longSensor <= 55.5)
  {
    dist = 5;
  }
  else if (longSensor > 36.4 && longSensor <= 45.7)
  {
    dist = 4;
  }
  else if (longSensor > 26.1 && longSensor <= 36.4)
  {
    dist = 3;
  }
  else if (longSensor > 17.25 && longSensor <= 26.1)
  {
    dist = 2;
  }
  else if (longSensor > 4.3 && longSensor <= 17.25)
  {
    dist = 1;
  }
  else if (longSensor <= 4.3)
  {
    dist = 0;
  }
  return dist;
}

// Serial print readings of sensors WITH offset
void RawSensorValues()
{
      double fl = ir_FL.getDistance();
      double fc = ir_FC.getDistance()+2;
      double fr = ir_FR.getDistance()+0.36;
      double lf = ir_LF.getDistance();
      double lb = ir_LB.getDistance();
      double r = ir_R.getDistance();
      delay(10);
      Serial.println(String(fl) + "," + String(fc) + "," + String(fr) + "," + String(lf) + "," + String(lb) + "," + String(r));
}

// Serial print reading of sensors WITHOUT offset
void noOffsetRawSensorValues(){
  double fl = round(ir_FL.getDistance());
  double fc = round(ir_FC.getDistance());
  double fr = round(ir_FR.getDistance());
  double lf = round(ir_LF.getDistance());
  double lb = round(ir_LB.getDistance());
  double r = round(ir_R.getDistance());

  Serial.println("RR| Front Left: " + String(fl) + ";\n" + "Front Center: "+ String(fc) + ";\n" + "Front Right: " + String(fr) + ";\n" + "Left Front: " + String(lf) + ";\n" + "Left Back: " + String(lb) + ";\n" + "Right: " + String(r) + ";\n");
  
}

void RawGridSensorValues()
{
  delay(20); //Delay in reading senosr
  int fl = normalizeFrontLeft(ir_FL.getDistance());
  int fc = normalizeFrontCenter(ir_FC.getDistance()+2);
  int fr = normalizeFrontRight(ir_FR.getDistance());
 
  int lf = normalizeLeftFront(ir_LF.getDistance());
  int lb = normalizeLeftBack(ir_LB.getDistance()+0.5);
  
  int r = normalizeLong(ir_R.getDistance());

  delay(20);

  Serial.println(String(fl) + "," + String(fc) + "," + String(fr) + "," + String(lf) + "," + String(lb) + "," + String(r));
}

int RawGridfl()
{
  delay(20); //Delay in reading senosr
  int fl = normalizeFrontLeft(ir_FL.getDistance());
  return fl;
}

int RawGridfr()
{
  delay(20); //Delay in reading senosr
  int fr = normalizeFrontRight(ir_FR.getDistance());
  return fr;
}

int RawGridlf()
{
  delay(20); //Delay in reading senosr
    
  int lf = normalizeLeftFront(ir_LF.getDistance());
  return lf;
}
int RawGridlb()
{
  delay(20); //Delay in reading senosr
    
  int lb = normalizeLeftBack(ir_LB.getDistance()+0.5);
  return lb;
}


void setup()
{
  Serial.begin(115200);

  //Changes ADC prescaler to 32, faster analogRead
  ADCSRA &= ~(bit(ADPS0) | bit(ADPS1) | bit(ADPS2)); // clear prescaler bits

  //ADCSRA |= bit(ADPS0) | bit(ADPS2);                 //  32 
  ADCSRA |= bit(ADPS0) | bit(ADPS1) | bit(ADPS2);   // 128

  enableInterrupt(M1A, m1Change, CHANGE);
  enableInterrupt(M1B, m1Change, CHANGE);

  enableInterrupt(M2A, m2Change, CHANGE);
  enableInterrupt(M2B, m2Change, CHANGE);

  motor.begin();
}

String commands;
int currIndex = 0; //Current command index
int commandLength = 0;
bool canCalibrate = false;
bool debugAutoCalibrate = false; //true; // set to false to disable auto calibration
bool exploration = true;

void FlushBuffer()
{
  commands.replace(commands, "");
  commandLength = 0;
  currIndex = 0;
}

void loop()
{
  char lastCommand;
  if (Serial.available()) //Checks how many bytes available. sizeof(char) = 1 byte
  {
    while (Serial.available() > 0)
    {
      char temp = Serial.read();
      commands +=   temp; //Adds the character into the string
    }
  }

  // Impt to remove these characters else string length will be wrong
  commands.replace("\n", ""); //Removes newline characters when pressing enter
  commands.replace("\r", ""); //Removes return characters when pressing enter

  for (commandLength = 0; commands[commandLength] != '\0'; ++commandLength);
  // commandLength is array size

  if ((commandLength - 1) - currIndex >= 2) //Total commands sent is more than 2 i.e. fastest path
  {
    exploration = false;
  }
  else
  {
    exploration = true;
  }

  //Serial.println(currIndex);
  while (currIndex < commandLength)
  {

    canCalibrate = true;
    char command = commands[currIndex++]; //Get command from String by treating it as an array
    //lastCommand = command;
    if (isDigit(command) && command != '1')
    {

      int forwardDist = (int)(command - 48); //Convert char to int 
      if (forwardDist == 0)
      {
        forwardDist = 10;
      }

      motor.Forward(forwardDist * 10);
    
      RawGridSensorValues();
    }
    else
    {
      switch (command)
      {
        case '1': {// move forward 1 grid
          //Serial.println(lastCommand);
          motor.Forward10();
          int lf = RawGridlf();
          int lb = RawGridlb();
          int fl = RawGridfl();
          int fr = RawGridfr();
          if (fl ==0 && fr==0 && lf==0 && lb==0){
            Calibrate_Corner();
            calibrate_sideAngle();
            delay(50);
          }
          else if (lf ==0 && lb==0){
            calibrate_sideAngle();
            delay(50);
            calibrate_sideAngle();
            delay(50);
            calibrate_sideAngle();
            delay(50);
          }
          else if(fl ==0 && fr==0){
            calibrate_frontAngle();
            delay(50);
            calibrate_forward();
            delay(50);
            calibrate_frontAngle();
            delay(50);
            calibrate_forward();
            delay(50);
            calibrate_frontAngle();
            delay(50);
          }
          lastCommand = '1';
          RawGridSensorValues();
          break;
        }
        case 'D': // turn right 90 degrees on the spot works
          motor.TurnRight90();
          lastCommand = 'D';
          delay(50);
          RawGridSensorValues();
          break; 
        case 'A': // turn left 90 degrees on the spot works
          //Serial.println(lastCommand);
          motor.TurnLeft90();
          lastCommand = 'A';
          delay(50);
          RawGridSensorValues();
          break;
        case 'Y': // Autocalibration on 3x3 grid at Starting Zone
          autoCalibrate3x3();
          delay(10);
          // This message is to tell algo that 3x3 autocallibration has ended.
          // The 0s are arbitary, meant for message passing from arduino > RPI > algo easier
          //Serial.println("E|0,0,0,0,0,0");
          break;
        case 'N': 
          delay(50);
          RawGridSensorValues();
          break;
        case 'M':
          RawSensorValues();
          break;
        default:
          RawGridSensorValues();
          Serial.println("Error");
          break;
      }
 
    /*if (!exploration)
    {
      autocalibrate_ForwardAngle();
      autocalibrate_ForwardDistance();
      autocalibrate_sideAngle();
    }*/
}
}
}

/*
 * This loop function is used to test specific functions without messing with the main loop
 */
/*void loop(){
  GridSensorValues();
  delay(2000);
}*/
