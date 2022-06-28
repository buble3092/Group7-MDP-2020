#include "RobotMotor.h"

//SharpIR brakeSensor(GP2Y0A21YK0F, A2);
/*
 * IMPORTANT: Please make sure that the voltage of DC battery is kept consistent
 *            so that there's no need to change the PID values everyday.
 *            
 * Calibration values work for DC battery with voltage ranging from 6.16 - 6.19 V
 * (be careful of weak/ overcharged DC battery)
 */

// The two arrays are for the move forward and turning respectively.
// forward10, forwardShort, forwardCertainCM
//double timesForward[] = {1.045, 1.04, 1.043, 0.0};
double timesForward[] = {1, 1.02, 0.8, 0.0};

int ticksForTurn[] = {121, 115, 118, 117, 123, 114, 119, 118};

// for PID library
double setPointL, inputL, outputL;
double setPointR, inputR, outputR;
double KpL, KiL, KdL;
double KpR, KiR, KdR;
PID leftMotorPID(&inputL, &outputL, &setPointL, KpL, KiL, KdL, DIRECT);
PID rightMotorPID(&inputR, &outputR, &setPointR, KpR, KiR, KdR, DIRECT);

void RobotMotor::begin()
{
	m1Ticks = 0;
	m2Ticks = 0;

	m1RPM = 0;
	m2RPM = 0;

	m1TargetCounter = 0;
	m2TargetCounter = 0;

	m1Power = 150;
	m2Power = 150;

	lastTime = 0;

	errSum = 0;
	lastErr = 0;

  // for PID library
  leftMotorPID.SetMode(AUTOMATIC);
  rightMotorPID.SetMode(AUTOMATIC);

	md.init();
}

void RobotMotor::CalcRPM()
{
	if ((millis() - lastTime) >= TICK_REFRESH_INTERVAL)
	{
		m1RPM = ((m1Ticks*(60000 / TICK_REFRESH_INTERVAL)) / TICKS_PER_REV); //Extrapolate RPM
		m2RPM = ((m2Ticks*(60000 / TICK_REFRESH_INTERVAL)) / TICKS_PER_REV); //100ms * 600ms = 1 minute (interpolate RPM)
		m1Ticks = 0;
		m2Ticks = 0;

		lastTime = millis();
	}
}

// old pid implemented with theory
double RobotMotor::ComputePID(double consKp, double consKi, double consKd, double targetRPM, int motor)
{
	int currentRPM;
	
	if (motor == MOTOR_LEFT)
		currentRPM = m1RPM;
	else
		currentRPM = m2RPM;

	//Setpoint - Input(Target Tick)
	double error = targetRPM - currentRPM;

	//Integral
	errSum += error;

	//Derivative
	double dErr = error - lastErr;
	lastErr = error;

	return ((consKp * error) + (consKi * errSum) + (consKd * dErr));
}

// To callibrate robot forward
void RobotMotor::CalibrationForward(double  cm, bool reverse)
{
	int targetTick = cm * TICKS_PER_CM;
	m1TargetCounter = m2TargetCounter = 0;
	m1Ticks = m2Ticks = 0;
	lastTime = 0;

	ResetPID();

	//Initial speed
	m1Power = 338; //offset starting speed of left motor
  m2Power = -326;

	md.setSpeeds(m1Power, m2Power);
	md.setBrakes(0, 0);

	while (m1TargetCounter < targetTick - 80 || m2TargetCounter < targetTick - 80)
	{
		CalcRPM();
		m1Power += ComputePID(0.000334, 0, 0.005, 80, MOTOR_LEFT);
		m2Power -= ComputePID(0.000342, 0, 0.005, 80, MOTOR_RIGHT);

		if (m1Power < 0)
			m1Power = 0;

		if (m2Power > 0)
			m2Power = 0;

		if(reverse)
			md.setSpeeds(-m1Power, -m2Power);
		else
			md.setSpeeds(m1Power, m2Power);

		delay(1);
	}

	md.setSpeeds(0, 0);
	md.setBrakes(400, 400);

	delay(100);
}

