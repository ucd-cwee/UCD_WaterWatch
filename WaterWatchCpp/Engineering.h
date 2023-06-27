/*
WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

Copyright (c) 2019 - 2023 by Robert Good, Erin Musabandesu, and David Linz. (Email: rtgood@ucdavis.edu). All rights reserved.

Created: RTG	/	2023
History: RTG	/	2023		1. Initial Release.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code.
*/

#pragma once
#include "Precompiled.h"
#include "Strings.h"
#include "List.h"
#include "Parser.h"
#include "cwee_math.h"
#include "vec.h"
#include "Curve.h"
#include "cweeUnitedValue.h"

INLINE int vec3d_compare(const void* a, const void* b) {
	vec3d& aR = *((vec3d*)a);
	vec3d& bR = *((vec3d*)b);
	if (aR.x < bR.x) return 1;
	else return 0;
	//return -1;
}
INLINE void vec3d_swap(vec3d* a, vec3d* b, vec3d& temp) {
	temp = *a;
	*a = *b;
	*b = temp;
}
INLINE int vec3d_partition(vec3d* arr, int low, int high) {
	auto& pivot = arr[cweeRandomInt(low, high)];    // pivot // was 'high' -- randomizing the pivot should result in lower complexity when the array is already sorted. 
	int i = (low - 1);			// Index of smaller element
	vec3d temp;
	for (int j = low; j <= high - 1; j++)
	{
		// If current element is smaller than or
		// equal to pivot
		if (vec3d_compare(&arr[j], &pivot) <= 0)
		{
			i++;    // increment index of smaller element
			vec3d_swap(&arr[i], &arr[j], temp);
		}
	}
	vec3d_swap(&arr[i + 1], &arr[high], temp);
	return (i + 1);
}
INLINE void vec3d_quickSort(vec3d* arr, int low, int high) {
	if (low < high)
	{
		/* pi is partitioning index, arr[p] is now
		   at right place */
		int pi = vec3d_partition(arr, low, high);

		// Separately sort elements before
		// partition and after partition
		vec3d_quickSort(arr, low, pi - 1);
		vec3d_quickSort(arr, pi + 1, high);
	}
}

namespace cweeEng {
	using namespace cwee_units;

	/*!
	Assumes head on y-axis and flow on x-axis.
	*/
	static foot_t SamplePumpHeadCurve(Curve const& curve, gallon_per_minute_t flow) {
		switch (curve.GetNumValues()) {
		case 0: return foot_t(flow() / 5.0f);		
		case 1: {
			Curve newCurve = curve;
			float designHead = newCurve.GetValue(0);
			float designFlow = newCurve.GetTime(0);

			newCurve.AddUniqueValue(0, designHead * 1.33f); // epanet approach
			newCurve.AddUniqueValue(designFlow * 2.0f, 0); // epanet approach

			return newCurve.GetCurrentValue(flow()); // catmull_rom spline sample
		}
		case 2: {
			Curve newCurve = curve;

			float designHead = cweeMath::Faverage({ newCurve.GetValue(0), newCurve.GetValue(1) });
			float designFlow = cweeMath::Faverage({ (float)newCurve.GetTime(0), (float)newCurve.GetTime(1) });
			float maxFlow = cweeMath::Fmax(designFlow * 2.0f, 1.5f * cweeMath::Fmax((float)newCurve.GetTime(0), (float)newCurve.GetTime(1)));
			float maxHead = cweeMath::Fmax(designHead * 1.33f, 1.15f * cweeMath::Fmax((float)newCurve.GetValue(0), (float)newCurve.GetValue(1)));

			newCurve.AddUniqueValue(0, maxHead);
			newCurve.AddUniqueValue(maxFlow, 0);

			return newCurve.GetCurrentValue(flow()); // catmull_rom spline sample
		}
		default: return curve.GetCurrentValue(flow()); // catmull_rom spline sample		
		}
	}

	static constexpr kilowatt_t CentrifugalPumpEnergyDemand_kW(gallon_per_minute_t Flow_gpm, foot_t Head_feet, scalar_t Efficiency_percent) {
		return ((cubic_meter_per_hour_t)Flow_gpm) * units::constants::d * units::constants::g * ((meter_t)Head_feet) / (Efficiency_percent / 100.0);
	};
	static constexpr square_foot_t SurfaceAreaCircle_ft2(foot_t Diameter_feet) {
		return units::math::cpow<2>(Diameter_feet) * (scalar_t)cweeMath::PI * (scalar_t)0.25;
	};
	static constexpr gallon_t VolumeCylinder_gal(foot_t Diameter_feet, foot_t Height_feet) {
		return SurfaceAreaCircle_ft2(Diameter_feet) * Height_feet; // converts from cubic_foot_t to gallon_t auto-magically. 
	};
	static constexpr feet_per_hour_t Cylinder_FlowRate_to_LevelRate_fph(gallon_per_minute_t FlowRate_gpm, foot_t Diameter_feet) {
		return cubic_foot_per_hour_t(FlowRate_gpm) / SurfaceAreaCircle_ft2(Diameter_feet);
	};
	static constexpr foot_t Cylinder_Volume_to_Level_f(cubic_foot_t volume, foot_t Diameter_feet) {
		return volume / SurfaceAreaCircle_ft2(Diameter_feet);
	};
	static constexpr pounds_per_square_inch_t Head_to_Pressure_psi(foot_t HydraulicHead_feet, foot_t BaseElevation_feet) {
		return (HydraulicHead_feet - BaseElevation_feet)() * 0.4329004329f;
	};
	static vec3 GetMadConversion(const measurement_t& from, const measurement_t& to) {
		return cweeUnits::GetMadConversion(from, to);
	};

	static vec3 RotatePositionXY(const vec3& start, float clockwiseRotationDegrees, const vec3& rotateAround = vec3(0, 0, 0)) {
		vec3 out;
		float angle = (360.0f - clockwiseRotationDegrees) * (cweeMath::PI / 180.0f);
		float x = cosf(angle);
		float y = sinf(angle);
		out.x = (start.x - rotateAround.x) * x - (start.y - rotateAround.y) * y + rotateAround.x;
		out.y = (start.y - rotateAround.y) * x + (start.x - rotateAround.x) * y + rotateAround.y;
		out.z = start.z;
		return out;
	};
	static vec3d RotatePositionXY(const vec3d& start, float clockwiseRotationDegrees, const vec3d& rotateAround = vec3d(0, 0, 0)) {
		vec3d out;
		double angle = (360.0f - clockwiseRotationDegrees) * (cweeMath::PI / 180.0f);
		double x = cosf(angle);
		double y = sinf(angle);
		out.x = (start.x - rotateAround.x) * x - (start.y - rotateAround.y) * y + rotateAround.x;
		out.y = (start.y - rotateAround.y) * x + (start.x - rotateAround.x) * y + rotateAround.y;
		out.z = start.z;
		return out;
	};

	// true only for the DW equation
	static inch_t EquivalentPipeDiameter_ConstantFlow(foot_t pipe1_length, foot_t pipe2_length, inch_t pipe1_diameter, inch_t pipe2_diameter) {
		// from: https://pdf.sciencedirectassets.com/278653/1-s2.0-S1877705817X00179/1-s2.0-S1877705817313991/main.pdf?X-Amz-Security-Token=IQoJb3JpZ2luX2VjEPD%2F%2F%2F%2F%2F%2F%2F%2F%2F%2FwEaCXVzLWVhc3QtMSJGMEQCIBvCRSY0I3te7nlj1iryyTb7MJTMPlVrOssmSDmmxV56AiAOJzRyuDwv%2BcxvCkzzBTbcYVnOlp9QuUNZXTAJtEbR9iq9AwjJ%2F%2F%2F%2F%2F%2F%2F%2F%2F%2F8BEAMaDDA1OTAwMzU0Njg2NSIMFIToVymz2aRmF6wtKpEDd85poqyoqiweTRIQd8GtOo8E9%2B%2BWPFX4UGDtF%2BCf3CvpqKTvXU0urOQZ65lYpDzzBYxIKwU2PaQCY7m0iy1blMFKDOS7GfGqB2RcmXVnl2yXpzI%2FbFs2DM8jVvjkwSIJxf3aNcQQJJ5zKMbU3phJkvg55wxYdXAx5FpH6jOfJO02Uq98AE6S0cWS2mIgLlJ%2BGCpDfPXhadC1ZD8BuWQb8Eva26Ps68HXE30%2BXlVnWfhMqvccsYQtWB6Ch7K9wQ3LYrE7Xwts67%2B%2B42zO%2BZ419T0%2FDW7cRUTqGCe17Ur1EaAYCUmXXTZNNGmX7g7hOvms6eDeO%2F8WkllFbotUW0voTLq%2FERQsfZpDVXLSTuOY%2FqOGlIiqvM3VQ6XVpX3K4WxthnRzKQuhsuKn6eU0rhbSsUsrUNximKHemJV5Ve4h%2FChM8q3BvQWMHN2A84EAKzPJ2ukkwuAHYD6PrtwDPiFQqY1m1T43%2BHlc2PSgxWtjm1jnUGpw1wMh0oMYpT369NPfk8unN3QLIij8ZMCtpAVObjww2s2tgAY67AE7DyXRJTBae%2F92UFkxxo7R8PPoP0RsSrYHHpp5zxYei0NOlWNgvZN%2FK%2FTfxzBycZuSvkLmhNm7FIp4c8Khddt3YSbmk1%2BN01vlAI7TymZmGn%2Ff%2Fq8tUyW6hJsdjivJXCW%2BGDeJ8msALEthedb3a0L7Ziic%2Bb6YrmbmvbusHF0LutJ6EWiMa%2B48RK5NkdehNzqWj9MmmkfmTk93RYP80Ll4R6srvlhixx%2Bl9KHxCJPZz8vSki0LFu86P0QwfPfYdcqHyWS0dDkgs2j3CQeCHBtxwtv3y3C8qC4gDEILsS2iPGECN3jFtL4GSWkIFA%3D%3D&X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Date=20210123T014359Z&X-Amz-SignedHeaders=host&X-Amz-Expires=300&X-Amz-Credential=ASIAQ3PHCVTYSWFXPUFL%2F20210123%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Signature=ebf9629e15e043d99b422b093bfef1cc1af095fe76298a81bc4ce33b8c812e2a&hash=16c0bc952b2da6894978af4488940ed24c84691d41e1de87abd2012b16027cfa&host=68042c943591013ac2b2430a89b270f6af2c76d8dfd086a07176afe7c76c2c61&pii=S1877705817313991&tid=spdf-24b18965-1fee-4f69-bcb5-d2f705f42550&sid=6e4529fc8bf8934ce51b64a3dcc0320f0b1dgxrqa&type=client
		return ::pow((pipe1_length + pipe2_length)(), 0.2) / ::pow(((pipe1_length() / ::pow(pipe1_diameter(), 5.0)) + (pipe2_length() / ::pow(pipe2_diameter(), 5.0))), 0.2 );
	};

	static double EquivalentPipeRoughness(inch_t desiredDiameter, foot_t pipe1_length, foot_t pipe2_length, inch_t pipe1_diameter, inch_t pipe2_diameter, const double& pipe1_roughness, const double& pipe2_roughness) {
		// From: https://ecommons.udayton.edu/cgi/viewcontent.cgi?article=1013&context=cee_fac_pub
		// Eq. 3.4

		AUTO L_r = pipe1_length + pipe2_length;
		AUTO D_r = desiredDiameter; // cweeMath::Faverage({ pipe1_diameter, pipe2_diameter });

		if (pipe1_diameter == pipe2_diameter && pipe1_roughness == pipe2_roughness) return pipe1_roughness;

		double C_r =
			::pow(L_r() / (::pow(D_r(), 4.87)), 0.54)
			*
			::pow( (pipe1_length() / ((::pow(pipe1_diameter(), 4.87)) * (::pow(pipe1_roughness, 1.85)))) + (pipe2_length() / ((::pow(pipe2_diameter(), 4.87)) * (::pow(pipe2_roughness, 1.85)))) , -0.54);

		return C_r;
	};

