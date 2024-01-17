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
#include "List.h"
#include "Strings.h"
#include "vec.h"
#include "SharedPtr.h"
#include "Mutex.h"
#include "Curve.h"
#include "cwee_math.h"
#include "cweeScheduler.h"

enum class kernel_bit {
	RADIAL_KERNEL,
	GAUSSIAN_KERNEL
}; // create an enumerate set of definitions that contain the various different kernel types
enum class data_type_bit {
	BINARY,
	INTEGER,
	CONTINUOUS
};
class MachineLearningInput {
public:
	/* values to use to predict using (temp, precipitation, etc.) */
	cweeList<cweeList<float>> features;
	/* values to predict in the future; (i.e. water usage) */
	cweeList<float> labels;
	/* Gamma for the fit function */
	float gamma = 0.1f;
	/* Constant for the fit function */
	float c = 1.0f;
	/* Number of Features */
	int NumFeatures() const { return features.Num(); };
	/* Minimum number of Observations */
	int NumObs() const;
	void addFeature(const cweeThreadedList<float>& input);
	cweeList<cweeList<float>> GetInvertedFeatures() const;
	static cweeList<cweeList<float>> GetInvertedFeatures(cweeList<cweeList<float>> const& source);
	cweeList<float> GetLabels() const;
};
class MachineLearning_SVR_Results {
public:
	MachineLearning_SVR_Results();

	cweeSharedPtr<cweeAny>	df2;

	MachineLearning_SVR_Results(MachineLearning_SVR_Results const&) = default;
	MachineLearning_SVR_Results(MachineLearning_SVR_Results&&) = default;
	MachineLearning_SVR_Results& operator=(MachineLearning_SVR_Results const&) = default;
	MachineLearning_SVR_Results& operator=(MachineLearning_SVR_Results&&) = default;

	cweeStr Serialize();
	void Deserialize(cweeStr& in);
};
class MachineLearning_Results {
public:
	MachineLearning_SVR_Results svr_results;
	bool learned = false;
	vec4 performance = vec4(0, cweeMath::INF, 0, cweeMath::INF);

	MachineLearning_Results() = default;
	MachineLearning_Results(MachineLearning_Results const&) = default;
	MachineLearning_Results(MachineLearning_Results&&) = default;
	MachineLearning_Results& operator=(MachineLearning_Results const&) = default;
	MachineLearning_Results& operator=(MachineLearning_Results&&) = default;

	cweeStr Serialize();
	void Deserialize(cweeStr& in);
};
class MachineLearning_Math {
public:
	static MachineLearning_Results		Learn(const MachineLearningInput& input);
	static MachineLearning_Results		LearnFast(const MachineLearningInput& input);
	static void CompileTimeTest();
};

class cweeMachineLearning {
private:
	static float Average(cweeThreadedList<float> const& values) {
		float avg = 0;
		int n = 0;
		for (auto& x : values) cweeMath::rollingAverageRef(avg, x, n);
		return avg;
	};

public:
	class SummarizeFeature {
	public:
		SummarizeFeature() {};
		SummarizeFeature(cweeThreadedList<float> const& in) { Eval(in); };
		float Normalized(float in) const { return (in - min) / range; };
		cweeThreadedList<float> Normalized(cweeThreadedList<float> in)const {
			for (auto& x : in) {
				x = Normalized(x);
			}
			return in;
		};
		float Unnormalized(float in) const {
			return (in * range) + min;
		};
		cweeThreadedList<float> Unnormalized(cweeThreadedList<float> in) const {
			for (auto& x : in) {
				x = Unnormalized(x);
			}
			return in;
		};
		float avg;
		float min;
		float max;
		float range;
		int num;
	private:
		void Eval(cweeThreadedList<float> const& d) {
			avg = Average(d);
			min = cweeMath::INF; for (auto& x : d) min = x < min ? x : min;
			max = -cweeMath::INF; for (auto& x : d) max = x > max ? x : max;
			range = max - min;
			num = d.Num();
		};
	};
	class cweeLearnedParams {
	public:
		cweeThreadedList< SummarizeFeature > featureSummaries;
		SummarizeFeature labelSummary;
		cweeThreadedList<float> fit;
	};

	class OptimizationObj : public cweeOptimizer::sharedClass {
	public:
		int numIterationsFailedImprovement = 0;
	};

