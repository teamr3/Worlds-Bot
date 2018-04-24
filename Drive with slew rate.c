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

/*		NOTE:
Things marked with //FIX are things that are probably dependant on external
factors and need to change depending. Just make sure they're all right before
testing or stuff.
*/



//   __        __   __                          __          __        ___  __
//  / _` |    /  \ |__)  /\  |       \  /  /\  |__) |  /\  |__) |    |__  /__`
//  \__> |___ \__/ |__) /~~\ |___     \/  /~~\ |  \ | /~~\ |__) |___ |___ .__/
//

float kP_drive = 2; //2
float kP_drift = 10; //10
float kD = 16; //15.6
float kI = 0.36; //0.36
float toMotor = 0;
float powerP = 0;
float powerI = 0;
float powerD = 0;

float headingAngle = 0;
int setTime = 0;

//Conversions
float inches_per_tile = 24; //23.25, 22.25
float ticks_per_revolution = 392; //FIX This is for the high speed configuration only
float ticks_per_inch = ticks_per_revolution/(PI*4); //FIX 4" omni wheels
float ticks_per_tile = ticks_per_inch*inches_per_tile;

//moveStraight variables
float error;
float power;
float driftPower;
float targetTicks;
float prevError;
float gyroError;
float derivative;
float integral = 0;
float integral_active_zone = 45; //FIX idk man arbitrary, 130, 40
float errorThreshold = 2; //errorThresholdInTicks=(ticks_per_inch)*(errorThresholdInInches) arbitrary

#pragma platform(VEX2)
#pragma competitionControl(Competition)
#include "Vex_Competition_Includes.c"



//   __   __   ___              ___  __
//  |__) |__) |__      /\  |  |  |  /  \ |\ |
//  |    |  \ |___    /~~\ \__/  |  \__/ | \|
//

void pre_auton()
{
	slaveMotor(R_lift, L_lift);
	SensorType[gyro] = sensorNone;
	wait1Msec(1000);
	SensorType[gyro] = sensorGyro;
	wait1Msec(2000);
	SensorValue[gyro] = 0;
	headingAngle = SensorValue[gyro]/10;
}



//    __       ___          __      ___  ___
//   /__` |   |__  |  |    |__)  /\  |  |__
//   .__/ |__ |___ |/\|    |  \ /~~\ |  |___
//

int motorSlewRate = 5; //20
float tempMotor = 0;
float motorReq[2];


void assignPower (int motorIndex, int motorReq_Index){ //REDrive is index 6, LEDrive is index 1
	tempMotor = motor[motorIndex];
	if (tempMotor != motorReq[motorReq_Index])
	{
		if (tempMotor < motorReq[motorReq_Index])
		{
			tempMotor = tempMotor +	motorSlewRate;
			if (tempMotor > motorReq[motorReq_Index])
			{
				tempMotor = motorReq[motorReq_Index];
			}
		}
		else
		{
			tempMotor = tempMotor -	motorSlewRate;
			if (tempMotor < motorReq[motorReq_Index])
			{
				tempMotor = motorReq[motorReq_Index];
			}
		}

		motor[motorIndex] = tempMotor;
	}
}

task slewRate()
{
	while (1)
	{
		assignPower(6, 1); //REDrive is index 6
		assignPower(1, 0); //LEDrive is index 1
		wait1Msec(15);
	}
}


//         __        ___     __  ___  __          __       ___
//   |\/| /  \ \  / |__     /__`  |  |__)  /\  | / _` |__|  |
//   |  | \__/  \/  |___    .__/  |  |  \ /~~\ | \__> |  |  |
//