	static constexpr float ReynoldsNumberInPipe(feet_per_second_t velocity_ftPerSec, inch_t diameter_inches, float kinematicViscosity_ft2PerSec = 0.000001004f) {
		return (velocity_ftPerSec * foot_t(diameter_inches))() / (kinematicViscosity_ft2PerSec == 0.0f ? 0.000001004 : kinematicViscosity_ft2PerSec);
	};

	static constexpr fahrenheit_t CelsiusToFahrenheit(celsius_t C) {
		return C; // return (C * 9.0f / 5.0f) + 32.0f;
	};
	static constexpr celsius_t FahrenheitToCelsius(fahrenheit_t F) {
		return F; // return (F - 32.0f) * 5.0f / 9.0f;
	};

	/*!
	Approximation based on regression.
	Source: https://www.ewra.net/ew/pdf/EW_2017_58_74.pdf
	*/
	static double PipeRoughnessConversion_HW_to_DW(double HW_roughness) {
		return
			cweeMath::Fmin(1.5f,
				cweeMath::Fmax(0.028f,
					(float)((-0.0000028183185026) * (::pow(HW_roughness, 4.0))
					+ (0.0014243470892602) * (::pow(HW_roughness, 3.0))
					- (0.267451030006944) * (::pow(HW_roughness, 2.0))
					+ (22.072188172073) * HW_roughness
					- 673.672526526534)
				)
			);
	};
	/*!
	Approximation based on regression.
	Source: https://www.ewra.net/ew/pdf/EW_2017_58_74.pdf
	*/
	static double PipeRoughnessConversion_DW_to_HW(double DW_roughness) {
		return
			cweeMath::Fmin(150.0f,
				cweeMath::Fmax(100.0f,
					(float)((-28.845364406152) * (::pow(DW_roughness, 4.0))
					+ (34.250225016229) * (::pow(DW_roughness, 3.0))
					- (62.591795817284) * (::pow(DW_roughness, 2.0))
					+ (103.276842824486) * DW_roughness
					- 149.373579659483)
				)
			);
	};

	/*!
	Approximation based on regression.
	Source: https://www.ewra.net/ew/pdf/EW_2017_58_74.pdf
	*/
	static double PipeFrictionFactor_HW_to_DWFF(double HW_roughness, double ReynoldsNumber, inch_t diameter_inches) {
		return
			1016.610
			* ::pow(HW_roughness, -1.85)
			* ::pow(ReynoldsNumber, -0.148)
			* ::pow(diameter_inches(), -0.0158);
	};
	static double PipeFrictionFactor_DW_to_HWFF(double DW_roughness, double ReynoldsNumber, inch_t diameter_inches) {
		if (ReynoldsNumber < 2000) {
			return 64.0 / ReynoldsNumber;
		}
		else if (ReynoldsNumber > 4000) {
			return 0.25 / ::pow(::log( (DW_roughness / (3.7 * foot_t(diameter_inches)())) + (5.74 / ::pow(ReynoldsNumber, 0.9))), 2.0);
		}
		else {
			double X1, R, X2, X3, X4, FA, FB, Y2, Y3;
			R = ReynoldsNumber / 2000.0;
			Y2 = (DW_roughness / (3.7 * foot_t(diameter_inches)())) + (5.74 / (::pow(ReynoldsNumber, 0.9)));
			Y3 = -0.86859 * ::log(
				(DW_roughness / (3.7 * foot_t(diameter_inches)()))
				+ (5.74 / (::pow(4000.0, 0.9)))
			);
			FA = ::pow(Y3, -2.0);
			FB = FA * (
				2.0
				- (0.00514215 / (Y2 * Y3))
				);

			X1 = 7 * FA - FB;
			X2 = 0.128 - 17.0 * FA + 2.5 * FB;
			X3 = -0.128 + 13.0 * FA - 2.0 * FB;
			X4 = R * (0.032 - 3.0 * FA + 0.5 * FB);

			return (X1) + R * ((X2) + R * ((X3) + (X4)));
		}
	};
	static float PipeResistanceCoefficient_HW(float HW_roughness, inch_t diameter_inches, foot_t length_feet, feet_per_second_t velocity_ftPerSes, float kinematicViscosity_ft2PerSec = 0.000001004f) {
		return
			4.727
			* PipeFrictionFactor_HW_to_DWFF(HW_roughness, ReynoldsNumberInPipe(velocity_ftPerSes, diameter_inches, kinematicViscosity_ft2PerSec), diameter_inches)
			* ::pow(foot_t(diameter_inches)(), -4.871)
			* length_feet();
	};
	static float PipeResistanceCoefficient_HW_to_DW(float HW_roughness, inch_t diameter_inches, foot_t length_feet) {
		return
			0.0252
			* ::pow(HW_roughness, -1.852)
			* ::pow(foot_t(diameter_inches)(), -5)
			* length_feet();
	};
	static float PipeResistanceCoefficient_DW_to_HW(float DW_roughness, inch_t diameter_inches, foot_t length_feet, feet_per_second_t velocity_ftPerSes, float kinematicViscosity_ft2PerSec = 0.000001004f) {
		return
			0.0252
			* PipeFrictionFactor_DW_to_HWFF(DW_roughness, ReynoldsNumberInPipe(velocity_ftPerSes, diameter_inches, kinematicViscosity_ft2PerSec), diameter_inches)
			* ::pow(foot_t(diameter_inches)(), -5)
			* length_feet();
	};

	static float PipeHeadloss_HW(float resistanceCoefficient, cubic_foot_per_second_t flowrate_cfs) {
		return resistanceCoefficient * ::pow(flowrate_cfs(), 1.852);
	};
	static float PipeHeadloss_DW(float resistanceCoefficient, cubic_foot_per_second_t flowrate_cfs) {
		return resistanceCoefficient * ::pow(flowrate_cfs(), 2.0);
	};
	static foot_t Approximate_EquivalentPipeDiameter(float FrictionFactor, foot_t length_feet, foot_t totalHeadloss_feet, cubic_foot_per_second_t totalPipeFlow_cfs) {
		return ::pow(
			(8.0 * FrictionFactor * length_feet())
			* (totalPipeFlow_cfs() * totalPipeFlow_cfs())
			/ (cweeMath::PI * cweeMath::PI * cweeMath::G * totalHeadloss_feet())
			, 0.2);
	};
	static inch_t Find_EquivalentPipeDiameter_DW(float DW_roughness, feet_per_second_t velocity_ftPerSes, foot_t length_feet, foot_t totalHeadloss_feet, cubic_foot_per_second_t totalPipeFlow_cfs) {
		constexpr float eps = 0.001f;
		float err = 1.0f;
		int trials = 1000;
		// assume Deq
		inch_t Deq = 8.0f; // 8 inches
		inch_t Deq_Prime = 8.0f; // 8 inches
		while (cweeMath::Fabs(err * err) > eps && trials >= 0) {
			float Re = ReynoldsNumberInPipe(velocity_ftPerSes, Deq);
			float ff = PipeFrictionFactor_DW_to_HWFF(DW_roughness, Re, Deq);

			Deq_Prime = Approximate_EquivalentPipeDiameter(ff, length_feet, totalHeadloss_feet, totalPipeFlow_cfs);

			err = (Deq_Prime - Deq)();
			Deq = Deq_Prime;

			trials--;
		}
		return Deq;
	};

	static inch_t Find_EquivalentPipeDiameter_HW(float HW_roughness, feet_per_second_t velocity_ftPerSes, foot_t length_feet, foot_t totalHeadloss_feet, cubic_foot_per_second_t totalPipeFlow_cfs) {
		constexpr float eps = 0.001f;
		float err = 1.0f;
		int trials = 1000;
		// assume Deq
		inch_t Deq = 8.0f; // 8 inches
		inch_t Deq_Prime = 8.0f; // 8 inches
		while (cweeMath::Fabs(err * err) > eps && trials >= 0) {
			float Re = ReynoldsNumberInPipe(velocity_ftPerSes, Deq);
			float ff = PipeFrictionFactor_HW_to_DWFF(HW_roughness, Re, Deq);

			Deq_Prime = Approximate_EquivalentPipeDiameter(ff, length_feet, totalHeadloss_feet, totalPipeFlow_cfs);

			err = (Deq_Prime - Deq)();
			Deq = Deq_Prime;

			trials--;
		}

		return Deq;

	};

	static vec2d CoordinateConversion_FeetToLongLat(vec2d XY_Feet,
		double centralMeridian = -120.5000000000000000,
		double LatFirstStandardParallel = 37.0666666666666666667,
		double LatSecondStandardParallel = 38.433333333333333333,
		double LatOrigin = 36.500000000000000000,
		double FalseNorthing = 1640416.6666666666666,
		double FalseEasting = 6561666.6666666666666) {

		constexpr double A = 20925604.47;                        	// major radius of GRS 1980 ellipsoid, feet
		constexpr double Ec = 0.0818191910435;                  	// eccentricity of GRD 1980 ellipsoid
		constexpr double Ec2 = Ec * Ec;                          	// eccentricity squared
		constexpr double EcD2 = Ec / 2.0;

		double Pi4, M0, P1, P2, P0, X0, Y0, m1, m2, t1, t2, t0, n, F, rho0, x, y, Pi2, rho, t, LonR, LatR;
		int j;

		Pi4 = cweeMath::PI / 4.0;                			// Pi / 4
		Pi2 = cweeMath::PI / 2.0;							// Pi / 2
		M0 = centralMeridian * cweeMath::AngRad;			// central meridian
		P1 = LatFirstStandardParallel * cweeMath::AngRad;	// latitude of first standard parallel
		P2 = LatSecondStandardParallel * cweeMath::AngRad;	// latitude of second standard parallel
		P0 = LatOrigin * cweeMath::AngRad;   				// latitude of origin
		X0 = FalseEasting;									// False easting of central meridian, map units
		Y0 = FalseNorthing;									// False northing of central meridian, map units

		m1 = ::cos(P1) / ::sqrt(1 - (Ec2 * ::pow((::sin(P1)), 2.0)));
		m2 = ::cos(P2) / ::sqrt(1.0 - (Ec2 * ::pow((::sin(P2)), 2.0)));

		t1 = ::tan(Pi4 - (P1 / 2.0)) / ::pow((1.0 - Ec * ::sin(P1)) / (1.0 + Ec * ::sin(P1)), EcD2);
		t2 = ::tan(Pi4 - (P2 / 2.0)) / ::pow((1.0 - Ec * ::sin(P2)) / (1.0 + Ec * ::sin(P2)), EcD2);
		t0 = ::tan(Pi4 - (P0 / 2.0)) / ::pow((1.0 - Ec * ::sin(P0)) / (1.0 + Ec * ::sin(P0)), EcD2);
		n = ::log(m1 / m2) / ::log(t1 / t2);
		F = m1 / (n * ::pow(t1, n));
		rho0 = A * F * ::pow(t0, n);

		x = XY_Feet.x - X0;
		y = XY_Feet.y - Y0;

		// calc longitude		
		rho = ::sqrt(::pow(x, 2.0) + (::pow(rho0 - y, 2.0)));
		t = ::pow(rho / (A * F), (1.0 / n));
		LonR = ::atan(x / (rho0 - y)) / n + M0;

		// Substitute the estimate into the iterative calculation that converges on the correct Latitude value.	
		LatR = Pi2 - (2.0 * atan(t));
		for (j = 0; j < 5; ++j) {
			LatR = Pi2 - (2.0 * ::atan(t * ::pow((1.0 - (Ec * ::sin(LatR))) / (1.0 + (Ec * ::sin(LatR))), EcD2)));
		}

		// Convert from radians to degrees.
		return vec2d(LonR / cweeMath::AngRad, LatR / cweeMath::AngRad);
	};


