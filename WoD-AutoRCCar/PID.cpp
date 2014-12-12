#include "PID.h"
#include "arduino.h"
#include <vector>
#include <iostream>
#include <fstream>

PID::PID()
{
	state.readingPrev = 0;
	state.errSum = 0;
	state.lastSampleTime = millis();
	state.output = 0;
}

void PID::setTarget(float setpt)
{
	params.setpt = setpt;
}

void PID::reverseControllerMode()
{
	params.Kd = params.Kd * -1;
	params.Ki = params.Ki * -1;
	params.Kp = params.Kp * -1;
	state.errSum = 0;
}

void PID::updateResponse(float reading)
{
	unsigned long currentTime = millis();

	if (!((currentTime - state.lastSampleTime) >= params.delT))
	{
		//delta of 10ms has not yet occurred, don't recompute
		return;
	}
	state.lastSampleTime = currentTime;

	float err = reading - params.setpt;
	float pidout = 0;
	

	//proportional component
	pidout = params.Kp * err;

	//integral component
	state.errSum += params.Ki * err;
	//clamp integral component to  half of max -50 to 50
	if (state.errSum > 50)
	{
		state.errSum = 50;
	}
	else if (state.errSum < -50)
	{
		state.errSum = -50;
	}
	pidout += state.errSum;
	
	//derivative component
	pidout += params.Kd * (reading - state.readingPrev);
	state.readingPrev = reading;

	pidout *= -1;

	//clamp output to -255 to 255
	if (pidout > 255)
	{
		return;
	}
	else if (pidout < -255)
	{
		return;
	}
	state.output = pidout;

}