//if robot is not straight by the end of loop, it will become the new heanding angle
//gyro did not take into account when the sensor value overflow
void moveStraight(int direction, float tiles){
	targetTicks = (tiles*ticks_per_tile) - prevError;

	if (tiles == 1){
		setTime = 2000;
		kP_drive = 2;//2
		kD = 15.7;
		kI = 0.36; //0.36
		if (direction == 1 || direction == 2){
			targetTicks = tiles*(ticks_per_tile - 65);
		}
		if (direction == -1){
			targetTicks = tiles*(ticks_per_tile - 85);
		}
	}

	if (tiles == 2 || tiles == 3){
		setTime = 4000;
		kP_drive = 2; //2
		kD = 15.7; //16
		kI = 0.36; //0.8, 0.36
		if (direction == 1){
			targetTicks = tiles*(ticks_per_tile - 60); //50, 65
		}
		if (direction == -1){
			targetTicks = tiles*(ticks_per_tile - 57); //45
		}
	}
	prevError = targetTicks;
	nMotorEncoder[LEDrive]=0;
	nMotorEncoder[REDrive]=0;
	error = targetTicks-((abs(nMotorEncoder[LEDrive])+abs(nMotorEncoder[REDrive]))/2);
	float scaling = 127/targetTicks;
	integral = 0;
	clearTimer(T1);
	while((time1[T1]<setTime)){ // && (error>errorThreshold)1500
		error = targetTicks-((abs(nMotorEncoder[LEDrive])+abs(nMotorEncoder[REDrive]))/2);
		gyroError = (SensorValue[gyro]/10)-headingAngle; //Error in degrees

		integral = integral + error;
		if(abs(error)>integral_active_zone){
			integral = 0;
		}
		if (abs(error) <= errorThreshold){
			integral = 0;
		}

		derivative = error - prevError;

		driftPower = gyroError * kP_drift;
		powerP = kP_drive*error*direction*scaling;
		powerI = kI*integral*direction*scaling;
		powerD = kD*derivative*direction*scaling;
		power = (powerP)+(powerI)+(powerD);

		if (power > 97){
			power = 97;
		}
		else if (power < -97){
			power = -97;
		}

		if (driftPower > 30){
			driftPower = 30;
		}
		else if (driftPower < -30){
			driftPower = -30;
		}

		if(abs(error)<10){
			driftPower = 0;
		}

		toMotor = power;
		motorReq[0] = toMotor + driftPower; //left
		motorReq[1] = toMotor - driftPower; //right
		prevError = error;
		wait1Msec(25);
	}
	toMotor = 0;
	motorReq[0] = toMotor;
	motorReq[1] = toMotor;
}


//   ___       __
//    |  |  | |__)|\ |
//    |  \__/ |  \| \|
//

// + counter clockwise
// - clockwise
// angle in degree
float turn_kP = 3; //1, 3
float turn_kI = 0.7; //0.3
float turn_kD = 16.7; //15
float turnError;
float turn_Power;
float turn_Integral = 0;
float turn_Derivative = 0;
float turn_Proportional = 0;
float current_Angle;
float angle;
void turn( int direction, float angle){
	float turn_LastError = 0;
	turn_Integral = 0;
	turn_Derivative = 0;
	turn_Proportional = 0;

	float turn_scale;

	setTime = 2000;
	headingAngle = headingAngle + (direction * angle);
	clearTimer(T1);

	while ((time1[T1]<setTime)){
		turn_scale = 127/angle;
		if (angle > 90){
			turn_scale = (127/90);
		}
		current_Angle = SensorValue[gyro]/10;
		turnError = headingAngle - current_Angle; // + turn left, - turn right
	 	turn_Proportional = turnError;
	 	turn_Derivative = turnError - turn_LastError;

	 	if (abs(turnError) < 10){ // active when error is less than 10 degree
	  	turn_Integral = turn_Integral + turnError;
		}
		else {
			turn_Integral = 0;
		}

		turn_Power = ((turn_kP * turn_Proportional) + (turn_kI * turn_Integral) + (turn_kD * turn_Derivative)) * turn_scale;

		if (turn_Power > 127){
			turn_Power = 127;
		}
		if (turn_Power < -127){
			turn_Power = -127;
		}
		motor [LEDrive] = -turn_Power;
		motor [REDrive] = turn_Power;
		turn_LastError = turnError;
		wait1Msec(25);
	}
}


//        __
//   /\  |__) |\/|
//  /~~\ |  \ |  |
//