	static cweeThreadedList<vec3> N_SidedCircle(int NumSides, const vec3& center, float radius) {
		cweeThreadedList<vec3> out;
		if (NumSides > 0) {
			out.SetGranularity(NumSides + 3);

			vec3 toAdd; toAdd.z = center.z;
			for (int n = 0; n < NumSides; n++) {
				toAdd.x = radius * ::cos(2.0f * cweeMath::PI * n / NumSides) + center.x;
				toAdd.y = radius * ::sin(2.0f * cweeMath::PI * n / NumSides) + center.y;
				out.Append(toAdd);
			}

		}
		return out;
	};
	static cweeThreadedList<vec3d> N_SidedCircle(int NumSides, const vec3d& center, float radius) {
		cweeThreadedList<vec3d> out;
		if (NumSides > 0) {
			out.SetGranularity(NumSides + 3);

			vec3d toAdd; toAdd.z = center.z;
			for (int n = 0; n < NumSides; n++) {
				toAdd.x = radius * ::cos(2.0f * cweeMath::PI * n / NumSides) + center.x;
				toAdd.y = radius * ::sin(2.0f * cweeMath::PI * n / NumSides) + center.y;
				out.Append(toAdd);
			}

		}
		return out;
	};

	static float AngleOffsetFromVerticle(const vec3& middle, const vec3& target)
	{
		double angle; double RadianToDegree = (360.0 / cweeMath::TWO_PI);
		// x and y args to atan2() swapped to rotate resulting angle 90 degrees
		// (Thus angle in respect to +Y axis instead of +X axis)
		angle = ::atan2(middle.x - target.x, target.y - middle.y) * RadianToDegree;

		// Ensure result is in interval [0, 360)
		// Subtract because positive degree angles go clockwise
		return ::fmod(360.0 - angle, 360.0);


		//vec3 vec = vec3((float)target.x - (float)middle.x, (float)target.y - (float)middle.y, 0);
		//vec3 verticle(0, 1, 0);
		//vec.Normalize();
		//auto dot = vec.x * verticle.x + vec.y * verticle.y;
		//auto det = vec.x * verticle.y - vec.y * verticle.x;
		//auto radian = ::atan2((double)det, (double)dot);
		//while (radian < 0) radian += cweeMath::TWO_PI;
		//return radian * (360.0f / cweeMath::TWO_PI); // radian to angle
	}
	static double AngleOffsetFromVerticle(const vec3d& middle, const vec3d& target)
	{
		double angle; double RadianToDegree = (360.0 / cweeMath::TWO_PI);
		angle = ::atan2(middle.x - target.x, target.y - middle.y) * RadianToDegree;
		return ::fmod(360.0 - angle, 360.0);
	}

	class pidLogic {
		/**
		* Copyright 2019 Bradley J. Snyder <snyder.bradleyj@gmail.com>
		*
		* Permission is hereby granted, free of charge, to any person obtaining a copy
		* of this software and associated documentation files (the "Software"), to deal
		* in the Software without restriction, including without limitation the rights
		* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
		* copies of the Software, and to permit persons to whom the Software is
		* furnished to do so, subject to the following conditions:
		*
		* The above copyright notice and this permission notice shall be included in
		* all copies or substantial portions of the Software.
		*
		* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
		* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
		* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
		* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
		* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
		* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
		* THE SOFTWARE.
		*/
	public:
		pidLogic(float maxOutput = 1, float minOutput = 0, float Kp = 0.1, float Kd = 0.1, float Ki = 0.5) :
			_max(maxOutput),
			_min(minOutput),
			_Ku(1),
			_Kp(Kp),
			_Kd(Kd),
			_Ki(Ki),
			_pre_error(0),
			_pre_output(1),
			_integral(0)
		{ };
		float calculate(float setpoint, float pv, const u64& dt = 360) {
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

			expectedPv = (((Pv_1 - Pv_0) / ((Pout_1 - Pout_0) == 0.0f ? 1 : (Pout_1 - Pout_0))) * (Pout_2 - Pout_1)) + Pv_2;

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
		};
		void setSettingsRange(float max, float min = cweeMath::EPSILON, float kU = cweeMath::EPSILON) {
			_max = max;
			_integral = 0;

			if (min != cweeMath::EPSILON) {
				_min = min;
			}
			if (kU != cweeMath::EPSILON) {
				_Ku = kU;
			}

			//check = 0;
			//_accum_error = -1;
		};
		cweeStr Serialize() {
			cweeStr delim = ":pidLogic:";
			cweeStr out;

			out.AddToDelimiter(_max, delim);
			out.AddToDelimiter(_min, delim);
			out.AddToDelimiter(_Ku, delim);
			out.AddToDelimiter(_Kp, delim);
			out.AddToDelimiter(_Kd, delim);
			out.AddToDelimiter(_Ki, delim);
			out.AddToDelimiter(_pre_error, delim);
			out.AddToDelimiter(_pre_output, delim);
			out.AddToDelimiter(_integral, delim);

			return out;
		};
		void Deserialize(const cweeStr& in) {
			cweeParser obj(in, ":pidLogic:", true);

			_max = (float)obj[0];
			_min = (float)obj[1];
			_Ku = (float)obj[2];
			_Kp = (float)obj[3];
			_Kd = (float)obj[4];
			_Ki = (float)obj[5];
			_pre_error = (float)obj[6];
			_pre_output = (float)obj[7];
			_integral = 0;
		};
		float _Ku;
	private:
		float _max;
		float _min;
		float _Kp;
		float _Kd;
		float _Ki;
		float _pre_error;
		float _pre_output;
		float _integral;

		//cweeThreadedList<float> selfTuner;
		//Pattern kU_trials;
		//float _accum_error = -1;
		//int check = 0;

		void tunePID() {
			// automatically self-tune the PID if (and as) necessary		

			//if (check == 100000) {
			//	check = -1; // no more 'calibration'

			//	float bestError = kU_trials.GetMinValue();
			//	_Ku = kU_trials.TimeForIndex(kU_trials.FindExactY(bestError));
			//	kU_trials.Clear();
			//	kU_trials.AddValue(_Ku, bestError);

			//}else if (check >= 0){
			//	selfTuner.Append(_pre_error);
			//	check++;
			//	if (selfTuner.Num() >= 10) {
			//		if (_accum_error < 0) {
			//			float min = cweeMath::INF; float max = -cweeMath::INF;
			//			_accum_error = 0; for (auto& x : selfTuner) {
			//				_accum_error += x;
			//				if (x < min) min = x;
			//				if (x > max) max = x;
			//			}
			//			_accum_error += (max - min); // the "range" of error should be minimized as well.
			//			kU_trials.AddValue(_Ku, cweeMath::Fabs(_accum_error));
			//		}
			//		else {
			//			float min = cweeMath::INF; float max = -cweeMath::INF;
			//			_accum_error = 0; for (auto& x : selfTuner) {
			//				_accum_error += x;
			//				if (x < min) min = x;
			//				if (x > max) max = x;
			//			}
			//			_accum_error += (max - min); // the "range" of error should be minimized as well.

			//			float t = cweeMath::Fabs(_accum_error);
			//			kU_trials.AddValue(_Ku, t);

			//			if (t >= _accum_error) {
			//				// we could do better. 					
			//				if (check % 100 == 0) {
			//					float bestError = kU_trials.GetMinValue();
			//					_Ku = kU_trials.TimeForIndex(kU_trials.FindExactY(bestError));
			//					kU_trials.Clear();
			//					kU_trials.AddValue(_Ku, bestError);
			//				}
			//				else {
			//					_Ku += cweeRandomFloat(-2.0f / (1.0f + (((float)check) / 1000.0f)), 2.0f / (1.0f + (((float)check) / 1000.0f)));
			//				}
			//			}

			//			_accum_error = t;
			//		}
			//		selfTuner.Clear();
			//	}
			//}

		};

	};

	class Solution {
	public:
		static bool mycmpA(vec3& a, vec3& b) {
			return a.x < b.x;
		}
		static int testSide(vec3& a, vec3& b, vec3& c) {
			// cross product of (AB and AC vectors)
			return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
		}
		static double distvec3Line(vec3& A, vec3& B, vec3& C) {
			// dist(line: ax+by+c=0, and vec3(x0, y0)): (a*x0 + b*y0 + c)/sqrt(a^2+b^2)
			// line: (y2-y1)*x - (x2-x1)*y + x2*y1 - y2*x1 = 0
			int a = B.y - A.y, b = B.x - A.x;
			return abs((a * C.x - b * C.y + B.x * A.y - B.y * A.x) / sqrt(a * a + b * b));
		}
		static void FindHull(std::vector<vec3>& vec3s, vec3& A, vec3& B, std::vector<vec3>& ret) {
			if (vec3s.empty())
				return;
			int idx = 0;
			double dist = distvec3Line(A, B, vec3s[0]);
			for (int i = 1; i < vec3s.size(); i++) {
				if (distvec3Line(A, B, vec3s[i]) > dist) {
					dist = distvec3Line(A, B, vec3s[i]);
					idx = i;
				}
			}
			ret.push_back(vec3s[idx]);
			std::vector<vec3> R, T;
			for (int i = 0; i < vec3s.size(); i++) {
				if (i != idx) {
					int tmp = testSide(A, vec3s[idx], vec3s[i]);
					if (tmp >= 0)
						R.push_back(vec3s[i]);
					else {
						tmp = testSide(vec3s[idx], B, vec3s[i]);
						if (tmp >= 0)
							T.push_back(vec3s[i]);
					}
				}
			}
			FindHull(R, A, vec3s[idx], ret);
			FindHull(T, vec3s[idx], B, ret);
			return;
		}
		static std::vector<vec3> outerTrees(std::vector<vec3>& vec3s) {
			std::vector<vec3> ret;

			// find the convex hull; use QuickHull algorithm
			if (vec3s.size() <= 1)
				return vec3s;
			// find the left most and right most two vec3s
			std::sort(vec3s.begin(), vec3s.end(), mycmpA);
			ret.push_back(vec3s[0]);
			ret.push_back(vec3s.back());
			// test whether a vec3 on the left side right side or on the line
			std::vector<vec3> Left, Right, Online;
			for (int i = 1; i < vec3s.size() - 1; i++) {
				int tmp = testSide(vec3s[0], vec3s.back(), vec3s[i]);
				if (tmp < 0)
					Right.push_back(vec3s[i]);
				else if (tmp > 0)
					Left.push_back(vec3s[i]);
				else
					Online.push_back(vec3s[i]);
			}
			// if Upper or Down is empty, Online should be pushed into ret
			if (Left.empty() || Right.empty())
				for (int i = 0; i < Online.size(); i++)
					ret.push_back(Online[i]);
			FindHull(Left, vec3s[0], vec3s.back(), ret);
			FindHull(Right, vec3s.back(), vec3s[0], ret);
			return ret;
		}

#if 0

