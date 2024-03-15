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
#include "DelayedInstantiation.h"
#include "Strings.h"
#include "List.h"
#include "Mutex.h"
#include "enum.h"

#include <random>
class cwee_rand {
public:
	class cwee_pcg {
	public:
		using result_type = uint32_t;
		static constexpr result_type(min)() { return 0; }
		static constexpr result_type(max)() { return UINT32_MAX; }

		cwee_pcg() noexcept : m_state(0), m_inc(0), rd() { seed(); };
		void seed() noexcept {
			uint64_t s0 = uint64_t(rd()) << 31 | uint64_t(rd());
			uint64_t s1 = uint64_t(rd()) << 31 | uint64_t(rd());
			m_state = 0;
			m_inc = (s1 << 1) | 1;
			(void)operator()();
			m_state += s0;
			(void)operator()();
		};
		result_type operator()() const noexcept {
			uint64_t oldstate = m_state.load();
			m_state.store(oldstate * 6364136223846793005ULL + m_inc);
			uint32_t xorshifted = uint32_t(((oldstate >> 18u) ^ oldstate) >> 27u);
			int rot = oldstate >> 59u;
			return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
		};
		void discard(unsigned long long n) const noexcept { unsigned long long i; i = 0;  for (; i < n; ++i) operator()(); };

	private:
		mutable std::atomic_int64_t m_state;
		uint64_t m_inc;
		std::random_device rd;
	};

private:
	mutable cwee_pcg rand;
	mutable std::uniform_real_distribution<u64> u;

public:
	cwee_rand() noexcept : rand(), u(0.0, 1.0) { Random_Impl(); /*Instantiate the range*/ };
	u64 Random(u64 t1 = 0.0, u64 t2 = 1.0) const noexcept { return Random_HighRes(std::move(t1), std::move(t2)); };
	double Random(double t1 = 0.0, double t2 = 1.0) const noexcept { return Random_HighRes(t1, t2); };
	float Random(float t1 = 0.0, float t2 = 1.0) const noexcept { return Random_HighRes(t1, t2); };
	int Random(int t1 = 0, int t2 = std::numeric_limits<int>::max()) const noexcept { return std::floor(Random_HighRes(t1, t2) + 0.5); };

private:
	u64 Random_Impl() const noexcept {
		return u(rand);
	};
	u64 Random_HighRes(u64 t1, u64 t2) const noexcept {
		t2 -= t1;
		t2 *= Random_Impl();
		t1 += t2;
		return t1;
	};
};
static DelayedInstantiation< cwee_rand > sharedCweeRandomGenerator = DelayedInstantiation< cwee_rand >( []()-> cwee_rand* { return new cwee_rand(); } );

/*! random float between 0 and 1 */ INLINE float cweeRandomFloat() { return sharedCweeRandomGenerator->Random(0.0f, 1.0f); };
/*! random float between 0 and max */ INLINE float cweeRandomFloat(float max) { return sharedCweeRandomGenerator->Random(0.0f, max); };
/*! random float between min and max */ INLINE float cweeRandomFloat(float min, float max) { return sharedCweeRandomGenerator->Random(min, max); };
/*! random int between 0 and cweeMath::INF */ INLINE int cweeRandomInt() { return sharedCweeRandomGenerator->Random(0, std::numeric_limits<int>::max()); };
/*! random int between 0 and max */ INLINE int cweeRandomInt(int max) { return sharedCweeRandomGenerator->Random(0, max); };
/*! random int between min and max */ INLINE int cweeRandomInt(int min, int max) { return sharedCweeRandomGenerator->Random(min, max); };