float armTarget = -600;
float armError;
float arm_kP = 1;
task arm(){
	while (true){
		armError = armTarget - nMotorEncoder[clawArm]; //negative means go down, starting position is up = 0
		motor[clawArm]= -1 * arm_kP * armError * (127/(armTarget));
		wait1Msec(15);
	}
}



//         __ ___
// |    | |__  |
// |___ | |    |
//


float lift_kP = 3; //1, 3
float lift_kI = 0; //0.3
float lift_kD = 0; //15
float lift_Error;
float lift_Power;
float lift_Integral = 0;
float lift_Derivative = 0;
float lift_Proportional = 0;
float lift_Target = 0;
float current_EncoderValue = 0;
float lift_MaxRange = 570;
float lift_MinRange = 50;
task lift(){
	float lift_LastError = 0;
	lift_Integral = 0;
	lift_Derivative = 0;
	lift_Proportional = 0;

	float lift_scale;

	while (true){
		lift_scale = 127/abs(lift_Target + 1);
		current_EncoderValue = (abs(nMotorEncoder[L_lift]) + abs(nMotorEncoder[R_lift])) / 2;
		lift_Error = lift_Target - current_EncoderValue; // + turn left, - turn right
	 	lift_Proportional = lift_Error;
	 	lift_Derivative = lift_Error - lift_LastError;

	 	if (abs(lift_Error) < 45){
	  	lift_Integral = lift_Integral + lift_Error;
		}
		else {
			lift_Integral = 0;
		}

		lift_Power = ((lift_kP * lift_Proportional) + (lift_kI * lift_Integral) + (lift_kD * lift_Derivative)) * lift_scale;

		if (lift_Power > 127){
			lift_Power = 127;
		}
		if (lift_Power < -127){
			lift_Power = -127;
		}
		motor [L_lift] = lift_Power;
		lift_LastError = lift_Error;
		wait1Msec(25);
	}
}


//   __       ___            __   __
//  |  \  /\   |   /\  |    /  \ / _`
//  |__/ /~~\  |  /~~\ |___ \__/ \__>
//

task datalog(){
	while(1){
		datalogDataGroupStart();
		datalogAddValue( 0, lift_Error);
		datalogAddValue( 1, lift_Power);
		datalogAddValue( 3, lift_Proportional);
		datalogAddValue( 4, lift_Integral);
		datalogAddValue( 5, lift_Derivative);
		datalogAddValue( 6, current_EncoderValue);
		datalogDataGroupEnd();
		wait1Msec(25);
	}
}

//            ___  __
//   /\  |  |  |  /  \
//  /~~\ \__/  |  \__/
//

task autonomous()
{
	startTask(slewRate);
	startTask(datalog);
	startTask(lift);

	lift_Target = lift_MaxRange;
	wait1Msec(5000);

	lift_Target = lift_MinRange;
	wait1Msec(5000);

	stopTask(slewRate);
	stopTask(datalog);
	stopTask(lift);
}



//        __   ___  __      __   __       ___  __   __
//  |  | /__` |__  |__)    /  ` /  \ |\ |  |  |__) /  \ |
//  \__/ .__/ |___ |  \    \__, \__/ | \|  |  |  \ \__/ |___
//

task usercontrol()
{
	int driveThreshold = 20;
	while (true){
		if(abs(vexRT[Ch3])>driveThreshold){
			motor[LEDrive] = vexRT[Ch3];
		}	else {
			motor[LEDrive] = 0;
		}
		if(abs(vexRT[Ch2])>driveThreshold){
			motor[REDrive] = vexRT[Ch2];
		}	else {
			motor[REDrive] = 0;
		}
		motor[clawArm] = (vexRT[Btn6UXmtr2]-vexRT[Btn5UXmtr2])*127;
		motor[claw] = vexRT[Btn7DXmtr2]*127;
		motor[fork] = (vexRT[Btn6U]-vexRT[Btn6D])*127;
		motor[L_lift] = (vexRT[Btn8DXmtr2]-vexRT[Btn8RXmtr2])*127;
	}
}