// For forward movement of 1 grid
void RobotMotor::Forward10()
{
  
  int targetTick = 10 * TICKS_PER_CM_FS;
  m1TargetCounter = m2TargetCounter = 0;
  m1Ticks = m2Ticks = 0;
  lastTime = 0;

  ResetPID();
  //m1 left
  //m2 right
  //Initial speed
  m1Power = 317.7; //offset starting speed of left motor
  m2Power = -358.5;
  double leftp = 0.00286;
  double rightp = 0.00335;
  
  md.setBrakes(0, 0);
  md.setSpeeds(m1Power, m2Power);
  
  while (m1TargetCounter < 0.10*(targetTick-60)|| m2TargetCounter < 0.10*(targetTick - 60))
  {
      CalcRPM();
    
      m1Power += ComputePID(leftp,0, 0.009, 90, MOTOR_LEFT);
      m2Power -= ComputePID(rightp,0, 0.009, 90, MOTOR_RIGHT); 
      if (m1Power < 0)
        m1Power = 0;

      if (m2Power > 0)
        m2Power = 0;

      md.setSpeeds(0.7*m1Power, 0.7*(m2Power * timesForward[2]));

      delay(5); //to slow down rate in which PID updates motorspeed
  }
  while (m1TargetCounter < 0.20*(targetTick-60)|| m2TargetCounter < 0.20*(targetTick - 60))
  {
      CalcRPM();
    
      m1Power += ComputePID(leftp,0, 0.009, 90, MOTOR_LEFT);
      m2Power -= ComputePID(rightp,0, 0.009, 90, MOTOR_RIGHT); 
      if (m1Power < 0)
        m1Power = 0;

      if (m2Power > 0)
        m2Power = 0;

      md.setSpeeds(0.8*m1Power, 0.8*(m2Power * timesForward[2]));

      delay(5); //to slow down rate in which PID updates motorspeed
  }
  while (m1TargetCounter < 0.30*(targetTick-60)|| m2TargetCounter < 0.30*(targetTick - 60))
  {
      CalcRPM();
    
      m1Power += ComputePID(leftp,0, 0.009, 90, MOTOR_LEFT);
      m2Power -= ComputePID(rightp,0, 0.009, 90, MOTOR_RIGHT); 
      if (m1Power < 0)
        m1Power = 0;

      if (m2Power > 0)
        m2Power = 0;

      md.setSpeeds(0.9*m1Power, 0.9*(m2Power * timesForward[2]));

      delay(5); //to slow down rate in which PID updates motorspeed
  }
  while (m1TargetCounter < 0.60*(targetTick-60)|| m2TargetCounter < 0.60*(targetTick - 60))
  {
      CalcRPM();

      m1Power += ComputePID(leftp,0, 0.009, 90, MOTOR_LEFT);
      m2Power -= ComputePID(rightp, 0, 0.009, 90, MOTOR_RIGHT); 
      if (m1Power < 0)
        m1Power = 0;

      if (m2Power > 0)
        m2Power = 0;

      md.setSpeeds(m1Power, (m2Power * timesForward[2]));
    
      delay(5); //to slow down rate in which PID updates motorspeed
  }
  while (m1TargetCounter < 0.8*(targetTick-60)|| m2TargetCounter < 0.8*(targetTick - 60))
  {
      CalcRPM();
    
      m1Power += ComputePID(leftp,0, 0.009, 90, MOTOR_LEFT);
      m2Power -= ComputePID(rightp,0, 0.009, 90, MOTOR_RIGHT); 

      if (m1Power < 0)
        m1Power = 0;

      if (m2Power > 0)
        m2Power = 0;

      md.setSpeeds(0.8*m1Power, 0.8*(m2Power * timesForward[2]));

      delay(5); //to slow down rate in which PID updates motorspeed
  }
  while (m1TargetCounter < 0.8*(targetTick-60)|| m2TargetCounter < 0.8*(targetTick - 60))
  {
      CalcRPM();
    
      m1Power += ComputePID(leftp,0, 0.009, 90, MOTOR_LEFT);
      m2Power -= ComputePID(rightp,0, 0.009, 90, MOTOR_RIGHT); 

      if (m1Power < 0)
        m1Power = 0;

      if (m2Power > 0)
        m2Power = 0;

      md.setSpeeds(0.6*m1Power, 0.6*(m2Power * timesForward[2]));

      delay(5); //to slow down rate in which PID updates motorspeed
  }
  while (m1TargetCounter < (targetTick-60)|| m2TargetCounter < (targetTick - 60))
  {
      CalcRPM();
    
      m1Power += ComputePID(leftp,0, 0.009, 90, MOTOR_LEFT);
      m2Power -= ComputePID(rightp,0, 0.009, 90, MOTOR_RIGHT); 

      if (m1Power < 0)
        m1Power = 0;

      if (m2Power > 0)
        m2Power = 0;

      md.setSpeeds(0.4*m1Power, 0.4*(m2Power * timesForward[2]));

      delay(5); //to slow down rate in which PID updates motorspeed
  }
  
   md.setSpeeds(0, 0);
  md.setBrakes(400, 400);

  delay(FORWARD_DELAY);

}