/*! cweeMath is a collection of various math functions for use throughout the EDMS */
class cweeMath {
public:
	static float					RSqrtFast(float x) {
		if (x != 0) {
			long i;
			float y, r;
			y = x * 0.5f;
			i = *reinterpret_cast<long*>(&x);
			i = 0x5f3759df - (i >> 1);
			r = *reinterpret_cast<float*>(&i);
			r = r * (1.5f - r * r * y);
			return r;
		}
		else {
			return 1e30f;
		}
	};
	static float					RSqrt(float x) {
		return InvSqrt(x);

		//if (x != 0) {
		//	long i;
		//	float y, r;
		//	y = x * 0.5f;
		//	i = *reinterpret_cast<long*>(&x);
		//	i = 0x5f3759df - (i >> 1);
		//	r = *reinterpret_cast<float*>(&i);
		//	r = r * (1.5f - r * r * y);
		//	return r;
		//}
		//else {
		//	return 1e30f;
		//}
	};
	static float					InvSqrt(float x) {
		return 1.0f/::sqrtf(x);

		//dword a = ((union _flint*)(&x))->i;
		//union _flint seed;
		//double y = x * 0.5f;
		//seed.i = ((((3 * EXP_BIAS - 1) - ((a >> EXP_POS) & 0xFF)) >> 1) << EXP_POS) | iSqrt[(a >> (EXP_POS - LOOKUP_BITS)) & LOOKUP_MASK];
		//double r = seed.f;
		//r = r * (1.5f - r * r * y);
		//r = r * (1.5f - r * r * y);
		//return (float)r;
	};
	static float					Sqrt(float x) {
		return ::sqrtf(x);
		// return x * InvSqrt(x);
	};
	static float					Sin(float a) {
		float s;
		if ((a < 0.0f) || (a >= (2.0f * 3.14159265358979323846f))) {
			a -= std::floor(a / (2.0f * 3.14159265358979323846f)) * (2.0f * 3.14159265358979323846f);
		}
		if (a < 3.14159265358979323846f) {
			if (a > (0.5f * 3.14159265358979323846f)) {
				a = 3.14159265358979323846f - a;
			}
		}
		else {
			if (a > 3.14159265358979323846f + (0.5f * 3.14159265358979323846f)) {
				a = a - (2.0f * 3.14159265358979323846f);
			}
			else {
				a = 3.14159265358979323846f - a;
			}
		}
		s = a * a;
		return a * (((((-2.39e-08f * s + 2.7526e-06f) * s - 1.98409e-04f) * s + 8.3333315e-03f) * s - 1.666666664e-01f) * s + 1.0f);
	};

	static double					Pow(double x, double y) {
		AUTO r = std::pow(x, y);
		if (std::isnan(r)) return 0;
		if (std::isinf(r)) return 0;
		return r;
	};	// x raised to the power y with 32 bits precision
	static double					Pow64(float x, float y) {
		return std::pow(x, y);
	};	// x raised to the power y with 64 bits precision

	static int						Rint(float f) {
		return std::floor(f + 0.5f);
	};
	static int						Ceil(float f) {
		return std::floor(f + 1.0f);
	};
	static int						Floor(float f) {
		return std::floor(f);
	};

	//	static int fasterfloor(float x) { return x > 0 ? (int)x : (int)x - 1; };
	//	static int ftoi_sse1(float f)
	//	{
	//		return _mm_cvtt_ss2si(_mm_load_ss(&f));     // SSE1 instructions for float->int
	//	};
	//
	//	static int Floor(float const& f) {
	//#if 1
	//		return ftoi_sse1(f);
	//#else
	//		return std::floorf(f);
	//#endif
	//	};

	static int						Abs(int x) {
		return abs(x);
	};				// returns the absolute value of the integer value (for reference only)
	static float					Fabs(float f) {
		return fabsf(f);
	};			// returns the absolute value of the floating point value

	static float					Frac(float f) {
		return f - Floor(f);
	};			// f - Floor( f )

	static float					Fmin(float a, float b) {
		bool left(true);
		(a < b) ? (left = true) : (left = false);
		if (left) return a;
		return b;
	};
	static int						min(int x, int y) {
		bool left(true);
		(x < y) ? (left = true) : (left = false);
		if (left) return x;
		return y;
	};
	static float					Fmax(float a, float b) {
		bool left(true);
		(a > b) ? (left = true) : (left = false);
		if (left) return a;
		return b;
	};
	static int						max(int x, int y) {
		bool left(true);
		(x > y) ? (left = true) : (left = false);
		if (left) return x;
		return y;
	};
	// TEST FUNCTION LIBRARY // 
	static double					SinusoidalFunction(std::vector<double> x_input, double y_weight_1 = 1, double y_weight_2 = 1, double A_weight = 2.5, double B_weight = 5) {

		//	A = 2.5; B = 5; y1 = 1; y2 = 1; 
		// for i = 1:n
		//	y1 = y1 * sin((x(i)) * pi / 180);
		//	y2 = y2 * sin(B * (x(i)) * pi / 180);
		//end
		// y = -A * y1 - y2; 
		double y1_tally = 0, y2_tally = 0;
		for (int index_dimension = 0; index_dimension < (int)x_input.size(); index_dimension++) {
			y1_tally = y1_tally + sin(x_input[index_dimension]) * cweeMath::PI / 180;
			y2_tally = y2_tally * sin(B_weight * x_input[index_dimension]) * cweeMath::PI / 180;
		}
		return (-A_weight * y_weight_1 - y_weight_2);
	};
	static double					RosenbrockObjectiveFunction(std::vector<double> x_input, double q_parameter_input = 100) {

		// case 17 % Rosenbrock problem q=30, f(x*)=0, x*=ones(n,1)
		//	y=0;
		//	for i=1:n-1
		//		y=y+100*(x(i+1)-x(i)^2)^2+(x(i)-1)^2;
		//	end	 
		double output_y = 0;
		for (int index_dimension = 0; index_dimension < (int)(x_input.size() - 1); index_dimension++) {
			output_y = output_y + q_parameter_input * (
				pow(x_input[index_dimension + 1] - pow(x_input[index_dimension], 2), 2) + pow((x_input[index_dimension] - 1), 2));
		}

		return output_y; // return the output
	};
	// END TEST FUNCTION LIBRARY //

