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

#include <random>
class cwee_rand {
public:
	class cwee_pcg {
	public:
		using result_type = uint32_t;
		static constexpr result_type(min)() { return 0; }
		static constexpr result_type(max)() { return UINT32_MAX; }

		cwee_pcg() noexcept : m_state(0), m_inc(0), rd() { seed(); };
		void seed() const noexcept {
			uint64_t s0 = uint64_t(rd()) << 31 | uint64_t(rd());
			uint64_t s1 = uint64_t(rd()) << 31 | uint64_t(rd());
			m_state = 0;
			m_inc = (s1 << 1) | 1;
			(void)operator()();
			m_state += s0;
			(void)operator()();
		};
		result_type operator()() const noexcept {
			uint64_t oldstate = m_state;
			m_state = oldstate * 6364136223846793005ULL + m_inc;
			uint32_t xorshifted = uint32_t(((oldstate >> 18u) ^ oldstate) >> 27u);
			int rot = oldstate >> 59u;
			return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
		};
		void discard(unsigned long long n) const noexcept { unsigned long long i; i = 0;  for (; i < n; ++i) operator()(); };
	private:
		mutable uint64_t m_state;
		mutable uint64_t m_inc;
		mutable std::random_device rd;
	};
private:
	mutable cweeSysMutex mut;
	mutable cwee_pcg rand;
	mutable std::uniform_real_distribution<u64> u;
public:
	cwee_rand() noexcept : mut(), rand(), u(0.0, 1.0) { Random_Impl(); /*Instantiate the range*/ };
	u64 Random(u64 t1 = 0.0, u64 t2 = 1.0) const noexcept { return Random_HighRes(std::move(t1), std::move(t2)); };
	double Random(double t1 = 0.0, double t2 = 1.0) const noexcept { return Random_HighRes(t1, t2); };
	float Random(float t1 = 0.0, float t2 = 1.0) const noexcept { return Random_HighRes(t1, t2); };
	int Random(int t1 = 0, int t2 = std::numeric_limits<int>::max()) const noexcept { return std::floor(Random_HighRes(t1, t2) + 0.5); };
private:
	u64 Random_Impl() const noexcept {
		u64 out(0);
		mut.Lock();
		out = u(rand);
		mut.Unlock();
		return std::move(out);
	};
	u64 Random_HighRes(u64 t1, u64 t2) const noexcept {
		t2 -= t1;
		t2 *= Random_Impl();
		t1 += t2;
		return t1;
	};
};
static DelayedInstantiation< cwee_rand > sharedCweeRandomGenerator;

/*! random float between 0 and 1 */ INLINE float cweeRandomFloat() { return sharedCweeRandomGenerator->Random(0.0f, 1.0f); };
/*! random float between 0 and max */ INLINE float cweeRandomFloat(float max) { return sharedCweeRandomGenerator->Random(0.0f, max); };
/*! random float between min and max */ INLINE float cweeRandomFloat(float min, float max) { return sharedCweeRandomGenerator->Random(min, max); };
/*! random int between 0 and cweeMath::INF */ INLINE int cweeRandomInt() { return sharedCweeRandomGenerator->Random(0, std::numeric_limits<int>::max()); };
/*! random int between 0 and max */ INLINE int cweeRandomInt(int max) { return sharedCweeRandomGenerator->Random(0, max); };
/*! random int between min and max */ INLINE int cweeRandomInt(int min, int max) { return sharedCweeRandomGenerator->Random(min, max); };

/*! cweeMath is a collection of various math functions for use throughout the EDMS */
class cweeMath {
public:
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
			a -= std::floorf(a / (2.0f * 3.14159265358979323846f)) * (2.0f * 3.14159265358979323846f);
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

	static float					Pow(float x, float y) {
		return powf(x, y);
	};	// x raised to the power y with 32 bits precision
	static double					Pow64(float x, float y) {
		return pow(x, y);
	};	// x raised to the power y with 64 bits precision

