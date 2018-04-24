#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, in8,    gyro,           sensorGyro)
#pragma config(Sensor, dgtl1,  touch,          sensorTouch)
#pragma config(Sensor, I2C_1,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_2,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_3,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_4,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_5,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Motor,  port2,           LEDrive,       tmotorVex393HighSpeed_MC29, openLoop, reversed, encoderPort, I2C_1)
#pragma config(Motor,  port3,            ,             tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port4,           fork,          tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port5,           L_lift,        tmotorVex393HighSpeed_MC29, openLoop, encoderPort, I2C_3)
#pragma config(Motor,  port6,           R_lift,        tmotorVex393HighSpeed_MC29, openLoop, reversed, encoderPort, I2C_4)
#pragma config(Motor,  port7,           REDrive,       tmotorVex393HighSpeed_MC29, openLoop, encoderPort, I2C_2)
#pragma config(Motor,  port8,           claw,          tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port9,           clawArm,       tmotorVex393HighSpeed_MC29, openLoop, encoderPort, I2C_5)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

/* MOTOR LAYOUT
----------------
Port 2: Left Encoder
Port 3: Left Drive
Port 4: Right Encoder
Port 5: Right Drive
*/
//datalog variables
float error;
float powerL;
float powerR;
task datalog(){
	while(1){
		datalogDataGroupStart();
		datalogAddValue( 0, error);
		datalogAddValue( 1, SensorValue(gyro));
		datalogAddValue( 2, powerL);
		datalogAddValue( 3, powerR);
		datalogDataGroupEnd();
		wait1Msec(25);
	}
}
const float gyro_ratio=(955.0/900.0); // real world gyro value tune ratio
const float kPt = 0.1; // P tune for turning
const float kDt = 0.08; // D tune for turning
const float kIt = 0.012; // I tune for turning
float Header = 0; // align header with gyro
const float max_turn_spd = 85;
const float max_int = 4000;

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*        Description: Competition template for VEX EDR                      */
/*                                                                           */
/*---------------------------------------------------------------------------*/

// This code is for the VEX cortex platform
#pragma platform(VEX2)

// Select Download method as "competition"
#pragma competitionControl(Competition)

//Main competition background code...do not modify!
#include "Vex_Competition_Includes.c"

/*---------------------------------------------------------------------------*/
/*                          Pre-Autonomous Functions                         */
/*                                                                           */
/*  You may want to perform some actions before the competition starts.      */
/*  Do them in the following function.  You must return from this function   */
/*  or the autonomous and usercontrol tasks will not be started.  This       */
/*  function is only called once after the cortex has been powered on and    */
/*  not every time that the robot is disabled.                               */
/*---------------------------------------------------------------------------*/

void pre_auton(void)
{
	nMotorEncoder[LEDrive]=0;
	nMotorEncoder[REDrive]=0;
	// Initialize Gyro
	SensorType(gyro) = sensorNone;
	wait1Msec(1000);
	SensorType(gyro) = sensorGyro;
	wait1Msec(2000);
	SensorValue(gyro) = 0;

	return;
}

void turn (float target, float time_turn); // declaration

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                              Autonomous Task                              */
/*                                                                           */
/*  This task is used to control your robot during the autonomous phase of   */
/*  a VEX Competition.                                                       */
/*                                                                           */
/*  You must modify the code to add your own robot specific commands here.   */
/*---------------------------------------------------------------------------*/

task autonomous()
{
	startTask(datalog);
	Header = 0;
	turn(900,3000); // align with Header, positive degrees = counterclockwise
	wait1Msec(1000);
	turn(0,3000);
	wait1Msec(1000);
	turn(900,3000);
	turn(1800,3000);
}

void turn (float target, float time_turn)
{
	Header = target*gyro_ratio;
	// initialize values
	float error_turn, prevError =0,integral=0,derivative=0,output_power;// initialize values
	clearTimer(T1);
	while(time1(T1)<time_turn)
	{
		error_turn = (Header) - (SensorValue(gyro)); // calculate left side degrees left to rotate
		// logic examples:
		// Header = 900, Sensor = 0 -> Error starts at 900, left side spins positive, right side spins negative
		// Header = 0, Sensor = 900 -> Error starts at -900, left side spins negative, right side spins positive
		// Header = 900, Gyro = -900 -> Error starts at 1800, left side spins positive, right side spins negative
		// Header = -900, Gyro = 900 -> Error starts at -1800, left side spins negative, right side spins positive
		// Header is 1800, Gyro is 900 -> Error starts at 900, left side spins positive, right side spins negative
		// Header is -1800, Gyro is -900 - > Error starts at -900, left side spins negative, right side spins positive

		if (abs(error_turn)<100)
		integral = error_turn+integral;

		if (integral>max_int)
			integral = max_int;

		if (integral< -max_int)
			integral = -max_int;

		derivative = error_turn - prevError;

		output_power = error_turn*kPt + integral*kIt + derivative*kDt; // calculate output power with PID tuning, direction included

		if (output_power > max_turn_spd)
			output_power = max_turn_spd;

		motor[REDrive]= output_power; // set right drive power
		motor[LEDrive]= -output_power; // set left drive power
		prevError = error_turn; // for derivative tuning
		wait1Msec(25);
		error = error_turn;
	}
	return;
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                              User Control Task                            */
/*                                                                           */
/*  This task is used to control your robot during the user control phase of */
/*  a VEX Competition.                                                       */
/*                                                                           */
/*  You must modify the code to add your own robot specific commands here.   */
/*---------------------------------------------------------------------------*/

task usercontrol()
{
	while (true)
	{
		motor[LEDrive]=vexRT[Ch3];
		motor[REDrive]=vexRT[Ch2];
	}
}