		static bool mycmpB(vec3d& a, vec3d& b) {
			return a.x < b.x;
		}
		static int testSide(vec3d& a, vec3d& b, vec3d& c) {
			// cross product of (AB and AC vectors)
			return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
		}
		static double distvec3dLine(vec3d& A, vec3d& B, vec3d& C) {
			// dist(line: ax+by+c=0, and vec3d(x0, y0)): (a*x0 + b*y0 + c)/sqrt(a^2+b^2)
			// line: (y2-y1)*x - (x2-x1)*y + x2*y1 - y2*x1 = 0
			int a = B.y - A.y, b = B.x - A.x;
			return abs((a * C.x - b * C.y + B.x * A.y - B.y * A.x) / sqrt(a * a + b * b));
		}
		static void FindHull(std::vector<vec3d>& vec3ds, vec3d& A, vec3d& B, std::vector<vec3d>& ret) {
			if (vec3ds.empty())
				return;
			int idx = 0;
			double dist = distvec3dLine(A, B, vec3ds[0]);
			for (int i = 1; i < vec3ds.size(); i++) {
				if (distvec3dLine(A, B, vec3ds[i]) > dist) {
					dist = distvec3dLine(A, B, vec3ds[i]);
					idx = i;
				}
			}
			ret.push_back(vec3ds[idx]);
			std::vector<vec3d> R, T;
			R.reserve(vec3ds.size() + 16);
			T.reserve(vec3ds.size() + 16);

			for (int i = 0; i < vec3ds.size(); i++) {
				if (i != idx) {
					int tmp = testSide(A, vec3ds[idx], vec3ds[i]);
					if (tmp >= 0)
						R.push_back(vec3ds[i]);
					else {
						tmp = testSide(vec3ds[idx], B, vec3ds[i]);
						if (tmp >= 0)
							T.push_back(vec3ds[i]);
					}
				}
			}
			FindHull(R, A, vec3ds[idx], ret);
			FindHull(T, vec3ds[idx], B, ret);
			return;
		}
		static void outerTrees(std::vector<vec3d>& vec3ds, std::vector<vec3d>& ret) {
			ret.reserve(vec3ds.size() + 16);

			// find the convex hull; use QuickHull algorithm
			if (vec3ds.size() <= 1) {
				ret = vec3ds;
				return;
			}
			// find the left most and right most two vec3ds
			std::sort(vec3ds.begin(), vec3ds.end(), mycmpB);
			ret.push_back(vec3ds[0]);
			ret.push_back(vec3ds.back());
			// test whether a vec3d on the left side right side or on the line
			std::vector<vec3d> Left, Right, Online;
			Left.reserve(vec3ds.size() + 16);
			Right.reserve(vec3ds.size() + 16);
			Online.reserve(vec3ds.size() + 16);

			for (int i = 1; i < vec3ds.size() - 1; i++) {
				int tmp = testSide(vec3ds[0], vec3ds.back(), vec3ds[i]);
				if (tmp < 0)
					Right.push_back(vec3ds[i]);
				else if (tmp > 0)
					Left.push_back(vec3ds[i]);
				else
					Online.push_back(vec3ds[i]);
			}
			// if Upper or Down is empty, Online should be pushed into ret
			if (Left.empty() || Right.empty())
				for (int i = 0; i < Online.size(); i++) ret.push_back(Online[i]);
			FindHull(Left, vec3ds[0], vec3ds.back(), ret);
			FindHull(Right, vec3ds.back(), vec3ds[0], ret);
			return;
		}

#else

		static int testSide(const vec3d& a, const vec3d& b, const vec3d& c) {
			// cross product of (AB and AC vectors)
			return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
		}
		static double distvec3dLine(const vec3d& A, const vec3d& B, const vec3d& C) {
			// dist(line: ax+by+c=0, and vec3d(x0, y0)): (a*x0 + b*y0 + c)/sqrt(a^2+b^2)
			// line: (y2-y1)*x - (x2-x1)*y + x2*y1 - y2*x1 = 0
			double a = B.y - A.y, b = B.x - A.x; // was int
			return abs((a * C.x - b * C.y + B.x * A.y - B.y * A.x) / sqrt(a * a + b * b));
		}
		static void FindHull(const cweeThreadedList<vec3d>& vec3ds, const vec3d& A, const vec3d& B, cweeThreadedList<vec3d>& ret) {
			if (vec3ds.Num() <= 0) return;

			int idx = 0;
			double dist = distvec3dLine(A, B, vec3ds[0]);
			double C;
			for (int i = 1; i < vec3ds.Num(); i++) {
				C = distvec3dLine(A, B, vec3ds[i]);
				if (C > dist) {
					dist = C;
					idx = i;
				}
			}
			ret.Append(vec3ds[idx]);
			cweeThreadedList<vec3d> R(vec3ds.Num() + 16);
			cweeThreadedList<vec3d> T(vec3ds.Num() + 16);
			int tmp;
			for (int i = 0; i < vec3ds.Num(); i++) {
				if (i != idx) {
					tmp = testSide(A, vec3ds[idx], vec3ds[i]);
					if (tmp >= 0)
						R.Append(vec3ds[i]);
					else {
						tmp = testSide(vec3ds[idx], B, vec3ds[i]);
						if (tmp >= 0)
							T.Append(vec3ds[i]);
					}
				}
			}
			FindHull(R, A, vec3ds[idx], ret);
			FindHull(T, vec3ds[idx], B, ret);
			return;
		}
		static void outerTrees(cweeThreadedList<vec3d>& vec3ds, cweeThreadedList<vec3d>& ret) {
			ret.SetGranularity(vec3ds.Num() + 16);

			int n = vec3ds.Num();

			// find the convex hull; use QuickHull algorithm
			if (n <= 1) {
				ret = vec3ds;
				return;
			}

			// find the left most and right most two vec3ds		
			vec3d_quickSort(vec3ds.Ptr(), 0, vec3ds.Num() - 1);
			ret.Append(vec3ds[0]);
			ret.Append(vec3ds[n - 1]);

			// test whether a vec3d on the left side right side or on the line
			cweeThreadedList<vec3d> Left(n + 16);
			cweeThreadedList<vec3d> Right(n + 16);
			cweeThreadedList<vec3d> Online(n + 16);
			int tmp;
			for (int i = 1; i < n - 1; i++) {
				tmp = testSide(vec3ds[0], vec3ds[n - 1], vec3ds[i]);
				if (tmp < 0)
					Right.Append(vec3ds[i]);
				else if (tmp > 0)
					Left.Append(vec3ds[i]);
				else
					Online.Append(vec3ds[i]);
			}

			// if Upper or Down is empty, Online should be pushed into ret
			if (Left.Num() <= 0 || Right.Num() <= 0)
				for (int i = 0; i < Online.Num(); i++)
					ret.Append(Online[i]);

			FindHull(Left, vec3ds[0], vec3ds[n - 1], ret);
			FindHull(Right, vec3ds[n - 1], vec3ds[0], ret);

			return;
		}

#endif

	};

	// reorders / drops points such that this can be drawn as a simple polygon without the lines self-crossing 
	static void ReorderConvexHull(cweeThreadedList<vec3>& points) {
		{ // get the convex hull
			std::vector<vec3> v; v = points;

			points.Clear();

			for (auto& x : Solution::outerTrees(v))
				points.Append(x);
		}
		{ // guarrantee the draw order (Clockwise)
			vec3 middle(0, 0, 0);
			int numSamplesX = 0; int numSamplesY = 0;
			for (auto& point : points)
			{
				cweeMath::rollingAverageRef(middle.x, point.x, numSamplesX);
				cweeMath::rollingAverageRef(middle.y, point.y, numSamplesY);
			}
			middle.z = 0;

	
			cweeCurve<vec3> ordered;
			float key;
			for (int i = 0; i < points.Num(); i++)
			{
				key = AngleOffsetFromVerticle(middle, points[i]);
				key = ::fmod(360.0 - key, 360.0);
				key += 360.0;
				ordered.AddUniqueValue(key, points[i]);
			}
			points.Clear();
			for (auto& x : ordered.GetKnotSeries()) points.Append(x.second);


		}
	};

	static void ReorderConvexHull(cweeThreadedList<vec3d>& points) {

		// get the convex hull
		cweeThreadedList<vec3d> returned;
		Solution::outerTrees(points, returned);

		{ // guarrantee the draw order (Clockwise)
			vec3d middle(0, 0, 0);
			int numSamplesX = 0; int numSamplesY = 0;
			for (auto& point : points)
			{
				cweeMath::rollingAverageRef(middle.x, point.x, numSamplesX);
				cweeMath::rollingAverageRef(middle.y, point.y, numSamplesY);
			}
			middle.z = 0;

			cweeCurve<vec3d> ordered; ordered.SetGranularity(returned.Num() + 16);
			double key;
			for (int i = 0; i < returned.Num(); i++)
			{
				key = AngleOffsetFromVerticle(middle, returned[i]);
				key = ::fmod(360.0 - key, 360.0);
				key += 360.0;
				ordered.AddUniqueValue(key, returned[i]);
			}

			int n = ordered.GetNumValues();
			points.SetNum(n);
			for (int i = 0; i < n; i++) points[i] = *ordered.GetValueAddress(i);
		}
	};

	static bool IsPointInPolygon(const cweeThreadedList<vec3>& polygon, const vec3& testPoint)
	{
		bool result = false;
		int j = polygon.Num() - 1;
		for (int i = 0; i < polygon.Num(); i++)
		{
			if (polygon[i].y < testPoint.y && polygon[j].y >= testPoint.y || polygon[j].y < testPoint.y && polygon[i].y >= testPoint.y)
			{
				if (polygon[i].x + (testPoint.y - polygon[i].y) / (polygon[j].y - polygon[i].y) * (polygon[j].x - polygon[i].x) < testPoint.x)
				{
					result = !result;
				}
			}
			j = i;
		}
		return result;
	};
	static bool IsPointInPolygon(const cweeThreadedList<vec3d>& polygon, const vec3d& testPoint)
	{
		bool result = false;
		int j = polygon.Num() - 1;
		for (int i = 0; i < polygon.Num(); i++)
		{
			if (polygon[i].y < testPoint.y && polygon[j].y >= testPoint.y || polygon[j].y < testPoint.y && polygon[i].y >= testPoint.y)
			{
				if (polygon[i].x + (testPoint.y - polygon[i].y) / (polygon[j].y - polygon[i].y) * (polygon[j].x - polygon[i].x) < testPoint.x)
				{
					result = !result;
				}
			}
			j = i;
		}
		return result;
	};

	static int WhichSide(const cweeThreadedList<vec3>& C, const vec2& D, const vec3& V)
	{
		int i; float t;
		// C vertices are projected to the form V+t*D.
		// Return value is +1 if all t > 0, -1 if all t < 0, 0 otherwise, in
		// which case the line splits the polygon.
		int positive = 0, negative = 0;
		for (i = 0; i < C.Num(); i++)
		{
			t = D.Dot((C[i] - V).GetVec2());
			if (t > 0) positive++; else if (t < 0) negative++;
			if (positive && negative) return 0;
		}
		return (positive ? +1 : -1);
	};
	static int WhichSide(const cweeThreadedList<vec3d>& C, const vec2d& D, const vec3d& V)
	{
		int i; float t;
		// C vertices are projected to the form V+t*D.
		// Return value is +1 if all t > 0, -1 if all t < 0, 0 otherwise, in
		// which case the line splits the polygon.
		int positive = 0, negative = 0;
		for (i = 0; i < C.Num(); i++)
		{
			t = D.Dot((C[i] - V).GetVec2());
			if (t > 0) positive++; else if (t < 0) negative++;
			if (positive && negative) return 0;
		}
		return (positive ? +1 : -1);
	};

