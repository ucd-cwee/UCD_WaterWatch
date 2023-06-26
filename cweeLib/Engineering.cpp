#include "../cweeLib/precompiled.h"
#pragma hdrstop

float cweeEng::pidLogic::calculate(float setpoint, float pv, const u64& dt) {
#if 0
	// Tune the PID values
	tunePID();

	// Calculate error
	float error = setpoint - pv;

	// Proportional term
	float Pout = _Kp * error * _Ku;

	// Integral term
	_integral += Pout;
	_integral *= 0.9f; // dampen the integral
	float Iout = _Ki * _integral * _Ku; // want to dampen the Iout over time ... if we are oscillating around the correct value then Iout should go away

	// Derivative term
	float derivative = (error - _pre_error) / (dt + cweeMath::EPSILON);
	float Dout = _Kd * derivative * _Ku;

	// Calculate total output
	float output = ((dt == 0.0f) ? Pout : Pout + Dout);

	// Save error to previous error
	_pre_error = error;

	// adjustment
	float lerp = cweeMath::Fmin(cweeMath::Fabs(error), 1);
	output =
		cweeMath::Lerp(
			(float)((output / setpoint) + _pre_output),
			(float)(output + _pre_output),
			(float)lerp
		);

	_pre_output = output;

	// Restrict to max/min
	if (output > _max)
		output = _max;
	else if (output < _min)
		output = _min;

	return output;
#elif 1
	float error, Pv_0, Pv_1, Pv_2, Pout_0, Pout_1, Pout_2, expectedPv, Pout, output;

	// Tune the PID values
	tunePID();

	// State
	error = setpoint - pv;

	Pv_0 = setpoint - _pre_error;
	Pv_1 = pv;
	Pv_2 = (2.0f * pv) - setpoint + _pre_error;

	Pout_0 = _Kp * (setpoint - Pv_0) * _Ku;
	Pout_1 = _Kp * (setpoint - Pv_1) * _Ku;
	Pout_2 = _Kp * (setpoint - Pv_2) * _Ku;

	expectedPv =
		(((Pv_1 - Pv_0) / ((Pout_1 - Pout_0) == 0.0f ? 1 : (Pout_1 - Pout_0))) * (Pout_2 - Pout_1)) + Pv_2;

	Pout = _Kp * ((setpoint - Pv_2) + (setpoint - expectedPv)) * _Ku;

	// Calculate total output
	output = Pout;// ((dt == 0.0f) ? Pout : Pout + Dout);

	// Save error to previous error
	_pre_error = error;

	// adjustment
	output = output + _pre_output; // (output / setpoint) + _pre_output

	// Restrict to max/min
	if (output > _max)
		output = _max;
	else if (output < _min)
		output = _min;

	_pre_output = output;

	return output;

#else
	// Tune the PID values
	tunePID();

	// Calculate error
	float error = setpoint - pv;

	// Proportional term
	float Pout = _Kp * error * _Ku;

	// Derivative term
	float derivative = (error - _pre_error) / (dt + cweeMath::EPSILON);
	float Dout = _Kd * derivative * _Ku;

	// Calculate total output
	float output = ((dt == 0.0f) ? Pout : Pout + Dout);

	// Save error to previous error
	_pre_error = error;

	// adjustment
	output = (output / setpoint) + _pre_output;

	_pre_output = output;

	// Restrict to max/min
	if (output > _max)
		output = _max;
	else if (output < _min)
		output = _min;

	return output;
#endif
}