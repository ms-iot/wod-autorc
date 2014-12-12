// Main.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include "arduino.h"
#include "PID.h"

#define LEFT_SENSOR 2
#define RIGHT_SENSOR 0
#define FWD_SENSOR 1
#define WHEEL_SENSOR 3
#define REAR_SENSOR 4

//define pins
#define  OUT_RIGHT 11
#define  OUT_UP 6
#define  OUT_LEFT 10
#define  OUT_DOWN 5

//constants
#define WREADING_BUF_SIZE 1000
#define WREADING_THRESHOLD 30
#define TURN_THRESHOLD 80
#define TURN_BOUNDARY 300
#define THROTTLE_THRESHOLD 80
#define SENSOR_COUNT 6
#define RAND_TRIPLICATOR 10922.0

const int SENSORS[] = { A0, A1, A2, A3, A4, A5 };
int WHEEL_READINGS[WREADING_BUF_SIZE];
int READINGS[] = { 0, 0, 0, 0, 0, 0 };

PID controller;
int wheel_reading;
int direction;
int averaging = 0;

enum DRIVING_STATE { FORWARD, BACKWARD };

DRIVING_STATE state;
bool stopped = false;
int _tmain(int argc, _TCHAR* argv[])
{
	return RunArduinoSketch();
}


/* Setup Arduino function */
void setup()
{
	
	//set motor pins to output mode
	pinMode(OUT_UP, OUTPUT);
	pinMode(OUT_RIGHT, OUTPUT);
	pinMode(OUT_DOWN, OUTPUT);
	pinMode(OUT_LEFT, OUTPUT);

	//set pin outputs to low
	digitalWrite(OUT_UP, LOW);
	digitalWrite(OUT_RIGHT, LOW);
	digitalWrite(OUT_DOWN, LOW);
	digitalWrite(OUT_LEFT, LOW);

	analogWrite(OUT_DOWN, 0);
	analogWrite(OUT_UP, 0);
	analogWrite(OUT_LEFT, 0);
	analogWrite(OUT_RIGHT, 0);

	for (int i = 0; i < 3; i++)
	{
		pinMode(SENSORS[i], INPUT);
	}

	controller.params.delT = 10;
	controller.params.Kp = 1.0;
	controller.params.Ki = 0.005;
	controller.params.Kd = 0.5;
	controller.params.setpt = 250;

	analogReadResolution(10);
	srand(millis());
	wheel_reading = 0;
	state = FORWARD;
}

void driveForward(int speed)
{
	analogWrite(OUT_UP, speed);
	analogWrite(OUT_DOWN, 0);
}

void driveBackward(int speed)
{
	analogWrite(OUT_UP, 0);
	analogWrite(OUT_DOWN, speed);
}

void turnLeft()
{
	analogWrite(OUT_LEFT, 255);
	analogWrite(OUT_RIGHT, 0);
}

void turnRight()
{
	analogWrite(OUT_RIGHT, 255);
	analogWrite(OUT_LEFT, 0);
}

void turnStraight()
{
	analogWrite(OUT_RIGHT, 0);
	analogWrite(OUT_LEFT, 0);
}

void stuckDetection()
{
	WHEEL_READINGS[wheel_reading] = READINGS[WHEEL_SENSOR];
	wheel_reading++;

	//check if we're stuck, if so change the state to backward
	if (wheel_reading == WREADING_BUF_SIZE)
	{
		wheel_reading = 0;
		int first_reading = WHEEL_READINGS[0];
		stopped = true;
		for (int i = 1; i < WREADING_BUF_SIZE; i++)
		{
			if (WHEEL_READINGS[i] > first_reading + WREADING_THRESHOLD
				|| WHEEL_READINGS[i] < first_reading - WREADING_THRESHOLD)
			{
				stopped = false;
				break;
			}
		}
		if (stopped)
		{
			stopped = false;
			//if there's space to reverse, go backwards, otherwise go forward
			if (state == FORWARD)
			{
				state = BACKWARD;
			}
			else
			{
				state = FORWARD;
			}
			direction = rand() / RAND_TRIPLICATOR;
			controller.reverseControllerMode();

		}
	}
}