	static bool ObjectsIntersect(const cweeThreadedList<vec3>& C0, const cweeThreadedList<vec3>& C1)
	{
		int i0, i1;
		vec2 E, D;

		// Test edges of C0 for separation. Because of the counterclockwise ordering,
		// the projection interval for C0 is [m,0] where m <= 0. Only try to determine
		// if C1 is on the ‘positive’ side of the line.
		for (i0 = 0, i1 = C0.Num() - 1; i0 < C0.Num(); i1 = i0, i0++)
		{
			E = (C0[i0] - C0[i1]).GetVec2(); // or precompute edges if desired
			D = vec2(E.y, -E.x);
			if (WhichSide(C1, D, C0[i0]) > 0)
			{ // C1 is entirely on ‘positive’ side of line C0.V(i0)+t*D
				return false;
			}
		}
		// Test edges of C1 for separation. Because of the counterclockwise ordering,
		// the projection interval for C1 is [m,0] where m <= 0. Only try to determine
		// if C0 is on the ‘positive’ side of the line.
		for (i0 = 0, i1 = C1.Num() - 1; i0 < C1.Num(); i1 = i0, i0++)
		{
			E = (C1[i0] - C1[i1]).GetVec2(); // or precompute edges if desired
			D = vec2(E.y, -E.x);
			if (WhichSide(C0, D, C1[i0]) > 0)
			{ // C0 is entirely on ‘positive’ side of line C1.V(i0)+t*D
				return false;
			}
		}
		return true;
	};
	static bool ObjectsIntersect(const cweeThreadedList<vec3d>& C0, const cweeThreadedList<vec3d>& C1)
	{
		int i0, i1;
		vec2d E, D;

		// Test edges of C0 for separation. Because of the counterclockwise ordering,
		// the projection interval for C0 is [m,0] where m <= 0. Only try to determine
		// if C1 is on the ‘positive’ side of the line.
		for (i0 = 0, i1 = C0.Num() - 1; i0 < C0.Num(); i1 = i0, i0++)
		{
			E = (C0[i0] - C0[i1]).GetVec2(); // or precompute edges if desired
			D = vec2d(E.y, -E.x);
			if (WhichSide(C1, D, C0[i0]) > 0)
			{ // C1 is entirely on ‘positive’ side of line C0.V(i0)+t*D
				return false;
			}
		}
		// Test edges of C1 for separation. Because of the counterclockwise ordering,
		// the projection interval for C1 is [m,0] where m <= 0. Only try to determine
		// if C0 is on the ‘positive’ side of the line.
		for (i0 = 0, i1 = C1.Num() - 1; i0 < C1.Num(); i1 = i0, i0++)
		{
			E = (C1[i0] - C1[i1]).GetVec2(); // or precompute edges if desired
			D = vec2d(E.y, -E.x);
			if (WhichSide(C0, D, C1[i0]) > 0)
			{ // C0 is entirely on ‘positive’ side of line C1.V(i0)+t*D
				return false;
			}
		}
		return true;
	};

	static bool PolygonsOverlap(const cweeThreadedList<vec3>& polygon1, const cweeThreadedList<vec3>& polygon2) {
		return ObjectsIntersect(polygon1, polygon2);
	};
	static bool PolygonsOverlap(const cweeThreadedList<vec3d>& polygon1, const cweeThreadedList<vec3d>& polygon2) {
		return ObjectsIntersect(polygon1, polygon2);
	};

	template<typename T>
	static AUTO R_Squared(const cweeList<T>& Real, const cweeList<T>& Estimate) {		
		T avg_real; { avg_real = 0; int numSamples = 0;
			for (auto& x : Real) cweeMath::rollingAverageRef(avg_real, x, numSamples);
		}


		int N = cweeMath::min(Real.Num(), Estimate.Num());
		double SumErrReal = 0;
		double SumErrEstimate = 0;

		for (int i = 0; i < N; i++) {
			double real_error = (double)(Real[i] - avg_real);
			double estimate_error = (double)(Real[i] - Estimate[i]);
			SumErrReal += real_error * real_error;
			SumErrEstimate += estimate_error * estimate_error;
		};

		AUTO out = avg_real / avg_real; 
		out = 1.0 - (SumErrEstimate / SumErrReal);
		return out;
	};
#if 0
	class LeakageModel {
	public:
		class FailureRecord {
		public:
			FailureRecord() {};
			FailureRecord(gallon_per_minute_t flowrate, time::hour_t time) : FailureFlowrate(flowrate), ResponseTime(time) {};
			gallon_per_minute_t FailureFlowrate;
			time::hour_t ResponseTime;		
		};
		class Inputs {
		public:
			acre_foot_per_year_t CurrentAnnualRealLosses = 1810.818137_ac_ft_y;
			scalar_t InfrastructureLeakageIndex = 1.042783299;
			mile_t TotalMileage = 852.95_mi; 
			acre_foot_per_year_t UnavoidableAnnualRealLosses = 1736.523915_ac_ft_y;
			Dollar_per_gallon_t VariableProductionCost = 396.1903737_USD / 1_ac_ft;
			scalar_t NumberOfConnections = 63665;
			pounds_per_square_inch_t AverageOperatingPressure = 109.45_psi;
			foot_t AverageLengthOfCustomerServiceLine = 0_ft;
			scalar_t InfrastructureConditionFactor = 1.5;
			scalar_t PercentageOfPipesThatAreRigid = 75;
			kilowatt_hour_per_acre_foot_t MarginalEnergyIntensity = 2500_kWh / 1_ac_ft;
			
			Dollar_t AverageLeakRepairCost = 3738.49_USD;
			Dollar_t IndividualPressureModulatedPRVCost = 6000.0_USD;
			Dollar_per_mile_t LeakDetectionSurveyCost = 353.29_USD / 1_mi;
			Dollar_per_mile_t HydraulicModelDevelopmentCost = 148.92_USD / 1_mi;
			scalar_t LeakDetectionMethodAccuracy = 70;

			std::map<inch_t, FailureRecord> FailureRecords = {
				{ 12_in, FailureRecord(245.7_gpm, 26.4_hr) }
				, { 10_in, FailureRecord(202.2_gpm, 26.4_hr) }
				, { 8_in, FailureRecord(167.4_gpm, 26.4_hr) }
				, { 6_in, FailureRecord(122.8_gpm, 26.4_hr) }
				, { 4_in, FailureRecord(69.4_gpm, 26.4_hr) }
				//, { 0_in, FailureRecord(0_gpm, 384_hr) }
			};

			year_t TimeHorizon = 30_yr;
			scalar_t DiscountRate = 0.04;
			scalar_t SocialCostOfCarbon_GrowthRate = 0.03;
			scalar_t WaterProductionCost_GrowthRate = 0.0248;
			Dollar_per_ton_t SocialCostOfCarbon = 42_USD / 1_t;
			ton_per_kilowatt_hour_t CarbonEmissionRate = 0.000237_t / 1_kWh;

			acre_foot_per_year_t AnnualRateOfRiseOfLeakage = 20_ac_ft_y;
			gallon_per_minute_t AverageFlowrateOfUnreportedDetectableLeaks = 2.9_gpm;

			gallon_t StorageCapacity = 78.12761648_MG;
			per_year_t BackgroundLeakageRateForStorage = 0.25_gpm / 1_MG;

			per_year_t MainFailureRate = (90.0 / (1_yr));
		};
		class PR_Costs {
		public:
			PR_Costs() {};
			PR_Costs(Inputs const& inputs) {
				N1 = 1.5 - (1.0 - (2.0 / 3.0) * (inputs.InfrastructureConditionFactor / inputs.InfrastructureLeakageIndex)) * inputs.PercentageOfPipesThatAreRigid / 100.0;

				auto numFailures = (inputs.MainFailureRate * 1_yr);
				TotalRealLossesFromFailures = 0;
				for (auto& failureType : inputs.FailureRecords) {
					TotalRealLossesFromFailures += (numFailures / (double)(inputs.FailureRecords.size())) * failureType.second.ResponseTime * failureType.second.FailureFlowrate / 1_yr;
				}

				cweeList<acre_foot_per_year_t> mains;
				mains.Append(
					(2.87 * 365 * inputs.TotalMileage / 325851.427242 * inputs.AverageOperatingPressure * std::pow((inputs.AverageOperatingPressure / 70_psi)(), N1()) * inputs.InfrastructureConditionFactor)()
				);
				mains.Append(
					(1.75 * 365 * inputs.TotalMileage / 325851.427242 * inputs.AverageOperatingPressure * std::pow((inputs.AverageOperatingPressure / 70_psi)(), N1()) * inputs.InfrastructureConditionFactor)()
				);
				mains.Append(
					(0.77 * 365 * inputs.TotalMileage / 325851.427242 * inputs.AverageOperatingPressure * std::pow((inputs.AverageOperatingPressure / 70_psi)(), N1()) * inputs.InfrastructureConditionFactor)()
				);

				cweeList<acre_foot_per_year_t> services;
				services.Append(
					(0.112 * 365 * inputs.NumberOfConnections / 325851.427242 * inputs.AverageOperatingPressure * std::pow((inputs.AverageOperatingPressure / 70_psi)(), N1()) * inputs.InfrastructureConditionFactor)()
				);
				services.Append(
					(0.007 * 365 * inputs.NumberOfConnections / 325851.427242 * inputs.AverageOperatingPressure * std::pow((inputs.AverageOperatingPressure / 70_psi)(), N1()) * inputs.InfrastructureConditionFactor)()
				);
				services.Append(
					(0.03 * 365 * inputs.NumberOfConnections / 325851.427242 * inputs.AverageOperatingPressure * std::pow((inputs.AverageOperatingPressure / 70_psi)(), N1()) * inputs.InfrastructureConditionFactor)()
				);

				totalBackgroundLeakage = mains[0] + services[0];
				totalReportedLeakage = mains[1] + services[1];
				totalUnreportedLeakage = mains[2] + services[2];
				total_UARL_Leakage = totalBackgroundLeakage + totalReportedLeakage + totalUnreportedLeakage;

				HydraulicModelCosts = inputs.HydraulicModelDevelopmentCost * inputs.TotalMileage;

				TotalLeaksFromStorage = inputs.StorageCapacity * inputs.BackgroundLeakageRateForStorage;
				TotalRealLossesFromFailures += totalReportedLeakage;

				ReducableRealBackgroundLosses = (inputs.CurrentAnnualRealLosses - TotalLeaksFromStorage) - TotalRealLossesFromFailures;
			}

			acre_foot_per_year_t TotalRealLossesFromFailures;

			scalar_t N1;
			Dollar_t HydraulicModelCosts;

			acre_foot_per_year_t TotalLeaksFromStorage;

			acre_foot_per_year_t ReducableRealBackgroundLosses;
			acre_foot_per_year_t totalBackgroundLeakage;
			acre_foot_per_year_t totalReportedLeakage;
			acre_foot_per_year_t totalUnreportedLeakage;
			acre_foot_per_year_t total_UARL_Leakage;

		};
		class Benefits {
		public:
			Benefits() {};
			Benefits(Inputs const& inputs) {
				NpvSccFactor = 
					((1.0-std::pow(((1.0+(inputs.SocialCostOfCarbon_GrowthRate))/(1.0+(inputs.DiscountRate)))(), (year_t(inputs.TimeHorizon) + 1_yr)())) /
					(1.0-((1.0+(inputs.SocialCostOfCarbon_GrowthRate))/(1.0+(inputs.DiscountRate)))))();

				ValueOfAvoidedCO2 = inputs.MarginalEnergyIntensity * inputs.CarbonEmissionRate * inputs.SocialCostOfCarbon;

				NpvOfCO2Savings = ValueOfAvoidedCO2 * NpvSccFactor;

				NpvWaterProductionFactor = 
					((1.0-std::pow(((1.0+(inputs.WaterProductionCost_GrowthRate))/(1.0+(inputs.DiscountRate)))(), (year_t(inputs.TimeHorizon) + 1_yr)())) /
					(1.0-((1.0+(inputs.WaterProductionCost_GrowthRate))/(1.0+(inputs.DiscountRate)))))();

				NpvOfAvoidedWaterProduction = NpvWaterProductionFactor * inputs.VariableProductionCost;

				NpvOfBenefits = NpvOfCO2Savings + NpvOfAvoidedWaterProduction;
			};