	static int						Rint(float f) {
		return std::floorf(f + 0.5f);
	};
	static int						Ceil(float f) {
		return std::floorf(f + 1.0f);
	};
	static int						Floor(float f) {
		return std::floorf(f);
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
		for (int index_dimension = 0; index_dimension < x_input.size(); index_dimension++) {
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
		for (int index_dimension = 0; index_dimension < (x_input.size() - 1); index_dimension++) {
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

		for (int index_i = 0; index_i < vec1.size(); index_i++) {
			output = output + (vec1.at(index_i) - vec2.at(index_i)) * (vec1.at(index_i) - vec2.at(index_i));
		}

		return output;
	}; // the sum of squares operation 
	static double					AvgSumOfSquares(std::vector<double> vec1, std::vector<double> vec2) {

		double output = 0;

		for (int index_i = 0; index_i < vec1.size(); index_i++) {
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
		int index_t; int index_r; int n = input_vec.size();
		for (index_r = 0; index_r < numCol; index_r++) {// invert the for loop
			if (input_vector.size() == n) {
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

		for (int index_i = 0; index_i < threshold_values.size(); index_i++) {
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

	struct Optim {

		enum class OptimType {				// a binding to each of the different optimization types, base:: OptimInput // 
			OPT_NONE,
			OPT_RANDOM_SEARCH_DIVERSE,		// object RandomSearchInput//
			OPT_RANDOM_SEARCH_NORMAL,		// object RandomSearchInput//
			OPT_NELDER_MEADE,				// object NelderMeadUpdate //
			OPT_PSO,						// object PSOInput_Update // 
			OPT_ESS,						// object essInput //
			OPT_CMA_ES,						// object CMAESInput //
			OPT_GENETIC,					// object GeneticInput_Update //
			OPT_RANDOM,						// object RandomSearchInput //
			OPT_HOOKES,						// object HJInput_Update //
			OPT_HYBRID						// object HybridOptimType //
		};
		/* Helper Functions */
#pragma region OPTIM_HELPER_FUNCTIONS
		/* enumerated types for non-random, random variables */
		enum non_random_type_t { // THIS DIRECTS DETERMINISTIC FUNCTIONS TO RETURN 
			__NONE_NON_RANDOM__ = 0,
			__LOW_LIMIT__ = BIT(1),
			__HIGH_LIMIT__ = BIT(2),
			__MIDDLE_MEDIAN__ = BIT(3),
			__ONE_QUARTILE_UP__ = BIT(4),
			__ONE_QUARTILE_DOWN__ = BIT(5)
		};
		/* get the mean of a set of inputted points  */
		cweeThreadedList <float> static get_mean_from_vector_set(cweeThreadedList < cweeThreadedList <float>> input_points) {

			/* test to to make sure that there is a non-empty input vector */
			if (input_points.Num() < 1) {
				return vector_repeated_scalar_values(input_points.Num(), 0);  // return the input points with no changes 
			}
			else { /* if this vector is not empty than we can compute the value */
				cweeThreadedList <float> output_vector = vector_repeated_scalar_values(input_points.Num(), 0);
				for (int outer_vector_index = 0; outer_vector_index < input_points.Num(); outer_vector_index++) {
					for (int inner_vector_index = 0; inner_vector_index < input_points[0].Num(); inner_vector_index++) {
						output_vector[inner_vector_index] = output_vector[inner_vector_index] + input_points[outer_vector_index][inner_vector_index]; // add the value into the vector //
						int t = 1;
					}
				}
				// divide by the number of entries 
				output_vector = scalar_vector_multiply((float)1 / input_points.Num(), output_vector);
				return output_vector;
			}
		}
		/* create a new vector by multiplying two vectors together */
		cweeThreadedList <float> static multiply_every_vector_entry(cweeThreadedList <float> entry_vector_1, cweeThreadedList <float> entry_vector_2) {
			cweeThreadedList <float> output;

			// if the size of the vectors are not equal, we return the first vector // 
			if (entry_vector_1.Num() != entry_vector_2.Num()) {
				return entry_vector_1;
			}
			// go through each of the entries and multiply // 
			for (int entry_index = 0; entry_index < entry_vector_1.Num(); entry_index++) {
				output.Append(entry_vector_1[entry_index] * entry_vector_2[entry_index]);
			}

			return output;  //
		}
		/* get the standard deviation of a set of inputted points */
		cweeThreadedList <float> static get_stddev_from_vector_set(cweeThreadedList < cweeThreadedList <float>> input_points) {
			cweeThreadedList <float> stddev_vector = {};
			cweeThreadedList <float> squared_sum_of_values = vector_repeated_scalar_values(input_points.Num(), 0);
			for (int outer_vector_index = 0; outer_vector_index < input_points.Num(); outer_vector_index++) {
				for (int inner_vector_index = 0; inner_vector_index < input_points[0].Num(); inner_vector_index++) {
					squared_sum_of_values[inner_vector_index] = squared_sum_of_values[inner_vector_index]
						+ (input_points[outer_vector_index][inner_vector_index] * input_points[outer_vector_index][inner_vector_index]); // add in the new squared values // 
				}
			}

			// develop the variance based on the formula //
			cweeThreadedList <float> variance_vector = subtract_vec(scalar_vector_multiply(1 / squared_sum_of_values.Num(), squared_sum_of_values), multiply_every_vector_entry(get_mean_from_vector_set(input_points), get_mean_from_vector_set(input_points)));
			for (auto value : variance_vector) {
				stddev_vector.Append(sqrt(value));
			}
			return stddev_vector; // return the standard deviation
		}
		/* return the the midpoints from each of the vector */
		cweeThreadedList <float> static get_midpoints_from_vector_sets(cweeThreadedList < cweeThreadedList <float>> input_points_upper, cweeThreadedList < cweeThreadedList <float>> input_points_lower) {
			cweeThreadedList <float> midpoints;
			if (input_points_upper.Num() != input_points_lower.Num() || input_points_upper.Num() == 0 || input_points_lower.Num() == 0) { // return the 
				return midpoints;
			}
			for (int index_input = 0; index_input < input_points_upper.Num(); index_input++) {
				midpoints.Append(add_vec(subtract_vec(input_points_upper[index_input], input_points_lower[index_input]), input_points_lower[index_input])); // insert the midpoint // 
			}
			return midpoints;
		}
		/* a method for generating a random number based on the */
		std::pair<float, float> static Marsaglia_Polar_Generator(bool random = true, float default_x = 0.5, float default_y = 0.5) {
			std::pair<float, float> return_values;
			float x = -6, y = -6, s = -6; // 
			if (random) {
				while (!(s < 1 && s > 0)) {
					y = cweeRandomFloat(-1, 1);
					x = cweeRandomFloat(-1, 1);
					s = x * x + y * y;
				}
			}
			else {
				y = default_y;
				x = default_x;
				s = (x * x) + (y * y);
			}
			return_values.first = x * Sqrt(-2 * log(s) / s);
			return_values.second = y * Sqrt(-2 * log(s) / s);		// 
			return return_values;									// return the values from the system // 
		};
		/* generate a vector of normal random variables */
		float static NormalRandomGenerator(float mean, float stddev, bool random = true, non_random_type_t type = __NONE_NON_RANDOM__, int seed_number = 0) {
			std::pair<float, float> return_values_std_normal = Marsaglia_Polar_Generator();
			if (type == __LOW_LIMIT__) { // UPPER AND LOWER RANDOM NUMBERS BASED ON THREE STANDARD DEVIATIONS // 
				return  -3 * stddev + mean;
			}
			else if (type == __HIGH_LIMIT__) { // UPPER AND LOWER RANDOM NUMBERS BASED ON THREE STANDARD DEVIATIONS // 
				return  3 * stddev + mean;
			}
			else if (type == __MIDDLE_MEDIAN__) {
				return mean;
			}
			else if (type == __ONE_QUARTILE_UP__) {
				return  0.67 * stddev + mean;
			}
			else if (type == __ONE_QUARTILE_DOWN__) {
				return -1 * 0.67 * stddev + mean;
			}
			else { // actual random number // 
				return return_values_std_normal.first * stddev + mean;
			}
		};
		/* generate a vector of standard normal random variables */
		static cweeThreadedList<float> DetermineStandardNormalRandomVector(int vector_size, bool random = true, non_random_type_t type = __NONE_NON_RANDOM__, int seed_number = -1) {
			cweeThreadedList<float> output_vector;
			while (output_vector.Num() < vector_size) {
				if (random) {
					std::pair<float, float> run_pair;
					run_pair = Marsaglia_Polar_Generator();
					output_vector.Append(run_pair.first);
					if (!(output_vector.Num() < vector_size)) {
						break;
					}
					output_vector.Append(run_pair.second);
				}
				else {
					output_vector.Append(NormalRandomGenerator(0, 1, random, type, seed_number));
				}
			}
			return output_vector;
		};
		/* generate a vector of normal random variables */
		cweeThreadedList<float> static DetermineNormalRandomVector(float mean, float stddev, int vector_size, bool random = true, non_random_type_t type = __NONE_NON_RANDOM__, int seed_number = -1) {
			cweeThreadedList<float> mean_vector;
			for (int return_index_size = 0; return_index_size < vector_size; return_index_size++) {
				mean_vector.Append(mean);
			}
			return cweeMath::Optim::add_vec(mean_vector, cweeMath::VectorScalarProduct(stddev, DetermineStandardNormalRandomVector(vector_size, random, type, seed_number)));
		};
		/* generate a vector of nor+mal random variables */
		cweeThreadedList<float> static DetermineNormalRandomVector_vectorInput(const cweeThreadedList<float>& mean, const cweeThreadedList<float>& stddev, bool random = true, non_random_type_t type = __NONE_NON_RANDOM__, int seed_number = -1) {
			cweeThreadedList<float> output_vector;
			for (int return_index_size = 0; return_index_size < mean.Num(); return_index_size++) {
				output_vector.Append(NormalRandomGenerator(mean[return_index_size], stddev[return_index_size], random, type, seed_number));
			}
			return output_vector; // return the output vector
		};
		/* get a random vector between box constraints */
		cweeThreadedList<float> static GetUniformRandomVector(const cweeThreadedList<float>& upper_bound, const cweeThreadedList<float>& lower_bound, bool random = true, non_random_type_t type = __NONE_NON_RANDOM__, int seed_number = -1) {
			cweeThreadedList<float> sampled_vector;
			int seed_non_random_counter = seed_number;

			for (int index_i = 0; index_i < upper_bound.Num(); index_i++) {				 // random vector //
				if (random) {
					sampled_vector.Append(cweeRandomFloat(lower_bound[index_i], upper_bound[index_i]));
				}
				else {
					if (type == __LOW_LIMIT__) {
						sampled_vector.Append(lower_bound[index_i]);
					}
					else if (type == __HIGH_LIMIT__) {
						sampled_vector.Append(upper_bound[index_i]);
					}
					else if (type == __MIDDLE_MEDIAN__) {
						sampled_vector.Append(((upper_bound[index_i] - lower_bound[index_i]) / 2 + lower_bound[index_i]));
					}
					else if (type == __ONE_QUARTILE_UP__) {
						sampled_vector.Append(((upper_bound[index_i] - lower_bound[index_i]) / 4 + lower_bound[index_i]));
					}
					else if (type == __ONE_QUARTILE_DOWN__) {
						sampled_vector.Append((-1 * (upper_bound[index_i] - lower_bound[index_i]) / 4 + lower_bound[index_i]));
					}
					else {
						sampled_vector.Append(0);
					}
				}
			}



			return sampled_vector; // returns the sampled vector
		};
		/* generate a vector unique integer values between two bounds */
		cweeThreadedList<float> static get_unique_random_in_limits(int low_bound, int up_bound, int num_points) {
			cweeThreadedList<float> output;
			if (num_points > (up_bound - low_bound)) { // if there are 
				return output;
			}

			int points_generated = 0;
			while (num_points > points_generated) {
				int new_random = cweeRandomInt(low_bound, up_bound);
				bool not_sampled = true;
				for (auto generated : output) {
					if (generated == new_random) {
						not_sampled = false;
						break;
					}
				}
				if (not_sampled) {
					output.Append(new_random);
					points_generated++;
				}
			}
			return output;
		};
		cweeThreadedList<int> static get_unique_random_in_limits_int(int low_bound, int up_bound, int num_points) {
			cweeThreadedList<int> output;

			if (num_points > (up_bound - low_bound)) { // if there are 
				return output;
			}
			int points_generated = 0;
			while (num_points > points_generated) {
				int new_random = cweeRandomInt(low_bound, up_bound);
				bool not_sampled = true;
				for (auto generated : output) {
					if (generated == new_random) {
						not_sampled = false;
						break;
					}
				}
				if (not_sampled) {
					output.Append(new_random);
					points_generated++;
				}
			}

			return output;
		};
		/* a scalar multiplication function for the vector */
		cweeThreadedList<float> static scalar_vector_multiply(float scalar_alpha, cweeThreadedList<float> input_vector) {
			cweeThreadedList<float> output = {};

			for (auto value : input_vector) {
				output.Append(value * scalar_alpha); // append the multiplied value 
			}

			return output;
		}
		/* creates a vector from repeated scalar values */
		cweeThreadedList<float> static vector_repeated_scalar_values(int vector_size, float scalar_value) {
			cweeThreadedList<float> output_vector; // 
			for (int v_index = 0; v_index < vector_size; v_index++) {
				output_vector.Append(scalar_value);
			}
			return output_vector;
		};
		/* order the values in inputed vector  */
		cweeThreadedList<float> static order_values(const cweeThreadedList<float>& values) {
			std::vector<float> updated_values = values;
			std::sort(updated_values.begin(), updated_values.end());
			return updated_values;
		};
		/* order values inside of a vector */
		cweeThreadedList <cweeThreadedList<float>> static order_vector_values(const cweeThreadedList<float>& values, const cweeThreadedList < cweeThreadedList<float> >& vector_in) {
			cweeThreadedList <cweeThreadedList<float>> output;
			std::vector<float> updated_values = values;
			std::vector <cweeThreadedList<float>> updated_vector = vector_in;
			while (updated_values.size() > 0) {
				int min_index = std::min_element(updated_values.begin(),
					updated_values.end()) - updated_values.begin();

				output.Append(updated_vector[min_index]);
				updated_values.erase(updated_values.begin() + min_index);
				updated_vector.erase(updated_vector.begin() + min_index);
			}

			return output;
		};
		/* append float vectors in the form of cweeThreaded lists */
		cweeThreadedList<float> static append_vectors(const cweeThreadedList<float>& front_vector, const cweeThreadedList<float>& back_vector) {
			cweeThreadedList<float> output = front_vector;
			for (auto element : back_vector) {
				output.Append(element);
			}
			return output;
		}
		/* append vector of vectors in the form of cweeeThreaded lists */
		cweeThreadedList <cweeThreadedList<float>> static append_vector_sets(const cweeThreadedList < cweeThreadedList<float>>& front_vectors, const cweeThreadedList < cweeThreadedList<float>>& back_vectors) {
			cweeThreadedList < cweeThreadedList<float>> output = front_vectors;

			for (auto element : back_vectors) {
				output.Append(element);
			}

			return output;
		}
		/* expand a provided vector to a larger dimension, filling in the new dimensions with random numbers selected uniformly */
		cweeThreadedList<float> expand_vector_with_random(const cweeThreadedList<float>& input,
			cweeThreadedList<bool> slot_locations,
			const cweeThreadedList<float>& full_upper_bound,
			const cweeThreadedList<float>& full_lower_bound, bool no_random = false, float default_val = 0) {

			cweeThreadedList<float> full_output;
			int index = 0;
			for (int index_dim = 0; index_dim < slot_locations.Num(); index_dim++) {
				if (slot_locations[index_dim]) {
					full_output.Append(input[index]);
					index++;
				}
				else {
					full_output.Append(cweeRandomFloat(full_lower_bound[index_dim], full_upper_bound[index_dim]));
				}
			}
			return full_output;
		};
		/* get the minimum index of the inputed vector */
		int static get_min_index(std::vector<float> input) { // get the minimum index of the vector for any given vector
			return (std::min_element(input.begin(), input.end()) - input.begin());
		};
		/* get the minimum index of the inputed vector */
		float static get_min(std::vector<float> input) { // get the minimum index of the vector for any given vector
			return (input[std::min_element(input.begin(), input.end()) - input.begin()]);
		};
		/* get the maximum MAGNITUDE sign */
		float static get_absolute_max(const cweeThreadedList<float>& input_vector) {
			cweeThreadedList<float> abs_vector;
			int index_of_max;
			for (int index_i = 0; index_i < input_vector.Num(); index_i++) { // go through the input vectors and apply an absolute value 
				abs_vector.Append(-1 * (std::abs(input_vector[index_i])));
			}
			index_of_max = get_min_index(abs_vector);
			return (input_vector[index_of_max]);  // return the maximum element of the vector
		};
		/* helper function that adds two float vectors  */
		cweeThreadedList<float> static add_vec(const cweeThreadedList<float>& input_vector_a, const cweeThreadedList<float>& input_vector_b) {
			cweeThreadedList<float> output_vector;
			for (int index_i = 0; index_i < input_vector_a.Num(); index_i++) {
				output_vector.Append(input_vector_a[index_i] + input_vector_b[index_i]);
			}
			return output_vector; // return the added vector //
		};
		/* helper functions that subtracts two float vectors */
		cweeThreadedList<float> static subtract_vec(const cweeThreadedList<float>& input_vector_a, const cweeThreadedList<float>& input_vector_b) {
			cweeThreadedList<float> output_vector;
			for (int index_i = 0; index_i < input_vector_a.Num(); index_i++) {
				output_vector.Append(input_vector_a[index_i] - input_vector_b[index_i]);
			}

			return output_vector; // return the subtracted vector
		};
		/* round the vector to the nearest integer */
		cweeThreadedList<float> static round_vector_to_nearest_int(const cweeThreadedList<float>& input) {
			cweeThreadedList<float> output;
			for (auto value : input) {
				output.Append(round(value));
			}
			return output; // return the output //
		}// round the vector to the nearest input //;
		/* a function that rounds the solution to the nearest bound */
		cweeThreadedList<float> static round_vector_to_nearest_bound(const cweeThreadedList<float>& input, const cweeThreadedList<float>& upper_bound, const cweeThreadedList<float>& lower_bound) {
			cweeThreadedList<float> output;
			if (upper_bound.Num() != input.Num() || lower_bound.Num() != input.Num()) {
				return output;
			}
			else {
				int index_bound = 0;
				for (auto value : input) {
					if (value > upper_bound[index_bound]) {
						output.Append(upper_bound[index_bound]);
					}
					else if (value < lower_bound[index_bound]) {
						output.Append(lower_bound[index_bound]);
					}
					else {
						output.Append(value);
					}
					index_bound++;
				}
				return output;
			}
		};
		/* a function to find the centroid of a set of points */
		cweeThreadedList<float> static GetCentroidPoints(const cweeThreadedList < cweeThreadedList<float> >& input_points) {
			cweeThreadedList<float> output = {};
			if (input_points.Num() == 0) {
				return output;
			}
			// go through each of the dimensions and calculate the centroid //
			for (int dimension_index = 0; dimension_index < input_points[0].Num(); dimension_index++) { // for each of the dimensions
				float sum_inside_dimension = 0;
				for (int particle_index = 0; particle_index < input_points.Num(); particle_index) {
					sum_inside_dimension = sum_inside_dimension + input_points[particle_index][dimension_index];
				}
				output.Append(sum_inside_dimension / input_points.Num());
			}
			return output; // 
		}
		/* a function that subsets the vector from the front  */
		cweeThreadedList < cweeThreadedList<float>> static subsetFromTheFrontVector(cweeThreadedList < cweeThreadedList < float>> input_vector, int size_subset) {
			cweeThreadedList < cweeThreadedList<float>> output;
			int index_for_entry = 0;
			for (auto entry : input_vector) {
				if (index_for_entry >= size_subset) {
					break;
				}
				output.Append(entry);
				index_for_entry++;   // update the index for the number of entries 
			}
			return output;
		}
		/* a function that subsets the vector from the front  */
		cweeThreadedList<float> static subsetFromTheFront(cweeThreadedList < float> input_vector, int size_subset) {
			cweeThreadedList<float> output;
			int index_for_entry = 0;
			for (auto entry : input_vector) {
				if (index_for_entry >= size_subset) {
					break;
				}
				output.Append(entry);
				index_for_entry++;   // update the index for the number of entries 
			}
			return output;
		}
		/* create a diverse set object */
		cweeThreadedList<cweeThreadedList<float>>  static subsetListofVectorsFromTheFront(cweeThreadedList < cweeThreadedList < float> > input_vector, int size_subset) {
			cweeThreadedList<cweeThreadedList<float>> output;

			int num_elements = cweeMath::min(cweeMath::min(size_subset, input_vector.Num()), 0);
			for (int input_index = 0; input_index < num_elements; input_index++) { // 
				output.Append(input_vector[input_index]);
			}

			return output; // return the output
		}
		/* the diverse set object used to track the distribution of points within sub-regions */
		class DiverseSearchObject { // this will be both a component of the input and the output for a diverse sampling method since we need to update a  //
		public:
			int num_sub_regions = -1;
			cweeThreadedList < float >						sampled_values = {};
			cweeThreadedList < cweeThreadedList<float> >	sampled_points = {};

			cweeThreadedList < cweeThreadedList<int  > >	sub_region_sample_freq = {};  // dimension d x region s 
			cweeThreadedList < cweeThreadedList<float> >	sub_region_sample_prob = {};  //
			cweeThreadedList < cweeThreadedList<float> >	sub_region_upperbound_constraints = {};  // dimensions d x sub-regions s
			cweeThreadedList < cweeThreadedList<float> >	sub_region_lowerbound_constraints = {};
			bool isNotPopulated() {
				if (sub_region_upperbound_constraints.Num() == 0 ||
					sub_region_lowerbound_constraints.Num() == 0 ||
					sub_region_sample_freq.Num() == 0 ||
					num_sub_regions == -1
					) {
					return false;
				}
				return true;
			}
			DiverseSearchObject& operator=(const DiverseSearchObject input) {
				/* the sample regions  */
				int num_sub_regions = input.num_sub_regions; // the number of sub-regions 
				sampled_points.Clear();
				sub_region_sample_freq.Clear();
				sub_region_sample_prob.Clear();
				sub_region_upperbound_constraints.Clear();  // dimensions d x sub-regions s
				sub_region_lowerbound_constraints.Clear();

				/* iteratively populate all of the entries*/
				for (auto element : input.sampled_points) { sampled_points.Append(element); }
				for (auto element : input.sub_region_sample_freq) { sub_region_sample_freq.Append(element); }
				for (auto element : input.sub_region_sample_prob) { sub_region_sample_prob.Append(element); }
				for (auto element : input.sub_region_upperbound_constraints) { sub_region_upperbound_constraints.Append(element); }
				for (auto element : input.sub_region_lowerbound_constraints) { sub_region_lowerbound_constraints.Append(element); }
				return *this; // 
			}
		};
		// Update Diverse with Sampled points // 
		DiverseSearchObject static UpdateDiverseObjectWithNewSampledPoints(DiverseSearchObject inputDiverseSet, cweeThreadedList < cweeThreadedList<float>> input_points, cweeThreadedList<float> input_values = { }) {
			DiverseSearchObject output = inputDiverseSet;
			if (input_points.Num() == 0 || inputDiverseSet.sub_region_lowerbound_constraints[0].Num() != inputDiverseSet.sub_region_upperbound_constraints[0].Num()) {
				return output; // return the output
			}
			for (int point_index = 0; point_index < input_points.Num(); point_index++) {
				for (int dimension_index = 0; dimension_index < input_points[0].Num(); dimension_index++) { // go through the dimensions // 
					for (int region_index = 0; region_index < inputDiverseSet.sub_region_lowerbound_constraints[dimension_index].Num(); region_index++) {
						if (input_points[point_index][dimension_index] < inputDiverseSet.sub_region_lowerbound_constraints[dimension_index][region_index]) {			// if we are above the lower bounds
							break;
						}
						if (inputDiverseSet.sub_region_lowerbound_constraints[dimension_index][region_index] < input_points[point_index][dimension_index]
							&& input_points[point_index][dimension_index] < inputDiverseSet.sub_region_upperbound_constraints[dimension_index][region_index]) {
							inputDiverseSet.sub_region_sample_freq[dimension_index][region_index]++;	// increment the sample frequency of the subregion
							break;  // increment the frequency counter here // 
						}
					}
				}
			}

			//	// update the main tracking objects //
			//	if (input_points.Num() > 0 && (input_points.Num() == input_values.Num())) {
			//		cweeThreadedList<cweeThreadedList<float>> update_point_lit = append_vector_sets();
			//		cweeThreadedList<float> update_values_lit = append_vectors(inputDiverseSet.); // update the values for the system // 
			//
			//	}

			return output; // return the output 
		}
		cweeThreadedList<cweeThreadedList<int>> getPointLocations(DiverseSearchObject inputDiverseSet, cweeThreadedList<cweeThreadedList<float>> input_points) {
			cweeThreadedList<cweeThreadedList<int>> output;

			if (input_points.Num() == 0 || inputDiverseSet.sub_region_lowerbound_constraints[0].Num() != inputDiverseSet.sub_region_upperbound_constraints[0].Num()) {
				return output; // return the output
			}

			for (int point_index = 0; point_index < input_points.Num(); point_index++) {
				cweeThreadedList<int> individual_location;
				for (int dimension_index = 0; dimension_index < input_points[0].Num(); dimension_index++) { // go through the dimensions // 
					for (int region_index = 0; region_index < inputDiverseSet.sub_region_lowerbound_constraints[dimension_index].Num(); region_index++) {
						if (input_points[point_index][dimension_index] < inputDiverseSet.sub_region_lowerbound_constraints[dimension_index][region_index]) {			// if we are above the lower bounds
							individual_location.Append(-1); // record negative one if we are out out bounds. 
							break;
						}
						if (inputDiverseSet.sub_region_lowerbound_constraints[dimension_index][region_index] < input_points[point_index][dimension_index]
							&& input_points[point_index][dimension_index] < inputDiverseSet.sub_region_upperbound_constraints[dimension_index][region_index]) {
							individual_location.Append(region_index);
							break;				// increment the frequency counter here // 
						}
						individual_location.Append(-1);
					}
				} // we only record the point if it is insid the bounds 
			}
			return output;
		}
		cweeThreadedList<float> getUpperBoundsFromRegionBasedOnPointLocation(cweeThreadedList<float> input_point, cweeThreadedList<int>, DiverseSearchObject inputDiverseSet) {
			cweeThreadedList<float> output;
			if (input_point.Num() == 0 || inputDiverseSet.sub_region_lowerbound_constraints[0].Num() != inputDiverseSet.sub_region_upperbound_constraints[0].Num()) {
				return output; // return the output
			}
			for (int dimension_index = 0; dimension_index < input_point.Num(); dimension_index++) { // go through the dimensions // 
				for (int region_index = 0; region_index < inputDiverseSet.sub_region_lowerbound_constraints[dimension_index].Num(); region_index++) {
					if (input_point[dimension_index] < inputDiverseSet.sub_region_lowerbound_constraints[dimension_index][region_index]) {			// if we are above the lower bounds
						output.Append(-1);
						break;
					}
					if (inputDiverseSet.sub_region_lowerbound_constraints[dimension_index][region_index] < input_point[dimension_index]
						&& input_point[dimension_index] < inputDiverseSet.sub_region_upperbound_constraints[dimension_index][region_index]) {
						output.Append(inputDiverseSet.sub_region_upperbound_constraints[dimension_index][region_index]); // append the value 
						break;  // increment the frequency counter here // 
					}
					output.Append(-1);
				}
			}

			return output; // return the output bounds 
		}
		cweeThreadedList<float> getLowerBoundsFromRegionBasedOnPointLocation(cweeThreadedList<float> input_point, cweeThreadedList<int>, DiverseSearchObject inputDiverseSet) {
			cweeThreadedList<float> output;
			if (input_point.Num() == 0 || inputDiverseSet.sub_region_lowerbound_constraints[0].Num() != inputDiverseSet.sub_region_upperbound_constraints[0].Num()) {
				return output; // return the output
			}

			for (int dimension_index = 0; dimension_index < input_point.Num(); dimension_index++) { // go through the dimensions // 
				for (int region_index = 0; region_index < inputDiverseSet.sub_region_lowerbound_constraints[dimension_index].Num(); region_index++) {
					if (input_point[dimension_index] < inputDiverseSet.sub_region_lowerbound_constraints[dimension_index][region_index]) {			// if we are above the lower bounds
						break;
					}
					if (inputDiverseSet.sub_region_lowerbound_constraints[dimension_index][region_index] < input_point[dimension_index]
						&& input_point[dimension_index] < inputDiverseSet.sub_region_upperbound_constraints[dimension_index][region_index]) {
						output.Append(inputDiverseSet.sub_region_lowerbound_constraints[dimension_index][region_index]);
						break;  // increment the frequency counter here // 
					}
				}
			}
			return output;
		}
		DiverseSearchObject static PopulateDiverseSampling(int num_sub_regions, cweeThreadedList<float> upper_bounds, cweeThreadedList<float> lower_bounds) {
			DiverseSearchObject output;
			if (upper_bounds.Num() != lower_bounds.Num() || lower_bounds.Num() == 0 || upper_bounds.Num() == 0) {
				return output;
			}

			// the upper and lower bound constraints are not populated then we will re-write the regions as equal partitions of the samples space into sub-regions // 
			int num_dimensions = lower_bounds.Num(); // set up the number of dimensions // 
			output.sub_region_upperbound_constraints.Clear();
			output.sub_region_upperbound_constraints.Clear();
			output.sub_region_sample_freq.Clear();

			for (int index_dimension = 0; index_dimension < num_dimensions; index_dimension++) {
				float bound_length = upper_bounds[index_dimension] - lower_bounds[index_dimension];
				cweeThreadedList<float> dim_constraints_upper, dim_constraints_lower;
				cweeThreadedList<int> freq_sample;
				cweeThreadedList<float> prob_sample;

				for (int index_subregions = 0; index_subregions < num_sub_regions; index_subregions++) {
					freq_sample.Append(0); // (record that there are zero sampled points in this sub-region)
					dim_constraints_upper.Append(lower_bounds[index_dimension] + (bound_length / num_sub_regions) * (index_subregions + 1)); // 
					dim_constraints_lower.Append(lower_bounds[index_dimension] + (bound_length / num_sub_regions) * (index_subregions));
					freq_sample.Append(0);
					prob_sample.Append(1 / num_sub_regions);
				}

				output.sub_region_upperbound_constraints.Append(dim_constraints_upper);  // save the upper and lower bound constraints //
				output.sub_region_lowerbound_constraints.Append(dim_constraints_lower);
				output.sub_region_sample_freq.Append(freq_sample);						// the frequency sample
				output.sub_region_sample_prob.Append(prob_sample);
			}
			return output;
		}
		/* sample diverse region based on specifications */
		DiverseSearchObject static sampleDiverseSet(int number_sample_points, DiverseSearchObject input,
			cweeThreadedList<float> upper_bounds = {}, cweeThreadedList<float> lower_bounds = {}, int num_sub_regions = -1) {

			// check the inputs to make sure that they are populated // 
			if (input.isNotPopulated() || (upper_bounds.Num() != 0 && lower_bounds.Num() != 0)) { // if the input is not populated then we populate as an empty diverse sampling object
				if (num_sub_regions > 0) {
					input = PopulateDiverseSampling(num_sub_regions, upper_bounds, lower_bounds);
				}
				else {
					input = PopulateDiverseSampling(1, upper_bounds, lower_bounds);
				}
			}
			else if (input.isNotPopulated()) {
				return input;  // we do not have enought information to sample with diverse sampling 
			}

			// create the number of sub-regions and number of dimensions //
			int num_sub_regions_used = input.num_sub_regions;
			int num_dimensions = input.sub_region_upperbound_constraints.Num();
			DiverseSearchObject output = input; // copy the input to the output
			cweeThreadedList < cweeThreadedList<float> > output_sampled = {};

			// check to see if the sample size is lower than the total number of sub-regions then we sample NOTHING  // 
			if (number_sample_points < num_sub_regions) {
				return output; // output a empty object
			}
			cweeThreadedList<cweeThreadedList<float>> sub_region_sample_prob = input.sub_region_sample_prob;
			cweeThreadedList<cweeThreadedList<int>> sub_region_sample_freq = input.sub_region_sample_freq;

			// for the first batch 1 for each of the sub-regions (diagonally or random) //			 
			for (int index_subregions = 0; index_subregions < num_sub_regions_used; index_subregions++) {
				cweeThreadedList<float> upper_bounds_sample, lower_bounds_sample; // the upper and lower bounds of the sampled vector // 
				for (int index_dimension = 0; index_dimension < num_dimensions; index_dimension++) {
					upper_bounds_sample.Append(input.sub_region_upperbound_constraints[index_dimension][index_subregions]); // push
					lower_bounds_sample.Append(input.sub_region_lowerbound_constraints[index_dimension][index_subregions]);
					sub_region_sample_freq[index_dimension][index_subregions]++;
				}
				output_sampled.Append(GetUniformRandomVector(upper_bounds_sample, lower_bounds_sample));  // uniformly sample to get the initial sample points // 
			}

			// update the probability of sampling across the dimension //
			for (int index_dimension = 0; index_dimension < num_dimensions; index_dimension++) { // go through each of the dimensions // 
				float f_col_prob_demoninator = 0; // reset the denominator value // 
				for (int index_subregions_first = 0; index_subregions_first < num_sub_regions; index_subregions_first++) {
					f_col_prob_demoninator += (1. / sub_region_sample_freq[index_dimension][index_subregions_first]); // add up the probabilities 
				}
				for (int index_subregions_second = 0; index_subregions_second < num_sub_regions; index_subregions_second++) {
					sub_region_sample_prob[index_dimension][index_subregions_second] = ((1 / sub_region_sample_freq[index_dimension][index_subregions_second]) / f_col_prob_demoninator);
				}
			}

			// now we go through all the remaining points and update the probabilities across each of the dimensions // 
			for (int remaining_index = 0; remaining_index < (number_sample_points - num_sub_regions); remaining_index++) {
				cweeThreadedList<float> upper_bounds_for_insert_vector = vector_repeated_scalar_values(num_dimensions, 0);
				cweeThreadedList<float> lower_bounds_for_insert_vector = vector_repeated_scalar_values(num_dimensions, 0);
				for (int index_dimension = 0; index_dimension < num_dimensions; index_dimension++) { // go through each of the dimensions // 
					float probability_total = 0;
					float prob_threshold = cweeRandomFloat();
					int region_selected = 0;
					for (int index_subregions = 0; index_subregions < num_sub_regions; index_subregions++) {
						// add the probability up  // 
						probability_total = probability_total + sub_region_sample_prob[index_dimension][index_subregions];

						// add the new entry into the system and update the probabilities  //
						if (prob_threshold <= probability_total) {  // if we cross the threshold // 
							region_selected = index_subregions;
							float f_col_prob_demoninator = 0; // reset up the 
							for (int index_subregions_first = 0; index_subregions_first < num_sub_regions; index_subregions_first++) {
								f_col_prob_demoninator += (1. / input.sub_region_sample_freq[index_dimension][index_subregions_first]); // add up the probabilities 
							}
							// update the probablitilies //
							for (int index_subregions_second = 0; index_subregions_second < num_sub_regions; index_subregions_second++) {
								sub_region_sample_prob[index_dimension][index_subregions_second] = (1. / input.sub_region_sample_freq[index_dimension][index_subregions_second]) / f_col_prob_demoninator; // add up the probabilities 
							}
							break;
						}
					}
					// assign the values for the upper and lower bounds of the sampling // 
					upper_bounds_for_insert_vector[index_dimension] = input.sub_region_upperbound_constraints[index_dimension][region_selected];
					lower_bounds_for_insert_vector[index_dimension] = input.sub_region_lowerbound_constraints[index_dimension][region_selected]; // create the upper and lower bound out of the regions selected
					sub_region_sample_freq[index_dimension][region_selected]++;
					// update the frequency here //
				}

				// push the vector into the output 
				output_sampled.Append(GetUniformRandomVector(upper_bounds_for_insert_vector, lower_bounds_for_insert_vector)); // append the new vector 
			}

			// return the sampled output and the frequency of sampled points // 
			output.sampled_points = output_sampled;
			output.sub_region_sample_freq = sub_region_sample_freq;
			output.sub_region_sample_prob = sub_region_sample_prob;

			return output;
		}
		/* sample uniform */
		cweeThreadedList< cweeThreadedList < float >> static sampleUniformSet(int number_sample_points, const cweeThreadedList<float>& upper_bound, const cweeThreadedList<float>& lower_bound, cweeThreadedList<float> dependent_bounds_upper = {}, cweeThreadedList<float> dependent_bounds_lower = {}, bool random = true, non_random_type_t type = __NONE_NON_RANDOM__, int seed_number = -1) {

			cweeThreadedList< cweeThreadedList < float >> output;
			int seed_non_random_counter = seed_number;

			for (int index_for_sample = 0; index_for_sample < number_sample_points; index_for_sample++) {

				cweeThreadedList<float> input = GetUniformRandomVector(upper_bound, lower_bound, random, type, seed_number);
				cweeThreadedList<float> modified_input;  // modify the upper and lower bounds // 
				modified_input = input;

				if (input.Num() == dependent_bounds_upper.Num() && input.Num() == dependent_bounds_lower.Num()) {
					for (int vector_index = 0; vector_index < dependent_bounds_lower.Num(); vector_index++) {
						if (dependent_bounds_lower[vector_index] >= 0 || dependent_bounds_lower[vector_index] >= 0) {
							cweeThreadedList<float> new_lower_bounds, new_upper_bounds;
							new_lower_bounds.Append(lower_bound[vector_index]);
							new_upper_bounds.Append(upper_bound[vector_index]);

							// modify the upper and lower bounds when the dependecy vector specifies // 
							if (dependent_bounds_lower[vector_index] >= 0) { new_lower_bounds[0] = input[dependent_bounds_lower[vector_index]]; }
							if (dependent_bounds_upper[vector_index] >= 0) { new_upper_bounds[0] = input[dependent_bounds_upper[vector_index]]; }
							modified_input[vector_index] = GetUniformRandomVector(new_upper_bounds, new_lower_bounds, random, type, seed_number)[0];
						}
					}
				}
				output.Append(modified_input); // append the modified input with the specificed bounds //
			}
			return output;
		}
		cweeThreadedList< cweeThreadedList < float >> static sampleNormalSet(int number_sample_points, const cweeThreadedList<float>& upper_bound, const cweeThreadedList<float>& lower_bound, const cweeThreadedList<float>& mean, const cweeThreadedList<float>& stddev, cweeThreadedList<float> dependent_bounds_upper = {}, cweeThreadedList<float> dependent_bounds_lower = {}, bool random = true, non_random_type_t type = __NONE_NON_RANDOM__, int seed_number = -1) {
			cweeThreadedList< cweeThreadedList < float >> output;

			for (int index_for_sample = 0; index_for_sample < number_sample_points; index_for_sample++) {

				// create the normal random vector // 
				cweeThreadedList<float> input = DetermineNormalRandomVector_vectorInput(mean, stddev, random, type, seed_number);    // get the random normal 
				cweeThreadedList<float> modified_input;  // modify the upper and lower bounds // 
				modified_input = input;
				cweeThreadedList<float> new_lower_bounds, new_upper_bounds;
				new_lower_bounds = lower_bound;
				new_upper_bounds = upper_bound;

				// modified vector based on the dependecy vectors // 
				if (input.Num() == dependent_bounds_upper.Num() && input.Num() == dependent_bounds_lower.Num()) {
					for (int vector_index = 0; vector_index < dependent_bounds_lower.Num(); vector_index++) {
						if (dependent_bounds_lower[vector_index] >= 0 || dependent_bounds_lower[vector_index] >= 0) {
							// modify the upper and lower bounds when the dependecy vector specifies // 
							if (dependent_bounds_lower[vector_index] >= 0) { new_lower_bounds[vector_index] = input[dependent_bounds_lower[vector_index]]; }
							if (dependent_bounds_upper[vector_index] >= 0) { new_upper_bounds[vector_index] = input[dependent_bounds_upper[vector_index]]; }
						}
					}
				}

				// constrain the vector to the upper and lower bounds // 
				output.Append(round_vector_to_nearest_bound(input, new_upper_bounds, new_lower_bounds));
			}

			return output;
		}

#pragma endregion
		/* The base search class from which all other classes are extended from */
#pragma region BASE_CLASS
		// we are going to track individual points that track the particular area of the domain where they fall along with their values // 
		class individual_stat_point {
		public:
			float evaluated_value;
			cweeThreadedList<float> policy_point;
			cweeThreadedList<float> region_sampled; // a vector representing which sub region this point comes from // 

			double cost;							/*!< Stores the cost of current Individual */
			double dist;							/*!< Stores the distance of current Individual to a set of other Individuals */
			int n_not_randomized;					/*!< Stores the number of times that the specific Individual updates but not randomize. It will be used to update the stats */
			int n_stuck;

			// the constructor and destructor //
			individual_stat_point() {}
			individual_stat_point(cweeThreadedList<float> point_input) { policy_point = point_input;  cost = PSEUDO_MAX; }
			individual_stat_point(cweeThreadedList<float> point_input, float value_at_point) : individual_stat_point(point_input) { cost = value_at_point; }
			~individual_stat_point() {}

			// assign the location of points based on a region distinction // 
			void assignLocation() { }
			bool isLocationAssigned() { }
			bool isCostAssigned() {}
			bool isLocationAssign() {}
			individual_stat_point& operator=(const individual_stat_point input) {  // we need to create a copy assignment operator // 
				policy_point = input.policy_point;
				region_sampled = input.region_sampled;							// a vector representing which sub region this point comes from // 
				cost = input.cost;												/*!< Stores the cost of current Individual */
				dist = input.dist;												/*!< Stores the distance of current Individual to a set of other Individuals */
				n_not_randomized = input.n_not_randomized;						/*!< Stores the number of times that the specific Individual updates but not randomize. It will be used to update the stats */
				n_stuck = input.n_stuck;

				return *this;
			}
		};
		// this vector will determine 
		enum sampling_type {
			_NO_SAMPLING_,
			_UNIFORM_RANDOM_SAMPLE_,
			_NORMAL_GLOBAL_PARAM_,
			_NORMAL_PARAM_PER_DIM_,
			_NORMAL_FROM_CENTROID_,
			_UNIFORM_DIVERSE_
		};
		class OptimInput { // this is the parent class for all other optimization objects
		public:
			// the number of dimensions, iterations completed, and box bounds, this should be the same FOR ALL REFATORED OPTIMIZERS //
			int iterations_completed = 0;										// length i
			int num_dimensions_d = 0;											// length d
			int num_particles_p = 0;											// length p (RENAME BATCH SIZE)
			cweeThreadedList<float> upper_box_constraints;						// length d
			cweeThreadedList<float> lower_box_constraints;						// length d
			cweeThreadedList < cweeThreadedList <float >> initial_points;		// only here in the event that the user provides an initial points
			cweeThreadedList<float> dependent_bound_vector = {};				// length d, the dependent vector specifies (DOCUMENT)

			// optional variables used for sampling the normal distribution (if this sampling method is used) // 
			float global_mean = 0;
			float global_stddev = 0;
			cweeThreadedList<float> mean_by_dimension = {};
			cweeThreadedList<float> stddev_by_dimension = {};
			sampling_type default_sampling_method = _UNIFORM_RANDOM_SAMPLE_;  // uniform random sampling //
			sampling_type resampling_method_to_conform_to_batch_size = _UNIFORM_RANDOM_SAMPLE_;
			bool conform_to_particle_size = true;				// if the optimization method forces the output to be the size of the 
			DiverseSearchObject diverseSamplingRecord;
			cweeThreadedList<float> mean_for_normal_sampling = { };
			cweeThreadedList<float> stddev_for_normal_sampling = { };

			// tracking values for the set of observered points // 
			cweeThreadedList<float> recorded_values;
			cweeThreadedList < cweeThreadedList<float>> recorded_points;
			cweeThreadedList<float> last_iteration_recorded_values;
			cweeThreadedList < cweeThreadedList<float>> last_iteration_recorded_points;
			void AddRecordedValues(cweeThreadedList<float> input_values) {
				if (input_values.Num() == 0) { // if this is an empty vector then we return and do nothing 
					return;
				}
				else {
					last_iteration_recorded_values = input_values;
					return;
				}
			}
			void AddNewRecordedPoints(cweeThreadedList<cweeThreadedList<float>> input_points) {
				if (input_points.Num() == 0) { // if this is an empty vector then we return and do nothing 
					return;
				}
				else {
					last_iteration_recorded_points = input_points;
					return;
				}
			}
			void ConfirmRecordsAndReorderRecords() {
				if (last_iteration_recorded_values.Num() == 0 || last_iteration_recorded_points.Num() == 0 || last_iteration_recorded_points.Num() != last_iteration_recorded_values.Num()) {
					return;
				}
				else {
					// order the values and the points based on their output // 
					cweeThreadedList<float> recorded_values_pre_order = append_vectors(recorded_values, last_iteration_recorded_values);
					cweeThreadedList< cweeThreadedList<float>> recorded_points_pre_order = append_vector_sets(recorded_points, last_iteration_recorded_points);
					recorded_points = order_vector_values(recorded_values_pre_order, recorded_points_pre_order);
					recorded_values = order_values(recorded_values_pre_order);

					last_iteration_recorded_values.Clear();
					last_iteration_recorded_points.Clear();
					return;
				}
				return;
			}

			// random variables, used for some //
			int non_random_seed_counter;
			bool adjust_to_bounds = true;
			bool integer_only = false;

			// constructors and destructors // 
			OptimInput() {}
			OptimInput(const cweeThreadedList<float>& upper_bound, const cweeThreadedList<float>& lower_bound, int num_particles_input = 5) {
				iterations_completed = 0;
				upper_box_constraints = upper_bound;
				lower_box_constraints = lower_bound;
				num_particles_p = num_particles_input;
				return;
			}
			~OptimInput() {}

			// two completely generatlized functions that we can extend to the other classes, each type of object // 
			virtual cweeThreadedList < cweeThreadedList<float> > OutputBatchPolicyVectors() = 0;
			virtual int BatchPolicyVectorSize() = 0;
			virtual void IterateOptimizer(const cweeThreadedList<float> inputed_particle_values = {}, bool is_random = true, non_random_type_t control_rand = __NONE_NON_RANDOM__) = 0;		// we are going to write further functions to perform general iterations 
			virtual void CopyOptimizerData(void* input_optimization_information, cweeThreadedList<float> search_space_map = {}) = 0;							// copy the optimization data 
			virtual OptimType GetOptimizationType() = 0; // getting the optimization type 
			virtual cweeStr GetOptimTypeIDString() = 0;



			cweeThreadedList < cweeThreadedList<float> > ExpandContractToMatchParticleSize(cweeThreadedList < cweeThreadedList<float>>* input_vectors) {
				cweeThreadedList < cweeThreadedList<float> >output_vectors = {};
				if (input_vectors) {
					if (input_vectors->Num() < num_particles_p) {
						cweeThreadedList< cweeThreadedList <float>> additional_appeneded_vectors = ReturnRandomVectorInMode(num_particles_p - input_vectors->Num());
						output_vectors = append_vector_sets(*input_vectors, additional_appeneded_vectors);
					}
					else if (input_vectors->Num() > num_particles_p) {
						output_vectors = subsetFromTheFrontVector(*input_vectors, num_particles_p);
					}
				}
				return output_vectors;
			}
			cweeThreadedList < cweeThreadedList < float >> ReturnRandomVectorInMode(int size_output = -1, sampling_type alternative_mode = _NO_SAMPLING_,
				cweeThreadedList < float > alternative_upper_bounds = {}, cweeThreadedList < float > alternative_lower_bounds = {},
				cweeThreadedList < float > dependent_bounds_upper = {}, cweeThreadedList < float > dependent_bounds_lower = {},
				bool random = true, non_random_type_t type = __NONE_NON_RANDOM__, int seed_number = -1) {

				// //
				cweeThreadedList < cweeThreadedList < float >> output;
				cweeThreadedList < float > upper_bounds_used, lower_bounds_used;
				sampling_type method_used;

				// check any of the inputs values // 
				if (size_output == -1) { size_output = num_particles_p; }
				if (alternative_mode == _NO_SAMPLING_) { method_used = default_sampling_method; }

				// check to see if the vectors for the upper and lower bounds are empty
				if (alternative_upper_bounds.Num() == 0) { upper_bounds_used = upper_box_constraints; }
				if (alternative_lower_bounds.Num() == 0) { lower_bounds_used = lower_box_constraints; }
				if (mean_for_normal_sampling.Num() == 0) {
					for (int index_bounds = 0; index_bounds < lower_bounds_used.Size(); index_bounds++) { mean_for_normal_sampling.Append((upper_bounds_used[index_bounds] - lower_bounds_used[index_bounds]) / 2 + lower_bounds_used[index_bounds]); }
				}
				if (stddev_for_normal_sampling.Num() == 0) {
					for (int index_bounds = 0; index_bounds < lower_bounds_used.Size(); index_bounds++) { stddev_for_normal_sampling.Append(1); }
				}
				// switch between the modes for developing 
				switch (default_sampling_method) {
				case _UNIFORM_RANDOM_SAMPLE_: {
					output = sampleUniformSet(size_output, upper_box_constraints, lower_box_constraints, dependent_bounds_upper, dependent_bounds_lower, random, type, seed_number);
				}
				case _NORMAL_GLOBAL_PARAM_: {
					output = sampleNormalSet(size_output, upper_box_constraints, lower_box_constraints, mean_for_normal_sampling, stddev_for_normal_sampling, dependent_bounds_upper, dependent_bounds_lower, random, type, seed_number);
				}
				case _NORMAL_PARAM_PER_DIM_: {
					output = sampleNormalSet(size_output, upper_box_constraints, lower_box_constraints, mean_for_normal_sampling, stddev_for_normal_sampling, dependent_bounds_upper, dependent_bounds_lower, random, type, seed_number);
				}
				case _UNIFORM_DIVERSE_: {
					DiverseSearchObject output_diverse_set = sampleDiverseSet(size_output, diverseSamplingRecord, upper_box_constraints, lower_box_constraints); // sample from the diverse set // 
					output = output_diverse_set.sampled_points;
					diverseSamplingRecord = output_diverse_set; // update the diverse set of points //
				}
				}
				return output;
			}
		};
		cweeThreadedList< cweeThreadedList < float >> static expandSampleSetToMeetBatchSize(int batch_size, cweeThreadedList< cweeThreadedList < float >> input_vector_set,
			cweeThreadedList < float > upper_bound = {}, cweeThreadedList < float > lower_bound = {}, sampling_type sampling_method_input = _UNIFORM_RANDOM_SAMPLE_,
			cweeThreadedList < float > dependent_bounds_upper = {}, cweeThreadedList < float >  dependent_bounds_lower = {}, bool random = true, non_random_type_t type = __NONE_NON_RANDOM__, int seed_number = 1) {
			switch (sampling_method_input) {
			case _NO_SAMPLING_: {
				return input_vector_set;  // if there is no sampling method we return the vector as is //
			}
			case _UNIFORM_RANDOM_SAMPLE_: {
				// append_vector_sets(const cweeThreadedList < cweeThreadedList<float>>& front_vectors, const cweeThreadedList < cweeThreadedList<float>>& back_vectors) // 
				return append_vector_sets(input_vector_set, sampleUniformSet(batch_size - input_vector_set.Num(), upper_bound, lower_bound, dependent_bounds_upper, dependent_bounds_lower, random, type, seed_number));
			}
			case _NORMAL_FROM_CENTROID_: { // calculate the centroid of the 
				if (input_vector_set.Num() > 0) {
					return append_vector_sets(input_vector_set, sampleNormalSet(batch_size - input_vector_set.Num(), upper_bound, lower_bound, GetCentroidPoints(input_vector_set), get_stddev_from_vector_set(input_vector_set), dependent_bounds_upper, dependent_bounds_lower, random, type, seed_number));
				}
				else {
					return append_vector_sets(input_vector_set, sampleNormalSet(batch_size - input_vector_set.Num(), upper_bound, lower_bound, vector_repeated_scalar_values(upper_bound.Num(), 0), vector_repeated_scalar_values(upper_bound.Num(), 1), dependent_bounds_upper, dependent_bounds_lower, random, type, seed_number));
				}
			}
			case _UNIFORM_DIVERSE_: {
				return append_vector_sets(input_vector_set, sampleUniformSet(batch_size - input_vector_set.Num(), upper_bound, lower_bound, dependent_bounds_upper, dependent_bounds_lower, random, type, seed_number)); // 
			}
			}
		}

		// Return the a random vecot of the specified size //
		template<typename OPTIM_TYPE>
		void static CopyOptimizerBaseData(OPTIM_TYPE* input_optimization_information, OPTIM_TYPE* output_optimization_information, cweeThreadedList<float> search_space_map = {}) {
			if (input_optimization_information != nullptr && output_optimization_information != nullptr) {
				if (search_space_map.Num() == 0) {
					output_optimization_information->iterations_completed = input_optimization_information->iterations_completed;
					output_optimization_information->num_dimensions_d = input_optimization_information->num_dimensions_d;
					output_optimization_information->num_particles_p = input_optimization_information->num_particles_p;
					output_optimization_information->upper_box_constraints = input_optimization_information->upper_box_constraints;
					output_optimization_information->lower_box_constraints = input_optimization_information->lower_box_constraints;
					output_optimization_information->initial_points = input_optimization_information->initial_points;
					output_optimization_information->dependent_bound_vector = input_optimization_information->dependent_bound_vector;
					output_optimization_information->global_mean = input_optimization_information->global_mean;
					output_optimization_information->global_stddev = input_optimization_information->global_stddev;
					output_optimization_information->mean_by_dimension = input_optimization_information->mean_by_dimension;
					output_optimization_information->stddev_by_dimension = input_optimization_information->stddev_by_dimension;
					output_optimization_information->default_sampling_method = input_optimization_information->default_sampling_method;
					output_optimization_information->resampling_method_to_conform_to_batch_size = input_optimization_information->resampling_method_to_conform_to_batch_size;
					output_optimization_information->conform_to_particle_size = input_optimization_information->conform_to_particle_size;
					output_optimization_information->diverseSamplingRecord = input_optimization_information->diverseSamplingRecord;
					output_optimization_information->mean_for_normal_sampling = input_optimization_information->mean_for_normal_sampling;
					output_optimization_information->stddev_for_normal_sampling = input_optimization_information->stddev_for_normal_sampling;
					output_optimization_information->non_random_seed_counter = input_optimization_information->non_random_seed_counter;
					output_optimization_information->adjust_to_bounds = input_optimization_information->adjust_to_bounds;
					output_optimization_information->integer_only = input_optimization_information->integer_only;
				}
			}
			return; // return from the copy 
		}

#pragma endregion 
#if 0
		/* The Nelder Meade Local Optimization Method*/
#pragma region RANDOM_SEARCH 
		/* Random Search Objects Begin */
		class RandomSearchInput : public OptimInput {
		public:
			cweeThreadedList< cweeThreadedList<float>> output_policy_sample; // output the policy samples
			bool integer_only = false;

			RandomSearchInput() {}
			~RandomSearchInput() {}
			RandomSearchInput(const cweeThreadedList<float>& upper_bound, const cweeThreadedList<float>& lower_bound, int num_particles_in = 5, sampling_type input = _UNIFORM_RANDOM_SAMPLE_) : OptimInput(upper_bound, lower_bound, num_particles_in)
			{ // there is no specific implementation for the random search input
			}

			cweeStr GetOptimTypeIDString() { // a debug function to return a string form the object 
				return "optimize_Random";
			}
			void CopyOptimizerData(void* input_optimization_information, cweeThreadedList<float> search_space_map) {
				// the basic parent class information copied  //
				if (input_optimization_information != nullptr) {
					CopyOptimizerBaseData((OptimInput*)input_optimization_information, (OptimInput*)this, search_space_map);
					output_policy_sample = output_policy_sample; // output the policy samples
					integer_only = integer_only;
				}
				return;
			}
			OptimType GetOptimizationType() { return cweeMath::Optim::OptimType::OPT_RANDOM; }   // return the the optimization type // 
			/*! output the batch size and batch locations */
			cweeThreadedList < cweeThreadedList<float> > OutputBatchPolicyVectors() { return output_policy_sample; }
			/*! Updated the Pure Random Search function with the new helper functions */
			void Pure_Random_Search_Iteration_Update_N(bool random = true, non_random_type_t type = __NONE_NON_RANDOM__) {
				output_policy_sample.Clear();
				for (int batch_index = 0; batch_index < num_particles_p; batch_index++) {
					output_policy_sample.Append(GetUniformRandomVector(upper_box_constraints, lower_box_constraints, random, type)); // 
				}
				iterations_completed++;
				return;
			};
			void Pure_Random_Search_Iteration_NormalRadial_N(float mean, float stddev, bool random = true, non_random_type_t type = __NONE_NON_RANDOM__) {
				for (int batch_index = 0; batch_index < num_particles_p; batch_index++) {
					output_policy_sample.Clear();
					output_policy_sample.Append(
						round_vector_to_nearest_bound(upper_box_constraints, lower_box_constraints, DetermineNormalRandomVector(mean, stddev, upper_box_constraints.Num(), random, type))); // 
				}
				iterations_completed++;
				return;	// return from the function and determine 
			}
			void Pure_Random_Search_Iteration_NormalRadial_Vector_N(cweeThreadedList<float> mean, cweeThreadedList<float> stddev, bool random = true, non_random_type_t type = __NONE_NON_RANDOM__) {
				output_policy_sample.Clear();
				output_policy_sample = DetermineNormalRandomVector_vectorInput(mean, stddev, random, type, non_random_seed_counter);
				iterations_completed++;
				return;	// return from the function and determine 
			}
			int BatchPolicyVectorSize() { return output_policy_sample.Num(); }
			void IterateOptimizer(const cweeThreadedList<float> inputed_particle_values = {}, bool is_random = true, non_random_type_t control_rand = __NONE_NON_RANDOM__) { // there are four modes through which we can 
				AddRecordedValues(inputed_particle_values);		// add the record values to the main object // 
				ConfirmRecordsAndReorderRecords();				//


				if (default_sampling_method == _UNIFORM_RANDOM_SAMPLE_) {
					Pure_Random_Search_Iteration_Update_N(is_random, control_rand);
				}
				else if (default_sampling_method == _NORMAL_GLOBAL_PARAM_) {
					Pure_Random_Search_Iteration_NormalRadial_N(global_mean, global_stddev, is_random, control_rand);
				}
				else if (default_sampling_method == _NORMAL_PARAM_PER_DIM_) {
					Pure_Random_Search_Iteration_NormalRadial_Vector_N(mean_by_dimension, stddev_by_dimension, is_random, control_rand);
				}
				else if (default_sampling_method == _UNIFORM_DIVERSE_) {
					// not implemented yet // 
				}
				AddNewRecordedPoints(OutputBatchPolicyVectors());  // add the recorded points to the object // 
				return;
			}
		};
		void static InitializeRandomSearchInput(RandomSearchInput* rand_input, const cweeThreadedList<float>& upper_bound, const cweeThreadedList<float>& lower_bound) {
			rand_input->iterations_completed = 0;
			rand_input->upper_box_constraints = upper_bound;
			rand_input->lower_box_constraints = lower_bound;
			return;
		};
		static void Pure_Random_Search_Iteration_Update(RandomSearchInput* parameters, bool random = true, non_random_type_t type = __NONE_NON_RANDOM__) {
			parameters->output_policy_sample.Clear();
			for (int batch_index = 0; batch_index < parameters->num_particles_p; batch_index++) {
				parameters->output_policy_sample.Append(GetUniformRandomVector(parameters->upper_box_constraints, parameters->lower_box_constraints, random, type)); // 
			}
			parameters->iterations_completed++;
			return;
		};
		static void Pure_Random_Search_Iteration_NormalRadial(RandomSearchInput* parameters, float mean, float stddev, bool random = true, non_random_type_t type = __NONE_NON_RANDOM__) {
			for (int batch_index = 0; batch_index < parameters->num_particles_p; batch_index++) {
				parameters->output_policy_sample.Clear();
				parameters->output_policy_sample.Append(
					round_vector_to_nearest_bound(parameters->upper_box_constraints, parameters->lower_box_constraints, DetermineNormalRandomVector(mean, stddev, parameters->upper_box_constraints.Num(), random, type))); // 
			}
			parameters->iterations_completed++;
			return;																									 // return from the function and determine 
		};

#pragma endregion
		/* Particle Swarm Optimization Algorithm */
#pragma region NELDER_MEAD
/* the Nelder-Meade method of pattern search */
		enum NM_Type { __NO_NMTPE__, __INITIAL__, __REFLECT__, __EXPAND__, __CONTRACT__, __SHRINK__ }; // there are four different types of iteration 
		class NelderMeadUpdate : public OptimInput {
		public:
			float alpha_reflect = 1;				// alpha > 0  (reflection) 
			float xi_expand = 2;				// xi > max(alpha, 1) (expansion) 
			float gam_contract = 0.5;				// 0 < gam < 1 (contraction)
			float sig_shrink = 0.5;				// 0 < sig < 1 (shrink factor)
			NM_Type previous_mode_for_run = __NO_NMTPE__;		//
			NM_Type mode_for_run = __INITIAL__;		//

			// the points for each of the new modes for the sampling // 
			cweeThreadedList <float> sorted_save_simplex_values;						// these are the values for the new simplex (always sorted after each iteration) // 
			cweeThreadedList< cweeThreadedList <float>> sorted_save_simplex_points;	// the saved reference points for this method of optimization (always sorted after each iteration) //  
			cweeThreadedList <float> sorted_save_simplex_values_minus_worst;
			cweeThreadedList< cweeThreadedList <float>> sorted_save_simplex_points_minus_worst;
			cweeThreadedList <float> worst_simplex_point;
			float worst_simplex_value;
			cweeThreadedList <float> simplex_centroid_x0;

			// cweeThreadedList< cweeThreadedList <float>> initial_points;
			cweeThreadedList< cweeThreadedList <float>> initial_simplex_points;
			cweeThreadedList< cweeThreadedList <float>> reflect_points, expand_points, contract_points, shrink_points;

			cweeStr GetOptimTypeIDString() { // a debug function to return a string form the object 
				return "optimize_Nelder_Meade";
			}

			// sorted vectors that are used for the reflection (these are used in between iterations) // 
			cweeThreadedList< cweeThreadedList <float>> sorted_reflection_points;
			cweeThreadedList <float> sorted_reflection_values;
			cweeThreadedList <float> best_reflect_point;
			float best_reflect_value;

			// Constructors and Destructors // 
			NelderMeadUpdate() {}
			NelderMeadUpdate(const cweeThreadedList<float>& upper_bound, const cweeThreadedList<float>& lower_bound, int num_particles_in, sampling_type input = _UNIFORM_RANDOM_SAMPLE_) : OptimInput(upper_bound, lower_bound, num_particles_in) {}
			~NelderMeadUpdate() {}

			void CopyOptimizerData(void* input_optimization_information, cweeThreadedList<float> search_space_map) {
				if (input_optimization_information != nullptr) {
					CopyOptimizerBaseData((OptimInput*)input_optimization_information, (OptimInput*)this, search_space_map);
					alpha_reflect = alpha_reflect;
					xi_expand = xi_expand;
					gam_contract = gam_contract;
					sig_shrink = sig_shrink;
					previous_mode_for_run = previous_mode_for_run;
					mode_for_run = mode_for_run;
					sorted_save_simplex_values = sorted_save_simplex_values;						// these are the values for the new simplex (always sorted after each iteration) // 
					sorted_save_simplex_points = sorted_save_simplex_points;	// the saved reference points for this method of optimization (always sorted after each iteration) //  
					sorted_save_simplex_values_minus_worst = sorted_save_simplex_values_minus_worst;
					sorted_save_simplex_points_minus_worst = sorted_save_simplex_points_minus_worst;
					worst_simplex_point = worst_simplex_point;
					worst_simplex_value = worst_simplex_value;
					simplex_centroid_x0 = simplex_centroid_x0;
					initial_simplex_points = initial_simplex_points;
					reflect_points = reflect_points;
					expand_points = expand_points;
					contract_points = contract_points;
					shrink_points = shrink_points;
					sorted_reflection_points = sorted_reflection_points;
					sorted_reflection_values = sorted_reflection_values;
					best_reflect_point = best_reflect_point;
					best_reflect_value = best_reflect_value;
				}
				return;
			}
			OptimType GetOptimizationType() { return cweeMath::Optim::OptimType::OPT_NELDER_MEADE; }   // return the the optimization type // 
			void IterateOptimizer(const cweeThreadedList<float> inputed_particle_values = {}, bool is_random = true, non_random_type_t control_rand = __NONE_NON_RANDOM__) {

				// record the iteration in the object //
				AddRecordedValues(inputed_particle_values);		// add the record values to the main object // 
				ConfirmRecordsAndReorderRecords();				//

				// make sure that we have the proper requirements for each of the rune modes // 
				if ((previous_mode_for_run == __REFLECT__ && inputed_particle_values.Num() != num_particles_p)
					|| (previous_mode_for_run == __EXPAND__ && inputed_particle_values.Num() != num_particles_p)
					|| (previous_mode_for_run == __CONTRACT__ && inputed_particle_values.Num() != num_particles_p)
					|| (previous_mode_for_run == __SHRINK__ && inputed_particle_values.Num() != num_particles_p)) {
					previous_mode_for_run = __INITIAL__; // if we have no inputed values then we need to run this in an "INITIAL" mode //
				}

				// switch on handing the previous mode to make sure that we are processing the batch correcting  // 
				switch (previous_mode_for_run) {
				case __NO_NMTPE__: {
					sorted_save_simplex_points.Clear(); // clear the list of the saved points 
					sorted_save_simplex_values.Clear();
				}
				case __INITIAL__: {
					sorted_save_simplex_values = order_values(inputed_particle_values);
					sorted_save_simplex_points = order_vector_values(inputed_particle_values, initial_simplex_points); // apend the vector
					sorted_save_simplex_points_minus_worst = subsetFromTheFrontVector(sorted_save_simplex_points, sorted_save_simplex_points.Num() - 1);
					sorted_save_simplex_values_minus_worst = subsetFromTheFront(sorted_save_simplex_values, sorted_save_simplex_values.Num() - 1);
					simplex_centroid_x0 = GetCentroidPoints(sorted_save_simplex_points_minus_worst);  // determine the new centroid
					mode_for_run = __REFLECT__;  // next stage will be to determine the centroid and reflect
				}
				case __REFLECT__: { // perform a new switch statement based on how the reflection is performed, we either expand, contract, or shrink // 

					sorted_reflection_points = order_vector_values(inputed_particle_values, reflect_points);
					sorted_reflection_values = order_values(inputed_particle_values);
					best_reflect_value = sorted_reflection_values[0];
					best_reflect_point = sorted_reflection_points[0];

					if (best_reflect_value < sorted_save_simplex_values[0]) {
						// IF WE ARE BEST THEN WE MOVE ON TO EXPAND // 
						mode_for_run = __EXPAND__;
					}
					else if (best_reflect_value > sorted_save_simplex_values[0] && best_reflect_value < sorted_save_simplex_values_minus_worst[sorted_save_simplex_values_minus_worst.Num() - 1]) {
						// add the reflection point to the simplex // 
						sorted_save_simplex_values = order_values(append_vectors(sorted_save_simplex_values_minus_worst, best_reflect_value));
						sorted_save_simplex_points = order_vector_values(append_vectors(sorted_save_simplex_values_minus_worst, best_reflect_value), append_vector_sets(sorted_save_simplex_points_minus_worst, best_reflect_point)); // apend the vectors 
						sorted_save_simplex_points_minus_worst = subsetFromTheFrontVector(sorted_save_simplex_points, sorted_save_simplex_points.Num() - 1);
						sorted_save_simplex_values_minus_worst = subsetFromTheFront(sorted_save_simplex_values, sorted_save_simplex_values.Num() - 1);
						simplex_centroid_x0 = GetCentroidPoints(sorted_save_simplex_points_minus_worst);
						mode_for_run = __REFLECT__;
					}
					else {
						// ELSE CONTRACT THE SOLUTION 
						mode_for_run = __CONTRACT__;
					}
				}
				case __EXPAND__: {
					cweeThreadedList<cweeThreadedList<float>>sorted_epanded_points = order_vector_values(inputed_particle_values, reflect_points);
					cweeThreadedList<float> sorted_expanded_values = order_values(inputed_particle_values);
					float best_epanded_value = sorted_reflection_values[0];
					cweeThreadedList<float> best_expand_point = sorted_epanded_points[0];

					if (best_epanded_value < best_reflect_value) {
						// add the expanded value to the new simplex 
						sorted_save_simplex_values = order_values(append_vectors(sorted_save_simplex_values_minus_worst, best_epanded_value));
						sorted_save_simplex_points = order_vector_values(append_vectors(sorted_save_simplex_values_minus_worst, best_epanded_value), append_vector_sets(sorted_save_simplex_points_minus_worst, best_expand_point)); // apend the vectors 
						sorted_save_simplex_points_minus_worst = subsetFromTheFrontVector(sorted_save_simplex_points, sorted_save_simplex_points.Num() - 1);
						sorted_save_simplex_values_minus_worst = subsetFromTheFront(sorted_save_simplex_values, sorted_save_simplex_values.Num() - 1);
						simplex_centroid_x0 = GetCentroidPoints(sorted_save_simplex_points_minus_worst);
						mode_for_run = __REFLECT__;
					}
					else {
						sorted_save_simplex_values = order_values(append_vectors(sorted_save_simplex_values_minus_worst, best_epanded_value));
						sorted_save_simplex_points = order_vector_values(append_vectors(sorted_save_simplex_values_minus_worst, best_epanded_value), append_vector_sets(sorted_save_simplex_points_minus_worst, best_expand_point)); // apend the vectors 
						sorted_save_simplex_points_minus_worst = subsetFromTheFrontVector(sorted_save_simplex_points, sorted_save_simplex_points.Num() - 1);
						sorted_save_simplex_values_minus_worst = subsetFromTheFront(sorted_save_simplex_values, sorted_save_simplex_values.Num() - 1);
						simplex_centroid_x0 = GetCentroidPoints(sorted_save_simplex_points_minus_worst);
						mode_for_run = __REFLECT__;
					}
					mode_for_run = __REFLECT__;
				}
				case __CONTRACT__: {
					cweeThreadedList<cweeThreadedList<float>>sorted_contract_points = order_vector_values(inputed_particle_values, reflect_points);
					cweeThreadedList<float> sorted_contract_values = order_values(inputed_particle_values);
					float best_contract_value = sorted_contract_values[0];
					cweeThreadedList<float> best_contract_point = sorted_contract_points[0];

					if (best_contract_value < worst_simplex_value) {
						// add the contracted value to the new simplex 
						sorted_save_simplex_values = order_values(append_vectors(sorted_save_simplex_values_minus_worst, best_contract_value));
						sorted_save_simplex_points = order_vector_values(append_vectors(sorted_save_simplex_values_minus_worst, best_contract_value), append_vector_sets(sorted_save_simplex_points_minus_worst, best_contract_point)); // apend the vectors 
						sorted_save_simplex_points_minus_worst = subsetFromTheFrontVector(sorted_save_simplex_points, sorted_save_simplex_points.Num() - 1);
						sorted_save_simplex_values_minus_worst = subsetFromTheFront(sorted_save_simplex_values, sorted_save_simplex_values.Num() - 1);
						simplex_centroid_x0 = GetCentroidPoints(sorted_save_simplex_points_minus_worst);
						mode_for_run = __REFLECT__;
					}
					else {
						mode_for_run = __SHRINK__;
					}
				}
				case __SHRINK__: {
					cweeThreadedList<cweeThreadedList<float>> new_simplex_points;
					cweeThreadedList<float> new_simplex_values;

					// add the best point //
					new_simplex_points.Append(sorted_save_simplex_points[0]);
					new_simplex_values.Append(sorted_save_simplex_values[0]);

					// add the new shrink values //
					for (int shrink_input_index = 0; shrink_input_index < shrink_points.Num(); shrink_input_index++) {
						new_simplex_points.Append(shrink_points[shrink_input_index]);
						new_simplex_values.Append(inputed_particle_values[shrink_input_index]);
					}

					// sort the new points // 
					sorted_save_simplex_values = order_values(new_simplex_values);
					sorted_save_simplex_points = order_vector_values(new_simplex_values, new_simplex_points); // apend the vecto
					sorted_save_simplex_points_minus_worst = subsetFromTheFrontVector(sorted_save_simplex_points, sorted_save_simplex_points.Num() - 1);
					sorted_save_simplex_values_minus_worst = subsetFromTheFront(sorted_save_simplex_values, sorted_save_simplex_values.Num() - 1);
					simplex_centroid_x0 = GetCentroidPoints(sorted_save_simplex_points_minus_worst);
					mode_for_run = __REFLECT__;
				}
				}
				switch (mode_for_run) {
				case __INITIAL__: { // completely reset the vectors // 
					initial_simplex_points = ReturnRandomVectorInMode(num_particles_p); // get the initial vectors 		
					previous_mode_for_run = __INITIAL__;
					//AddRecordedPoints(initial_simplex_points);
				}
				case __REFLECT__: { // rename this as something like FIRST_PART_BASE_ITERATION //
					reflect_points.Clear();
					reflect_points.Append(add_vec(simplex_centroid_x0, scalar_vector_multiply(alpha_reflect, subtract_vec(simplex_centroid_x0, worst_simplex_point))));  // perform the reflection operation x_r = x_0 + alpha * (x_0 - x_n+1 )					
					previous_mode_for_run = __REFLECT__;
				}
				case __EXPAND__: {
					// are we comiong from an expansion step ? 
					expand_points.Append(add_vec(simplex_centroid_x0, scalar_vector_multiply(xi_expand, subtract_vec(best_reflect_point, simplex_centroid_x0))));  // perform the reflection operation x_r = x_0 + alpha * (x_0 - x_n+1 )
					previous_mode_for_run = __EXPAND__;
				}
				case __CONTRACT__: {
					//
					contract_points.Append(add_vec(simplex_centroid_x0, scalar_vector_multiply(gam_contract, subtract_vec(worst_simplex_point, simplex_centroid_x0))));  // perform the reflection operation x_r = x_0 + alpha * (x_0 - x_n+1 )
					previous_mode_for_run = __CONTRACT__;
				}
				case __SHRINK__: {
					// shrink all points but the best
					for (int index_simplex_points = 1; index_simplex_points < sorted_save_simplex_points.Num(); index_simplex_points++) {
						shrink_points.Append(add_vec(sorted_save_simplex_points[0], scalar_vector_multiply(gam_contract, subtract_vec(sorted_save_simplex_points[index_simplex_points], sorted_save_simplex_points[0]))));
					}
					previous_mode_for_run = __SHRINK__;
				}
				}

				// add the new points //
				AddNewRecordedPoints(OutputBatchPolicyVectors());  // add the recorded points to the object // 
				return;
			}
			cweeThreadedList < cweeThreadedList<float> > OutputBatchPolicyVectors() {
				cweeThreadedList < cweeThreadedList<float> > output;
				switch (previous_mode_for_run) {
				case __INITIAL__: {
					output = initial_simplex_points;
				}
				case __REFLECT__: {
					output = reflect_points;
				}
				case __EXPAND__: {
					output = expand_points;
				}
				case __CONTRACT__: {
					output = contract_points;
				}
				case __SHRINK__: {
					output = shrink_points;
				}
				}
				return output; // return the output
			}
			int BatchPolicyVectorSize() {
				switch (mode_for_run) {
				case __INITIAL__: {
					return initial_simplex_points.Num();
				}
				case __REFLECT__: {
					return reflect_points.Num();
				}
				case __EXPAND__: {
					return expand_points.Num();
				}
				case __CONTRACT__: {
					return contract_points.Num();
				}
				case __SHRINK__: {
					return shrink_points.Num();
				}
				}
			}
		};
#pragma endregion
		/* The Random Search Algorithm */
#pragma region PSO_CODE 
		/* Particle Swarm Code */
		class  PSOInput_Update : public OptimInput {

		public:
			// the particles location ( NOT USED YET, BUT THIS WILL MAKE THE REFACTOR MORE CLEAR) //
			cweeThreadedList < cweeThreadedList< float >> particle_location;
			cweeThreadedList< float > particle_value;
			cweeThreadedList < cweeThreadedList< float >> particle_velocity;

			// the best values and position indexed by particle number //
			cweeThreadedList< float > best_local_particle_value;							// length p
			cweeThreadedList< cweeThreadedList <float> > best_local_particle_positions;		// length p x d

			// global information //
			float best_global_particle_value;
			cweeThreadedList<float> best_global_particle_location;							// length d								

			// optimization parameters //
			float g_cor = 0.5, l_cor = 0.5, inertia = 0.5;									// the local and global intertia
			float percent_length_initial_velocity = 0.25;
			bool use_random = true;

			// output the batch size and batch locations // 
			PSOInput_Update() {}
			PSOInput_Update(const cweeThreadedList<float>& upper_bound, const cweeThreadedList<float>& lower_bound, int num_particles_in = 5, sampling_type input = _UNIFORM_RANDOM_SAMPLE_) : OptimInput(upper_bound, lower_bound, num_particles_in) {}
			~PSOInput_Update() {}

			cweeStr GetOptimTypeIDString() { // a debug function to return a string form the object 
				return "optimize_PSO";
			}
			void CopyOptimizerData(void* input_optimization_information, cweeThreadedList<float> search_space_map) {
				if (input_optimization_information != nullptr) {
					CopyOptimizerBaseData((OptimInput*)input_optimization_information, (OptimInput*)this, search_space_map);
					particle_location = particle_location;
					particle_value = particle_value;
					particle_velocity = particle_velocity;
					best_local_particle_value = best_local_particle_value;
					best_local_particle_positions = best_local_particle_positions;
					best_global_particle_value = best_global_particle_value;
					best_global_particle_location = best_global_particle_location;
					g_cor = g_cor;
					l_cor = l_cor;
					inertia = inertia;								// the local and global intertia
					percent_length_initial_velocity = percent_length_initial_velocity;
					use_random = use_random;
				}
			}
			OptimType GetOptimizationType() { return cweeMath::Optim::OptimType::OPT_PSO; }   // return the the optimization type // 
			cweeThreadedList < cweeThreadedList<float> > OutputBatchPolicyVectors() { return particle_location; }
			int BatchPolicyVectorSize() { return particle_location.Num(); }
			void IterateOptimizer(const cweeThreadedList<float> inputed_particle_values = {}, bool is_random = true, non_random_type_t control_rand = __NONE_NON_RANDOM__) {

				AddRecordedValues(inputed_particle_values);		// add the record values to the main object // 
				ConfirmRecordsAndReorderRecords();				//

				// STEP 0, SET UP ALL NEW VALUES IF THERE IS NO PREVIOUS ITERATION (DEFAULT) // 
				if (iterations_completed == 0 ||
					inputed_particle_values.Num() == 0 ||
					particle_velocity.Num() != num_particles_p ||
					particle_location.Num() != num_particles_p ||
					best_local_particle_value.Num() != num_particles_p ||
					particle_value.Num() != num_particles_p ||
					best_local_particle_positions.Num() != num_particles_p) {

					// 0.1, all previous vectors concerning used to track system information  // 
					particle_velocity.Clear();
					particle_location.Clear();
					particle_value.Clear();
					best_local_particle_positions.Clear();
					best_local_particle_value.Clear();

					// 0.2, create the initial locations and values //
					for (int index_p = 0; index_p < num_particles_p; index_p++) {
						cweeThreadedList<float> upper_velocity_bounds = vector_repeated_scalar_values(num_dimensions_d, percent_length_initial_velocity * (get_absolute_max(upper_box_constraints) - get_min(lower_box_constraints))); // 
						cweeThreadedList<float> lower_velocity_bounds = vector_repeated_scalar_values(num_dimensions_d, -1 * percent_length_initial_velocity * (get_absolute_max(upper_box_constraints) - get_min(lower_box_constraints)));
						particle_velocity.Append(GetUniformRandomVector(upper_velocity_bounds, lower_velocity_bounds, is_random, control_rand, non_random_seed_counter + index_p * num_dimensions_d));
						particle_location.Append(GetUniformRandomVector(upper_box_constraints, lower_box_constraints, is_random, control_rand, non_random_seed_counter + index_p * num_dimensions_d));
						particle_value.Append(PSEUDO_MAX);  // we use a pseudo max since this value is currently unassigned // 
					}
					// 0.3, replace the random vectors with provided initial points if available // 
					for (int index_initial_points = 0; index_initial_points < initial_points.Num(); index_initial_points++) {
						particle_location[index_initial_points] = initial_points[index_initial_points];
					}
					for (int index_p = 0; index_p < num_particles_p; index_p++) {
						best_local_particle_positions.Append(particle_location[index_p]);
						best_local_particle_value.Append(PSEUDO_MAX);
					}
					for (int index_particles = 0; index_particles < num_particles_p; index_particles++) {
						if (integer_only) {
							particle_location[index_particles] = round_vector_to_nearest_int(particle_location[index_particles]);
						}
						if (adjust_to_bounds) {
							particle_location[index_particles] = round_vector_to_nearest_bound(particle_location[index_particles], upper_box_constraints, lower_box_constraints);
						}
					}
					iterations_completed++;
					return;
				}
				else {   // 0.4, set up all of the previous values for the particles are set up
					particle_value = inputed_particle_values;
				}

				// STEP 1, DETERMINE THE BEST POINTS  //
				//  1.1, determine the best local points // 
				for (int index_particles = 0; index_particles < num_particles_p; index_particles++) {
					if (particle_value[index_particles] < best_local_particle_value[index_particles]) {
						best_local_particle_value[index_particles] = particle_value[index_particles];
						best_local_particle_positions[index_particles] = particle_location[index_particles];
					}
				}

				// 1.2, determine the best global point
				for (int index_particles = 0; index_particles < num_particles_p; index_particles++) {
					if (best_local_particle_value[index_particles] < best_global_particle_value) {
						best_global_particle_value = best_local_particle_value[index_particles];
						best_global_particle_location = best_local_particle_positions[index_particles];
					}
				}

				// STEP 2, UPDATE THE VEOCITY AND POSITION // 
				// 2.0, reset the current velocity 
				cweeThreadedList < cweeThreadedList<float>> previous_velocity = particle_velocity;
				cweeThreadedList < cweeThreadedList<float>> previous_location = particle_location;
				particle_velocity.Clear();
				particle_location.Clear();
				for (int index_particles = 0; index_particles < num_particles_p; index_particles++) {
					cweeThreadedList<float> new_velocity;
					for (int index_dims = 0; index_dims < num_dimensions_d; index_dims++) { //
						float random_loc, random_global;

						// 2.1, determine the random elements in updating the velocity //
						if (use_random) {
							random_loc = cweeRandomFloat(0, 1);
							random_global = cweeRandomFloat(0, 1);
						}
						else {
							random_loc = 0.5;
							random_global = 0.5;
						}
						// 2.2, apply the update as a scaled linear combination of traveling towards the global best point and the local best point //
						float vel_comp = inertia * previous_velocity[index_particles][index_dims]
							+ random_loc * l_cor * (best_local_particle_positions[index_particles][index_dims] - previous_location[index_particles][index_dims])
							+ random_global * g_cor * (best_global_particle_location[index_dims] - previous_location[index_particles][index_dims]);
						new_velocity.Append(vel_comp);
					}
					particle_velocity.Append(new_velocity);
				}

				// STEP 3, MOVE THE PARTCILES BY THE VELOCITY (UPDATE) AND RETURN THE NEW POTENTIAL POINTS // 
				for (int index_particles = 0; index_particles < num_particles_p; index_particles++) {
					particle_location.Append(add_vec(particle_velocity[index_particles], previous_location[index_particles]));
				}

				// STEP 4, MAKE SPECIFIED ADJUSTMENT FOR VECTORS //
				for (int index_particles = 0; index_particles < num_particles_p; index_particles++) {
					if (integer_only) {
						particle_location[index_particles] = round_vector_to_nearest_int(particle_location[index_particles]);
					}
					if (adjust_to_bounds) {
						particle_location[index_particles] = round_vector_to_nearest_bound(particle_location[index_particles], upper_box_constraints, lower_box_constraints);
					}
				}

				AddNewRecordedPoints(OutputBatchPolicyVectors());  // add the recorded points to the object // 
				iterations_completed++;
				return;
			}
		};
		static void InitializePSOInput_Update(PSOInput_Update* pso_input, const cweeThreadedList<float>& ub, const cweeThreadedList<float>& lb, const cweeThreadedList < cweeThreadedList<float> >& initial_points_inputs = {}) { // initialize the 
			if (pso_input == nullptr) {  // check to see if the memory is not allocated
				return;
			}
			else {
				// the upper and lower bounds //
				pso_input->upper_box_constraints = ub;
				pso_input->lower_box_constraints = lb;
				pso_input->num_dimensions_d = ub.Num();
				pso_input->best_global_particle_value = PSEUDO_MAX;
				pso_input->initial_points = initial_points_inputs;

				// clear the data from the best vectors //
				pso_input->best_local_particle_positions.Clear(); // clear the input vectors (in case they are already allocated)
				pso_input->best_local_particle_value.Clear();
				//	for (int dimension_index = 0; dimension_index < ub.Num(); dimension_index++) {
				//	}
				pso_input->percent_length_initial_velocity = 1;
			}
		}
		static void Particle_Swarm_Iteration_Refactor(PSOInput_Update* pso_input, const cweeThreadedList<float> inputed_particle_values = {}, bool is_random = true, non_random_type_t control_rand = __NONE_NON_RANDOM__) {
			if (pso_input == nullptr) {  // check to see if the memory is not allocated
				return; // this does nothing is the pointer is null // 
			}
			else {
				// STEP 0, SET UP ALL NEW VALUES IF THERE IS NO PREVIOUS ITERATION (DEFAULT) // 
				if (pso_input->iterations_completed == 0 ||
					inputed_particle_values.Num() == 0 ||
					pso_input->particle_velocity.Num() != pso_input->num_particles_p ||
					pso_input->particle_location.Num() != pso_input->num_particles_p ||
					pso_input->best_local_particle_value.Num() != pso_input->num_particles_p ||
					pso_input->particle_value.Num() != pso_input->num_particles_p ||
					pso_input->best_local_particle_positions.Num() != pso_input->num_particles_p) {

					// 0.1, all previous vectors concerning used to track system information  // 
					pso_input->particle_velocity.Clear();
					pso_input->particle_location.Clear();
					pso_input->particle_value.Clear();
					pso_input->best_local_particle_positions.Clear();
					pso_input->best_local_particle_value.Clear();

					// 0.2, create the initial locations and values //
					for (int index_p = 0; index_p < pso_input->num_particles_p; index_p++) {
						cweeThreadedList<float> upper_velocity_bounds = vector_repeated_scalar_values(pso_input->num_dimensions_d, pso_input->percent_length_initial_velocity * (get_absolute_max(pso_input->upper_box_constraints) - get_min(pso_input->lower_box_constraints))); // 
						cweeThreadedList<float> lower_velocity_bounds = vector_repeated_scalar_values(pso_input->num_dimensions_d, -1 * pso_input->percent_length_initial_velocity * (get_absolute_max(pso_input->upper_box_constraints) - get_min(pso_input->lower_box_constraints)));
						pso_input->particle_velocity.Append(GetUniformRandomVector(upper_velocity_bounds, lower_velocity_bounds, is_random, control_rand, pso_input->non_random_seed_counter + index_p * pso_input->num_dimensions_d));
						pso_input->particle_location.Append(GetUniformRandomVector(pso_input->upper_box_constraints, pso_input->lower_box_constraints, is_random, control_rand, pso_input->non_random_seed_counter + index_p * pso_input->num_dimensions_d));
						pso_input->particle_value.Append(PSEUDO_MAX);  // we use a pseudo max since this value is currently unassigned // 
					}
					// 0.3, replace the random vectors with provided initial points if available // 
					for (int index_initial_points = 0; index_initial_points < pso_input->initial_points.Num(); index_initial_points++) {
						pso_input->particle_location[index_initial_points] = pso_input->initial_points[index_initial_points];
					}
					for (int index_p = 0; index_p < pso_input->num_particles_p; index_p++) {
						pso_input->best_local_particle_positions.Append(pso_input->particle_location[index_p]);
						pso_input->best_local_particle_value.Append(PSEUDO_MAX);
					}
					for (int index_particles = 0; index_particles < pso_input->num_particles_p; index_particles++) {
						if (pso_input->integer_only) {
							pso_input->particle_location[index_particles] = round_vector_to_nearest_int(pso_input->particle_location[index_particles]);
						}
						if (pso_input->adjust_to_bounds) {
							pso_input->particle_location[index_particles] = round_vector_to_nearest_bound(pso_input->particle_location[index_particles], pso_input->upper_box_constraints, pso_input->lower_box_constraints);
						}
					}
					pso_input->iterations_completed++;
					return;
				}
				else {   // 0.4, set up all of the previous values for the particles are set up
					pso_input->particle_value = inputed_particle_values;
				}

				//  STEP 1, DETERMINE THE BEST POINTS //
				//  1.1, determine the best local points // 
				for (int index_particles = 0; index_particles < pso_input->num_particles_p; index_particles++) {
					if (pso_input->particle_value[index_particles] < pso_input->best_local_particle_value[index_particles]) {
						pso_input->best_local_particle_value[index_particles] = pso_input->particle_value[index_particles];
						pso_input->best_local_particle_positions[index_particles] = pso_input->particle_location[index_particles];
					}
				}
				// 1.2, determine the best global point //																																																																																																																																																															
				for (int index_particles = 0; index_particles < pso_input->num_particles_p; index_particles++) {
					if (pso_input->best_local_particle_value[index_particles] < pso_input->best_global_particle_value) {
						pso_input->best_global_particle_value = pso_input->best_local_particle_value[index_particles];
						pso_input->best_global_particle_location = pso_input->best_local_particle_positions[index_particles];
					}
				}

				// STEP 2, UPDATE THE VEOCITY AND POSITION // 
				// 2.0, reset the current velocity //
				cweeThreadedList < cweeThreadedList<float>> previous_velocity = pso_input->particle_velocity;
				cweeThreadedList < cweeThreadedList<float>> previous_location = pso_input->particle_location;
				pso_input->particle_velocity.Clear();
				pso_input->particle_location.Clear();
				for (int index_particles = 0; index_particles < pso_input->num_particles_p; index_particles++) {
					cweeThreadedList<float> new_velocity;
					for (int index_dims = 0; index_dims < pso_input->num_dimensions_d; index_dims++) { //
						float random_loc, random_global;

						// 2.1, determine the random elements in updating the velocity //
						if (pso_input->use_random) {
							random_loc = cweeRandomFloat(0, 1);
							random_global = cweeRandomFloat(0, 1);
						}
						else {
							random_loc = 0.5;
							random_global = 0.5;
						}
						// 2.2, apply the update as a scaled linear combination of traveling towards the global best point and the local best point //
						float vel_comp = pso_input->inertia * previous_velocity[index_particles][index_dims]
							+ random_loc * pso_input->l_cor * (pso_input->best_local_particle_positions[index_particles][index_dims] - previous_location[index_particles][index_dims])
							+ random_global * pso_input->g_cor * (pso_input->best_global_particle_location[index_dims] - previous_location[index_particles][index_dims]);
						new_velocity.Append(vel_comp);
					}
					pso_input->particle_velocity.Append(new_velocity);
				}

				// STEP 3, MOVE THE PARTCILES BY THE VELOCITY (UPDATE) AND RETURN THE NEW POTENTIAL POINTS // 
				for (int index_particles = 0; index_particles < pso_input->num_particles_p; index_particles++) {
					pso_input->particle_location.Append(add_vec(pso_input->particle_velocity[index_particles], previous_location[index_particles]));
				}

				// STEP 4, MAKE SPECIFIED ADJUSTMENT FOR VECTORS //
				for (int index_particles = 0; index_particles < pso_input->num_particles_p; index_particles++) {
					if (pso_input->integer_only) {
						pso_input->particle_location[index_particles] = round_vector_to_nearest_int(pso_input->particle_location[index_particles]);
					}
					if (pso_input->adjust_to_bounds) {
						pso_input->particle_location[index_particles] = round_vector_to_nearest_bound(pso_input->particle_location[index_particles], pso_input->upper_box_constraints, pso_input->lower_box_constraints);
					}
				}

				pso_input->iterations_completed++;
				return;
			}
		}  /* Particle Search Objects End */;

#pragma endregion 
		/* HJ CODE */
#pragma region HJ_CODE
		/* */
		class HJInput_Update : public OptimInput {
		public:
			// the control points, these will be in two sets(size p) //
			cweeThreadedList<float> current_particle_values;									//
			cweeThreadedList<cweeThreadedList<float>> current_particle_locations;					// 
			cweeThreadedList<int> current_particle_label_vectors;							//
			cweeThreadedList<float> current_particle_delta_update;							//

			// the values for the next point (size n * p (approximately since this varies based on improvement)) //
			cweeThreadedList < float >  neighborhood_particle_values;						// 
			cweeThreadedList< cweeThreadedList < float > > neighborhood_particle_locations;		// 
			cweeThreadedList<int> neighborhood_particle_labels;								//
			cweeThreadedList<int> neighborhood_particle_dimension_for_move;					// I think that this can be managed on the stack // 
			cweeThreadedList<int> neighborhood_particle_up_down_for_move;						// I think that this can be managed on the stack // 

			// The parameters (unchanging variables) // 
			float percent_of_dimension_space = 0.5;  // used in initialization // 
			float rho_adjustment_factor = 0.5;
			int max_directions_vectors_per_it_n = 1;
			bool integer_only = false;
			bool both_directions = false;
			// end parameters //


			cweeStr GetOptimTypeIDString() { // a debug function to return a string form the object 
				return "optimize_HJ";
			}

			cweeThreadedList < cweeThreadedList<float> > OutputBatchPolicyVectors() { return current_particle_locations; }
			int BatchPolicyVectorSize() { return current_particle_locations.Num(); }
			void InitializeHJInput_Update_N(const cweeThreadedList<float>& upper_bound, const cweeThreadedList<float>& lower_bound, const cweeThreadedList < cweeThreadedList<float> >& initial_points_inputs = {}) {
				if (upper_bound.Num() != lower_bound.Num()) {
					return;
				}
				else {
					// RESET ALL EXISTING VECTORS // 						
					current_particle_locations.Clear();							// we will keep these for the time being 
					neighborhood_particle_labels.Clear();							//
					neighborhood_particle_dimension_for_move.Clear();				//
					neighborhood_particle_up_down_for_move.Clear();				//
					neighborhood_particle_locations.Clear();

					//  set the variables that we are going to be using for this upper and lower box constraints // 
					upper_box_constraints = upper_bound;
					lower_box_constraints = lower_bound;
					num_dimensions_d = upper_bound.Num();

					float min_span_dimension_domain = PSEUDO_MAX;
					for (int index_d = 0; index_d < num_dimensions_d; index_d++) {
						if (min_span_dimension_domain > (upper_box_constraints[index_d] - lower_box_constraints[index_d])) {
							min_span_dimension_domain = (upper_box_constraints[index_d] - lower_box_constraints[index_d]);
						}
					}
					for (int index_p = 0; index_p < num_particles_p; index_p++) {
						current_particle_values.Append(PSEUDO_MAX);
						current_particle_delta_update.Append(min_span_dimension_domain * percent_of_dimension_space);
					}
					return;
				}
			}
			void IterateOptimizer(const cweeThreadedList<float> inputed_particle_values = {}, bool is_random = true, non_random_type_t control_rand = __NONE_NON_RANDOM__) {
				AddRecordedValues(inputed_particle_values);		// add the record values to the main object // 
				ConfirmRecordsAndReorderRecords();				//

				// STEP 0, SET UP THE OPTIMIZATION IF THERE ARE NO PREVIOUS ITERATIONS 
				if (iterations_completed == 0 ||
					neighborhood_particle_locations.Num() == 0 ||
					neighborhood_particle_labels.Num() == 0 ||
					current_particle_locations.Num() == 0 ||
					current_particle_label_vectors.Num() == 0 ||
					current_particle_values.Num() == 0 ||
					inputed_particle_values.Num() == 0
					) {

					// 0.1, clear each of the vectors that will be set in this portion of the code
					neighborhood_particle_locations.Clear();
					neighborhood_particle_labels.Clear();
					current_particle_locations.Clear();
					current_particle_label_vectors.Clear();
					current_particle_values.Clear();

					for (int index_p = 0; index_p < num_particles_p; index_p++) { // randomly select a vector inside of the domain and return
						//is_random, control_rand, gen_input->non_random_seed_counter + index_p*gen_input->num_dimensions_d);
						cweeThreadedList<float> new_vector = GetUniformRandomVector(upper_box_constraints, lower_box_constraints, is_random, control_rand, non_random_seed_counter + index_p * num_dimensions_d);
						neighborhood_particle_locations.Append(new_vector);  // push back the vector //
						neighborhood_particle_labels.Append(index_p);
						neighborhood_particle_values.Append(PSEUDO_MAX);
						neighborhood_particle_dimension_for_move.Append(0);
						neighborhood_particle_up_down_for_move.Append(0);
						current_particle_locations.Append(new_vector);
						current_particle_label_vectors.Append(index_p);
						current_particle_values.Append(PSEUDO_MAX);
					}
					for (int index_initial_points = 0; index_initial_points < initial_points.Num(); index_initial_points++) {// replace the random vectors with provided initial points if available // 
						neighborhood_particle_locations[index_initial_points] = initial_points[index_initial_points];
						current_particle_locations[index_initial_points] = initial_points[index_initial_points];
					}

					// Adjust the output to meet bounds and integer constraints // 
					for (int index_particles = 0; index_particles < num_particles_p; index_particles++) {
						if (integer_only) {
							neighborhood_particle_locations[index_particles] = round_vector_to_nearest_int(neighborhood_particle_locations[index_particles]);
							current_particle_locations[index_particles] = round_vector_to_nearest_int(current_particle_locations[index_particles]);
						}
						if (adjust_to_bounds) {
							neighborhood_particle_locations[index_particles] = round_vector_to_nearest_bound(neighborhood_particle_locations[index_particles], upper_box_constraints, lower_box_constraints);
							current_particle_locations[index_particles] = round_vector_to_nearest_bound(current_particle_locations[index_particles], upper_box_constraints, lower_box_constraints);
						}
					}

					AddNewRecordedPoints(OutputBatchPolicyVectors());  // add the recorded points to the object // 
					iterations_completed++;
					return;
				}
				else {

					// STEP 1, set the neighborhood values computed on the previous generation's input;
					neighborhood_particle_values = inputed_particle_values;
					cweeThreadedList<bool> particle_improved; // tracks whether the particle moved or didn't move in the system //
					cweeThreadedList<int> best_dimension_improved;
					cweeThreadedList<int> up_down_move;

					// STEP 2, MOVE THE CURRENT PARTICLES LOCATION IF THERE IS AN IMPROVING NEIGHBORHOOD POINT // 
					for (int index_p = 0; index_p < num_particles_p; index_p++) {
						if (iterations_completed > 1) {
							bool improved = false;
							float best_value = current_particle_values[index_p];
							cweeThreadedList<float> location = current_particle_locations[index_p];
							int dimension, direction;

							// 2.0, after the first iteration we need to update the neighborhood points // 
							for (int index_neighbors = 0; index_neighbors < neighborhood_particle_labels.Num(); index_neighbors++) {
								if (neighborhood_particle_labels[index_neighbors] == index_p) {
									if (neighborhood_particle_values[index_neighbors] < best_value) {
										dimension = neighborhood_particle_dimension_for_move[index_neighbors];
										direction = neighborhood_particle_up_down_for_move[index_neighbors];
										improved = true;
										location = neighborhood_particle_locations[index_neighbors];  // setup the location for the 
									}
								}
							}
							// 2.1 update the current particle values with the best value, location, and direction //
							current_particle_values[index_p] = best_value;
							current_particle_locations[index_p] = location;
							current_particle_label_vectors[index_p] = index_p;

							// 2.2, record the best dimension that improved and the direction //
							best_dimension_improved.Append(dimension);
							up_down_move.Append(direction);
							particle_improved.Append(improved);
						}
						else {
							// (alterantive 2.1, simply assign the new particle to the current point) // 
							current_particle_values[index_p] = neighborhood_particle_values[index_p]; // update the current values with the new particle values
							particle_improved.Append(false);
							best_dimension_improved.Append(-1);
							up_down_move.Append(0);
						}
					}

					// STEP 3, CLEAR THE NEIGHBORHOOD VECTORS AFTER UPDATE 
					neighborhood_particle_locations.Clear();
					neighborhood_particle_labels.Clear();
					neighborhood_particle_values.Clear();
					neighborhood_particle_dimension_for_move.Clear();
					neighborhood_particle_up_down_for_move.Clear();

					// STEP 4, FOR ALL DIRECTIONS THAT DIDN'T IMPROVE PICK A NEW SET RANDOM DIRECTIONS AT A SMALLER DELTA, IMPROVING PARTICLES WILL NOW HAVE 1 NEIGHBOR, 
					for (int index_p = 0; index_p < num_particles_p; index_p++) {
						if (particle_improved[index_p]) { // if improved... move in the same direction  //

							// 4.1, set up the updated vector location 
							cweeThreadedList<float> vector_location_update = current_particle_locations[index_p];
							if (best_dimension_improved.Num() > index_p && vector_location_update.Num() > best_dimension_improved[index_p] && up_down_move.Num() > index_p && current_particle_delta_update.Num() > index_p) {
								vector_location_update[best_dimension_improved[index_p]] += (((float)up_down_move[index_p]) * current_particle_delta_update[index_p]); // move in the same direction 
							}

							// 4.2, update the new neighborhood particles //
							neighborhood_particle_locations.Append(vector_location_update);
							neighborhood_particle_labels.Append(index_p);
							neighborhood_particle_dimension_for_move.Append(best_dimension_improved[index_p]);
							neighborhood_particle_up_down_for_move.Append(up_down_move[index_p]);
						}
						else { // if not improved then we can then  //

							cweeThreadedList<int> dimensions_for_update;

							// 4.3, update the new neighborhood particles //
							if (is_random) {
								dimensions_for_update = get_unique_random_in_limits_int(0, num_dimensions_d - 1, max_directions_vectors_per_it_n);
							}
							else {
								for (int index_direction = 0; index_direction < max_directions_vectors_per_it_n; index_direction++) {
									dimensions_for_update.Append(index_direction);
								}
							}

							// 4.4, update the adjustment factor (to search inside a smaller neighborhood) //
							current_particle_delta_update[index_p] = current_particle_delta_update[index_p] * rho_adjustment_factor;

							for (int dimension : dimensions_for_update) {

								// 4.5, pick a random direction (up or down) and then move delta in the selected dimension //
								cweeThreadedList<float> vector_location_update_inner = current_particle_locations[index_p];;
								int direction = 0;
								if (is_random) {
									int direction = 1 - 2 * cweeRandomInt(0, 1); // be decide whether to go up or down on the selected dimension (flip coin)
								}
								else {
									direction = 1; // if there is no random, we will always use a positive direction 
								}

								vector_location_update_inner[dimension] = vector_location_update_inner[dimension] + direction * current_particle_delta_update[index_p];

								// 4.6, record the updated neighborhood point //
								neighborhood_particle_locations.Append(vector_location_update_inner);
								neighborhood_particle_labels.Append(index_p);
								neighborhood_particle_dimension_for_move.Append(dimension);
								neighborhood_particle_up_down_for_move.Append(direction);
							}
						}
					}
					// STEP 5, ADJUST THE BOUNDS FOR THE OUTPUT VECTORS // 
					for (int index_particles = 0; index_particles < num_particles_p; index_particles++) {
						if (integer_only) {
							neighborhood_particle_locations[index_particles] = round_vector_to_nearest_int(neighborhood_particle_locations[index_particles]);
						}
						if (adjust_to_bounds) {
							neighborhood_particle_locations[index_particles] = round_vector_to_nearest_bound(neighborhood_particle_locations[index_particles], upper_box_constraints, lower_box_constraints);
						}
					}

					AddNewRecordedPoints(OutputBatchPolicyVectors());  // add the recorded points to the object // 
					iterations_completed++;
					return;
				}
			}
			HJInput_Update() {}
			HJInput_Update(const cweeThreadedList<float>& upper_bound, const cweeThreadedList<float>& lower_bound, int num_particles_in, sampling_type input = _UNIFORM_RANDOM_SAMPLE_) : OptimInput(upper_bound, lower_bound, num_particles_in) {}
			~HJInput_Update() {}

			void CopyOptimizerData(void* input_optimization_information, cweeThreadedList<float> search_space_map) {
				if (input_optimization_information != nullptr) {
					CopyOptimizerBaseData((OptimInput*)input_optimization_information, (OptimInput*)this, search_space_map);
					current_particle_values = current_particle_values;
					current_particle_locations = current_particle_locations;
					current_particle_label_vectors = current_particle_label_vectors;
					current_particle_delta_update = current_particle_delta_update;
					neighborhood_particle_values = neighborhood_particle_values;
					neighborhood_particle_locations = neighborhood_particle_locations;
					neighborhood_particle_labels = neighborhood_particle_labels;
					neighborhood_particle_dimension_for_move = neighborhood_particle_dimension_for_move;
					neighborhood_particle_up_down_for_move = neighborhood_particle_up_down_for_move;
					percent_of_dimension_space = percent_of_dimension_space;	  // used in initialization // 
					rho_adjustment_factor = rho_adjustment_factor;
					max_directions_vectors_per_it_n = max_directions_vectors_per_it_n;
					integer_only = integer_only;
					both_directions = both_directions;
				}
				return;
			}
			OptimType GetOptimizationType() { return cweeMath::Optim::OptimType::OPT_HOOKES; }   // return the the optimization type // 

		};

		/*
		void static InitializeHJInput_Update(HJInput_Update* hj_input, const cweeThreadedList<float>& upper_bound, const cweeThreadedList<float>& lower_bound) {
			if (hj_input == nullptr) {  // check to see if the memory is not allocated
				return;
			}
			else {
				if (upper_bound.Num() != lower_bound.Num()) {
					return;
				}
				else {
					// RESET ALL EXISTING VECTORS //
					hj_input->current_particle_locations.Clear();							// we will keep these for the time being
					hj_input->neighborhood_particle_labels.Clear();							//
					hj_input->neighborhood_particle_dimension_for_move.Clear();				//
					hj_input->neighborhood_particle_up_down_for_move.Clear();				//
					hj_input->neighborhood_particle_locations.Clear();

					//  set the variables that we are going to be using for this upper and lower box constraints //
					hj_input->upper_box_constraints = upper_bound;
					hj_input->lower_box_constraints = lower_bound;
					hj_input->num_dimensions_d = upper_bound.Num();

					float min_span_dimension_domain = PSEUDO_MAX;
					for (int index_d = 0; index_d < hj_input->num_dimensions_d; index_d++) {
						if (min_span_dimension_domain > (hj_input->upper_box_constraints[index_d] - hj_input->lower_box_constraints[index_d])) {
							min_span_dimension_domain = (hj_input->upper_box_constraints[index_d] - hj_input->lower_box_constraints[index_d]);
						}
					}
					for (int index_p = 0; index_p < hj_input->num_particles_p; index_p++) {
						hj_input->current_particle_values.Append(PSEUDO_MAX);
						hj_input->current_particle_delta_update.Append(min_span_dimension_domain * hj_input->percent_of_dimension_space);
					}
					return;
				}
			}
		};
		*/
		/*
		void static HJ_Iterations_Update_Refactor(HJInput_Update* hj_input, const cweeThreadedList<float>& inputed_particle_values = {}, bool is_random = true, non_random_type_t control_rand = __NONE_NON_RANDOM__) {
			if (hj_input == nullptr) {
				return;
			}
			else {
				// STEP 0, SET UP THE OPTIMIZATION IF THERE ARE NO PREVIOUS ITERATIONS
				if (hj_input->iterations_completed == 0 ||
					hj_input->neighborhood_particle_locations.Num() == 0 ||
					hj_input->neighborhood_particle_labels.Num() == 0 ||
					hj_input->current_particle_locations.Num() == 0 ||
					hj_input->current_particle_label_vectors.Num() == 0 ||
					hj_input->current_particle_values.Num() == 0 ||
					inputed_particle_values.Num() == 0
					) {

					// 0.1, clear each of the vectors that will be set in this portion of the code
					hj_input->neighborhood_particle_locations.Clear();
					hj_input->neighborhood_particle_labels.Clear();
					hj_input->current_particle_locations.Clear();
					hj_input->current_particle_label_vectors.Clear();
					hj_input->current_particle_values.Clear();

					for (int index_p = 0; index_p < hj_input->num_particles_p; index_p++) { // randomly select a vector inside of the domain and return
						//is_random, control_rand, gen_input->non_random_seed_counter + index_p*gen_input->num_dimensions_d);
						cweeThreadedList<float> new_vector = GetUniformRandomVector(hj_input->upper_box_constraints, hj_input->lower_box_constraints, is_random, control_rand, hj_input->non_random_seed_counter + index_p * hj_input->num_dimensions_d);
						hj_input->neighborhood_particle_locations.Append(new_vector);  // push back the vector //
						hj_input->neighborhood_particle_labels.Append(index_p);
						hj_input->neighborhood_particle_values.Append(PSEUDO_MAX);
						hj_input->neighborhood_particle_dimension_for_move.Append(0);
						hj_input->neighborhood_particle_up_down_for_move.Append(0);
						hj_input->current_particle_locations.Append(new_vector);
						hj_input->current_particle_label_vectors.Append(index_p);
						hj_input->current_particle_values.Append(PSEUDO_MAX);
					}
					for (int index_initial_points = 0; index_initial_points < hj_input->initial_points.Num(); index_initial_points++) {// replace the random vectors with provided initial points if available //
						hj_input->neighborhood_particle_locations[index_initial_points] = hj_input->initial_points[index_initial_points];
						hj_input->current_particle_locations[index_initial_points] = hj_input->initial_points[index_initial_points];
					}

					// Adjust the output to meet bounds and integer constraints //
					for (int index_particles = 0; index_particles < hj_input->num_particles_p; index_particles++) {
						if (hj_input->integer_only) {
							hj_input->neighborhood_particle_locations[index_particles] = round_vector_to_nearest_int(hj_input->neighborhood_particle_locations[index_particles]);
							hj_input->current_particle_locations[index_particles] = round_vector_to_nearest_int(hj_input->current_particle_locations[index_particles]);
						}
						if (hj_input->adjust_to_bounds) {
							hj_input->neighborhood_particle_locations[index_particles] = round_vector_to_nearest_bound(hj_input->neighborhood_particle_locations[index_particles], hj_input->upper_box_constraints, hj_input->lower_box_constraints);
							hj_input->current_particle_locations[index_particles] = round_vector_to_nearest_bound(hj_input->current_particle_locations[index_particles], hj_input->upper_box_constraints, hj_input->lower_box_constraints);
						}
					}
					hj_input->iterations_completed++;
					return;
				}
				else{

				// STEP 1, set the neighborhood values computed on the previous generation's input;
					hj_input->neighborhood_particle_values = inputed_particle_values;
					cweeThreadedList<bool> particle_improved; // tracks whether the particle moved or didn't move in the system //
					cweeThreadedList<int> best_dimension_improved;
					cweeThreadedList<int> up_down_move;

				// STEP 2, MOVE THE CURRENT PARTICLES LOCATION IF THERE IS AN IMPROVING NEIGHBORHOOD POINT //
					for (int index_p = 0; index_p < hj_input->num_particles_p; index_p++) {
						if (hj_input->iterations_completed > 1) {
							bool improved = false;
							float best_value = hj_input->current_particle_values[index_p];
							cweeThreadedList<float> location = hj_input->current_particle_locations[index_p];
							int dimension, direction;

							// 2.0, after the first iteration we need to update the neighborhood points //
							for (int index_neighbors = 0; index_neighbors < hj_input->neighborhood_particle_labels.Num(); index_neighbors++) {
								if (hj_input->neighborhood_particle_labels[index_neighbors] == index_p) {
									if (hj_input->neighborhood_particle_values[index_neighbors] < best_value) {
										dimension		= hj_input->neighborhood_particle_dimension_for_move[index_neighbors];
										direction		= hj_input->neighborhood_particle_up_down_for_move[index_neighbors];
										improved		= true;
										location		= hj_input->neighborhood_particle_locations[index_neighbors];  // setup the location for the
									}
								}
							}

							// 2.1 update the current particle values with the best value, location, and direction //
							hj_input->current_particle_values[index_p]			= best_value;
							hj_input->current_particle_locations[index_p]		= location;
							hj_input->current_particle_label_vectors[index_p]	= index_p;

							// 2.2, record the best dimension that improved and the direction //
							best_dimension_improved.Append(dimension);
							up_down_move.Append(direction);
							particle_improved.Append(improved);
						}
						else {
							// (alterantive 2.1, simply assign the new particle to the current point) //
							hj_input->current_particle_values[index_p] = hj_input->neighborhood_particle_values[index_p]; // update the current values with the new particle values
							particle_improved.Append(false);
							best_dimension_improved.Append(-1);
							up_down_move.Append(0);
						}
					}

				// STEP 3, CLEAR THE NEIGHBORHOOD VECTORS AFTER UPDATE
					hj_input->neighborhood_particle_locations.Clear();
					hj_input->neighborhood_particle_labels.Clear();
					hj_input->neighborhood_particle_values.Clear();
					hj_input->neighborhood_particle_dimension_for_move.Clear();
					hj_input->neighborhood_particle_up_down_for_move.Clear();

				// STEP 4, FOR ALL DIRECTIONS THAT DIDN'T IMPROVE PICK A NEW SET RANDOM DIRECTIONS AT A SMALLER DELTA, IMPROVING PARTICLES WILL NOW HAVE 1 NEIGHBOR,
					for (int index_p = 0; index_p < hj_input->num_particles_p; index_p++) {
						if (particle_improved[index_p]) { // if improved... move in the same direction  //

							// 4.1, set up the updated vector location
							cweeThreadedList<float> vector_location_update = hj_input->current_particle_locations[index_p];
							vector_location_update[best_dimension_improved[index_p]]
								= vector_location_update[best_dimension_improved[index_p]] + up_down_move[index_p] * hj_input->current_particle_delta_update[index_p]; // move in the same direction

							// 4.2, update the new neighborhood particles //
							hj_input->neighborhood_particle_locations.Append(vector_location_update);
							hj_input->neighborhood_particle_labels.Append(index_p);
							hj_input->neighborhood_particle_dimension_for_move.Append(best_dimension_improved[index_p]);
							hj_input->neighborhood_particle_up_down_for_move.Append(up_down_move[index_p]);
						}
						else { // if not improved then we can then  //

							cweeThreadedList<int> dimensions_for_update;

							// 4.3, update the new neighborhood particles //
							if (is_random) {
								dimensions_for_update = get_unique_random_in_limits_int(0, hj_input->num_dimensions_d - 1, hj_input->max_directions_vectors_per_it_n);
							}
							else {
								for (int index_direction = 0; index_direction < hj_input->max_directions_vectors_per_it_n; index_direction++) {
									dimensions_for_update.Append(index_direction);
								}
							}

							// 4.4, update the adjustment factor (to search inside a smaller neighborhood) //
							hj_input->current_particle_delta_update[index_p] = hj_input->current_particle_delta_update[index_p] * hj_input->rho_adjustment_factor;

							for (int dimension : dimensions_for_update) {

								// 4.5, pick a random direction (up or down) and then move delta in the selected dimension //
								cweeThreadedList<float> vector_location_update_inner = hj_input->current_particle_locations[index_p];;
								int direction = 0;
								if (is_random) {
									int direction = 1 - 2 * cweeRandomInt(0, 1); // be decide whether to go up or down on the selected dimension (flip coin)
								}
								else {
									direction = 1; // if there is no random, we will always use a positive direction
								}

								vector_location_update_inner[dimension] = vector_location_update_inner[dimension] + direction * hj_input->current_particle_delta_update[index_p];

								// 4.6, record the updated neighborhood point //
								hj_input->neighborhood_particle_locations.Append(vector_location_update_inner);
								hj_input->neighborhood_particle_labels.Append(index_p);
								hj_input->neighborhood_particle_dimension_for_move.Append(dimension);
								hj_input->neighborhood_particle_up_down_for_move.Append(direction);
							}
						}
					}

				// STEP 5, ADJUST THE BOUNDS FOR THE OUTPUT VECTORS //
					for (int index_particles = 0; index_particles < hj_input->num_particles_p; index_particles++) {
						if (hj_input->integer_only) {
							hj_input->neighborhood_particle_locations[index_particles] = round_vector_to_nearest_int(hj_input->neighborhood_particle_locations[index_particles]);
						}
						if (hj_input->adjust_to_bounds) {
							hj_input->neighborhood_particle_locations[index_particles] = round_vector_to_nearest_bound(hj_input->neighborhood_particle_locations[index_particles], hj_input->upper_box_constraints, hj_input->lower_box_constraints);
						}
					}
					hj_input->iterations_completed++;
					return;
				}
			}
		};
		*/

#pragma endregion
		/* The Genetic Search rewrite */
#pragma region GENETIC_CODE
/* following the basic form described in MATLAB,
// https://www.mathworks.com/help/gads/how-the-genetic-algorithm-works.html#:~:text=The%20following%20outline%20summarizes%20how,to%20create%20the%20next%20population
*/
		enum RandomizeModes { __UNIFORM_MUTATION__, __NORMAL_MUTATION__ };
		enum Genetic_modes { _ELITE_, _RECOM_, _MUTANT_, _NO_GEN_SETTING_ }; // the three types of Genetic MODE that describe the incoming particles
		class GeneticInput_Update : public OptimInput {

		public:
			// the current generation particles //
			cweeThreadedList< cweeThreadedList< float> >	current_gen_population;
			cweeThreadedList < Genetic_modes >				current_gen_labels;
			cweeThreadedList< float >						current_gen_values;

			// the next generation particles //
			cweeThreadedList< cweeThreadedList< float> >	next_gen_population;
			cweeThreadedList < Genetic_modes >				next_gen_labels;
			cweeThreadedList< float>						next_gen_values;

			// OPTIMIZATION PARAMETERS //
			RandomizeModes mode = __UNIFORM_MUTATION__;
			int num_elites = 2;
			float percent_recombination = 0.4f;
			float percent_dim_for_normal_variance = 0.4f; // only used for normal distribuition variance // 

			// METHODS // 
			GeneticInput_Update() {}
			GeneticInput_Update(const cweeThreadedList<float>& upper_bound, const cweeThreadedList<float>& lower_bound, int num_particles_in = 5, sampling_type input = _UNIFORM_RANDOM_SAMPLE_) : OptimInput(upper_bound, lower_bound, num_particles_in) {}
			~GeneticInput_Update() {}

			cweeStr GetOptimTypeIDString() { // a debug function to return a string form the object 
				return "optimize_Genetic";
			}

			void CopyOptimizerData(void* input_optimization_information, cweeThreadedList<float> search_space_map) {
				if (input_optimization_information != nullptr) {
					CopyOptimizerBaseData((OptimInput*)input_optimization_information, (OptimInput*)this, search_space_map);
					current_gen_population = current_gen_population;
					current_gen_labels = current_gen_labels;
					current_gen_values = current_gen_values;
					next_gen_population = next_gen_population;
					next_gen_labels = next_gen_labels;
					next_gen_values = next_gen_values;
					mode = mode;
					num_elites = num_elites;
					percent_recombination = percent_recombination;
					percent_dim_for_normal_variance = percent_dim_for_normal_variance; // only used for normal distribuition variance // 
				}
				return;
			}
			OptimType GetOptimizationType() { return cweeMath::Optim::OptimType::OPT_GENETIC; }   // return the the optimization type // 
			cweeThreadedList < cweeThreadedList<float>> OutputBatchPolicyVectors() { return next_gen_population; }
			int BatchPolicyVectorSize() { return next_gen_population.Num(); }
			void InitializeGeneticInput_Update_N(const cweeThreadedList<float>& upper_bound, const cweeThreadedList<float>& lower_bound) {
				if (upper_bound.Num() != lower_bound.Num()) {
					return;
				}
				else {
					num_dimensions_d = upper_bound.Num(); // get the number of dimensions //
					upper_box_constraints = upper_bound;
					lower_box_constraints = lower_bound;

					current_gen_population.Clear();
					current_gen_values.Clear();
					current_gen_labels.Clear();

					next_gen_population.Clear();
					next_gen_labels.Clear();
					next_gen_values.Clear();

					return;
				}
			}
			void IterateOptimizer(const cweeThreadedList<float> inputed_particle_values = {}, bool is_random = true, non_random_type_t control_rand = __NONE_NON_RANDOM__) {

				AddRecordedValues(inputed_particle_values);		// add the record values to the main object // 
				ConfirmRecordsAndReorderRecords();				//

				// STEP 0, SET UP THE INITIAL SAMPLE IF THERE WAS NOT A PREVIOUS ITERATION OR INPUTED VALUES (DEFAULT) //
				if (iterations_completed == 0
					|| current_gen_population.Num() == 0
					|| current_gen_labels.Num() == 0
					|| current_gen_values.Num() == 0
					|| inputed_particle_values.Num() == 0
					) {
					for (int index_p = 0; index_p < num_particles_p; index_p++) { // go through each of the points
						cweeThreadedList<float> new_vector = GetUniformRandomVector(upper_box_constraints, lower_box_constraints, is_random, control_rand, non_random_seed_counter + index_p * num_dimensions_d);
						next_gen_population.Append(new_vector);
						next_gen_labels.Append(_NO_GEN_SETTING_);
						next_gen_values.Append(PSEUDO_MAX);   // push back the pseudo max // 
					}
					// replace the random vectors with provided initial points if available // 
					for (int index_initial_points = 0; index_initial_points < initial_points.Num(); index_initial_points++) {
						next_gen_population[index_initial_points] = initial_points[index_initial_points];
					}

					// on the first iteration, we will be using the next generation labels // 
					current_gen_population = next_gen_population;
					current_gen_labels = next_gen_labels;
					current_gen_values = next_gen_values;

					// 
					AddNewRecordedPoints(OutputBatchPolicyVectors());  // add the recorded points to the object // 
					iterations_completed++;
					return;
				}
				else {
					// STEP 1, UPDATE THE CURENT POPULATION OF PARTICLES AND CLEAR NEXT GEN VARIABLES // 
					// 1.1, replace the variables 
					current_gen_population = next_gen_population;
					current_gen_labels = next_gen_labels;  // we don't use the current generation labels for anything algorithmically, but they are useful for de-bugging / symmetry
					current_gen_values = inputed_particle_values;

					// 1.2, clear the next generation labels that we will fill out on the next iteration.
					next_gen_population.Clear();
					next_gen_values.Clear();
					next_gen_labels.Clear();

					// STEP 2, SORT THE CURRENT POPULATION, SET THE DESIGNATED GENERATION METHODS FOR THE NEXT GENERATION // 
					int num_elites_to_label = min(num_elites, num_dimensions_d); // 
					int number_recom = min((num_particles_p - num_elites_to_label), ceil(num_particles_p * percent_recombination));
					int number_mutant = max(0, (num_particles_p - number_recom - num_elites_to_label));   // the number of mutants in the new generation //

					// 2.1, first order the current points in the system // 
					cweeThreadedList< float> order_current_gen_Values = cweeMath::Optim::order_values(current_gen_values);
					cweeThreadedList< cweeThreadedList< float> > orderded_current_gen_population = cweeMath::Optim::order_vector_values(current_gen_values, current_gen_population);

					// 2.2, establish the labels (ELITE, RECOMBINATION, MUTANT)(  // 
					for (int index_p = 0; index_p < num_particles_p; index_p++) {
						if (index_p < num_elites_to_label) {
							// STEP 2, POPULATE THE NEXT GENERATION OF POINTS (ELITES) //
							next_gen_labels.Append(_ELITE_);
							next_gen_population.Append(orderded_current_gen_population[index_p]);
						}
						else if (index_p < num_elites_to_label + number_recom) {
							// STEP 3, POPULATE THE NEXT GENERATION OF POINTS (RECOMBINED) // 

							// 3.0, start with the base of the current population and 
							cweeThreadedList<float> recom_output = orderded_current_gen_population[index_p]; // set the new recombined point to the order position //
							cweeThreadedList<int> gene_locations_from_first_parent;
							int number_gene_locations_to_modify;
							int elite_parent_index;
							if (is_random) {
								// 3.1, determine which elite point is going combined 
								number_gene_locations_to_modify = cweeRandomInt(0, num_dimensions_d);
								elite_parent_index = cweeRandomInt(0, (num_elites - 1)); // randomly select one of the 

								// 3.2, randomly determine which locations (genes) will be recombined from the elite selected (if randomization is turned off then we do the first and the last dimension)
								gene_locations_from_first_parent = get_unique_random_in_limits_int(0, num_dimensions_d, number_gene_locations_to_modify);
							}
							else { // the deterministic case for testing
								// (alt 3.1 set deterministic location to recombine)
								elite_parent_index = 0;

								// (alt 3.2 determine the genelocatios)
								gene_locations_from_first_parent.Append(0);
							}

							// 3.3, Recombination to get new children
							for (auto locaction_index : gene_locations_from_first_parent) {
								recom_output[locaction_index] = orderded_current_gen_population[elite_parent_index][locaction_index];
							}
							next_gen_labels.Append(_RECOM_);
							next_gen_population.Append(recom_output);
						}
						else if (index_p < num_elites_to_label + number_recom + number_mutant) {
							// STEP 4, POPULATE THE NEXT GENERATION OF POINTS (MUTANTS) //
							cweeThreadedList<float> mutant_vector;

							// 4.1, if we are using the uniform sampling on the domain to obtain mutants 
							if (mode == __UNIFORM_MUTATION__) { // 4.1, A completely uniform restarting of the sample space // 
								mutant_vector = GetUniformRandomVector(upper_box_constraints, lower_box_constraints, is_random, control_rand, non_random_seed_counter + (index_p * num_dimensions_d));
							}
							// 4.2, if we are using normal sampling 
							else if (mode == __NORMAL_MUTATION__) { // 4.2, A completely normal mutation centered around each variable for a given mutating partcile // 
								cweeThreadedList<float> mean_vector, stdv_vector;
								for (int index_dimension = 0; index_dimension < num_dimensions_d; index_dimension++) {
									mean_vector.Append(orderded_current_gen_population[index_p][index_dimension]);
									stdv_vector.Append(percent_dim_for_normal_variance * (upper_box_constraints[index_dimension] - lower_box_constraints[index_dimension]));
								}
								mutant_vector = DetermineNormalRandomVector_vectorInput(mean_vector, stdv_vector, is_random, control_rand, non_random_seed_counter + (index_p * num_dimensions_d));
							}
							// 4.3, label the new entry as a mutant
							next_gen_labels.Append(_MUTANT_); // 4.3, label the new vector as a mutant
							next_gen_population.Append(mutant_vector);
						}
					}

					// STEP 5, adjust the outputed values to match the boundaries and possible integer constraints
					for (int index_particles = 0; index_particles < num_particles_p; index_particles++) {
						if (integer_only) {
							next_gen_population[index_particles] = round_vector_to_nearest_int(next_gen_population[index_particles]);
						}
						if (adjust_to_bounds) {
							next_gen_population[index_particles] = round_vector_to_nearest_bound(next_gen_population[index_particles], upper_box_constraints, lower_box_constraints);
						}
					}

					AddNewRecordedPoints(OutputBatchPolicyVectors());  // add the recorded points to the object // 
					iterations_completed++;
					return;
				}
			}
		};

		/*
		void static InitializeGeneticInput_Update(GeneticInput_Update* gen_input, const cweeThreadedList<float>& upper_bound, const cweeThreadedList<float>& lower_bound) {
			if (gen_input == nullptr) {  // check to see if the memory is not allocated
				return;
			}
			else {
				if (upper_bound.Num() != lower_bound.Num()) {
					return;
				}
				else {
					gen_input->num_dimensions_d = upper_bound.Num(); // get the number of dimensions //
					gen_input->upper_box_constraints = upper_bound;
					gen_input->lower_box_constraints = lower_bound;

					gen_input->current_gen_population.Clear();
					gen_input->current_gen_values.Clear();
					gen_input->current_gen_labels.Clear();

					gen_input->next_gen_population.Clear();
					gen_input->next_gen_labels.Clear();
					gen_input->next_gen_values.Clear();
					return;
				}
			}
		};
		*/
		/*
		void static Genetic_Iterations_Refactor(GeneticInput_Update* gen_input, const cweeThreadedList<float>& inputed_values = {}, bool is_random = true, non_random_type_t control_rand = __NONE_NON_RANDOM__) {

			if (gen_input == nullptr) {
				return;
			}
			else {
				// STEP 0, SET UP THE INITIAL SAMPLE IF THERE WAS NOT A PREVIOUS ITERATION OR INPUTED VALUES (DEFAULT) //
				if (gen_input->iterations_completed == 0
					|| gen_input->current_gen_population.Num() == 0
					|| gen_input->current_gen_labels.Num()		== 0
					|| gen_input->current_gen_values.Num()		== 0
					|| inputed_values.Num() == 0
					) {

					for (int index_p = 0; index_p < gen_input->num_particles_p; index_p++) { // go through each of the points
						cweeThreadedList<float> new_vector = GetUniformRandomVector(gen_input->upper_box_constraints, gen_input->lower_box_constraints, is_random, control_rand, gen_input->non_random_seed_counter + index_p * gen_input->num_dimensions_d);
						gen_input->next_gen_population.Append(new_vector);
						gen_input->next_gen_labels.Append(_NO_GEN_SETTING_);
						gen_input->next_gen_values.Append(PSEUDO_MAX);   // push back the pseudo max //
					}
					// replace the random vectors with provided initial points if available //
					for (int index_initial_points = 0; index_initial_points < gen_input->initial_points.Num(); index_initial_points++) {
						gen_input->next_gen_population[index_initial_points] = gen_input->initial_points[index_initial_points];
					}

					// on the first iteration, we will be using the next generation labels //
					gen_input->current_gen_population		= gen_input->next_gen_population;
					gen_input->current_gen_labels			= gen_input->next_gen_labels;
					gen_input->current_gen_values			= gen_input->next_gen_values;

					//
					gen_input->iterations_completed++;
					return;
				}
				else {
					// STEP 1, UPDATE THE CURENT POPULATION OF PARTICLES AND CLEAR NEXT GEN VARIABLES //
					// 1.1, replace the variables
					gen_input->current_gen_population = gen_input->next_gen_population;
					gen_input->current_gen_labels = gen_input->next_gen_labels;  // we don't use the current generation labels for anything algorithmically, but they are useful for de-bugging / symmetry
					gen_input->current_gen_values = inputed_values;

					// 1.2, clear the next generation labels that we will fill out on the next iteration.
					gen_input->next_gen_population.Clear();
					gen_input->next_gen_values.Clear();
					gen_input->next_gen_labels.Clear();

					// STEP 2, SORT THE CURRENT POPULATION, SET THE DESIGNATED GENERATION METHODS FOR THE NEXT GENERATION //
					int num_elites_to_label = min(gen_input->num_elites, gen_input->num_dimensions_d); //
					int number_recom = min((gen_input->num_particles_p - num_elites_to_label), ceil(gen_input->num_particles_p * gen_input->percent_recombination));
					int number_mutant = max(0, (gen_input->num_particles_p - number_recom - num_elites_to_label));   // the number of mutants in the new generation //

					// 2.1, first order the current points in the system //
					cweeThreadedList< float> order_current_gen_Values = cweeMath::Optim::order_values(gen_input->current_gen_values);
					cweeThreadedList< cweeThreadedList< float> > orderded_current_gen_population = cweeMath::Optim::order_vector_values(gen_input->current_gen_values, gen_input->current_gen_population);

					// 2.2, establish the labels (ELITE, RECOMBINATION, MUTANT)(  //
					for (int index_p = 0; index_p < gen_input->num_particles_p; index_p++) {
						if (index_p < num_elites_to_label)  {
							// STEP 2, POPULATE THE NEXT GENERATION OF POINTS (ELITES) //
							gen_input->next_gen_labels.Append(_ELITE_);
							gen_input->next_gen_population.Append(orderded_current_gen_population[index_p]);
						}
						else if (index_p < num_elites_to_label+number_recom) {
							// STEP 3, POPULATE THE NEXT GENERATION OF POINTS (RECOMBINED) //

							// 3.0, start with the base of the current population and
							cweeThreadedList<float> recom_output = orderded_current_gen_population[index_p]; // set the new recombined point to the order position //
							cweeThreadedList<int> gene_locations_from_first_parent;
							int number_gene_locations_to_modify;
							int elite_parent_index;
							if (is_random) {
								// 3.1, determine which elite point is going combined
								number_gene_locations_to_modify = cweeRandomInt(0, gen_input->num_dimensions_d);
								elite_parent_index = cweeRandomInt(0, (gen_input->num_elites - 1)); // randomly select one of the

								// 3.2, randomly determine which locations (genes) will be recombined from the elite selected (if randomization is turned off then we do the first and the last dimension)
								gene_locations_from_first_parent = get_unique_random_in_limits_int(0, gen_input->num_dimensions_d, number_gene_locations_to_modify);
							}
							else { // the deterministic case for testing
								// (alt 3.1 set deterministic location to recombine)
								elite_parent_index = 0;

								// (alt 3.2 determine the genelocatios)
								gene_locations_from_first_parent.Append(0);
							}
							// 3.3, Recombination to get new children
							for (auto locaction_index : gene_locations_from_first_parent) {
								recom_output[locaction_index] = orderded_current_gen_population[elite_parent_index][locaction_index];
							}
							gen_input->next_gen_labels.Append(_RECOM_);
							gen_input->next_gen_population.Append(recom_output);
						}
						else if (index_p < num_elites_to_label + number_recom+number_mutant) {
							// STEP 4, POPULATE THE NEXT GENERATION OF POINTS (MUTANTS) //
							cweeThreadedList<float> mutant_vector;

							// 4.1, if we are using the uniform sampling on the domain to obtain mutants
							if (gen_input->mode == __UNIFORM_MUTATION__) { // 4.1, A completely uniform restarting of the sample space //
								mutant_vector = GetUniformRandomVector(gen_input->upper_box_constraints, gen_input->lower_box_constraints, is_random, control_rand, gen_input->non_random_seed_counter + (index_p * gen_input->num_dimensions_d));
							}
							// 4.2, if we are using normal sampling
							else if (gen_input->mode == __NORMAL_MUTATION__) { // 4.2, A completely normal mutation centered around each variable for a given mutating partcile //
								cweeThreadedList<float> mean_vector, stdv_vector;
								for (int index_dimension = 0; index_dimension < gen_input->num_dimensions_d; index_dimension++) {
									mean_vector.Append(orderded_current_gen_population[index_p][index_dimension]);
									stdv_vector.Append(gen_input->percent_dim_for_normal_variance * (gen_input->upper_box_constraints[index_dimension] - gen_input->lower_box_constraints[index_dimension]));
								}
								mutant_vector = DetermineNormalRandomVector_vectorInput(mean_vector, stdv_vector, is_random, control_rand, gen_input->non_random_seed_counter + (index_p * gen_input->num_dimensions_d));
							}
							// 4.3, label the new entry as a mutant
							gen_input->next_gen_labels.Append(_MUTANT_); // 4.3, label the new vector as a mutant
							gen_input->next_gen_population.Append(mutant_vector);
						}
					}

					// STEP 5, adjust the outputed values to match the boundaries and possible integer constraints
					for (int index_particles = 0; index_particles < gen_input->num_particles_p; index_particles++) {
						if (gen_input->integer_only) {
							gen_input->next_gen_population[index_particles] = round_vector_to_nearest_int(gen_input->next_gen_population[index_particles]);
						}
						if (gen_input->adjust_to_bounds) {
							gen_input->next_gen_population[index_particles] = round_vector_to_nearest_bound(gen_input->next_gen_population[index_particles], gen_input->upper_box_constraints, gen_input->lower_box_constraints);
						}
					}
					gen_input->iterations_completed++;
					return;
				}
			}

		}
		*/

#pragma endregion
		/* The Scatter Search Method */
#pragma region ESS_SEARCH 

		enum essType { __DIVERSE_SAMPLE__, __RECOMBINE__, __STD_DEV_EST__, __RERANDOMIZE__, __GO_BEYOND__, __LOCAL_OPTIMIZE__, __NONE__ }; // there are four different types of iteration 
		enum local_optimizer_type { __NO_LOCAL__, __RANDOM__, __NELDER__, __HOOKES__ };	// type used of the local optimizer

		class essInput : public OptimInput { // the input for scatter search algorithm

		public:
			essType previous_mode = __NONE__;											// 
			essType mode = __NONE__;											// the type of iteration we are preforming since there are multiple sampling steps
			int num_subregions = 0;														// this is consistent across all of the dimensions // 
			int scatter_set_size;														// we will default this to size of the particles // 
			int reference_set_size = 4;													// this needs to be determined independently of the size of the batch

			cweeThreadedList<individual_stat_point> reference_set = {};		// a reference set that represents 
			cweeThreadedList<individual_stat_point> child_reference_set = {};		// a reference set that represents 
			cweeThreadedList<individual_stat_point> candidate_set = {};		// a reference set that represents 
			cweeThreadedList<individual_stat_point> output_set = {};		// temp that probably can be replaced with (see if we can't replace this with cweeThreadedList<cweeeThreadList<float>> //

			int starting_index_for_go_beyond = 0;										// 
			float go_beyond_gamma = 1;
			int base_number_of_points_to_go_beyond;
			bool go_beyond_improvement;													// did the "go beyond" area of the code actually show improvement on this iteration // 
			cweeThreadedList < individual_stat_point >		RefSet;						// this is the reference set for the 		};
			cweeThreadedList < cweeThreadedList<float> >	return_batch_policy;		//
			DiverseSearchObject								sampling_space_record;		// each of the sub-regions for each of the points, we keep track of the sampled bounds
			bool improved = false;
			int improvement = 0;
			int number_policies_for_local_optimization;
			int iterations_for_local_optimizer;
			void* local_optimizer;

			cweeStr GetOptimTypeIDString() { // a debug function to return a string form the object 
				return "optimize_ESS";
			}

			cweeThreadedList < cweeThreadedList<float> > OutputBatchPolicyVectors() { return return_batch_policy; }
			int BatchPolicyVectorSize() { return return_batch_policy.Num(); }
			cweeThreadedList<individual_stat_point> sortReferenceSet(cweeThreadedList<individual_stat_point> candidate_set) { // sort a provided reference set to get the values in the right order		
				cweeThreadedList<individual_stat_point> output;			// 
				cweeThreadedList <float > cost_values = {};
				for (auto entry : candidate_set) { cost_values.Append(entry.cost); } // go through every element of the candidate set  // 

				std::vector<float> updated_values = cost_values;
				std::vector<individual_stat_point> updated_vector = candidate_set;

				while (updated_values.size() > 0) {
					int min_index = std::min_element(updated_values.begin(),
						updated_values.end()) - updated_values.begin();
					output.Append(updated_vector[min_index]);
					updated_values.erase(updated_values.begin() + min_index);
					updated_vector.erase(updated_vector.begin() + min_index);
				}
				return output;
			}
			std::pair<float, float> CalculateStatistics(cweeThreadedList<individual_stat_point> input_set) {
				std::pair<float, float> output;
				double Sum = 0;
				double Sum_sqr = 0;

				for (int i = 0; i < input_set.Num(); ++i) {
					Sum = Sum + input_set[i].cost;
					Sum_sqr = Sum_sqr + input_set[i].cost * input_set[i].cost;
				}

				double variance = (Sum_sqr - (Sum * Sum) / input_set.Num()) / (input_set.Num() - 1);
				output.first = Sum / input_set.Num();
				output.second = sqrt(variance);
				return output;
			}
			void IterateOptimizer(const cweeThreadedList<float> inputed_particle_values = {}, bool is_random = true, non_random_type_t control_rand = __NONE_NON_RANDOM__) {

				AddRecordedValues(inputed_particle_values);		// add the record values to the main object // 
				ConfirmRecordsAndReorderRecords();				//

				// determine which mode we ran on the previous iteration and update the objects inside 
				switch (previous_mode) {
				case __NONE__: {
					mode = __DIVERSE_SAMPLE__;
				}
				case __DIVERSE_SAMPLE__: {
					// integrate the policy values into the candidate set // 
					for (int index_reference_set = 0; index_reference_set < candidate_set.Num(); index_reference_set++) {
						output_set[index_reference_set].cost = inputed_particle_values[index_reference_set];
					}

					// sort the reference set and the child set // 
					cweeThreadedList<individual_stat_point> sorted_output_set = sortReferenceSet(output_set); // sort the reference set according to the inputed values 
					cweeThreadedList<individual_stat_point> sorted_reference_set = sortReferenceSet(reference_set);
					int min_limit_index = std::min(reference_set.Num(), sorted_output_set.Num());
					mode = __DIVERSE_SAMPLE__;
				}
				case __RECOMBINE__: {
					mode = __DIVERSE_SAMPLE__;
				}
				case __GO_BEYOND__: {  // try to improve the candidate set in order to improve the solution that is given					
					for (int output_set_index = 0; output_set_index < output_set.Num(); output_set_index++) {
						output_set[output_set_index].cost = inputed_particle_values[output_set_index]; // update the output set with the new values
					}
					int number_successful_go_beyond_this_it = 0;
					int index_for_input_vector = 0;
					for (int index_output_points = starting_index_for_go_beyond; index_output_points < std::min(starting_index_for_go_beyond + base_number_of_points_to_go_beyond, child_reference_set.Num()); index_output_points++) {
						// evaluate if the new child is better or worse than the previous //
						if (inputed_particle_values[index_for_input_vector] < child_reference_set[index_output_points].cost) {
							child_reference_set[index_output_points] = output_set[index_for_input_vector];
							index_for_input_vector++;

							// IMPROVE: Move the copy out of the for, so, there is only one
							improved = true;
							improvement++;
							if (2 == improvement) { // every time we get at least two improvements, we lower the gamma value // 
								improvement = 0;
								go_beyond_gamma /= 2;
							}
							number_successful_go_beyond_this_it++;
						}
						if (number_successful_go_beyond_this_it < base_number_of_points_to_go_beyond) {
							mode = __DIVERSE_SAMPLE__;
						}
						else {
							mode = __GO_BEYOND__;  // keep trying to find more points to "go beyond"
						}
					}
				}
				case __STD_DEV_EST__: {
					mode = __DIVERSE_SAMPLE__;
				}
				case __RERANDOMIZE__: {
					mode = __DIVERSE_SAMPLE__;
				}
				}
				// determine which mode we are running to get the new points  // 
				switch (mode) {
				case __DIVERSE_SAMPLE__: { // 
					if (sampling_space_record.isNotPopulated()) {
						sampling_space_record = PopulateDiverseSampling(num_subregions, upper_box_constraints, lower_box_constraints);
					}
					sampling_space_record = sampleDiverseSet(num_particles_p, sampling_space_record, upper_box_constraints, lower_box_constraints, num_subregions); // sample the number of regions 
				} //
				case __RECOMBINE__: { // we start by recombining each of the points 
					for (int input_index = 0; input_index < reference_set_size; ++input_index) {
						float refset_extension_side_one, refset_extension_side_two;  // we linearly combine two extensions of the selected point
						float alpha_direction_component, beta_direction_component;  // we have two components that determine the "up vs down" direction that 
						int candidate_set_insertion_index = 0;
						for (int ref_set_index = 0; ref_set_index < reference_set_size; ++ref_set_index) {
							alpha_direction_component = (input_index < ref_set_index) ? 1 : -1;
							beta_direction_component = abs(ref_set_index - input_index);
							for (int dimension_index = 0; dimension_index < num_dimensions_d; ++dimension_index) {
								float distance_from_selected_point = -reference_set[input_index].policy_point[dimension_index]; // 

								// upddate the extensions of both sizes 
								refset_extension_side_one = reference_set[input_index].policy_point[dimension_index] - distance_from_selected_point * (1 + alpha_direction_component * beta_direction_component);
								refset_extension_side_two = reference_set[input_index].policy_point[dimension_index] + distance_from_selected_point * (1 + alpha_direction_component * beta_direction_component);

								// conform to the upper and lower bounds // 
								refset_extension_side_one = std::max(lower_box_constraints[dimension_index], refset_extension_side_one);
								refset_extension_side_one = std::min(lower_box_constraints[dimension_index], refset_extension_side_one);
								refset_extension_side_two = std::max(lower_box_constraints[dimension_index], refset_extension_side_two);
								refset_extension_side_two = std::min(lower_box_constraints[dimension_index], refset_extension_side_two);

								candidate_set[candidate_set_insertion_index].policy_point[dimension_index] = refset_extension_side_one + (refset_extension_side_two - refset_extension_side_one) * cweeRandomFloat(0, 1);
							}
							candidate_set_insertion_index++;
						}
					}
					previous_mode = __RECOMBINE__;
				}
				case __STD_DEV_EST__: {
					previous_mode = __STD_DEV_EST__;
				}
				case __RERANDOMIZE__: {
					previous_mode = __RERANDOMIZE__;
				}
				case __GO_BEYOND__: {  // we are going to replace this mode with "GO_BEYOND" 
					bool improved;
					int gamma = 1;
					int improvement = 1;
				}
				case __LOCAL_OPTIMIZE__: {
					previous_mode = __LOCAL_OPTIMIZE__;
				}
				}

				AddNewRecordedPoints(OutputBatchPolicyVectors());  // add the recorded points to the object // 
				return;
			}
			essInput() {}
			essInput(const cweeThreadedList<float>& upper_bound, const cweeThreadedList<float>& lower_bound, int num_particles_in = 5, sampling_type input = _UNIFORM_RANDOM_SAMPLE_) : OptimInput(upper_bound, lower_bound, num_particles_in) {
				mode = __DIVERSE_SAMPLE__;
				if (num_particles_in < 0) {
					scatter_set_size = num_particles_p;
				}
				num_subregions = floor(scatter_set_size / num_dimensions_d);
				sampling_space_record = PopulateDiverseSampling(num_subregions, upper_box_constraints, lower_box_constraints);
			}
			~essInput() {}
			void CopyOptimizerData(void* input_optimization_information, cweeThreadedList<float> search_space_map) {
				if (input_optimization_information != nullptr) {
					CopyOptimizerBaseData((OptimInput*)input_optimization_information, (OptimInput*)this, search_space_map);
					previous_mode = previous_mode;
					mode = mode;
					num_subregions = num_subregions;
					scatter_set_size = scatter_set_size;
					reference_set_size = reference_set_size;
					reference_set = reference_set;
					child_reference_set = child_reference_set;
					candidate_set = candidate_set;
					output_set = output_set;
					starting_index_for_go_beyond = starting_index_for_go_beyond;
					go_beyond_gamma = go_beyond_gamma;
					base_number_of_points_to_go_beyond = base_number_of_points_to_go_beyond;
					go_beyond_improvement = go_beyond_improvement;
					RefSet = RefSet;
					return_batch_policy = return_batch_policy;
					sampling_space_record = sampling_space_record;
					improved = improved;
					improvement = improvement;
					number_policies_for_local_optimization = number_policies_for_local_optimization;
				}
			}
			OptimType GetOptimizationType() { return cweeMath::Optim::OptimType::OPT_ESS; }   // return the the optimization type // 
		};

#pragma endregion
		/* CMAES - Search */
#pragma region CMAES_CODE 
		/* CMAES Algorithm */
		class CMAESInput : public OptimInput {
		public:
			bool integer_only = false;
			cweeThreadedList < cweeThreadedList<float> > output_for_sampling;  // the points that we are outputting relative to our sampling method we are using this iteration //

			// output the batch size and batch locations // 
			cweeThreadedList < cweeThreadedList<float>> OutputBatchPolicyVectors() { return output_for_sampling; }
			int BatchPolicyVectorSize() { return output_for_sampling.Num(); } // return
			void IterateOptimizer(const cweeThreadedList<float> inputed_particle_values = {}, bool is_random = true, non_random_type_t control_rand = __NONE_NON_RANDOM__) {}


			cweeStr GetOptimTypeIDString() { // a debug function to return a string form the object 
				return "optimize_CMAES";
			}

			CMAESInput() {}
			CMAESInput(const cweeThreadedList<float>& upper_bound, const cweeThreadedList<float>& lower_bound, int num_particles_in = 5, sampling_type input = _UNIFORM_RANDOM_SAMPLE_) : OptimInput(upper_bound, lower_bound, num_particles_in) {}
			~CMAESInput() {}
			void CopyOptimizerData(void* input_optimization_information, cweeThreadedList<float> search_space_map) {
				if (input_optimization_information != nullptr) {
					CopyOptimizerBaseData((OptimInput*)input_optimization_information, (OptimInput*)this, search_space_map);
				}
				return;
			}
			OptimType GetOptimizationType() { return cweeMath::Optim::OptimType::OPT_CMA_ES; }   // return the the optimization type // 
		};
		/* Initialization CMAES Input  */
		void static InitializeCMAESInput(CMAESInput* cmaes_input, cweeThreadedList<float> upper_bounds, cweeThreadedList<float> lower_bounds) {
			return;
		}
		void static CMAESInput_PerformIteration(CMAESInput* cmaes_input) {
			return;
		}

#pragma endregion
		/* The Hybrid Code */
#pragma region HYBRID_CODE

// try this (for ease of reading), 
// For the integer modes that we are using, 
// MODE = 0, straight copy over // 
// MODE = 1, focus on the centroid of a collection of points //  
// MODE = 2, alternative parametrization 
		template <typename T_1, typename T_2>
		void static transfer_function_to_explore(T_2* input_pointer, int mode, float threshold_value_input, T_1* output_pointer) {
			if (input_pointer->num_dimensions_d != input_pointer->num_dimensions_d) { // if the dimensions of the two objects have to match otherwise we return without making any modifications // 
				return;
			}
			if (mode == 0) { // right now mode 0, is the only thing that we support, a direct duplication of the parameters with no modification of any other optimizer information (base case) 
				output_pointer->num_dimensions_d = input_pointer->num_dimensions_d;								// length d
				output_pointer->num_particles_p = input_pointer->num_particles_p;								// length p (RENAME BATCH SIZE)
				output_pointer->upper_box_constraints = input_pointer->upper_box_constraints;							// length d
				output_pointer->lower_box_constraints = input_pointer->lower_box_constraints;							// length d
				output_pointer->initial_points = input_pointer->initial_points;								// only here in the event that the user provides an initial points
				output_pointer->dependent_bound_vector = input_pointer->dependent_bound_vector;

				// get the new points based on the new upper and lower bounds of the system // 
				output_pointer->recorded_values = input_pointer->recorded_values;
				output_pointer->recorded_points = input_pointer->recorded_points;
			}
			else if (mode == 1) {
				return;
			}
			else if (mode == 2) {
				return;
			}
			return;
		}
		template <typename T_1, typename T_2>
		void static transfer_function_to_eploit(T_2* input_pointer, int mode, float threshold_value_input, T_1* output_pointer) {
			if (input_pointer->num_dimensions_d != input_pointer->num_dimensions_d) { // if the dimensions of the two objects have to match otherwise we return without making any modifications // 
				return;
			}
			if (mode == 0) { // right now mode 0, is the only thing that we support, a direct duplication of the parameters with no modification of any other optimizer information (base case) 
				output_pointer->num_dimensions_d = input_pointer->num_dimensions_d;								// length d
				output_pointer->num_particles_p = input_pointer->num_particles_p;								// length p (RENAME BATCH SIZE)
				output_pointer->upper_box_constraints = input_pointer->upper_box_constraints;							// length d
				output_pointer->lower_box_constraints = input_pointer->lower_box_constraints;							// length d
				output_pointer->initial_points = input_pointer->initial_points;								// only here in the event that the user provides an initial points
				output_pointer->dependent_bound_vector = input_pointer->dependent_bound_vector;

				// get the new points based on the new upper and lower bounds of the system // 
				output_pointer->recorded_values = input_pointer->recorded_values;
				output_pointer->recorded_points = input_pointer->recorded_points;
				return;
			}
			else if (mode == 1) {

				// //
				output_pointer->num_dimensions_d = input_pointer->num_dimensions_d;								// length d
				output_pointer->num_particles_p = input_pointer->num_particles_p;								// length p (RENAME BATCH SIZE)

				// // 
				output_pointer->lower_box_constraints = getLowerBoundsFromRegionBasedOnPointLocation(GetCentroidPoints(&subsetListofVectorsFromTheFront(input_pointer->recorded_points), (int)threshold_value_input), input_pointer->diverseSamplingRecord);
				output_pointer->upper_box_constraints = getUpperBoundsFromRegionBasedOnPointLocation(GetCentroidPoints(&subsetListofVectorsFromTheFront(input_pointer->recorded_points), (int)threshold_value_input), input_pointer->diverseSamplingRecord);
				output_pointer->initial_points = input_pointer->initial_points;								// only here in the event that the user provides an initial points
				output_pointer->dependent_bound_vector = input_pointer->dependent_bound_vector;

				// get the new points based on the new upper and lower bounds of the system // 
				output_pointer->recorded_values = input_pointer->recorded_values;
				output_pointer->recorded_points = input_pointer->recorded_points;
				return;
			}
			else if (mode == 2) {
				return;
			}
			return;
		}
		template<>
		void static transfer_function_to_explore(NelderMeadUpdate* input_pointer, int mode, float threshold_value_input, essInput* output_pointer) {
			if (input_pointer->num_dimensions_d != input_pointer->num_dimensions_d) { // if the dimensions of the two objects have to match otherwise we return without making any modifications // 
				return;
			}
			if (mode == 0) { // right now mode 0, is the only thing that we support, a direct duplication of the parameters with no modification of any other optimizer information (base case) 
				output_pointer->num_dimensions_d = input_pointer->num_dimensions_d;								// length d
				output_pointer->num_particles_p = input_pointer->num_particles_p;								// length p (RENAME BATCH SIZE)
				output_pointer->upper_box_constraints = input_pointer->upper_box_constraints;							// length d
				output_pointer->lower_box_constraints = input_pointer->lower_box_constraints;							// length d
				output_pointer->initial_points = input_pointer->initial_points;								// only here in the event that the user provides an initial points
				output_pointer->dependent_bound_vector = input_pointer->dependent_bound_vector;
			}
			else if (mode == 1) {
				return;
			}
			else if (mode == 2) {
				return;
			}
			return;
		}
		template<>
		void static transfer_function_to_eploit(essInput* input_pointer, int mode, float threshold_value_input, NelderMeadUpdate* output_pointer) {
			if (input_pointer->num_dimensions_d != input_pointer->num_dimensions_d) {		// if the dimensions of the two objects have to match otherwise we return without making any modifications // 
				return;
			}
			if (mode == 0) {		// right now mode 0, is the only thing that we support, a direct duplication of the parameters with no modification of any other optimizer information (base case) 
				output_pointer->num_dimensions_d = input_pointer->num_dimensions_d;										// length d
				output_pointer->num_particles_p = input_pointer->num_particles_p;										// length p (RENAME BATCH SIZE)
				output_pointer->upper_box_constraints = input_pointer->upper_box_constraints;							// length d
				output_pointer->lower_box_constraints = input_pointer->lower_box_constraints;							// length d
				output_pointer->initial_points = input_pointer->initial_points;											// only here in the event that the user provides an initial points
				output_pointer->dependent_bound_vector = input_pointer->dependent_bound_vector;
			}
			else if (mode == 1) {
				return;
			}
			else if (mode == 2) {
				return;
			}
			return;
		}
		template<>
		void static transfer_function_to_explore(NelderMeadUpdate* input_pointer, int mode, float threshold_value_input, PSOInput_Update* output_pointer) {
			if (input_pointer->num_dimensions_d != input_pointer->num_dimensions_d) { // if the dimensions of the two objects have to match otherwise we return without making any modifications // 
				return;
			}
			if (mode == 0) { // right now mode 0, is the only thing that we support, a direct duplication of the parameters with no modification of any other optimizer information (base case) 
				output_pointer->num_dimensions_d = input_pointer->num_dimensions_d;									// length d
				output_pointer->num_particles_p = input_pointer->num_particles_p;									// length p (RENAME BATCH SIZE)
				output_pointer->upper_box_constraints = input_pointer->upper_box_constraints;						// length d
				output_pointer->lower_box_constraints = input_pointer->lower_box_constraints;						// length d
				output_pointer->initial_points = input_pointer->initial_points;										// only here in the event that the user provides an initial points
				output_pointer->dependent_bound_vector = input_pointer->dependent_bound_vector;
			}
			else if (mode == 1) {
				return;
			}
			else if (mode == 2) {
				return;
			}
			return;
		}
		template<>
		void static transfer_function_to_eploit(PSOInput_Update* input_pointer, int mode, float threshold_value_input, NelderMeadUpdate* output_pointer) {
			if (input_pointer->num_dimensions_d != input_pointer->num_dimensions_d) { // if the dimensions of the two objects have to match otherwise we return without making any modifications // 
				return;
			}
			if (mode == 0) { // right now mode 0, is the only thing that we support, a direct duplication of the parameters with no modification of any other optimizer information (base case) 
				output_pointer->num_dimensions_d = input_pointer->num_dimensions_d;								// length d
				output_pointer->num_particles_p = input_pointer->num_particles_p;								// length p (RENAME BATCH SIZE)
				output_pointer->upper_box_constraints = input_pointer->upper_box_constraints;							// length d
				output_pointer->lower_box_constraints = input_pointer->lower_box_constraints;							// length d
				output_pointer->initial_points = input_pointer->initial_points;								// only here in the event that the user provides an initial points
				output_pointer->dependent_bound_vector = input_pointer->dependent_bound_vector;
			}
			else if (mode == 1) {
				return;
			}
			else if (mode == 2) {
				return;
			}
			return;
		}

		// For the integer modes that we are using, 
		// MODE = 0, is no transition return the same point as is inputted // 
		// MODE = 1, are the iterations threshold, using the number of iterations completed to select the new optimizer
		// MODE = 2, we use a statistics measurement (e.g. the variance and or the mean between all of the diverse regions) 
		// MODE = 3, is the alternative determined by the method   
		template <typename T_1>
		bool static WillExploitNext(int mode_for_transfer, float threshold, T_1* input_pointer) {
			// determines whether the next iteration will try to eploit the area
			switch (mode_for_transfer) {
			case 0: {
				return false;
			}
			case 1: {  // if the iterations completed meet the threshold value // 
				if (input_pointer->iterations_completed > threshold) { return true; }
			}
			case 2: {
				return false;
			}
			case 3: {
				return false;
			}
			}

			return false; // return the fact that we are not actually going to be changing the optimizer // 
		}
		template <typename T_1>
		bool static WillExploreNext(int mode_for_transfer, float threshold, T_1* input_pointer) {

			// determines whether the next iteration will try to eploit the area
			switch (mode_for_transfer) {
			case 0: {
				return false;
			}
			case 1: {
				if (input_pointer->iterations_completed > threshold) { return true; }
			}
			case 2: {
				return false;
			}
			case 3: {
				return false;
			}
			}

			return false; // return the fact that we are not actually going to be changing the optimizer // 
		}


		/* instantiate all of the functions that we need in order to properly do the transfer */
		// the hybrid template that will take in two types that are optimization type classses
		class HybridOptimType {

		public:
			// two floats that characterzie our entry with a mode that we use to determine the method of transition // 
			float DecisionThresholdValue, ConversionThresholdValue; // the thresholds used for the decision and conversion value
			OptimType exploit_method = cweeMath::Optim::OptimType::OPT_NONE;
			OptimType explore_method = cweeMath::Optim::OptimType::OPT_NONE;
			OptimType previous_method_executed = cweeMath::Optim::OptimType::OPT_NONE;
			bool is_current_mode_explore = true;
			int mode_for_tranfer, mode_for_decision;

			// constructors and destructors // 
			HybridOptimType() { }
			HybridOptimType(OptimType type_1, OptimType type_2) { exploit_method = type_1; explore_method = type_2; }
			~HybridOptimType() { }
		};

#pragma endregion
#endif
	};

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