// For forward movement that are more than or equals to 4 grids
void RobotMotor::Forward(double cm)
{
  int targetTick = cm * TICKS_PER_CM_F;
  m1TargetCounter = m2TargetCounter = 0;
  m1Ticks = m2Ticks = 0;
  lastTime = 0;

  ResetPID();
  //m1 left
  //m2 right
  //Initial speed
  m1Power = 317;//290; //offset starting speed of left motor
  m2Power = -363;//-361;
  double leftp = 0.000;//0.00256;
  double rightp = 0.000;//0.00195;
  double lefti = 0.0000;//0.00256;
  double righti = 0.0000;//0.00195;
  double leftd = 0.0000;//0.00256;
  double rightd = 0.0000;//0.00195;
  

  md.setBrakes(0, 0);
  md.setSpeeds(m1Power, m2Power);

  while (m1TargetCounter < 0.05*(targetTick - 60)|| m2TargetCounter < 0.05*(targetTick - 60))
  {
      CalcRPM();
    
      m1Power += ComputePID(leftp,lefti, leftd, 90, MOTOR_LEFT);
      m2Power -= ComputePID(rightp, righti, rightd, 90, MOTOR_RIGHT); 

      if (m1Power < 0)
        m1Power = 0;

      if (m2Power > 0)
        m2Power = 0;

      md.setSpeeds(m1Power*0.8, (m2Power * timesForward[2] * 0.8));
      delay(5); //to slow down rate in which PID updates motorspeed
  }
 
  while (m1TargetCounter < 0.1*(targetTick - 60)|| m2TargetCounter < 0.1*(targetTick - 60))
  {
      CalcRPM();
    
      m1Power += ComputePID(leftp,lefti, leftd, 90, MOTOR_LEFT);
      m2Power -= ComputePID(rightp, righti, rightd, 90, MOTOR_RIGHT); 

      if (m1Power < 0)
        m1Power = 0;

      if (m2Power > 0)
        m2Power = 0;

      md.setSpeeds(m1Power*0.9, (m2Power * timesForward[2] * 0.9));
      delay(5); //to slow down rate in which PID updates motorspeed
  }
 
  while (m1TargetCounter < 0.9*(targetTick - 60)|| m2TargetCounter < 0.9*(targetTick - 60))
  {
      CalcRPM();
    
      m1Power += ComputePID(leftp,lefti, leftd, 90, MOTOR_LEFT);
      m2Power -= ComputePID(rightp, righti, rightd, 90, MOTOR_RIGHT); 

      if (m1Power < 0)
        m1Power = 0;

      if (m2Power > 0)
        m2Power = 0;

      md.setSpeeds(m1Power, (m2Power * timesForward[2]));
      delay(5); //to slow down rate in which PID updates motorspeed
  }
  while (m1TargetCounter < 0.95*(targetTick - 60)|| m2TargetCounter < 0.95*(targetTick - 60))
  {
      CalcRPM();
    
      m1Power += ComputePID(leftp,lefti, leftd, 90, MOTOR_LEFT);
      m2Power -= ComputePID(rightp, righti, rightd, 90, MOTOR_RIGHT); 

      if (m1Power < 0)
        m1Power = 0;

      if (m2Power > 0)
        m2Power = 0;

      md.setSpeeds(m1Power * 0.88, (m2Power * timesForward[2]) * 0.9 );
      delay(5); //to slow down rate in which PID updates motorspeed
  }
  
  while (m1TargetCounter < (targetTick - 60)|| m2TargetCounter < (targetTick - 60))
  {
      CalcRPM();
    
      m1Power += ComputePID(leftp,lefti, leftd, 90, MOTOR_LEFT);
      m2Power -= ComputePID(rightp, righti, rightd, 90, MOTOR_RIGHT); 

      if (m1Power < 0)
        m1Power = 0;

      if (m2Power > 0)
        m2Power = 0;

      md.setSpeeds(m1Power * 0.8, (m2Power * timesForward[2]) * 0.8);
      delay(5); //to slow down rate in which PID updates motorspeed
  }
  md.setSpeeds(0, 0);
  md.setBrakes(400, 400);

  delay(FORWARD_DELAY);
}