	/*
	out = Floor(value*( 1/a ) + 0.5) * a
	*/
	static float					roundNearest(float a, const float magnitude) {
		float factor = RSqrt(magnitude);
		factor *= factor;
		return Floor(a * factor + 0.5) * magnitude;
	};
	/*
	out = Floor(value*( 1/a )) * a
	*/
	static float					roundDownNearest(float a, const float magnitude) {
		float factor = RSqrt(magnitude) * RSqrt(magnitude);
		return Floor(a * factor) * magnitude;
	};
	/*
	Return the rolling average, given the previous average, number of samples, and incoming sample.
	*/
	static float					rollingAverage(float currentAverage, float newSample, int numSamples);
	/*
	Generate the rolling average, given the previous average, number of samples, and incoming sample, as reference instead of return.
	*/
	template<typename T> static void	rollingAverageRef(T& currentAverage, T newSample, int& numSamples) {
		numSamples++;
		currentAverage -= (T)((double)currentAverage / (double)numSamples);
		currentAverage += (T)((double)newSample / (double)numSamples);
	};

	/*!
	Return average of input vector of floats
	*/
	static float					Faverage(std::vector<float> const& input) {
		float out(0);
		int count(0);
		for (auto& x : input) rollingAverageRef(out, x, count);
		return out;
	};

	/*!
	Return average of input vector of integers
	*/
	static float					average(std::vector<int> const& input) {
		float out(0);
		int count(0);
		for (auto& x : input) rollingAverageRef(out, (float)x, count);
		return out;
	};


	static int						medianIndex(int l, int r)
	{
		int n = r - l + 1;
		n = (n + 1) / 2 - 1;
		return n + l;
	};

	template<typename T>
	static inline T Lerp(T v0, T v1, T t)
	{
		return (1.0 - t) * v0 + t * v1;
	}

	template<typename T>
	static inline std::vector<T> Quantile(const std::vector<T>& inData, const std::vector<T>& probs)
	{
		if (inData.empty())
		{
			return std::vector<T>();
		}

		if (1 == inData.size())
		{
			return std::vector<T>(1, inData[0]);
		}

		std::vector<T> data = inData;
		std::sort(data.begin(), data.end());
		std::vector<T> quantiles;

		for (size_t i = 0; i < probs.size(); ++i)
		{
			T poi = Lerp<T>(-0.5, data.size() - 0.5, probs[i]);

			size_t left = std::max(int64_t(std::floor(poi)), int64_t(0));
			size_t right = std::min(int64_t(std::ceil(poi)), int64_t(data.size() - 1));

			T datLeft = data.at(left);
			T datRight = data.at(right);

			T quantile = cweeMath::Lerp<T>(datLeft, datRight, poi - left);

			quantiles.push_back(quantile);
		}

		return quantiles;
	}

	/*!
		Returns (Min, Q1, Median, Q3, Max)
	*/
	static cweeThreadedList<float>	interQuartileRanges(const cweeThreadedList<float>& data) {
		// early exit if there is 0 objects
		if (data.Num() <= 0) return cweeThreadedList<float>({ 0, 0, 0, 0, 0 });

		std::vector<float> d = data;	std::sort(d.begin(), d.end());
		cweeThreadedList<float> out;

		auto quartiles = Quantile<float>(d, { 0.00, 0.25, 0.50, 0.75, 1.00 });

		return quartiles;

		//int medianIndex = cweeMath::medianIndex(0, data.Num());
		//float min, Q1, median, Q3, max;

		//min = (*d.begin());
		//Q1 = d[cweeMath::medianIndex(0, medianIndex)];
		//median = d[medianIndex];
		//Q3 = d[medianIndex + cweeMath::medianIndex(medianIndex + 1, data.Num())];
		//max = (*d.end());

		//out.Append(min);
		//out.Append(Q1);
		//out.Append(median);
		//out.Append(Q3);
		//out.Append(max);

		//return out;
	};

