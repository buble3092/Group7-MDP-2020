#pragma once
#include <DualVNH5019MotorShield.h>
#include <PID_v1.h>
// #include <PID_AutoTune_v0.h>

#include "SharpIR.h"
#include <Arduino.h>



#define TICK_REFRESH_INTERVAL 200//ms
#define TICKS_PER_CM 130
#define TICKS_PER_CM_F 119
#define TICKS_PER_CM_FS 120
#define TICKS_PER_REV 2248.86

#define MOTOR_LEFT 1
#define MOTOR_RIGHT 2

#define FORWARD_DELAY 100
#define ROTATE_DELAY 150


class RobotMotor
{
private:

	volatile double m1Ticks;
	volatile double m2Ticks;

	volatile double m1TargetCounter; //For counting to target ticks bnased on distance
	volatile double m2TargetCounter;

	double m1RPM, m2RPM;

  // for PID library
  double oldM1RPM;

	double m1Power, m2Power;

	//For PID computation
	double errSum;
	double lastErr;

	unsigned long lastTime;

	DualVNH5019MotorShield md;
	
public:
	inline void M1Change() { m1Ticks++; m1TargetCounter++; }
	inline void M2Change() { m2Ticks++; m2TargetCounter++; }
	inline void ResetPID() { errSum = 0; lastErr = 0; }
	inline int getM1Tick() {
		return m1Ticks;
	}

	inline int getM2Tick() {
		return m2Ticks;
	}
	
	inline DualVNH5019MotorShield GetMotor() {	return md; }
	
	//Char dir is for forward and backward
	//motorNo 1 for Left Motor, 2 for Right Motor
	//
	double ComputePID(double consKp, double consKi, double consKd, double targetRPM, int motor);

	void begin();
	void CalcRPM();

	//Returns true when hit target tick
	void CalibrationForward(double cm, bool reverse);
	
	void Forward(double cm);
	void ForwardShort(double cm);

	void Forward10();
  // void Forward10Auto();
  void Forward10Testing();

	void Turn(double angle);

	//Specially calibrated version of moving forward

	//Specially calibrated version of Left and right 90 degree turn.
	void TurnRight90();
	void TurnLeft90();
  void TurnRight90M();
  void TurnLeft90M();
  void TurnRight90L();
  void TurnLeft90L();

	void Turn180();
};