			scalar_t NpvSccFactor;
			Dollar_per_gallon_t ValueOfAvoidedCO2;
			Dollar_per_gallon_t NpvOfCO2Savings;
			scalar_t NpvWaterProductionFactor;
			Dollar_per_gallon_t NpvOfAvoidedWaterProduction;
			Dollar_per_gallon_t NpvOfBenefits;
		};

		class LDR_Costs {
		public:
			LDR_Costs() {};
			LDR_Costs(Inputs const& inputs) {
				AverageWaterLostPerYearPerLeak = inputs.AverageFlowrateOfUnreportedDetectableLeaks / 2.0;

				AdjustedPipeRepairCost = inputs.AverageLeakRepairCost / (inputs.LeakDetectionMethodAccuracy / 100.0);
				LeakDetectionSurveyCost = inputs.LeakDetectionSurveyCost * inputs.TotalMileage;

				totalVolume = std::map<int, acre_foot_t>({ {0,0} });
				leakageThisYear = std::map<int, acre_foot_per_year_t>({ {0,0} });
				NpvSccFactorThisYear = std::map<int, scalar_t>({ {0,1} });
				NpvWaterProductionFactorThisYear = std::map<int, scalar_t>({ {0,1} });
				TotalBenefitThisYear = std::map<int, Dollar_per_gallon_t>({ {0,0} });
				TotalBenefit_Dollars_ThisYear = std::map<int, Dollar_t>({ {0,0} });
				TotalCost_Dollars_ThisYear = std::map<int, Dollar_t>({ {0,0} });
				BenefitsMinusCost_To_SurveyFrequency = std::map<Dollar_t, int>({ { 0, 0 } });
				Dollar_per_gallon_t ValueOfAvoidedCO2 = inputs.MarginalEnergyIntensity * inputs.CarbonEmissionRate * inputs.SocialCostOfCarbon;

				for (double surveyFreq = 1; surveyFreq <= 100; surveyFreq++) {
					totalVolume[surveyFreq] = (inputs.AnnualRateOfRiseOfLeakage * surveyFreq * surveyFreq / 2.0)();
				}
				for (double surveyFreq = 1; surveyFreq <= 100; surveyFreq++) {
					leakageThisYear[surveyFreq] = (totalVolume[surveyFreq] - totalVolume[surveyFreq - 1]) / 1_yr;
				}
				for (double surveyFreq = 1; surveyFreq <= 100; surveyFreq++) {
					NpvSccFactorThisYear[surveyFreq] = 
						((1.0-std::pow(((1.0+(inputs.SocialCostOfCarbon_GrowthRate))/(1.0+(inputs.DiscountRate)))(), (year_t(surveyFreq) + 1_yr)())) /
						(1.0-((1.0+(inputs.SocialCostOfCarbon_GrowthRate))/(1.0+(inputs.DiscountRate)))))();
				}
				for (double surveyFreq = 1; surveyFreq <= 100; surveyFreq++) {
					NpvWaterProductionFactorThisYear[surveyFreq] =
						((1.0-std::pow(((1.0+(inputs.WaterProductionCost_GrowthRate))/(1.0+(inputs.DiscountRate)))(), (year_t(surveyFreq) + 1_yr)())) /
						(1.0-((1.0+(inputs.WaterProductionCost_GrowthRate))/(1.0+(inputs.DiscountRate)))))();
				}
				for (double surveyFreq = 1; surveyFreq <= 100; surveyFreq++) {
					TotalBenefitThisYear[surveyFreq] = ValueOfAvoidedCO2 * NpvSccFactorThisYear[surveyFreq] + inputs.VariableProductionCost * NpvWaterProductionFactorThisYear[surveyFreq];						
				}
				for (double surveyFreq = 1; surveyFreq <= 100; surveyFreq++) {
					TotalBenefit_Dollars_ThisYear[surveyFreq] = TotalBenefitThisYear[surveyFreq] * totalVolume[surveyFreq];
				}
				for (double surveyFreq = 1; surveyFreq <= 100; surveyFreq++) {
					TotalCost_Dollars_ThisYear[surveyFreq] = 
						((leakageThisYear[surveyFreq])/(AverageWaterLostPerYearPerLeak))*(AdjustedPipeRepairCost)+(LeakDetectionSurveyCost);
				}
				for (double surveyFreq = 1; surveyFreq <= 100; surveyFreq++) {
					BenefitsMinusCost_To_SurveyFrequency[TotalBenefit_Dollars_ThisYear[surveyFreq] - TotalCost_Dollars_ThisYear[surveyFreq]] = surveyFreq;
				}

				BreakEven_SurveyFrequency = 0;;
				BreakEven_BenefitsUsed = 0;
				BreakEven_TotalCost = 0;
				BreakEven_EconomicPercentOfSystemPerYear = 0;
				for (auto& option : BenefitsMinusCost_To_SurveyFrequency) {
					if (option.first > 0_USD) {
						BreakEven_SurveyFrequency = option.second;
						BreakEven_TotalCost = TotalCost_Dollars_ThisYear[option.second];
						BreakEven_BenefitsUsed = TotalBenefitThisYear[option.second];
						BreakEven_EconomicPercentOfSystemPerYear = 1.0 / (double)(option.second);
						break;
					}
				}
			};

			std::map<int, acre_foot_t> totalVolume;// = { {0,0} };
			std::map<int, acre_foot_per_year_t> leakageThisYear;// = { {0,0} };
			std::map<int, scalar_t> NpvSccFactorThisYear;// = { {0,1} };
			std::map<int, scalar_t> NpvWaterProductionFactorThisYear;// = { {0,1} };
			std::map<int, Dollar_per_gallon_t> TotalBenefitThisYear;// = { {0,0} };
			std::map<int, Dollar_t> TotalBenefit_Dollars_ThisYear;// = { {0,0} };
			std::map<int, Dollar_t> TotalCost_Dollars_ThisYear;// = { {0,0} };
			std::map<Dollar_t, int> BenefitsMinusCost_To_SurveyFrequency;// = { { 0, 0 } };

			acre_foot_per_year_t AverageWaterLostPerYearPerLeak;
			Dollar_t AdjustedPipeRepairCost;
			Dollar_t LeakDetectionSurveyCost;
			
			year_t BreakEven_SurveyFrequency;
			Dollar_per_gallon_t BreakEven_BenefitsUsed;
			Dollar_t BreakEven_TotalCost;
			scalar_t BreakEven_EconomicPercentOfSystemPerYear;

		};

		class TimeHorizon {
		public:
			TimeHorizon() {};
			TimeHorizon(Inputs inputs, PR_Costs pr_costs, Benefits benefits, LDR_Costs ldr_Costs) {
				
				totalCostOfSurveysOverTimeHorizon = 0_USD;
				totalVolumeOverTimeHorizon = 0_ac_ft;
				for (int yearOut = 1; yearOut <= inputs.TimeHorizon(); yearOut++) {
					/* 1,2,3,4,5,1,2,3,4,5 */ 
					year_t yearPattern = (yearOut % (int)(ldr_Costs.BreakEven_SurveyFrequency())) == 0 ? ldr_Costs.BreakEven_SurveyFrequency() : yearOut % (int)(ldr_Costs.BreakEven_SurveyFrequency());
					/* 10, 30, 50, 70 */
					acre_foot_t VolumeThisYear = ldr_Costs.leakageThisYear[yearPattern()] * 1_yr;
					scalar_t discounting = 1.0 / std::pow(1.0+inputs.DiscountRate(), yearOut);
					Dollar_t CostOfSurvey = (ldr_Costs.LeakDetectionSurveyCost) * discounting * (1.0 / ldr_Costs.BreakEven_SurveyFrequency());

					totalCostOfSurveysOverTimeHorizon += CostOfSurvey;
					totalVolumeOverTimeHorizon += VolumeThisYear;
				}

			};
			Dollar_t totalCostOfSurveysOverTimeHorizon;
			acre_foot_t totalVolumeOverTimeHorizon;


		};

		Inputs inputs;
		PR_Costs pr_costs;
		Benefits benefits;
		LDR_Costs ldr_Costs;
		TimeHorizon timeHorizon;

		LeakageModel() : inputs(), pr_costs(), ldr_Costs(), benefits(), timeHorizon() {
			Recalculate();
		};

		void Recalculate() {
			pr_costs = PR_Costs(inputs);
			ldr_Costs = LDR_Costs(inputs);
			benefits = Benefits(inputs);
			timeHorizon = TimeHorizon(inputs, pr_costs, benefits, ldr_Costs);


		};

	};

	class LeakageModel2 {
	public:
		mile_t TotalMileage = 852.95_mi;
		Dollar_per_gallon_t VariableProductionCost = 396.1903737_USD / 1_ac_ft;
		scalar_t NumberOfConnections = 63665;
		
		// only under under one period, if do nothing, init = initial leakge rate
		static constexpr gallon_t Vol_Lost_Per_s(year_t s, scalar_t gamma, gallon_per_year_t R1, gallon_per_year_t init) {
			// RG: Units don't match in the first component, so have to force conversion! It better be perfect...
			return gallon_t((gamma * R1 * s * s / 2.0)()) + (init * s); //
		};

		// all of the small losses added together
		static constexpr gallon_t Vol_Lost_Total(year_t s, year_t T, gallon_t vol_s) {
			return (T / s) * vol_s;
		};

		static constexpr gallon_t Vol_Saved(year_t s, scalar_t gamma, gallon_per_year_t R1, year_t T, gallon_per_year_t init) {
			return Vol_Lost_Per_s(T, 1, R1, init/*=0*/) - Vol_Lost_Total(s, T, Vol_Lost_Per_s(s, gamma, R1, init/*=0*/)); // # assume the straight line, gamma = 1 for do nothing, init is not normally zero, just testing
		};

		static constexpr Dollar_t LDR_Survey_Repair_Cost(
			year_t s, gallon_per_year_t R1, gallon_per_year_t L, Dollar_t Calr, scalar_t Dldr, Dollar_per_mile_t M, mile_t d
		) {
			return ((((R1 * s) / L) * (Calr / Dldr)) / 1_yr) + (M * d); // dividing by 1_yr to make the units work
		};

		static constexpr Dollar_t Benefits(
			Dollar_per_gallon_t Cvp, gallon_per_year_t R1, year_t s, scalar_t gamma, gallon_per_year_t init, year_t T, scalar_t discount
		) {
			return Cvp * Vol_Saved(s, gamma, R1, T, init); // NEED TO DISCOUNT THIS STILL!!
		};

		static constexpr Dollar_t PR_Cost(year_t s) { return 0_USD; };

		static Dollar_t TotalCost(
			year_t s, scalar_t gamma, gallon_per_year_t R1,
			Dollar_per_gallon_t Cvp, gallon_per_year_t L, Dollar_t Calr,
			scalar_t Dldr, Dollar_per_mile_t M, mile_t d, scalar_t discount) 
		{
			AUTO squared_years_type = year_t(2) * year_t(2);
			AUTO TwoSquareYears = decltype(squared_years_type)(2);

			AUTO A_p1 = gamma * R1 * Cvp; // dollars per year
			AUTO A_p2 = (units::math::exp(scalar_t((-discount * s)())) * (-discount * s * (discount * s + 2.0_yr) - TwoSquareYears) + TwoSquareYears); // years squared
			AUTO A_p3 = (2.0 * discount * discount * discount); // unitless

			// These units don't work (Dollar*year is the final unit), and so I am forcing the conversion -- it better be perfect!
			Dollar_t A = (A_p1 * A_p2 / A_p3)(); // wolfram alpha integration yields this equation
			Dollar_t B = units::math::exp(scalar_t((-discount * s)())) * LDR_Survey_Repair_Cost(s, R1, L, Calr, Dldr, M, d);
			Dollar_t C = PR_Cost(s);
			scalar_t D = 1.0 - units::math::exp(scalar_t((-discount * s)()));
			return (A + B + C) / D;
		}