	static double					VectorInnerProductD(const cweeThreadedList<float>& vec1, const cweeThreadedList<float>& vec2) {
		double output = 0;
		if (vec1.Num() != vec2.Num()) {
			return -1;
		}
		for (int index_t = 0; index_t < vec1.Num(); index_t++) {
			output = output + (vec1[index_t] * vec2[index_t]);
		}

		return output;
	};
	static cweeThreadedList<float>		VectorScalarProduct(const double& scalar, const cweeThreadedList<float>& input) {
		cweeThreadedList<float> output;
		for (auto value : input) {
			output.Append(scalar * value);
		}

		return output;
	};
	static std::vector<float>		GenerateRandomVector(int length, double upper_bound, double lower_bound) {

		std::vector<float> return_rnd_vector;
		double new_value;
		for (int index_val = 0; index_val < length; index_val++) {
			double f = cweeRandomFloat(0, 1);
			new_value = f * (upper_bound - lower_bound);
			return_rnd_vector.push_back(new_value);
		}
		return return_rnd_vector;
	};
	static std::vector<double>		CreateZeroVector(int length) {

		std::vector<double> zero_vec;

		for (int index_i = 0; index_i < length; index_i++)
			zero_vec.push_back(0);
		return zero_vec;
	};
	static double					SumOfSquares(std::vector<double> vec1, std::vector<double> vec2) {

		double output = 0;

		for (int index_i = 0; index_i < (int)vec1.size(); index_i++) {
			output = output + (vec1.at(index_i) - vec2.at(index_i)) * (vec1.at(index_i) - vec2.at(index_i));
		}

		return output;
	}; // the sum of squares operation 
	static double					AvgSumOfSquares(std::vector<double> vec1, std::vector<double> vec2) {

		double output = 0;

		for (int index_i = 0; index_i < (int)vec1.size(); index_i++) {
			output = output + (vec1.at(index_i) - vec2.at(index_i)) * (vec1.at(index_i) - vec2.at(index_i));
		}

		output = output / vec1.size(); // compute the average sum of squares value

		return output;
	};
	static std::vector < std::vector <float> >  TransposeNestedVector(const std::vector < std::vector <float> >& input_vec) {
		int numCol = 0;
		for (auto& row : input_vec) numCol = cweeMath::max(numCol, row.size());
		std::vector < std::vector <float> > output; output.reserve(numCol);
		std::vector<float> input_vector;
		long long index_t; long long index_r; long long n = input_vec.size();
		for (index_r = 0; index_r < numCol; index_r++) {// invert the for loop
			if ((long long)input_vector.size() == n) {
				for (index_t = 0; index_t < n; index_t++) { // go through and rearrange the values in the vector
					input_vector[index_t] = input_vec[index_t][index_r];
				}
				output.push_back(input_vector);
			}
			else {
				input_vector.clear(); input_vector.reserve(n);
				for (index_t = 0; index_t < n; index_t++) { // go through and rearrange the values in the vector
					input_vector.push_back(input_vec[index_t][index_r]);
				}
				output.push_back(input_vector);
			}
		}
		return output;
	};
	static int						categorizeThreshold(double input_real_number, std::vector<float> threshold_values) {
		int category = -1;

		for (int index_i = 0; index_i < (long long)threshold_values.size(); index_i++) {
			if (input_real_number < threshold_values.at(index_i)) {
				category = index_i;
				break;
			}
		}

		return category;
	};
	static constexpr double			AngRad = 0.017453292519943299;				// number of radians in a degree
	static constexpr double			G = 9.81f;									// gravity at Earth's surface
	static constexpr double			PI = 314159.265358979323846264338327950288e-05;	// pi
	static constexpr double			TWO_PI = 2.0f * PI;							// pi * 2
	static constexpr double			HALF_PI = 0.5f * PI;						// pi / 2
	static constexpr double			ONEFOURTH_PI = 0.25f * PI;					// pi / 4
	static constexpr double			E = 2.71828182845904523536f;				// e
	static constexpr double			SQRT_TWO = 1.41421356237309504880;			// sqrt( 2 )
	static constexpr double			SQRT_THREE = 1.73205080756887729352;		// sqrt( 3 )
	static constexpr double			SQRT_1OVER2 = 0.70710678118654752440;		// sqrt( 1 / 2 )
	static constexpr double			SQRT_1OVER3 = 0.57735026918962576450;		// sqrt( 1 / 3 )
	static constexpr float			INF = 1e30f;								// huge number which should be larger than any valid number used
	static constexpr float 			EPSILON = 1.192092896e-07f;					// smallest positive number such that 1.0+FLT_EPSILON != 1.0

private:
	enum {
		LOOKUP_BITS = 8,
		EXP_POS = 23,
		EXP_BIAS = 127,
		LOOKUP_POS = (EXP_POS - LOOKUP_BITS),
		SEED_POS = (EXP_POS - 8),
		SQRT_TABLE_SIZE = (2 << LOOKUP_BITS),
		LOOKUP_MASK = (SQRT_TABLE_SIZE - 1)
	};
	union _flint {
		dword					i;
		float					f;
	};
};