// For turning of wheels. +ve angle = clockwise, -ve angle = anti-clockwise
void RobotMotor::Turn(double angle)
{
	// convert angle to cm
	float distance = PI * 17 * (abs(angle) / 360.0);
	int targetTick = distance * ticksForTurn[7];
	m1TargetCounter = m2TargetCounter = 0;
	m1Ticks = m2Ticks = 0;
	lastTime = 0;		//For calculating ticks

	ResetPID();

	m1Power = 275;
	m2Power = -260;

	md.setSpeeds(m1Power, -m2Power);
	md.setBrakes(0, 0);

	while (m1TargetCounter < targetTick - 50 || m2TargetCounter < targetTick - 50)
	{
		CalcRPM();

		m1Power += ComputePID(0.000385, 0, 0.005, 100, MOTOR_LEFT);
		m2Power -= ComputePID(0.000385, 0, 0.005, 100, MOTOR_RIGHT);

		if (angle > 0) // Positive angle indicates clockwise
			md.setSpeeds(m1Power, -m2Power);
		else
			md.setSpeeds(-m1Power, m2Power);

		delay(1);
	}

	md.setSpeeds(0, 0);
	md.setBrakes(400, 400);
}

// For turning of wheels 90 degrees anti-clockwise
void RobotMotor::TurnLeft90()
{
	//Convert angle to cm
	float distance = PI * 17.5 * (90.0 / 360.0); //17.0W
	int targetTick = distance * ticksForTurn[0];
	m1TargetCounter = m2TargetCounter = 0;
	m1Ticks = m2Ticks = 0;
	lastTime = 0;		//For calculating ticks

	ResetPID();

	m1Power = 281;
	m2Power = -270;
	md.setSpeeds(-m1Power, m2Power);
	md.setBrakes(0, 0);

	while (m1TargetCounter < targetTick - 160|| m2TargetCounter < targetTick - 160)
	{
		CalcRPM();

    //m1Power += ComputePID(0.00680, 0, 0.005, 80, MOTOR_LEFT);
    //m2Power -= ComputePID(0.00584, 0, 0.005, 80, MOTOR_RIGHT);
    m1Power += ComputePID(0.00240, 0, 0.005, 80, MOTOR_LEFT);
    m2Power -= ComputePID(0.00215, 0, 0.005, 80, MOTOR_RIGHT);

		md.setSpeeds(-m1Power, m2Power);

		delay(1);
	}

	md.setSpeeds(0, 0);
	md.setBrakes(400, 400);

	delay(ROTATE_DELAY);
}
// For turning of wheels 90 degrees clockwise
void RobotMotor::TurnRight90()
{
	//Convert angle to cm
	float distance = PI * 17.0 * (90.0 / 360.0);
	int targetTick = distance * ticksForTurn[1];
	m1TargetCounter = m2TargetCounter = 0;
	m1Ticks = m2Ticks = 0;
	lastTime = 0;		//For calculating ticks 

	ResetPID();

  // reset PID using PID lib
  leftMotorPID.SetTunings(0, 0, 0);
  rightMotorPID.SetTunings(0, 0, 0);
  inputL = inputR = setPointL = setPointR = 0;
  leftMotorPID.Compute(); rightMotorPID.Compute();

//	m1Power = m2Power = 100;
//	md.setSpeeds(m1Power, -m2Power);
//	md.setBrakes(0, 0);

  m1Power = 272;
  m2Power = -275;
  md.setSpeeds(m1Power, -m2Power);
  md.setBrakes(0, 0);

	while (m1TargetCounter < targetTick - 65 || m2TargetCounter < targetTick - 65)
	{
		CalcRPM();

		m1Power += ComputePID(0.00350, 0, 0.005, 80, MOTOR_LEFT);
	  m2Power -= ComputePID(0.00202, 0, 0.005, 80, MOTOR_RIGHT);
      

		md.setSpeeds(m1Power, -m2Power);

		delay(1);
	}

	md.setSpeeds(0, 0);
	md.setBrakes(400, 400);

	delay(ROTATE_DELAY);
}