		scalar_t	discount = 0.04;
		year_t		T = 30_yr;
		Dollar_per_mile_t M = 605_USD / 1_mi; // M = 353.29 ; // cost per mile of mains to survey ; // AMANDA's PAPER USED 605 instead of data given... a standared value? why?
		gallon_per_year_t L = 500000.00_gal / 1_yr; // flowrate per leak average(gal / year)
		Dollar_t Calr = 5340.7_USD; // cost per leak to repair
		scalar_t Dldr = 1; // 0.70?  // true positive rate for leak detection, cannot find for Marin in Amanda's data
		gallon_per_year_t init = 27094354.62_gal / 1_yr; // initial leakage rate, gallons / yr

		// initial rate of rise
		gallon_per_year_t R1 = gallon_t(NumberOfConnections() * 4.0) / 1_d; //  92950900.0; // this is 4 * 365 the number of service connections, so 4 gal per day per connection, ? ? ? ? ? ? ? ?

		
		
		Dollar_t NetBenefits(year_t surveyFreq, scalar_t ROR_gamma) {
			Dollar_t Bs = Benefits(VariableProductionCost, R1, surveyFreq, ROR_gamma, init, T, discount); // gamma = 1 for do nothing
			Dollar_t CT = TotalCost(surveyFreq, ROR_gamma, R1, VariableProductionCost, L, Calr, Dldr, M, TotalMileage, discount);
			return Bs - CT;
		};

		template<typename xtype, typename ytype, bool findMinimum>
		static xtype BinarySearch(xtype min, xtype max, std::function<ytype(xtype)> func) {
			xtype toReturn;
			ytype bestV = std::numeric_limits<ytype>::max();
			ytype ActualBest = std::numeric_limits<ytype>::max();
			xtype offset;
			bool res;

			xtype left = min;
			xtype right = max;
			xtype len = right - left;
			xtype mid1 = left + len / 3.0;
			xtype mid2 = right - len / 3.0;

			if (len <= xtype(0)) return min;

			ytype leftV = func(left);
			ytype rightV = func(right);
			ytype mid1V = func(mid1);
			ytype mid2V = func(mid2);
			int numFailures = 0;

			for (int i = 0; i < 100; i++) {
				bool isNewBest = false;
				if constexpr (findMinimum) {
					bestV = std::min({ leftV, mid1V, mid2V, rightV }, std::less<ytype>());
					if (bestV < ActualBest) { isNewBest = true; }
				}
				else {
					bestV = std::max({ leftV, mid1V, mid2V, rightV }, std::greater<ytype>());
					if (bestV > ActualBest) { isNewBest = true; }
				}

				if (bestV == leftV) {
					right = mid1;
					rightV = mid1V;
					if (isNewBest) toReturn = left;
				}
				else if (bestV == mid1V) {
					right = mid2;
					rightV = mid2V;
					if (isNewBest) toReturn = mid1;
				}
				else if (bestV == mid2V) {
					left = mid1;
					leftV = mid1V;
					if (isNewBest) toReturn = mid2;
				}
				else if (bestV == rightV) {
					left = mid2;
					leftV = mid2V;
					if (isNewBest) toReturn = right;
				}
				
				if (!isNewBest) {
					numFailures++;
				}
				else {
					numFailures = 0;
				}

				if (numFailures > 100) break;

				len = right - left;
				mid1 = left + len / 3.0;
				mid2 = right - len / 3.0;
				mid1V = func(mid1);
				mid2V = func(mid2);
			}

			return toReturn;
		};

		year_t BestSurveyFrequency(scalar_t ROR_gamma) {
			year_t bestSurveyFrequency = LeakageModel2::BinarySearch<year_t, Dollar_t, true>(0_yr, 30_yr, [this, ROR_gamma](year_t t) -> Dollar_t {
				return NetBenefits(t, ROR_gamma);
			});
			return bestSurveyFrequency;
		};


	};
#endif
	class LeakageModel {
	public:
		typedef unit<std::ratio<2628000>, seconds> month;
		typedef unit_t <month> month_t;
		typedef unit_t< compound_unit<million_gallon, squared<inverse<month>>> > rateOfRise_t;
		typedef unit_t< compound_unit<million_gallon, inverse<month>> > million_gallon_per_month_t;

		static constexpr million_gallon_t Vol_Lost_At_t(month_t t, scalar_t gamma, rateOfRise_t R, million_gallon_per_month_t init, scalar_t eta) { //only under under one period, if do nothing, init = initial leakge rate
			return (gamma * R * t * t / 2.0) + (eta * init * t); // eta is zero for the case where the first surveyand repair eliminates all leaks
		};
		template <int months, typename T> static constexpr T Get_F_Given_P(T input, scalar_t rate) {
			return input * units::math::cpow<months>(1.0 + rate);
		};
		template <typename T> static T Get_F_Given_P(T input, scalar_t rate, month_t time) {
			return input * std::pow(1.0 + rate, time());
		};
		template <int months, typename T> static T  Get_P_Given_F(T input, scalar_t rate) {
			return input * units::math::pow<-1 * months>(1.0 + rate);
		};
		template <typename T> static T Get_P_Given_F(T input, scalar_t rate, month_t time) {
			return input * std::pow(1.0 + rate(), -1 * time());
		};

		static Dollar_t CostOfPRV(inch_t diam, scalar_t rate) {
			return Get_F_Given_P<1 * 12>(
				1474.8_USD * std::exp(0.2094 * diam()) 
				+ 500_USD * std::exp(0.167 * diam())
				+ (255.36_USD * diam / 1_in - 300_USD)
				+ (1047_USD * diam / 1_in - 512_USD)
				, rate
			);
		};

		class ValveDefinition {
		public:
			kilowatt_t E_Production_Weekly_Summer = 0;
			kilowatt_t E_Production_Weekly_Winter = 0;
			inch_t dia = 6_cm;
			bool IsPRV() const {
				return E_Production_Weekly_Summer <= 0_kW && E_Production_Weekly_Winter <= 0_kW;
			};
			cubic_meter_per_second_t Qbep = 1; // # average, m3 / s best efficiency point = bep
			meter_t Hbep = 1; // # average m
		};

		/* Where pressure reduction will be identical, does the energy generation outpace the increased costs? */
		static bool IsPrvMoreEffective(ValveDefinition const& valve) {
			constexpr scalar_t N_PRV = 20.0;
			constexpr scalar_t w_prv = 0.025 / 12.0;
			constexpr scalar_t w_pat = 0.025 / 12;
			constexpr scalar_t N_PAT = 20.0;
			constexpr scalar_t w_E = 0.025 / 12;
			constexpr Dollar_per_kilowatt_hour_t Ce = 0.12; // $ / KWh

			Dollar_t PRV_Install_Cost; {
				AUTO C_PRV = CostOfPRV(valve.dia, w_prv);
				PRV_Install_Cost = N_PRV * C_PRV;
			}
			Dollar_t PRV_OM_Cost; {
				PRV_OM_Cost = 0; // assumed 0;
			}

			Dollar_t ERT_Install_Cost = 0;
			Dollar_t ERT_OM_Cost = 0;
			{
				AUTO C_PAT = Get_F_Given_P<4 * 12>(11913.91_USD * valve.Qbep() * std::sqrt(valve.Hbep()), w_pat); // # average, inflated from 2019 to 2023 dollars
				C_PAT += (1.0 - 0.26) * C_PAT / 0.26;
				ERT_Install_Cost = N_PAT * C_PAT;

				for (int i = 0; i <= 12*30; i++) {
					ERT_OM_Cost += Get_F_Given_P(C_PAT * (0.15 / 12), w_pat, i);
				};
			}

			Dollar_t ERT_Energy_Production_Benefits = 0;
			{
				kilowatt_hour_t E_Production_Monthly = ((valve.E_Production_Weekly_Summer + valve.E_Production_Weekly_Winter) / 2.0) * month_t(1);
				for (int i = 0; i <= 12 * 30; i++) {
					ERT_Energy_Production_Benefits += E_Production_Monthly * Get_F_Given_P(Ce, w_E, i);
				};
			}

			if ((ERT_Install_Cost + ERT_OM_Cost - ERT_Energy_Production_Benefits) > (PRV_Install_Cost + PRV_OM_Cost)) {
				return true;
			}
			else {
				return false;
			}
		};

