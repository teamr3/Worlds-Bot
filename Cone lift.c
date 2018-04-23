#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, in8,    gyro,           sensorGyro)
#pragma config(Sensor, dgtl1,  touch,          sensorTouch)
#pragma config(Sensor, I2C_1,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_2,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_3,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_4,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_5,  ,               sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Motor,  port2,           LEDrive,       tmotorVex393HighSpeed_MC29, openLoop, reversed, encoderPort, I2C_1)
#pragma config(Motor,  port3,           clawArm2,      tmotorVex393HighSpeed_MC29, openLoop, reversed)
#pragma config(Motor,  port4,           fork,          tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port5,           L_lift,        tmotorVex393HighSpeed_MC29, openLoop, encoderPort, I2C_3)
#pragma config(Motor,  port6,           R_lift,        tmotorVex393HighSpeed_MC29, openLoop, reversed, encoderPort, I2C_4)
#pragma config(Motor,  port7,           REDrive,       tmotorVex393HighSpeed_MC29, openLoop, encoderPort, I2C_2)
#pragma config(Motor,  port8,           claw,          tmotorVex393HighSpeed_MC29, openLoop)
#pragma config(Motor,  port9,           clawArm,       tmotorVex393HighSpeed_MC29, openLoop, encoderPort, I2C_5)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

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

const float kP=2;
const float kI=0;
const float kD=2;
int height=0; //height preset for auton: 1 for preload height, 2 for cones 1-4 drop height, 3 for cones 5-8/9 drop height
int target=0;
int lifterror;
int liftpower;
int integral1;


void pre_auton()
{
	slaveMotor(L_lift,R_lift);
	slaveMotor(clawArm2,clawArm);
  // Set bStopTasksBetweenModes to false if you want to keep user created tasks
  // running between Autonomous and Driver controlled modes. You will need to
  // manage all user created tasks if set to false.
  bStopTasksBetweenModes = true;

	// Set bDisplayCompetitionStatusOnLcd to false if you don't want the LCD
	// used by the competition include file, for example, you might want
	// to display your team name on the LCD in this function.
	// bDisplayCompetitionStatusOnLcd = false;

  // All activities that occur before the competition starts
  // Example: clearing encoders, setting servo positions, ...
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                              Autonomous Task                              */
/*                                                                           */
/*  This task is used to control your robot during the autonomous phase of   */
/*  a VEX Competition.                                                       */
/*                                                                           */
/*  You must modify the code to add your own robot specific commands here.   */
/*---------------------------------------------------------------------------*/

task coneArm(){

}


task coneLift(){
	int error;
	int prevError;
	int integral;
	int derivative;
	int power;
	int ticks;
	while(1){
		prevError=0;
		derivative=0;
		switch(height){
			case 1:
				ticks=150:
				break;
			case 2:
				ticks=200;
				break;
			case 3:
				ticks=400;
				break;
			default:
				if(ticks==400){
					ticks=200;
					wait1Msec(500);
				}
				ticks=0;
				break;
		}

		while((abs(getMotorEncoder(R_lift))<(ticks-10))||(abs(getMotorEncoder(R_lift))>(ticks+10))){
			error=ticks-abs(getMotorEncoder(R_lift));
			lifterror=error;
			integral=integral+error;
			if(error==0||(abs(getMotorEncoder(R_lift))>ticks)){
				integral=0;
			}
			if(error>5000){
				integral=0;
			}
			derivative=error-prevError;
			prevError=error;
			power=error*kP+integral*kI+derivative*kD;
			liftpower=power;
			motor[R_lift]=power;
			wait1Msec(15);
		}
		motor[R_lift]=0;
	}
}

float armPower;
float armTarget;
float arm_kP = 1;
float arm_kD = 3;

task arm(){
	float armError;
	float armPrevError=0;
	float armD;
	while (true){
		while((getMotorEncoder(clawArm)<armTarget)||(getMotorEncoder(clawArm)>armTarget)){
			armError = armTarget - getMotorEncoder(clawArm); //negative means go down, starting position is up = 0
			armD=armError-armPrevError;
			armPrevError=armError;
			motor[clawArm]= arm_kP * armError + arm_kD * armD;
			armPower=arm_kP * armError + arm_kD * armD;
			wait1Msec(15);
		}
	}
}

task datalog(){
	while(1){
		datalogDataGroupStart();
		datalogAddValue( 0, lifterror);
		datalogAddValue( 1, liftpower);
		datalogAddValue( 2, SensorValue[gyro]);
		datalogAddValue( 3, integral1);
		datalogAddValue( 4, motor[LEDrive]);
		datalogAddValue( 5, motor[REDrive]);
		datalogAddValue( 6, getMotorEncoder(R_lift));
		datalogAddValue( 7, armPower);
		datalogDataGroupEnd();
		wait1Msec(25);
	}
}



task autonomous()
{
  // ..........................................................................
  // Insert user code here.
  // ..........................................................................

  // Remove this function call once you have "real" code.
	//startTask(coneArm);
	startTask(coneLift);
	startTask(arm);
	startTask(datalog);
	height=1;
	wait1Msec(1000);
	armTarget=130;
	wait1Msec(500);
	height=0;
	wait1Msec(500);
	height=1;
	wait1Msec(500);
	armTarget=0;
	//wait1Msec(2000);
	//height=5;
	//wait1Msec(2000);
	//height=0;

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
  // User control code here, inside the loop

  while (true)
  {
    // This is the main execution loop for the user control program.
    // Each time through the loop your program should update motor + servo
    // values based on feedback from the joysticks.

    // ........................................................................
    // Insert user code here. This is where you use the joystick values to
    // update your motors, etc.
    // ........................................................................

    // Remove this function call once you have "real" code.
    UserControlCodePlaceholderForTesting();
  }
}