	static cweeLearnedParams Learn_Opt(cweeSharedPtr<cweeThreadedList<float>> trueLabels, cweeSharedPtr<cweeThreadedList<cweeThreadedList<float>>> trueFeatures);
	static float Forecast_Opt(const cweeLearnedParams& out, const cweeThreadedList<float>& features);
	static cweeThreadedList<float> Forecast_Opt(const cweeLearnedParams& out, const cweeThreadedList<cweeThreadedList<float>>& features);

public:
	class dlibLearnedParams {
	public:
		cweeThreadedList< SummarizeFeature > featureSummaries;
		SummarizeFeature labelSummary;
		MachineLearning_SVR_Results fit;
	};
	static dlibLearnedParams Learn_Dlib(cweeThreadedList<float> const& trueLabels, cweeThreadedList<cweeThreadedList<float>> const& trueFeatures);
	static float Forecast_Dlib(const dlibLearnedParams& out, const cweeThreadedList<float>& features);
	static cweeThreadedList<float> Forecast_Dlib(const dlibLearnedParams& out, const cweeThreadedList<cweeThreadedList<float>>& features);

public: // Public Interface for Generalized Machine Learning
	/*!
	Generate the machine learned parameters. Optionally, automatically perform a random split of the dataset and generate fit performance utilizing it.
	std::pair<vec2, vec2>  -->  [ [ train_R^2 , train_MSE ] , [ test_R^2 , test_MSE ] ]
	*/
	static MachineLearning_Results Learn(const cweeThreadedList<float>& trueLabels, const cweeThreadedList<cweeThreadedList<float>>& features, std::pair<vec2, vec2>* fit = nullptr, const float percentSplit = 10);
	static MachineLearning_Results LearnFast(const cweeThreadedList<float>& trueLabels, const cweeThreadedList<cweeThreadedList<float>>& features, std::pair<vec2, vec2>* fit = nullptr, const float percentSplit = 10);

	/*!
	Generate the machine learned parameters. Optionally, automatically perform a random split of the dataset and generate fit performance utilizing it.
	std::pair<vec2, vec2>  -->  [ [ train_R^2 , train_MSE ] , [ test_R^2 , test_MSE ] ]
	*/
	static MachineLearning_Results Learn(const cweeThreadedList<std::pair<u64, float>>& trueLabels, const cweeThreadedList<cweeThreadedList<std::pair<u64, float>>>& features, std::pair<vec2, vec2>* fit = nullptr, const float percentSplit = 10);

	/*!
	Generate a machine learned forecast using input features. Note, must match the feature arrangement used during training.
	*/
	static float Forecast(const MachineLearning_Results& controller, const cweeThreadedList<float>& features, const float roundNearest = 0.001f);

	/*!
	Generate a machine learned forecast using input features. Note, must match the feature arrangement used during training.
	*/
	static cweeThreadedList<float> Forecast(const MachineLearning_Results& controller, const cweeThreadedList<cweeThreadedList<float>>& features, const float roundNearest = 0.001f);

	/*!
	Generate a machine learned forecast using input features. Note, must match the feature arrangement used during training.
	*/
	static cweeThreadedList<std::pair<u64, float>> Forecast(const MachineLearning_Results& controller, const cweeThreadedList<cweeThreadedList<std::pair<u64, float>>>& Features, const float roundNearest = 0.001f);

	/*!
	Calculate the error between the observed and modeled data sets.
	Vec2 = [ R^2, MSE ]
	*/
	static vec2 CalculateError(const cweeThreadedList<float>& observed, const cweeThreadedList<float>& modeled);

	/*!
	Calculate the error between the observed and modeled data sets.
	Vec2 = [ R^2, MSE ]
	*/
	static float CalculateLikelihood(const cweeThreadedList<float>& observed, const cweeThreadedList<float>& modeled);

	/*!
	Calculate the error between the observed and modeled data sets.
	Vec2 = [ R^2, MSE ]
	*/
	static vec2 CalculateError(const cweeThreadedList<std::pair<u64, float>>& observed, const cweeThreadedList<std::pair<u64, float>>& modeled);

private: // Private Interface for Machine Learning
	/*!
	Use a support vector to calculate the machine learning parameters.
	*/
	static MachineLearning_Results LearnSVR(const cweeThreadedList<float>& trueLabels, const cweeThreadedList<cweeThreadedList<float>>& features, const cweeThreadedList<float>& testLabels = cweeThreadedList<float>(), const cweeThreadedList<cweeThreadedList<float>>& testfeatures = cweeThreadedList<cweeThreadedList<float>>());
	static MachineLearning_Results LearnSVRFast(const cweeThreadedList<float>& trueLabels, const cweeThreadedList<cweeThreadedList<float>>& features, const cweeThreadedList<float>& testLabels = cweeThreadedList<float>(), const cweeThreadedList<cweeThreadedList<float>>& testfeatures = cweeThreadedList<cweeThreadedList<float>>());

	/*!
	Split the incoming features/labels randomly into outgoing features/labels.
	*/
	static void SplitRandomly(cweeThreadedList<cweeThreadedList<float>>& featuresIn, cweeThreadedList<float>& labelsIn, cweeThreadedList<cweeThreadedList<float>>& featuresOut, cweeThreadedList<float>& labelsOut, float percentSplit);

	/*!
	Split the incoming features/labels randomly into outgoing features/labels.
	*/
	static void SplitRandomly(cweeThreadedList<cweeThreadedList<std::pair<u64, float>>>& featuresIn, cweeThreadedList<std::pair<u64, float>>& labelsIn, cweeThreadedList<cweeThreadedList<std::pair<u64, float>>>& featuresOut, cweeThreadedList<std::pair<u64, float>>& labelsOut, float percentSplit);

public:
	/*!
	Example method using the cweeMachineLearning class.
	*/
	static cweeThreadedList<float> Example(void);
};

#include "chaiscript_wrapper.h"
#include "WaterWatch_Module_Header.h"

namespace chaiscript {
	namespace WaterWatch_Lib {
		[[nodiscard]] ModulePtr MachineLearning_Library();
	};
}; // namespace chaiscript