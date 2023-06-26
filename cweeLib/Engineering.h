#ifndef __ENG_H__
#define __ENG_H__

INLINE int vec3d_compare(const void* a, const void* b)
{
	vec3d& aR = *((vec3d*)a);
	vec3d& bR = *((vec3d*)b);
	if (aR.x < bR.x) return 1;
	else return 0;
	//return -1;
}
INLINE void vec3d_swap(vec3d* a, vec3d* b, vec3d& temp)
{
	temp = *a;
	*a = *b;
	*b = temp;
}
INLINE int vec3d_partition(vec3d* arr, int low, int high)
{
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
INLINE void vec3d_quickSort(vec3d* arr, int low, int high)
{
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

class cweeEng {
public: // Public Interface for Generalized Optimization Method

	template <typename globalDataType, typename particleDataType>
	struct optimizationData {

		// JOBS TO BE CALLED //
		jobRun_t ScheduleIteration = NULL;
		jobRun_t EvaluatePolicy = NULL;
		jobRun_t FinishIteration = NULL; 
		jobRun_t ReviewStopConditions = NULL;

		// GLOBAL DATA DURING OPTIMIZATION //
		globalDataType* optimizationDatabase = NULL;

		// PARTICLE-UNIQUE DATA //
		particleDataType policyData;

		// METHODS //
		bool isIndividualPolicy(void) {
			return (policyData == nullptr ? true : false);
		};
	};
	
	template<typename optimizationDatabase_t, typename policyData_t, jobRun_t ScheduleIteration, jobRun_t EvaluatePolicy, jobRun_t FinishIteration, jobRun_t ReviewStopConditions>
	static void Optimize(
		const optimizationDatabase_t& optimizationDatabase
	) {
		optimizationDatabase_t* io = new optimizationDatabase_t;
		*io = optimizationDatabase;

		cweeEng::StartOptimization<
			optimizationDatabase_t,
			policyData_t,
			ScheduleIteration,
			EvaluatePolicy,
			FinishIteration,
			ReviewStopConditions
		>(
			io
		);
	};

	template<typename optimizationDatabase_t, typename policyData_t, jobRun_t ScheduleIteration, jobRun_t EvaluatePolicy, jobRun_t FinishIteration, jobRun_t ReviewStopConditions>
	static void Optimize(
		const optimizationDatabase_t& optimizationDatabase, bool useAllThreads
	) {
		if (!useAllThreads) {
			cweeEng::Optimize<
				optimizationDatabase_t,
				policyData_t,
				ScheduleIteration,
				EvaluatePolicy,
				FinishIteration,
				ReviewStopConditions
			>(
				optimizationDatabase
				);
		}
		else {
			optimizationDatabase_t* io = new optimizationDatabase_t;
			*io = optimizationDatabase;

			cweeEng::StartOptimization<
				optimizationDatabase_t,
				policyData_t,
				ScheduleIteration,
				EvaluatePolicy,
				FinishIteration,
				ReviewStopConditions
			>(
				io, true
			);
		}
	};

private: // Private Support Methods for Generalized Optimization Method

	// A = SharedOptimizationDataType, B = ParticleOptimizationDataType
	template< typename A, typename B, jobRun_t ScheduleIteration, jobRun_t EvaluatePolicy, jobRun_t FinishIteration, jobRun_t ReviewStopConditions>
	static void StartOptimization(A* optimizationDatabase) {
		// Start iteration. 
		optimizationData<A, B>* io = new optimizationData<A, B>; {
			io->optimizationDatabase = optimizationDatabase;
			io->ScheduleIteration = ScheduleIteration;
			io->EvaluatePolicy = EvaluatePolicy;
			io->FinishIteration = FinishIteration;
			io->ReviewStopConditions = ReviewStopConditions;
		}

		// Allow user to schedule jobs within the iteration. 
		cweeMultithreading::ADD_JOB(
			(jobRun_t)cweeEng::Opt_ScheduleIteration<A, B>,
			io,
			jobType::OPT_thread
		);
	};

	template< typename A, typename B, jobRun_t ScheduleIteration, jobRun_t EvaluatePolicy, jobRun_t FinishIteration, jobRun_t ReviewStopConditions>
	static void StartOptimization(A* optimizationDatabase, bool useAllThreads) {

		if (!useAllThreads) {
			cweeEng::StartOptimization<
				A,
				B,
				ScheduleIteration,
				EvaluatePolicy,
				FinishIteration,
				ReviewStopConditions
			>(
				optimizationDatabase
			);
		}
		else {
			// Start iteration. 
			optimizationData<A, B>* io = new optimizationData<A, B>; {
				io->optimizationDatabase = optimizationDatabase;
				io->ScheduleIteration = ScheduleIteration;
				io->EvaluatePolicy = EvaluatePolicy;
				io->FinishIteration = FinishIteration;
				io->ReviewStopConditions = ReviewStopConditions;
			}

			// Allow user to schedule jobs within the iteration. 
			cweeMultithreading::ADD_JOB(
				(jobRun_t)cweeEng::Opt_ScheduleIterationAllThreads<A, B>,
				io
			);
		}
	};


	template< typename A, typename B >
	static void Opt_ScheduleIteration(optimizationData<A, B>* in) {		
		// Allow user to schedule the iteration. 
		in->ScheduleIteration(in);

		// Perform the iteration.
		in->optimizationDatabase->policies.Lock();
		AUTO policies = in->optimizationDatabase->policies.UnsafeRead();		
		for (int i = 0; i < policies->Num(); i++) {
			// Increment the counter of jobs to-be-done. 
			{
				in->optimizationDatabase->policyProgress.Lock();
				auto ptr = in->optimizationDatabase->policyProgress.UnsafeRead();
				if (ptr) *ptr += 1;
				in->optimizationDatabase->policyProgress.Unlock();
			}
		}
		for (int i = 0; i < policies->Num(); i++) {
			// Submit the job.
			cweeMultithreading::ADD_JOB(
				(jobRun_t)cweeEng::Opt_EvaluatePolicy<A, B>,
				&policies->operator[](i),
				jobType::OPT_thread
			);
		}
		in->optimizationDatabase->policies.Unlock();

		// End the iteration.
		cweeMultithreading::ADD_JOB(
		(jobRun_t)cweeEng::Opt_FinishIteration<A, B>,
			in,
			jobType::OPT_thread
		);
	};

	template< typename A, typename B >
	static void Opt_ScheduleIterationAllThreads(optimizationData<A, B>* in) {
		// Allow user to schedule the iteration. 
		in->ScheduleIteration(in);

		// Perform the iteration.
		in->optimizationDatabase->policies.Lock();
		cweeThreadedList< optimizationData< A, B > >* policies = in->optimizationDatabase->policies.UnsafeRead();
		for (int i = 0; i < policies->Num(); i++) {
			// Increment the counter of jobs to-be-done. 
			{
				in->optimizationDatabase->policyProgress.Lock();
				auto ptr = in->optimizationDatabase->policyProgress.UnsafeRead();
				if (ptr) *ptr += 1;
				in->optimizationDatabase->policyProgress.Unlock();
			}
		}
		for (int i = 0; i < policies->Num(); i++) {
			// Submit the job.
			cweeMultithreading::ADD_JOB(
				(jobRun_t)cweeEng::Opt_EvaluatePolicy<A, B>,
				&policies->operator[](i)
			);
		}
		in->optimizationDatabase->policies.Unlock();

		// End the iteration.
		cweeMultithreading::ADD_JOB(
			(jobRun_t)cweeEng::Opt_FinishIterationAllThreads<A, B>,
			in
		);
	};


	template< typename A, typename B >
	static void Opt_EvaluatePolicy(optimizationData<A, B>* in) {
		in->EvaluatePolicy(in);

		bool temp = false;
		if (cweeGetManualPolicyCompletionTrigger(*in->optimizationDatabase, temp) && temp) {
			// this will complete itself and count itself down when ready.

		}
		else {
			// we are responsible for counting this down.

			// Decrement the counter of jobs to-be-done. 
			if (in->optimizationDatabase) {
				in->optimizationDatabase->policyProgress.Lock();
				auto ptr = in->optimizationDatabase->policyProgress.UnsafeRead();
				if (ptr) *ptr -= 1;
				in->optimizationDatabase->policyProgress.Unlock();
			}
		}



	};

	template< typename A, typename B >
	static void Opt_FinishIteration(optimizationData<A, B>* in) {
		// Spin in place till the expected number of jobs are complete.	
		if (in->optimizationDatabase->policyProgress > 0) {
			// try again later. This prevents this job from stalling other jobs on low-end computers.
			cweeMultithreading::ADD_JOB(
				(jobRun_t)cweeEng::Opt_FinishIteration<A, B>,
				in,
				jobType::OPT_thread
			);
		}
		else {
			in->optimizationDatabase->policyProgress = 0;

			// Ensure all previous policies are gone.
			in->optimizationDatabase->policies.Clear();

			// Allow user to perform post-processing.
			in->FinishIteration((A*)in->optimizationDatabase);

			// Allow user to perform a review of stop conditions based on final results. 
			cweeMultithreading::ADD_JOB(
				(jobRun_t)cweeEng::Opt_ReviewStopConditions<A, B>,
				in,
				jobType::OPT_thread
			);
		}
	};

	template< typename A, typename B >
	static void Opt_FinishIterationAllThreads(optimizationData<A, B>* in) {
		// Spin in place till the expected number of jobs are complete.	
		if (in->optimizationDatabase->policyProgress > 0) {
			// try again later. This prevents this job from stalling other jobs on low-end computers.
			cweeMultithreading::ADD_JOB(
				(jobRun_t)cweeEng::Opt_FinishIterationAllThreads<A, B>,
				in
			);
		}
		else {
			in->optimizationDatabase->policyProgress = 0;

			// Ensure all previous policies are gone.
			in->optimizationDatabase->policies.Clear();

			// Allow user to perform post-processing.
			in->FinishIteration((A*)in->optimizationDatabase);

			// Allow user to perform a review of stop conditions based on final results. 
			cweeMultithreading::ADD_JOB(
				(jobRun_t)cweeEng::Opt_ReviewStopConditions<A, B>,
				in
			);
		}
	};


	template< typename A, typename B >
	static void Opt_ReviewStopConditions(optimizationData<A, B>* in) {
		// Allow user to perform a review of stop conditions based on final results. 
		in->ReviewStopConditions((A*)in->optimizationDatabase);		

		// End iteration.
		
		if (in) {
			if (in->optimizationDatabase)
				delete in->optimizationDatabase;
			delete in;
		}
	};

public: // General engineering calculations

	/*!
	Assumes head on y-axis and flow on x-axis.
	*/
	static float SamplePumpHeadCurve(Curve curve, float flow) {
		float out = 0;
		switch (curve.GetNumValues()) {
		case 0: {
			// nothing can be done.			
			break;
		}
		case 1: {
			float designHead = curve.GetValue(0);
			float designFlow = curve.GetTime(0);

			curve.AddUniqueValue(0, designHead * 1.33f); // epanet approach
			curve.AddUniqueValue(designFlow * 2.0f, 0); // epanet approach

			out = curve.GetCurrentValue(flow); // catmull_rom spline sample
			break;
		}
		case 2: {
			float designHead = cweeMath::Faverage({ curve.GetValue(0), curve.GetValue(1) });
			float designFlow = cweeMath::Faverage({ (float)curve.GetTime(0), (float)curve.GetTime(1) });
			float maxFlow = cweeMath::Fmax(designFlow * 2.0f , 1.5f * cweeMath::Fmax((float)curve.GetTime(0), (float)curve.GetTime(1)));
			float maxHead = cweeMath::Fmax(designHead * 1.33f , 1.15f * cweeMath::Fmax((float)curve.GetValue(0), (float)curve.GetValue(1))); 

			curve.AddUniqueValue(0, maxHead);
			curve.AddUniqueValue(maxFlow, 0);

			out = curve.GetCurrentValue(flow); // catmull_rom spline sample
			break;
		}
		case 3: 
		default:
		{
			out = curve.GetCurrentValue(flow); // catmull_rom spline sample
			break;
		}
		}
		return out;
	}

	static float CentrifugalPumpEnergyDemand_kW(float Flow_gpm, float Head_feet, float Efficiency_percent) {
		return (Flow_gpm * Head_feet / Efficiency_percent) * ((0.0022 * 100) / (8.81 * 1.341));
	};
	static float SurfaceAreaCircle_ft2(float Diameter_feet) {
		return cweeMath::PI * Diameter_feet * Diameter_feet / 4.0f;
	};
	static float VolumeCylinder_gal(float Diameter_feet, float Height_feet) {
		return SurfaceAreaCircle_ft2(Diameter_feet)*Height_feet* 7.48052f;
	};
	static float Cylinder_FlowRate_to_LevelRate_fph(float FlowRate_gpm, float Diameter_feet) {
		float out(0);
		float surfaceArea = SurfaceAreaCircle_ft2(Diameter_feet);
		out = FlowRate_gpm * (60.0f) * (1.0f / 7.48052f) * (1.0f / surfaceArea);
		return out;
	};
	static float Cylinder_Volume_to_Level_f(float volume, float Diameter_feet) {
		return volume / SurfaceAreaCircle_ft2(Diameter_feet);
	};
	static float Head_to_Pressure_psi(float HydraulicHead_feet, float BaseElevation_feet) {
		return (HydraulicHead_feet - BaseElevation_feet) * 0.4329004329f;
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

	static float EquivalentPipeDiameter_ConstantFlow(float pipe1_length, float pipe2_length, float pipe1_diameter, float pipe2_diameter) {
		// from: https://pdf.sciencedirectassets.com/278653/1-s2.0-S1877705817X00179/1-s2.0-S1877705817313991/main.pdf?X-Amz-Security-Token=IQoJb3JpZ2luX2VjEPD%2F%2F%2F%2F%2F%2F%2F%2F%2F%2FwEaCXVzLWVhc3QtMSJGMEQCIBvCRSY0I3te7nlj1iryyTb7MJTMPlVrOssmSDmmxV56AiAOJzRyuDwv%2BcxvCkzzBTbcYVnOlp9QuUNZXTAJtEbR9iq9AwjJ%2F%2F%2F%2F%2F%2F%2F%2F%2F%2F8BEAMaDDA1OTAwMzU0Njg2NSIMFIToVymz2aRmF6wtKpEDd85poqyoqiweTRIQd8GtOo8E9%2B%2BWPFX4UGDtF%2BCf3CvpqKTvXU0urOQZ65lYpDzzBYxIKwU2PaQCY7m0iy1blMFKDOS7GfGqB2RcmXVnl2yXpzI%2FbFs2DM8jVvjkwSIJxf3aNcQQJJ5zKMbU3phJkvg55wxYdXAx5FpH6jOfJO02Uq98AE6S0cWS2mIgLlJ%2BGCpDfPXhadC1ZD8BuWQb8Eva26Ps68HXE30%2BXlVnWfhMqvccsYQtWB6Ch7K9wQ3LYrE7Xwts67%2B%2B42zO%2BZ419T0%2FDW7cRUTqGCe17Ur1EaAYCUmXXTZNNGmX7g7hOvms6eDeO%2F8WkllFbotUW0voTLq%2FERQsfZpDVXLSTuOY%2FqOGlIiqvM3VQ6XVpX3K4WxthnRzKQuhsuKn6eU0rhbSsUsrUNximKHemJV5Ve4h%2FChM8q3BvQWMHN2A84EAKzPJ2ukkwuAHYD6PrtwDPiFQqY1m1T43%2BHlc2PSgxWtjm1jnUGpw1wMh0oMYpT369NPfk8unN3QLIij8ZMCtpAVObjww2s2tgAY67AE7DyXRJTBae%2F92UFkxxo7R8PPoP0RsSrYHHpp5zxYei0NOlWNgvZN%2FK%2FTfxzBycZuSvkLmhNm7FIp4c8Khddt3YSbmk1%2BN01vlAI7TymZmGn%2Ff%2Fq8tUyW6hJsdjivJXCW%2BGDeJ8msALEthedb3a0L7Ziic%2Bb6YrmbmvbusHF0LutJ6EWiMa%2B48RK5NkdehNzqWj9MmmkfmTk93RYP80Ll4R6srvlhixx%2Bl9KHxCJPZz8vSki0LFu86P0QwfPfYdcqHyWS0dDkgs2j3CQeCHBtxwtv3y3C8qC4gDEILsS2iPGECN3jFtL4GSWkIFA%3D%3D&X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Date=20210123T014359Z&X-Amz-SignedHeaders=host&X-Amz-Expires=300&X-Amz-Credential=ASIAQ3PHCVTYSWFXPUFL%2F20210123%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Signature=ebf9629e15e043d99b422b093bfef1cc1af095fe76298a81bc4ce33b8c812e2a&hash=16c0bc952b2da6894978af4488940ed24c84691d41e1de87abd2012b16027cfa&host=68042c943591013ac2b2430a89b270f6af2c76d8dfd086a07176afe7c76c2c61&pii=S1877705817313991&tid=spdf-24b18965-1fee-4f69-bcb5-d2f705f42550&sid=6e4529fc8bf8934ce51b64a3dcc0320f0b1dgxrqa&type=client
		return 
			cweeMath::Pow64(
				pipe1_length + pipe2_length
				, 0.2f) 
			/ 
			cweeMath::Pow64(
				(pipe1_length / cweeMath::Pow64(pipe1_diameter, 5.0f))
				+ 
				(pipe2_length / cweeMath::Pow64(pipe2_diameter, 5.0f))
				, 0.2f
			);
	};

	static double EquivalentPipeRoughness(const double& desiredDiameter, const double& pipe1_length, const double& pipe2_length, const double& pipe1_diameter, const double& pipe2_diameter, const double& pipe1_roughness, const double& pipe2_roughness) {
		// From: https://ecommons.udayton.edu/cgi/viewcontent.cgi?article=1013&context=cee_fac_pub
		// Eq. 3.4

		double L_r = pipe1_length + pipe2_length;
		double D_r = desiredDiameter; // cweeMath::Faverage({ pipe1_diameter, pipe2_diameter });

		if (pipe1_diameter == pipe2_diameter && pipe1_roughness == pipe2_roughness) return pipe1_roughness;

		double C_r =
			::pow(L_r / (::pow(D_r, 4.87)), 0.54)
			*
			::pow(
				(pipe1_length / ((::pow(pipe1_diameter, 4.87)) * (::pow(pipe1_roughness, 1.85))))
				+ 
				(pipe2_length / ((::pow(pipe2_diameter, 4.87)) * (::pow(pipe2_roughness, 1.85))))
				, -0.54);

		return C_r;
	};

	static float ReynoldsNumberInPipe(float velocity_ftPerSes, float diameter_inches, float kinematicViscosity_ft2PerSec = 0.000001004f) {
		float diameter_feet = diameter_inches / 12.0f;
		return velocity_ftPerSes * diameter_feet / (kinematicViscosity_ft2PerSec == 0.0f ? 0.000001004 : kinematicViscosity_ft2PerSec);
	};

	static float CelsiusToFahrenheit(float C) {
		return (C * 9.0f / 5.0f) + 32.0f;
	};

	static float FahrenheitToCelsius(float F) {
		return (F - 32.0f) * 5.0f / 9.0f;
	};

	/*! 
	Approximation based on regression. 
	Source: https://www.ewra.net/ew/pdf/EW_2017_58_74.pdf
	*/
	static float PipeRoughnessConversion_HW_to_DW(float HW_roughness) {		
		return
			cweeMath::Fmin(1.5,
				cweeMath::Fmax(0.028,
					(-0.0000028183185026) * (::pow(HW_roughness,4)) 
					+ (0.0014243470892602) * (::pow(HW_roughness, 3)) 
					- (0.267451030006944) * (::pow(HW_roughness, 2)) 
					+ (22.072188172073) * HW_roughness 
					- 673.672526526534
				)
			);
	};
	/*! 
	Approximation based on regression. 
	Source: https://www.ewra.net/ew/pdf/EW_2017_58_74.pdf
	*/
	static float PipeRoughnessConversion_DW_to_HW(float DW_roughness) {
		return
			cweeMath::Fmin(150.0,
				cweeMath::Fmax(100.0,
					(-28.845364406152) * (::pow(DW_roughness, 4))
					+ (34.250225016229) * (::pow(DW_roughness, 3))
					- (62.591795817284) * (::pow(DW_roughness, 2))
					+ (103.276842824486) * DW_roughness
					- 149.373579659483
				)
			);
	};

	/*!
	Approximation based on regression.
	Source: https://www.ewra.net/ew/pdf/EW_2017_58_74.pdf
	*/
	static float PipeFrictionFactor_HW_to_DWFF(float HW_roughness, float ReynoldsNumber, float diameter_inches) {
		return 
			1016.610 
			* ::pow(HW_roughness, -1.85) 
			* ::pow(ReynoldsNumber, -0.148) 
			* ::pow(diameter_inches, -0.0158);
	};

	static float PipeFrictionFactor_DW(float DW_roughness, float ReynoldsNumber, float diameter_inches) {
		if (ReynoldsNumber < 2000) {		
			return 64.0 / ReynoldsNumber;
		}
		else if (ReynoldsNumber > 4000) {
			return 0.25 / ::pow(
				::log(
					(DW_roughness / (3.7 * (diameter_inches / 12.0)))
					+ (5.74 / ::pow(ReynoldsNumber, 0.9))		
				)							
				, 2.0);
		}
		else {
			double X1, R, X2, X3, X4, FA, FB, Y2, Y3;
			R = ReynoldsNumber / 2000.0;			
			Y2 = (DW_roughness / (3.7 * diameter_inches / 12.0)) + (5.74 / (::pow(ReynoldsNumber, 0.9))); 
			Y3 = -0.86859 * ::log(
				(DW_roughness / (3.7 * diameter_inches / 12.0))
				+ (5.74 / (::pow(4000.0,0.9)))
			);
			FA = ::pow(Y3, -2.0);
			FB = FA * (
					2.0
					- ( 0.00514215 / (Y2*Y3) )				
				);

			X1 = 7 * FA - FB;
			X2 = 0.128 - 17.0*FA + 2.5*FB;
			X3 = -0.128 + 13.0 * FA - 2.0 * FB;
			X4 = R * (0.032 - 3.0 * FA + 0.5 * FB);

			return X1 + R*(X2 + R*(X3 + X4));
		}		
	};

	static float PipeResistanceCoefficient_HW(float HW_roughness, float diameter_inches, float length_feet, float velocity_ftPerSes, float kinematicViscosity_ft2PerSec = 0.000001004f) {
		return
			4.727
			* PipeFrictionFactor_HW_to_DWFF(HW_roughness, ReynoldsNumberInPipe(velocity_ftPerSes, diameter_inches, kinematicViscosity_ft2PerSec), diameter_inches)
			* ::pow(diameter_inches / 12.0, -4.871)
			* length_feet;
	};

	static float PipeResistanceCoefficient_HW_to_DW(float HW_roughness, float diameter_inches, float length_feet) {
		return
			0.0252
			* ::pow(HW_roughness, -1.852)
			* ::pow(diameter_inches / 12.0, -5)
			* length_feet;
	};

	static float PipeResistanceCoefficient_DW(float DW_roughness, float diameter_inches, float length_feet, float velocity_ftPerSes, float kinematicViscosity_ft2PerSec = 0.000001004f) {
		return
			0.0252
			* PipeFrictionFactor_DW(DW_roughness, ReynoldsNumberInPipe(velocity_ftPerSes, diameter_inches, kinematicViscosity_ft2PerSec), diameter_inches)
			* ::pow(diameter_inches / 12.0, -5)
			* length_feet;
	};

	static float PipeHeadloss_HW(float resistanceCoefficient, float flowrate_cfs) {
		return resistanceCoefficient * ::pow(flowrate_cfs, 1.852);
	};

	static float PipeHeadloss_DW(float resistanceCoefficient, float flowrate_cfs) {
		return resistanceCoefficient * ::pow(flowrate_cfs, 2.0);
	};

	static float Approximate_EquivalentPipeDiameter(float FrictionFactor, float length_feet, float totalHeadloss_feet, float totalPipeFlow_cfs) {
		return ::pow(
			(8.0 * FrictionFactor * length_feet)
			* (totalPipeFlow_cfs * totalPipeFlow_cfs)
			/ (cweeMath::PI*cweeMath::PI*cweeMath::G* totalHeadloss_feet)
			, 0.2);
	};

	static float Find_EquivalentPipeDiameter_DW(float DW_roughness, float velocity_ftPerSes, float length_feet, float totalHeadloss_feet, float totalPipeFlow_cfs) {
		constexpr float eps = 0.001f;
		float err = 1.0f;
		int trials = 1000;
		// assume Deq
		float Deq = 8.0f; // 8 inches
		float Deq_Prime = 8.0f; // 8 inches
		while (cweeMath::Fabs(err * err) > eps && trials >= 0) {
			float Re = ReynoldsNumberInPipe(velocity_ftPerSes, Deq);
			float ff = PipeFrictionFactor_DW(DW_roughness, Re, Deq);

			Deq_Prime = 12.0 * Approximate_EquivalentPipeDiameter(ff, length_feet, totalHeadloss_feet, totalPipeFlow_cfs);

			err = Deq_Prime - Deq;
			Deq = Deq_Prime;

			trials--;
		}

		return Deq;

	};

	static float Find_EquivalentPipeDiameter_HW(float HW_roughness, float velocity_ftPerSes, float length_feet, float totalHeadloss_feet, float totalPipeFlow_cfs) {
		constexpr float eps = 0.001f;
		float err = 1.0f;
		int trials = 1000;
		// assume Deq
		float Deq = 8.0f; // 8 inches
		float Deq_Prime = 8.0f; // 8 inches
		while (cweeMath::Fabs(err * err) > eps && trials >= 0) {
			float Re = ReynoldsNumberInPipe(velocity_ftPerSes, Deq);
			float ff = PipeFrictionFactor_HW_to_DWFF(HW_roughness, Re, Deq);

			Deq_Prime = 12.0 * Approximate_EquivalentPipeDiameter(ff, length_feet, totalHeadloss_feet, totalPipeFlow_cfs);

			err = Deq_Prime - Deq;
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
		if (NumSides > 0){
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
		float calculate(float setpoint, float pv, const u64& dt = 360);
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

private:

#if 1
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
#endif

public:
	// reorders / drops points such that this can be drawn as a simple polygon without the lines self-crossing 
	static void ReorderConvexHull(cweeThreadedList<vec3>& points) {
#if 1
		{ // get the convex hull
			std::vector<vec3> v; v = points;

			points.Clear();

			for (auto& x : Solution::outerTrees(v))
				points.Append(x);
		}
#else
		{ // get the convex hull
			std::vector<vec3> v; v = points;
			cweeThreadedList<vec3> hull = Solution::outerTrees(v);
			for (int i = points.Num() - 1; i >= 0; i--) {							
				// if the point is in the list, it's OK. 
				if (hull.Find(points[i])) {
					continue;
				}
				if (!IsPointInPolygon(hull, points[i])) {
					hull.Append(points[i]);
				}							
			}
			v = hull;
			points = Solution::outerTrees(v);
		}
#endif

#if 1
		{ // guarrantee the draw order (Clockwise)
			vec3 middle(0, 0, 0);
			int numSamplesX = 0; int numSamplesY = 0;
			for (auto& point : points)
			{
				cweeMath::rollingAverageRef(middle.x, point.x, numSamplesX);
				cweeMath::rollingAverageRef(middle.y, point.y, numSamplesY);
			}
			middle.z = 0;

#if 1		
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

#else
			std::map<int, vec3> ordered;
			constexpr float multiplier = (((float)std::numeric_limits<int>::max()) / 360.0f);
			int key;
			for (int i = 0; i < points.Num(); i++)
			{

				angle = ::fmod(360.0 - angle, 360.0);

				key = cweeMath::Fmax(-360.0f, cweeMath::Fmin(360.0f, AngleOffsetFromVerticle(middle, points[i]))) * multiplier;
#if 1			// test for distance during angle calculation 
				if (ordered.find(key) != ordered.end()) {
					// angle already exists
					if (middle.Distance2d(ordered[key]) < middle.Distance2d(points[i]))
						ordered[key] = points[i]; // override because this is "further out" than the other existing value and should result in more pleasing convex hulls
				}
				else
#endif
					ordered[key] = points[i]; // new angle
			}
			points.Clear();
			for (auto& x : ordered) {
				points.Append(x.second);
			}
#endif
		}
#endif
	};

#if 0
	static void ReorderConvexHull(cweeThreadedList<vec3d>& points) {
		std::vector<vec3d> Out;

		{ // get the convex hull
			std::vector<vec3d> v; v.reserve(points.Num() + 16); v = points;
			Solution::outerTrees(v, Out);
		}

		{ // guarrantee the draw order (Clockwise)
			vec3d middle(0, 0, 0);
			int numSamplesX = 0; int numSamplesY = 0;
			for (auto& point : Out)
			{
				cweeMath::rollingAverageRef(middle.x, point.x, numSamplesX);
				cweeMath::rollingAverageRef(middle.y, point.y, numSamplesY);
			}
			middle.z = 0;

			cweeCurve<vec3d> ordered; ordered.SetGranularity(Out.size() + 16);
			double key;
			for (int i = 0; i < Out.size(); i++)
			{
				key = AngleOffsetFromVerticle(middle, Out[i]);
				key = ::fmod(360.0 - key, 360.0);
				key += 360.0;
				ordered.AddUniqueValue(key, Out[i]);
			}

			points.Clear();
			int n = ordered.GetNumValues();
			for (int i = 0; i < n; i++) points.Append(ordered.GetValue(i));
		}
	};
#else
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
#endif

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

	static bool PolygonsOverlap(const cweeThreadedList<vec3>& polygon1, const cweeThreadedList<vec3>& polygon2) {
		return ObjectsIntersect(polygon1, polygon2);
	};
	static bool PolygonsOverlap(const cweeThreadedList<vec3d>& polygon1, const cweeThreadedList<vec3d>& polygon2) {
		return ObjectsIntersect(polygon1, polygon2);
	};

private:
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

};


#endif