void drive()
{

	

	if (state == BACKWARD)
	{
		if (abs(READINGS[RIGHT_SENSOR] - READINGS[LEFT_SENSOR]) < TURN_THRESHOLD)
		{
			turnStraight();
		}
		else if (READINGS[LEFT_SENSOR] > TURN_BOUNDARY
			&& READINGS[RIGHT_SENSOR] < (READINGS[LEFT_SENSOR] - TURN_THRESHOLD)
			&& abs(controller.state.output) > THROTTLE_THRESHOLD)
		{
			turnLeft();
		}
		else if (READINGS[RIGHT_SENSOR] > TURN_BOUNDARY
			&& READINGS[LEFT_SENSOR] < (READINGS[RIGHT_SENSOR] - TURN_THRESHOLD)
			&& abs(controller.state.output) > THROTTLE_THRESHOLD)
		{
			turnRight();
		}
		else
		{

			if (direction == 0)
			{
				turnLeft();
			}
			else if (direction == 1)
			{
				turnRight();
			}
			else
			{
				turnStraight();
			}
		}


		controller.updateResponse(1.0 * READINGS[REAR_SENSOR]);
	}
	else
	{

		
		if (abs(READINGS[RIGHT_SENSOR] - READINGS[LEFT_SENSOR]) < TURN_THRESHOLD)
		{
			turnStraight();
		}
		else if (READINGS[LEFT_SENSOR] > TURN_BOUNDARY
			&& READINGS[RIGHT_SENSOR] < (READINGS[LEFT_SENSOR] - TURN_THRESHOLD)
			&& abs(controller.state.output) > THROTTLE_THRESHOLD)
		{
			turnRight();
		}
		else if (READINGS[RIGHT_SENSOR] > TURN_BOUNDARY
			&& READINGS[LEFT_SENSOR] < (READINGS[RIGHT_SENSOR] - TURN_THRESHOLD)
			&& abs(controller.state.output) > THROTTLE_THRESHOLD)
		{
			turnLeft();
		}
		else
		{
			turnStraight();
		}

		controller.updateResponse(1.0 * READINGS[FWD_SENSOR]);
		
	}

	if (controller.state.output > 0)
	{
		driveForward(controller.state.output);
	}
	else
	{
		driveBackward(abs(controller.state.output));
	}
}

int compare_ints(const void* a, const void* b)   // comparison function
{
	int arg1 = *reinterpret_cast<const int*>(a);
	int arg2 = *reinterpret_cast<const int*>(b);
	if (arg1 < arg2) return -1;
	if (arg1 > arg2) return 1;
	return 0;
}

void medianFilter()
{
	static const size_t FRAME_SIZE = 20;
	static int data[SENSOR_COUNT][FRAME_SIZE];
	static int sorted_data[SENSOR_COUNT][FRAME_SIZE];
	static size_t data_size = 0;

	//shift data
	data_size = min(FRAME_SIZE, data_size + 1);
	for (int n = 0; n < SENSOR_COUNT; n++)
	{
		for (int i = data_size - 1; i > 0; i--) {
			data[n][i] = data[n][i - 1];
		}
	}

	//add in new value
	for (int n = 0; n < SENSOR_COUNT; n++)
	{
		data[n][0] = READINGS[n];
	}

	// Create a sorted array
	for (int n = 0; n < SENSOR_COUNT; n++)
	{
		memcpy(sorted_data[n], data[n], data_size * sizeof(int));
		std::qsort(sorted_data[n], data_size, sizeof(int), compare_ints);
	}

	// Get median
	for (int n = 0; n < SENSOR_COUNT; n++)
	{
		READINGS[n] = sorted_data[n][data_size / 2];
 	}
}


//Loop Arduino function
void loop()
{
	for (int i = 0; i < SENSOR_COUNT; i++)
	{
		READINGS[i] = analogRead(SENSORS[i]);
	}

	medianFilter();
	stuckDetection();
	drive();
	


}

