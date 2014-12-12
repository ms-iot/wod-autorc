// Copyright(c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the BSD 2 - Clause License.
// See License.txt in the project root for license information.

struct  PID_params
{
	// Gain parameters
	float  Kp;        // Loop gain parameter
	float  Ki;           // Integrator time constant
	float  Kd;           // Differentiator time constant
	float  delT;         // Update time interval

	// Setpoint parameters
	float  setpt;        // Regulated level to maintain
};

struct  PID_state
{
	// Controller state
	float  errSum;   // Summation of setpoint errors
	float  readingPrev;      // Previous input reading
	float output;
	unsigned long lastSampleTime; //the time the last sample was taken
};

class PID
{
public:
	PID_params params;
	PID_state state;
	PID();
	void setTarget(float setpt);
	void updateResponse(float reading);
	void reverseControllerMode();
};