		static std::map<std::string, cweeUnitValues::unit_value> LeakModelResults(
			cweeList< ValveDefinition > valves,
			mile_t MilesMains,
			year_t surveyFrequency,
			pounds_per_square_inch_t initialPressure, 
			pounds_per_square_inch_t newPressure, 
			scalar_t ICF = 1.3, // # infrastructure condition factor
			scalar_t ILI = 1.1 // # infrastructure leakage index
		) {
			// internal 
			constexpr scalar_t unitScalar = 1;

			// CONSTANTS, INDEPENDANT OF SYSTEM
			constexpr scalar_t dis = 0.04 / 12.0; // discount rate
			constexpr scalar_t g = 0.025 / 12.0; //growth rate
			constexpr scalar_t inflation = 0.025 / 12.0;
			constexpr scalar_t leak_reduction_eta = 0; // 0 ONLY WE ASSUME ALL LEAKS FIXED AT TIME 0 SO NO LEAKAGE REMAINING
			// need to know if we are counting the initial leak reductionand repair as benefits, eta is a variable that gives the fraction of the leakage(gal / yr) remaining after initial survey / repair
			constexpr scalar_t do_nothing_eta = 1.0; // set to 1 to include the benefits of the initial survey and repair over the study period
			constexpr scalar_t w_prv = 0.025 / 12.0;
			constexpr scalar_t N_PRV = 20.0;
			constexpr scalar_t w_pat = 0.025 / 12;
			constexpr scalar_t N_PAT = 20.0;
			constexpr scalar_t w_E = 0.025 / 12;
			constexpr Dollar_per_kilowatt_hour_t Ce = 0.12; // $ / KWh
			constexpr Dollar_per_gallon_t Cvp = 0.001215862; // $ / gallon
			constexpr million_gallon_per_month_t connectionLeakRate = 4_gpd;
			constexpr AUTO LeakRatePerMile = connectionLeakRate * ((92950900.0 / 4.0) / 365.0) / 852.95_mi;
			constexpr scalar_t PercRigid = 0.75;
			constexpr scalar_t w_F = 0.025 / 12.0;
			constexpr scalar_t w_M = 0.025 / 12.0;
			constexpr scalar_t w_su = 0.025 / 12.0;
			constexpr million_gallon_per_month_t  L = 500000.00_gal / 1_yr; // flowrate per leak average(gal / year) / months per year = gal / month
			constexpr Dollar_t 	Calr = 5340.7; // cost per leak to repair
			constexpr scalar_t 	Dldr = 1; // true positive rate for leak detection, cannot find for Marin in Amanda's data
			constexpr Dollar_per_mile_t M = Get_F_Given_P<1 * 12>(605_USD_p_mi, w_M);
			constexpr Dollar_t F = Get_F_Given_P<1 * 12>(Calr / Dldr, w_F); // adjust for one year, 2022 publication

			// SYSTEM CHARACTERISTICS / ANALYSIS VARIABLES
			month_t s = surveyFrequency;
			pounds_per_square_inch_t P0 = initialPressure; // # initial pressure
			pounds_per_square_inch_t P1 = newPressure; // # pressure after management
			mile_t 	d = MilesMains; // miles of mains

			// months
			cweeList<month_t> t;  for (int i = 0; i <= 12 * 30; i++) t.Append((month_t)(i));

			// capital costs of PRV's
			cweeList<Dollar_t> Ci_PRV_T; Ci_PRV_T.AssureSize(t.size(), 0_USD);
			// operations and maintenance cost of PRVs
			cweeList<Dollar_t> OM_PRV; OM_PRV.AssureSize(t.size(), 0_USD); // assumed negligable
			for (auto& valve : valves) {
				if (valve.IsPRV()) {
					AUTO C_PRV = CostOfPRV(valve.dia, w_prv);
					Ci_PRV_T[0] += N_PRV * C_PRV;
				}
			}

			// capital costs of PAT and generators
			cweeList<Dollar_t> Ci_PAT; Ci_PAT.AssureSize(t.size(), 0_USD);
			// ## operations and maintenance cost of PATs + generators
			cweeList<Dollar_t> OM_PAT; OM_PAT.AssureSize(t.size(), 0_USD);
			for (auto& valve : valves) {
				if (!valve.IsPRV()) {
					AUTO C_PAT = Get_F_Given_P<4 * 12>(11913.91_USD * valve.Qbep() * std::sqrt(valve.Hbep()), w_pat); // # average, inflated from 2019 to 2023 dollars
					C_PAT += (1.0 - 0.26) * C_PAT / 0.26; 

					AUTO OM_PAT_o = (0.15 / 12) * C_PAT; //  # 15 % annually of other costs, already made to be 2023 dollars

					for (int i = 0; i < OM_PAT.size(); i++) {
						OM_PAT[i] += Get_F_Given_P(OM_PAT_o, w_pat, i);
					};

					Ci_PAT[0] += N_PAT * C_PAT; 
				}
			}
			OM_PAT[0] = 0_USD;

			// ## capital costs of pressure reduction
			cweeList<Dollar_t> Ci = Ci_PRV_T; for (int i = 0; i < Ci_PAT.size(); i++) { Ci[i] += Ci_PAT[i]; };

			// ## operations and maintenance cost of pressure reduction
			cweeList<Dollar_t> OM = OM_PRV; for (int i = 0; i < OM_PAT.size(); i++) { OM[i] += OM_PAT[i]; };

			// energy production			
			cweeList<Dollar_t> EN; EN.AssureSize(t.size(), 0_USD);
			for (auto& valve : valves) {
				if (!valve.IsPRV()) {
					kilowatt_hour_t E_Production_Monthly = ((valve.E_Production_Weekly_Summer + valve.E_Production_Weekly_Winter) / 2.0) * month_t(1);
					for (int i = 0; i < EN.size(); i++) {
						EN[i] += E_Production_Monthly * Get_F_Given_P(Ce, w_E, i);
					};
				}
			}
			EN[0] = 0_USD;

			// define the rate of rise of leakage
			rateOfRise_t R = LeakRatePerMile * MilesMains / 365_d;
			scalar_t N1 = 1.5 - (1.0 - (2.0 / 3.0) * (ICF / ILI)) * PercRigid;
			scalar_t gamma = std::pow((P1 / P0)(), N1()); // # fraction leakage rate remaining after pressure reduction, about 0.6 right now
			million_gallon_per_month_t init = R * 2_yr; // # initial leakage rate, gallons / yr

			// calculate water lost volume and value
			cweeList<million_gallon_t> Water_Lost_Volume; Water_Lost_Volume.AssureSize(t.size(), 0_MG);
			cweeList<Dollar_t> Water_Lost_Value; Water_Lost_Value.AssureSize(t.size(), 0_USD); {
				month_t idx_i = 0;
				bool firstPass = true;
				month_t zeroMonth = 0;
				for (month_t idx = 0; idx < ((month_t)(t.size())); idx++) {
					if (idx == zeroMonth || units::math::round(units::math::fmod(idx, s)) == zeroMonth) {
						idx_i = zeroMonth;
						if (firstPass) {
							Water_Lost_Volume[idx()] = 0;
							firstPass = false;
						}
						else {
							Water_Lost_Volume[idx()] = 
								Vol_Lost_At_t(s, gamma, R, init, leak_reduction_eta) - 
								Vol_Lost_At_t(s - month_t(1), gamma, R, init, leak_reduction_eta);
						}
					}
					else {
						AUTO vol0 = Vol_Lost_At_t(
							t[idx_i()],
							gamma,
							R,
							init,
							leak_reduction_eta
						);
						AUTO vol1 = Vol_Lost_At_t(
							t[(idx_i - month_t(1))()],
							gamma,
							R,
							init,
							leak_reduction_eta
						);
						Water_Lost_Volume[idx()] =  vol0 - vol1;
					}
					idx_i += month_t(1);
				}
				for (month_t idx = 0; idx < ((month_t)(t.size())); idx++) {
					Water_Lost_Value[idx()] = Water_Lost_Volume[idx()] * Get_F_Given_P(Cvp, g, t[idx()]);
				}
			}

			// calculate the water lost under the do-nothing scenario
			cweeList<million_gallon_t> Water_Lost_Do_Nothing; Water_Lost_Do_Nothing.AssureSize(t.size(), 0_MG); {
				month_t idx_i = 0;
				bool firstPass = true;
				month_t zeroMonth = 0;
				for (month_t idx = 0; idx < ((month_t)(t.size())); idx++) {
					if (firstPass) {
						Water_Lost_Do_Nothing[idx()] = 0;
						firstPass = false;
					}
					else {
						Water_Lost_Do_Nothing[idx()] =
							Vol_Lost_At_t(t[idx()], 1, R, init, do_nothing_eta) -
							Vol_Lost_At_t(t[idx() - 1], 1, R, init, do_nothing_eta);
					}
				}
			}

			// calculate the water saved volume and value for each time period
			cweeList<million_gallon_t> Water_Saved_Volume; Water_Saved_Volume.AssureSize(t.size(), 0_MG); {
				for (int i = 0; i < Water_Saved_Volume.size(); i++) {
					Water_Saved_Volume[i] = Water_Lost_Do_Nothing[i] - Water_Lost_Volume[i];
				}
			}
			cweeList<Dollar_t> Water_Saved_Value; Water_Saved_Value.AssureSize(t.size(), 0_USD); {
				month_t idx_i = 0;
				for (month_t idx = 0; idx < ((month_t)(t.size())); idx++) {
					Water_Saved_Value[idx()] = Water_Saved_Volume[idx()] * Get_F_Given_P(Cvp, g, t[idx()]);
				}
			}

			// calculate costs of surveying over time period
			Dollar_t 	D = d * M; // D full leak detection survey cost

			// surveys after initial
			Dollar_t Cs_0 = (R * s / L) * F + D; // cost of survey in year 0 dollars
			cweeList<Dollar_t> SurveyRep; SurveyRep.AssureSize(t.size(), 0_USD); {
				month_t idx_i = 0;
				month_t zeroMonth = 0;
				for (month_t idx = 0; idx < ((month_t)(t.size())); idx++) {
					if (units::math::fmod(idx, s) == zeroMonth) {
						SurveyRep[idx()] = Get_F_Given_P(Cs_0, w_su, t[idx()]);
					}else{ // only take the periods the surveys are actually done
						SurveyRep[idx()] = 0_USD;
					}
				}
			}

			// initial survey
			
			if (do_nothing_eta >= unitScalar) {
				SurveyRep[0] = (init / L) * F + D; // Initial survey cost
			}
			else {
				SurveyRep[0] = 0_USD;
			}

			// calculate net present values, smarter ways to do this, not worrying about it now
			Dollar_t NPV_costs = 0;
			{ 
				for (auto& x : Ci) NPV_costs += x;				
			}
			{ 
				for (month_t idx = 0; idx < ((month_t)(t.size())); idx++) {
					try {
						NPV_costs += Get_P_Given_F(OM[idx()], dis, t[idx()]);
						NPV_costs += Get_P_Given_F(SurveyRep[idx()], dis, t[idx()]);
						NPV_costs += Get_P_Given_F(Water_Lost_Value[idx()], dis, t[idx()]);
					}catch(...){}
				}
			}

			Dollar_t NPV_benefits = 0;
			{
				month_t idx_i = 0;
				for (month_t idx = 0; idx < ((month_t)(t.size())); idx++) {
					NPV_benefits += Get_P_Given_F(EN[idx()], dis, t[idx()]);
					NPV_benefits += Get_P_Given_F(Water_Saved_Value[idx()], dis, t[idx()]);
				}
			}

			std::map<std::string, cweeUnitValues::unit_value> results;
			{
				results["Total Costs"] = cweeUnitValues::Dollar(NPV_costs);
				results["Total Benefits"] = cweeUnitValues::Dollar(NPV_benefits);
				results["Net Benefits"] = cweeUnitValues::Dollar(NPV_benefits - NPV_costs);
			}

			return results;
		};


		template<typename xtype, typename ytype, bool findMinimum>
		static xtype BinarySearch(xtype min, xtype max, std::function<ytype(xtype)> func) {
			xtype toReturn;
			ytype bestV = std::numeric_limits<ytype>::max();
			ytype ActualBest = std::numeric_limits<ytype>::max();
			xtype offset;
			bool res;

			xtype left = min;
			xtype right = max;
			xtype len = right - left;
			xtype mid1 = left + len / 3.0;
			xtype mid2 = right - len / 3.0;

			if (len <= xtype(0)) return min;

			ytype leftV = func(left);
			ytype rightV = func(right);
			ytype mid1V = func(mid1);
			ytype mid2V = func(mid2);
			int numFailures = 0;

			for (int i = 0; i < 100; i++) {
				bool isNewBest = false;
				if constexpr (findMinimum) {
					bestV = std::min({ leftV, mid1V, mid2V, rightV }, std::less<ytype>());
					if (bestV < ActualBest) { isNewBest = true; }
				}
				else {
					bestV = std::max({ leftV, mid1V, mid2V, rightV }, std::greater<ytype>());
					if (bestV > ActualBest) { isNewBest = true; }
				}

				if (bestV == leftV) {
					right = mid1;
					rightV = mid1V;
					if (isNewBest) toReturn = left;
				}
				else if (bestV == mid1V) {
					right = mid2;
					rightV = mid2V;
					if (isNewBest) toReturn = mid1;
				}
				else if (bestV == mid2V) {
					left = mid1;
					leftV = mid1V;
					if (isNewBest) toReturn = mid2;
				}
				else if (bestV == rightV) {
					left = mid2;
					leftV = mid2V;
					if (isNewBest) toReturn = right;
				}

				if (!isNewBest) {
					numFailures++;
				}
				else {
					numFailures = 0;
				}

				if (numFailures > 100) break;

				len = right - left;
				mid1 = left + len / 3.0;
				mid2 = right - len / 3.0;
				mid1V = func(mid1);
				mid2V = func(mid2);
			}

			return (mid1 + mid2) / 2.0;
		};

		static year_t IdealSurveyFrequency(
			cweeList< ValveDefinition > valves,
			mile_t MilesEvaluated,
			pounds_per_square_inch_t initialPressure,
			pounds_per_square_inch_t newPressure,
			scalar_t ICF = 1.3, // # infrastructure condition factor
			scalar_t ILI = 1.1 // # infrastructure leakage index
		) {
			year_t bestSurveyFrequency = BinarySearch<year_t, cweeUnitValues::Dollar, true>(1_yr, 30_yr, [=](year_t t) -> cweeUnitValues::Dollar {
				AUTO res = LeakModelResults(valves, MilesEvaluated, t, initialPressure, newPressure, ICF, ILI);
				return res["Total Costs"] - res["Total Benefits"];
			});
			return bestSurveyFrequency;
		};

	};


};