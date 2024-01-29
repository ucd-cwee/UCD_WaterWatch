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
#include "MachineLearning.h"
#include "Clock.h"
// #define hideMachineLearning

// Machine Learning Libraries
#ifndef hideMachineLearning
#include "../dlib-19.17/source/dlib/bayes_utils.h"
#include "../dlib-19.17/source/dlib/graph_utils.h"
#include "../dlib-19.17/source/dlib/graph.h"
#include "../dlib-19.17/source/dlib/directed_graph.h"
#include "../dlib-19.17/source/dlib/svm.h"
#include "../dlib-19.17/source/dlib/mlp.h"
#endif

int MachineLearningInput::NumObs() const {
	float n = labels.Num() == 0 ? cweeMath::INF : labels.Num();
	for (auto& feat : features) {
		n = cweeMath::Fmin(feat.Num(), n);
	}
	return n >= cweeMath::INF || n < 0 ? (int)0 : (int)n;
};
void MachineLearningInput::addFeature(const cweeThreadedList<float>& input) {
	features.Append(input);
};
cweeList<cweeList<float>> MachineLearningInput::GetInvertedFeatures() const {
	cweeList<cweeList<float>> out(NumObs());
	for (int i = 0; i < NumObs(); i++) {
		cweeList<float> row(NumFeatures());
		for (auto& feat : features) {
			row.Append(feat[i]);
		}
		out.Append(row);
	}
	return out;
};
cweeList<cweeList<float>> MachineLearningInput::GetInvertedFeatures(cweeList<cweeList<float>> const& source) {
	cweeList<cweeList<float>> out;
	if (source.Num() > 0) {
		int n = source[0].Num();
		for (int i = 0; i < n; i++) {
			cweeList<float> row(source.Num());
			for (auto& feat : source) {
				row.Append(feat[i]);
			}
			out.Append(row);
		}
	}
	return out;
};
cweeList<float> MachineLearningInput::GetLabels() const {
	cweeList <float> out = labels;
	if (NumObs() > 0) {
		out.Resize(NumObs());
	}
	return out;
};

#ifndef hideMachineLearning
using sample_type = dlib::matrix<float, 0, 1>;
using kernel_type = dlib::radial_basis_kernel<sample_type>;
// using kernel_type = dlib::linear_kernel<sample_type>;
using df2_t = dlib::decision_function<kernel_type>;
INLINE df2_t* GetDf2(cweeSharedPtr<cweeAny> const& p) {
	if (p && p->IsTypeOf<df2_t>()) {
		df2_t* ptr = p->cast();
		return ptr;
	}
	return nullptr;
};
#endif

MachineLearning_SVR_Results::MachineLearning_SVR_Results() : df2(
	make_cwee_shared<cweeAny>(
#ifndef hideMachineLearning
		new cweeAny(df2_t())
#endif
	)
) {};

MachineLearning_Results		MachineLearning_Math::Learn(const MachineLearningInput& input) {
	MachineLearning_Results output_parameters;
#ifndef hideMachineLearning
	AUTO labels = input.GetLabels();

	std::vector<sample_type> samples;
	{
		auto features_vec = input.GetInvertedFeatures();
		sample_type m;
		samples.reserve(features_vec.Num()); // create vector of different values
		m.set_size(features_vec[0].Num()); // now we train our object on a few samples of the sinc function.
		int fvIFN;
		for (int index_f = 0; index_f < features_vec.Num(); index_f++) {
			fvIFN = features_vec[index_f].Num();

			for (int index_element = 0; index_element < fvIFN; index_element++)
				m(index_element) = features_vec[index_f][index_element];

			samples.push_back(m);
		}
	}

	dlib::svr_trainer<kernel_type> trainer;
	{
		trainer.set_kernel(input.gamma);													//	"0.1"	//	kernal		// 1 / n features (i.e. 1/4 for auto self-regression)
		trainer.set_c(1.0 / input.gamma);																//	"1"		//	c value		// higher = higher accuracy, lower = reduced accuracy. 
		{
			// CPU_DATA this_cpu;
			Computer_Usage this_ram;

			int RAM_mb = this_ram.Ram_MB(); float maxRam;
			{
				RAM_mb *= (100 - cweeMath::Abs(this_ram.PercentMemoryUsed())) / 100.0f;
				maxRam = cweeMath::Fmax(200.0f, cweeMath::Fmin(0.8f * (RAM_mb / 4 /* this_cpu.m_numLogicalCpuCores */), 1000.0f));
			}

			// each thread may utilize UP TO:
			trainer.set_cache_size(maxRam);														//	"1000"	//	memory use	// maximum RAM usage
		}
		{
			float minLabel(std::numeric_limits<float>::max()), maxLabel(-std::numeric_limits<float>::max());
			for (auto& x : labels) {
				if (minLabel > x) minLabel = x;
				if (maxLabel < x) maxLabel = x;
			}
			float diff(1);
			if (maxLabel > minLabel) {
				diff = maxLabel - minLabel;  // i.e. 23, 0.01, 0
				// our desired accuracy is within 0.1% of this difference. If the range of this timeseries is 1, it results in an eps. of 1 / 1000 = 0.001;
			}
			trainer.set_epsilon(diff / 10000.0f); // i.e. 0.01% of the range of the dataset	
			// trainer.set_epsilon(0.001); //	"0.01"	//	epsilon		// maximum error. smaller - longer learning
		}
	}

	*output_parameters.svr_results.df2 = trainer.train(samples, (std::vector<float>)labels);
#endif
	return output_parameters; // output parameters
};
MachineLearning_Results		MachineLearning_Math::LearnFast(const MachineLearningInput& input) {
	MachineLearning_Results output_parameters;
#ifndef hideMachineLearning
	AUTO labels = input.GetLabels();

	std::vector<sample_type> samples;
	{
		auto features_vec = input.GetInvertedFeatures();
		sample_type m;
		samples.reserve(features_vec.Num()); // create vector of different values
		m.set_size(features_vec[0].Num()); // now we train our object on a few samples of the sinc function.
		int fvIFN;
		for (int index_f = 0; index_f < features_vec.Num(); index_f++) {
			fvIFN = features_vec[index_f].Num();

			for (int index_element = 0; index_element < fvIFN; index_element++)
				m(index_element) = features_vec[index_f][index_element];

			samples.push_back(m);
		}
	}

	dlib::svr_trainer<kernel_type> trainer;
	{
		trainer.set_kernel(input.gamma);													//	"0.1"	//	kernal		// 1 / n features (i.e. 1/4 for auto self-regression)
		trainer.set_c(1.0 / input.gamma);																//	"1"		//	c value		// higher = higher accuracy, lower = reduced accuracy. 
		{
			// CPU_DATA this_cpu;
			Computer_Usage this_ram;

			float maxRam;
			{
				int RAM_mb = this_ram.Ram_MB();
				RAM_mb *= (100 - cweeMath::Abs(this_ram.PercentMemoryUsed())) / 100.0f;
				maxRam = cweeMath::Fmax(200.0f, cweeMath::Fmin(0.8f * (RAM_mb / 4 /* this_cpu.m_numLogicalCpuCores */), 1000.0f));
			}
			maxRam *= 0.25f; // further restrain the ram for fast fitting

			// each thread may utilize UP TO:
			trainer.set_cache_size(maxRam);														//	"1000"	//	memory use	// maximum RAM usage
		}
		{
			float minLabel(std::numeric_limits<float>::max()), maxLabel(-std::numeric_limits<float>::max());
			for (auto& x : labels) {
				if (minLabel > x) minLabel = x;
				if (maxLabel < x) maxLabel = x;
			}
			float diff(1);
			if (maxLabel > minLabel) {
				diff = maxLabel - minLabel;  // i.e. 23, 0.01, 0
				// our desired accuracy is within 1% of this difference. If the range of this timeseries is 1, it results in an eps. of 1 / 100 = 0.01;
			}
			trainer.set_epsilon(diff / 1000.0f); // i.e. 0.1% of the range of the dataset
		}
	}

	*output_parameters.svr_results.df2 = trainer.train(samples, (std::vector<float>)labels);

#endif
	return output_parameters; // output parameters
};
void MachineLearning_Math::CompileTimeTest() {
	MachineLearningInput ml_input;
	ml_input.labels.Append(0);
	ml_input.labels.Append(1);
	ml_input.labels.Append(2);
	ml_input.labels.Append(3);

	ml_input.addFeature(ml_input.labels); // predict using itself
	ml_input.addFeature(ml_input.labels); // predict using itself

	AUTO ml_result = MachineLearning_Math::Learn(ml_input);
	std::cout << ml_result.performance.ToString() << std::endl;
};


MachineLearning_Results cweeMachineLearning::Learn(const cweeThreadedList<float>& trueLabels, const cweeThreadedList<cweeThreadedList<float>>& features, std::pair<vec2, vec2>* fit, const float percentSplit) {
	cweeThreadedList<cweeThreadedList<float>> Features;
	MachineLearning_Results out;
	Features.SetGranularity(cweeMath::max(100, features.Num()));

	if (fit == nullptr) {
		out = LearnSVR(trueLabels, features);
		if (percentSplit > 0) {
			auto ML_result = Forecast(out, features);
			vec2 t1 = CalculateError(trueLabels, ML_result);
			out.performance.x = t1.x;
			out.performance.y = t1.y;
		}
	}
	else {
		cweeThreadedList<cweeThreadedList<float>> featuresLearn = features;
		cweeThreadedList<cweeThreadedList<float>> featuresTest(featuresLearn.Num());
		cweeThreadedList<float> labelsLearn = trueLabels;
		cweeThreadedList<float> labelsTest(labelsLearn.Num());

		SplitRandomly(featuresLearn, labelsLearn, featuresTest, labelsTest, percentSplit); // i.e. 10% split.

		//Features = featuresLearn;
		for (auto& x : featuresLearn) {
			cweeThreadedList<float> saver;
			saver.SetGranularity(cweeMath::max(100, x.Num()));
			for (auto& y : x) {
				saver.Append((float)y);
			}
			Features.Append(saver);
		}
		out = LearnSVR(labelsLearn, Features); // to-do, switch statement.

		auto ML_result = Forecast(out, featuresLearn);
		fit->first = CalculateError(labelsLearn, ML_result);

		ML_result = Forecast(out, featuresTest);
		fit->second = CalculateError(labelsTest, ML_result);

		out.performance.x = fit->first.x;
		out.performance.y = fit->first.y;
		out.performance.z = fit->second.x;
		out.performance.w = fit->second.y;
	}
	out.learned = true;
	return out;
};
MachineLearning_Results cweeMachineLearning::LearnFast(const cweeThreadedList<float>& trueLabels, const cweeThreadedList<cweeThreadedList<float>>& features, std::pair<vec2, vec2>* fit, const float percentSplit) {
	cweeThreadedList<cweeThreadedList<float>> Features;
	MachineLearning_Results out;
	Features.SetGranularity(cweeMath::max(100, features.Num()));

	if (fit == nullptr) {
		//Features = features;
		for (auto& x : features) {
			cweeThreadedList<float> saver;
			saver.SetGranularity(cweeMath::max(100, x.Num()));
			for (auto& y : x) {
				saver.Append((float)y);
			}
			Features.Append(saver);
		}
		cweeThreadedList<float> TrueLabels;
		TrueLabels = trueLabels;

		out = LearnSVRFast(TrueLabels, Features); // to-do, switch statement.
		//auto ML_result = Forecast(out, Features);
		//vec2 t1 = CalculateError(TrueLabels, ML_result);
		//out.performance.x = t1.x;
		//out.performance.y = t1.y;
	}
	else {
		cweeThreadedList<cweeThreadedList<float>> featuresLearn = features;
		cweeThreadedList<cweeThreadedList<float>> featuresTest(featuresLearn.Num());
		cweeThreadedList<float> labelsLearn = trueLabels;
		cweeThreadedList<float> labelsTest(labelsLearn.Num());

		SplitRandomly(featuresLearn, labelsLearn, featuresTest, labelsTest, percentSplit); // i.e. 10% split.

		//Features = featuresLearn;
		for (auto& x : featuresLearn) {
			cweeThreadedList<float> saver;
			saver.SetGranularity(cweeMath::max(100, x.Num()));
			for (auto& y : x) {
				saver.Append((float)y);
			}
			Features.Append(saver);
		}
		out = LearnSVRFast(labelsLearn, Features); // to-do, switch statement.

		auto ML_result = Forecast(out, featuresLearn);
		fit->first = CalculateError(labelsLearn, ML_result);

		ML_result = Forecast(out, featuresTest);
		fit->second = CalculateError(labelsTest, ML_result);

		out.performance.x = fit->first.x;
		out.performance.y = fit->first.y;
		out.performance.z = fit->second.x;
		out.performance.w = fit->second.y;
	}
	out.learned = true;
	return out;
};

MachineLearning_Results cweeMachineLearning::Learn(const cweeThreadedList<std::pair<u64, float>>& trueLabels, const cweeThreadedList<cweeThreadedList<std::pair<u64, float>>>& features, std::pair<vec2, vec2>* fit, const float percentSplit) {
	cweeThreadedList<cweeThreadedList<float>> Features;
	cweeThreadedList<float> Labels;

	cweeThreadedList<cweeThreadedList<float>> TestFeatures;
	cweeThreadedList<float> TestLabels;

	Features.SetGranularity(cweeMath::max(100, features.Num() + 16));
	Labels.SetGranularity(cweeMath::max(100, trueLabels.Num() + 16));
	TestFeatures.SetGranularity(cweeMath::max(100, features.Num() + 16));
	TestLabels.SetGranularity(cweeMath::max(100, trueLabels.Num() + 16));

	MachineLearning_Results out;

	if (fit == nullptr) {
		for (auto& x : features) {
			cweeThreadedList<float> saver;
			saver.SetGranularity(cweeMath::max(100, x.Num()));
			for (auto& y : x) {
				saver.Append((float)y.second);
			}
			Features.Append(saver);
		}

		for (auto& x : trueLabels) {
			Labels.Append((float)x.second);
		}
		out = LearnSVR(Labels, Features); // to-do, switch statement.
		auto ML_result = Forecast(out, Features);
		vec2 t1 = CalculateError(Labels, ML_result);
		out.performance.x = t1.x;
		out.performance.y = t1.y;
	}
	else {
		cweeThreadedList<cweeThreadedList<std::pair<u64, float>>> featuresLearn = features;
		cweeThreadedList<cweeThreadedList<std::pair<u64, float>>> featuresTest(featuresLearn.Num());
		cweeThreadedList<std::pair<u64, float>> labelsLearn = trueLabels;
		cweeThreadedList<std::pair<u64, float>> labelsTest(labelsLearn.Num());

		SplitRandomly(featuresLearn, labelsLearn, featuresTest, labelsTest, percentSplit); // i.e. 10% split.

		for (auto& x : featuresLearn) {
			cweeThreadedList<float> saver;
			saver.SetGranularity(cweeMath::max(100, x.Num()));
			for (auto& y : x) {
				saver.Append((float)y.second);
			}
			Features.Append(saver);
		}

		for (auto& x : labelsLearn) {
			Labels.Append((float)x.second);
		}

		for (auto& x : featuresTest) {
			cweeThreadedList<float> saver(cweeMath::max(100, x.Num()));
			for (auto& y : x) {
				saver.Append((float)y.second);
			}
			TestFeatures.Append(saver);
		}

		for (auto& x : labelsTest) {
			TestLabels.Append((float)x.second);
		}

		out = LearnSVR(Labels, Features, TestLabels, TestFeatures); // to-do, switch statement.
		auto ML_result = Forecast(out, featuresLearn);
		fit->first = CalculateError(labelsLearn, ML_result);

		ML_result = Forecast(out, featuresTest);
		fit->second = CalculateError(labelsTest, ML_result);
	}
	out.learned = true;
	return out;
};

float cweeMachineLearning::Forecast(const MachineLearning_Results& controller, const cweeThreadedList<float>& features, const float roundNearest) {
	std::vector<float> Features = features;
	float out = 0;
#ifndef hideMachineLearning
	auto mat = dlib::mat(Features);
	AUTO ptr = GetDf2(controller.svr_results.df2);
	
	if (ptr) {
		out = ptr->operator()(mat);
#ifdef useLearnedAdjustment
		if (controller.learned && controller.learnedAdjustment.GetNumValues() > 0) {
			out = controller.learnedAdjustment.GetCurrentValue(out);
		}
#endif
	}
#endif
	return cweeMath::roundNearest(out, roundNearest);
};

cweeThreadedList<float> cweeMachineLearning::Forecast(const MachineLearning_Results& controller, const cweeThreadedList<cweeThreadedList<float>>& features, const float roundNearest) {
	cweeThreadedList<float> out;
	if (features.Num() > 0) out.SetGranularity(features[0].Num() + 16);

	MachineLearningInput featureObservations;
	{
		for (int i = 0; i < features.Num(); i++) {
			featureObservations.addFeature(features[i]);
		}
	}

	// perform the forecast
	float temp; std::vector<float> t;
	AUTO features_flipped = featureObservations.GetInvertedFeatures();
#ifndef hideMachineLearning
	AUTO ptr = GetDf2(controller.svr_results.df2);
	if (ptr) {
		for (int index_v = 0; index_v < features_flipped.Num(); index_v++) {
			t = features_flipped[index_v];
			temp = ptr->operator()(dlib::mat(t));
#ifdef useLearnedAdjustment
			if (controller.learned && controller.learnedAdjustment.GetNumValues() > 0) {
				temp = controller.learnedAdjustment.GetCurrentValue(temp);
			}
#endif
			out.Append(
				cweeMath::roundNearest(
					temp,
					roundNearest
				)
			);
		}
	}
#endif
	return out;
};

cweeThreadedList<std::pair<u64, float>> cweeMachineLearning::Forecast(const MachineLearning_Results& controller, const cweeThreadedList<cweeThreadedList<std::pair<u64, float>>>& Features, const float roundNearest) {
	cweeThreadedList<std::pair<u64, float>> out;
	if (Features.Num() > 0) out.SetGranularity(Features[0].NumRef() + 16);

	cweeThreadedList<u64> times;
	cweeThreadedList<cweeThreadedList<float>> features(Features.NumRef() + 16); {
		// fill the time vector			
		for (auto& x : Features) {
			times.SetGranularity(x.NumRef() + 16);
			for (auto& y : x)
				times.Append(y.first);
			break;
		}
		// fill the features vectors
		for (int i = 0; i < Features.Num(); i++) {
			cweeThreadedList<float> saver(Features[i].Num() + 16);
			for (auto& y : Features[i])
				saver.Append((float)y.second);
			features.Append(saver);
		}
	}
	MachineLearningInput featureObservations; {
		for (auto& x : features)
			featureObservations.addFeature(x);
	}

	// perform the forecast
	float temp; std::vector<float> t;
	AUTO features_flipped = featureObservations.GetInvertedFeatures();
#ifndef hideMachineLearning
	AUTO ptr = GetDf2(controller.svr_results.df2);
	if (ptr) {
		for (int index_v = 0; index_v < features_flipped.NumRef(); index_v++) {  // go through and find the labels for each of the points
			t = features_flipped[index_v];
			temp = ptr->operator()(dlib::mat(t));
#ifdef useLearnedAdjustment
			if (controller.learned && controller.learnedAdjustment.GetNumValues() > 0) {
				temp = controller.learnedAdjustment.GetCurrentValue(temp);
		}
#endif
			out.Append(
				std::make_pair(
					times[index_v],
					cweeMath::roundNearest(
						temp,
						roundNearest
					)
				)
			);
	}
	}
#endif

	return out;
};

vec2 cweeMachineLearning::CalculateError(const cweeThreadedList<float>& observed, const cweeThreadedList<float>& modeled) {
	vec2 out;

	float y_Bar(0);
	float sum_Square_Total(0);
	float sum_Square_Residuals(0);
	float R_squared(0);
	float Mean_Squared_Error(0);

	// y_Bar
	{
		int i(0);
		for (auto& x : observed) {

			cweeMath::rollingAverageRef(y_Bar, x, i);
		}
	}

	// sum_Square_Total
	{
		for (auto& x : observed) {
			sum_Square_Total += (x - y_Bar) * (x - y_Bar);
		}
	}

	// sum_Square_Residuals
	{
		int j(0);
		for (int i = 0; i < observed.Num() && i < modeled.Num(); i++) {
			sum_Square_Residuals += (observed[i] - modeled[i]) * (observed[i] - modeled[i]);
			cweeMath::rollingAverageRef(Mean_Squared_Error, (observed[i] - modeled[i]) * (observed[i] - modeled[i]), j);
		}
	}

	// R_squared
	R_squared = 1 - (sum_Square_Total == 0.0f ? 0.0f : sum_Square_Residuals / (sum_Square_Total + cweeMath::EPSILON));

	out = vec2(R_squared, Mean_Squared_Error);

	return out;
};

float cweeMachineLearning::CalculateLikelihood(const cweeThreadedList<float>& observed, const cweeThreadedList<float>& modeled) {
	// x=observed, y=predicted
	Curve organizer; for (int i = 0; i < observed.Num() && i < modeled.Num(); i++)  organizer.AddValue(observed[i], modeled[i]);
	// list of unique observed values
	cweeThreadedList<float> uniqueObserved; for (auto& x : observed) uniqueObserved.AddUnique(x);
	// list of list of predicted values for the indexed observed value
	cweeThreadedList<cweeThreadedList<float>> predictedValuesForObservedIndex; {
		for (int i = 0; i < uniqueObserved.Num(); i++)  predictedValuesForObservedIndex.Append(cweeThreadedList<float>()); // set size
		int currentIndex = -1; float currentObserved = -1; // optimized search
		for (auto& knot : organizer.GetKnotSeries()) { // append values
			if (currentObserved != knot.first) {
				currentIndex = uniqueObserved.FindIndex(knot.first);
				currentObserved = knot.first;
			}
			if (currentIndex >= 0 && currentIndex < predictedValuesForObservedIndex.Num()) {
				predictedValuesForObservedIndex[currentIndex].Append(knot.second);
			}
		}
	}





	return 1.0f;
};

vec2 cweeMachineLearning::CalculateError(const cweeThreadedList<std::pair<u64, float>>& observed, const cweeThreadedList<std::pair<u64, float>>& modeled) {
	vec2 out;

	cweeThreadedList<float> trueFloats(observed.Num() + 16);
	cweeThreadedList<float> estimateFloats(modeled.Num() + 16);

	for (auto& x : observed) trueFloats.Append(x.second);
	for (auto& x : modeled) estimateFloats.Append(x.second);

	out = CalculateError(trueFloats, estimateFloats);

	return out;
};

MachineLearning_Results cweeMachineLearning::LearnSVR(const cweeThreadedList<float>& trueLabels, const cweeThreadedList<cweeThreadedList<float>>& features, const cweeThreadedList<float>& testLabels, const cweeThreadedList<cweeThreadedList<float>>& testfeatures) {
	MachineLearning_Results out;
	int numFeatures = features.Num();

	MachineLearningInput input;

	input.labels = trueLabels;
	for (int i = 0; i < features.Num(); i++) {
		input.addFeature(features[i]);
	}

	if (input.NumObs() > 0 && input.NumFeatures() > 0) {
		float previousPerformance = cweeMath::INF, perf;

		cweeThreadedList<float> forecasted;
		MachineLearning_Results temp;
		vec2 fit1;

		for (double exponent = -2; exponent <= 2; exponent += 0.5) {
			input.gamma = std::pow(10.0, exponent);

			temp = MachineLearning_Math::Learn(input);

			forecasted = Forecast(temp, features);
			fit1 = CalculateError(trueLabels, forecasted);

			perf = fit1.y;

			if (perf < previousPerformance) {
				previousPerformance = perf;
				out = temp;
				if (perf <= 0) return out; // early exit because... it's good enough. 
			}
		}
	}

	return out;
};
MachineLearning_Results cweeMachineLearning::LearnSVRFast(const cweeThreadedList<float>& trueLabels, const cweeThreadedList<cweeThreadedList<float>>& features, const cweeThreadedList<float>& testLabels, const cweeThreadedList<cweeThreadedList<float>>& testfeatures) {
	MachineLearning_Results out;
	int numFeatures = features.Num();

	MachineLearningInput input;

	input.labels = trueLabels;
	for (int i = 0; i < features.Num(); i++) {
		input.addFeature(features[i]);
	}

	if (input.NumObs() > 0 && input.NumFeatures() > 0) {
		out = MachineLearning_Math::Learn(input); // using the 'best' version, do the best learn we can do. 
	}

	return out;
};

void cweeMachineLearning::SplitRandomly(cweeThreadedList<cweeThreadedList<float>>& featuresIn, cweeThreadedList<float>& labelsIn, cweeThreadedList<cweeThreadedList<float>>& featuresOut, cweeThreadedList<float>& labelsOut, float percentSplit) {
	featuresOut.Clear();
	labelsOut.Clear();

	int numArrays(featuresIn.Num());
	int numPoints(labelsIn.Num());

	for (auto& x : featuresIn) {
		cweeThreadedList < float > Array;
		featuresOut.Append(Array);
	}

	for (int i = numPoints - 1; i >= 0; i--) {
		if (cweeRandomInt(0, 100) < percentSplit) {
			labelsOut.Insert(labelsIn[i]);
			labelsIn.RemoveIndexFast(i);
			for (int j = 0; j < numArrays; j++) {
				featuresOut[j].Insert(featuresIn[j][i]);
				featuresIn[j].RemoveIndexFast(i);
			}
		}
	}
};

void cweeMachineLearning::SplitRandomly(cweeThreadedList<cweeThreadedList<std::pair<u64, float>>>& featuresIn, cweeThreadedList<std::pair<u64, float>>& labelsIn, cweeThreadedList<cweeThreadedList<std::pair<u64, float>>>& featuresOut, cweeThreadedList<std::pair<u64, float>>& labelsOut, float percentSplit) {
	featuresOut.Clear();
	labelsOut.Clear();

	int numArrays(featuresIn.Num());
	int numPoints(labelsIn.Num());

	for (int i = 0; i < numArrays; i++) {
		featuresOut.Append(cweeThreadedList<std::pair<u64, float>>());
	}
	for (auto& x : featuresOut) x.SetGranularity((numPoints * (percentSplit + 1) / 100.0f) + 16);
	labelsOut.SetGranularity((numPoints * (percentSplit + 1) / 100.0f) + 16);

	for (int i = numPoints - 1; i >= 0; i--) {
		if (cweeRandomInt(0, 100) < percentSplit) {
			labelsOut.Append(labelsIn[i]);
			labelsIn.RemoveIndexFast(i);
			for (int j = 0; j < numArrays; j++) {
				featuresOut[j].Append(featuresIn[j][i]);
				featuresIn[j].RemoveIndexFast(i);
			}
		}
	}
};

cweeThreadedList<float> cweeMachineLearning::Example(void) {
	std::vector<float> labels_source = { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f };
	std::vector<float> feature1_source = { 0.4f, 1.3f, 2.1f, 3.3f, 4.4f, 5.4f, 6.3f, 7.4f, 8.4f, 9.2f, 10.2f, 11.4f, 12.2f, 13.4f, 14.4f };
	std::vector<float> feature2_source = { 0.3f, 1.2f, 2.1f, 3.2f, 4.1f, 5.2f, 6.2f, 7.2f, 8.2f, 9.4f, 10.4f, 11.2f, 12.4f, 13.2f, 14.2f };

	cweeThreadedList<float>							labels(labels_source);
	cweeThreadedList < cweeThreadedList<float> >	features;
	{
		cweeThreadedList<float> data1(feature1_source);
		cweeThreadedList<float> data2(feature2_source);
		features.Append(data1);
		features.Append(data2);
	}

	std::pair<vec2, vec2> fit;

	auto ML_param = cweeMachineLearning::Learn(labels, features, &fit);

	float TRAINING_R_squared = fit.first.x;
	float TRAINING_MSE = fit.first.y;

	float TESTING_R_squared = fit.second.x;
	float TESTING_MSE = fit.second.y;

	std::vector<float> feature1_forecast = { 0.1f, 1.21f, 2.2f, 3.4f, 4.0f, 5.3f, 6.6f, 7.7f, 8.3f, 9.5f, 10.5f, 11.3f, 12.5f, 13.3f, 14.3f };
	std::vector<float> feature2_forecast = { 0.1f, 1.21f, 2.2f, 3.4f, 4.0f, 5.3f, 6.6f, 7.7f, 8.3f, 9.5f, 10.5f, 11.3f, 12.5f, 13.3f, 14.3f };
	features.Clear();
	{
		cweeThreadedList<float> data1(feature1_forecast);
		cweeThreadedList<float> data2(feature2_forecast);
		features.Append(data1);
		features.Append(data2);
	}

	return cweeMachineLearning::Forecast(ML_param, features);
};

#include <cweeUnitPattern.h>

namespace chaiscript {
	namespace WaterWatch_Lib {
		[[nodiscard]] ModulePtr MachineLearning_Library() {
			auto lib = chaiscript::make_shared<Module>();

			class PatternLearner {
			public:
				cweeUnitPattern Labels;
				bool annualEffect = true;
				bool monthlyEffect = true;
				bool weekdayEffect = false;
				bool hourlyEffect = false;

			private:
				cweeUnitPattern LineOfBestFit;
				MachineLearning_Results seasonal; // 0 || 1, summer & winter
				MachineLearning_Results monthly; // 0 || 1 x 12, january through december
				MachineLearning_Results weekday; // 0 || 1, M-F vs Sa-Su
				MachineLearning_Results daily; // 0.0 through 1.0, daily time-seconds. 

			private:
				class PatternSummary {
				public:
					PatternSummary() : average(), minimum(), maximum(), minTime(), maxTime(), minTimestep(), include(true) {};
					PatternSummary(cweeUnitPattern const& obj) : average(obj.GetAvgValue()), minimum(obj.GetMinValue()), maximum(obj.GetMaxValue()), minTime(obj.GetMinTime()), maxTime(obj.GetMaxTime()), minTimestep(obj.GetMinimumTimeStep()), include(true) {};
					PatternSummary(PatternSummary const&) = default;
					PatternSummary(PatternSummary&&) = default;
					PatternSummary& operator=(PatternSummary const&) = default;
					PatternSummary& operator=(PatternSummary&&) = default;

					cweeUnitValues::unit_value average;
					cweeUnitValues::unit_value minimum;
					cweeUnitValues::unit_value maximum;
					cweeUnitValues::unit_value minTime;
					cweeUnitValues::unit_value maxTime;
					cweeUnitValues::unit_value minTimestep;
					bool include;
				};

				PatternSummary LabelSummary;

				cweeUnitValues::unit_value SeasonalEstimate(cweeUnitValues::unit_value const& time) const {
					cweeTime t(time);

					if (t.tm_mon() >= 4 && t.tm_mon() <= 10) {
						return 1.0;
					}
					else {
						return 0.0;
					}
				};
				cweeUnitValues::unit_value MonthlyEstimate(cweeUnitValues::unit_value const& time, int month) const {
					cweeTime t(time);

					if ((t.tm_mon() + 1) == month) {
						return 1;
					}
					else {
						return 0;
					}
				};
				cweeUnitValues::unit_value WeekdayEstimate(cweeUnitValues::unit_value const& time, int dayOfWeek) const {
					cweeTime t(time);

					if (t.tm_wday() == dayOfWeek) {
						return 1;
					}
					else {
						return 0;
					}
				};
				cweeUnitValues::unit_value DailyEstimate(cweeUnitValues::unit_value const& time) const {
					cweeTime t(time);
					return  cweeUnitValues::second((t.tm_hour() * 3600.0) + (t.tm_min() * 60.0) + (t.tm_sec())) / cweeUnitValues::second(cweeUnitValues::day(1));
				};

				void LearnSeasonal(cweeUnitPattern const& mask, cweeUnitPattern const& prevBest) {
					AUTO ToPredict = (Labels - prevBest).Blur(cweeUnitValues::year(LabelSummary.maxTime - LabelSummary.minTime)() * 2.0, mask);
					cweeThreadedList<float>							label_list;
					cweeThreadedList < cweeThreadedList<float> >	feature_list; 

					feature_list.Alloc(); // is summer
					feature_list.Alloc(); // is winter
					
					{
						int i; cweeUnitValues::unit_value est;
						for (auto& x : (ToPredict + mask).GetKnotSeries()) {
							if (mask.GetCurrentValue(x.first) > 0) continue;
							
							label_list.Append(ToPredict.GetCurrentValue(x.first)());
							
							est = SeasonalEstimate(x.first);

							feature_list[0].Append(est());
							feature_list[1].Append(1.0 - est());
						}
					}

					seasonal = cweeMachineLearning::LearnFast(label_list, feature_list, nullptr, 0);
				};
				cweeUnitPattern ForecastSeasonal(cweeUnitPattern const& prevBest) const {
					auto out{ cweeUnitPattern(cweeUnitValues::second(), 1) };
					if (seasonal.learned) {
						AUTO df2Ptr = GetDf2(seasonal.svr_results.df2);
						if (df2Ptr) {
							AUTO df2 = *df2Ptr;
							std::vector<float> sampleVec = {0.0f, 0.0f};

							auto step{ cweeUnitValues::day(1) };
							for (cweeUnitValues::second t = LabelSummary.minTime; t < LabelSummary.maxTime; t += step) {
								sampleVec[0] = SeasonalEstimate(t)();
								sampleVec[1] = 1.0f - sampleVec[0];

								out.AddValue(t, df2(dlib::mat(sampleVec)));
							}
						}
					}
					return out + prevBest;
				};
								
				void LearnMonthly(cweeUnitPattern const& mask, cweeUnitPattern const& prevBest) {
					AUTO ToPredict = (Labels - prevBest).Blur(cweeUnitValues::year(LabelSummary.maxTime - LabelSummary.minTime)() * 12.0, mask);
					cweeThreadedList<float>							label_list;
					cweeThreadedList < cweeThreadedList<float> >	feature_list;

					for (int i = 0; i < 12; i++) {
						feature_list.Alloc();
					}

					{
						int i; cweeUnitValues::unit_value est;
						for (auto& x : (ToPredict + mask).GetKnotSeries()) {
							if (mask.GetCurrentValue(x.first) > 0) continue;

							label_list.Append(ToPredict.GetCurrentValue(x.first)());

							for (int i = 0; i < 12; i++) {
								feature_list[i].Append(MonthlyEstimate(x.first, i)());
							}
						}
					}

					monthly = cweeMachineLearning::LearnFast(label_list, feature_list, nullptr, 0);
				};
				cweeUnitPattern ForecastMonthly(cweeUnitPattern const& prevBest) const {
					auto out{ cweeUnitPattern(cweeUnitValues::second(), 1) };
					if (monthly.learned) {
						AUTO df2Ptr = GetDf2(monthly.svr_results.df2);
						if (df2Ptr) {
							AUTO df2 = *df2Ptr;
							std::vector<float> sampleVec = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

							auto step{ cweeUnitValues::day(1) };
							int i;
							for (cweeUnitValues::second t = LabelSummary.minTime; t < LabelSummary.maxTime; t += step) {
								for (i = 0; i < 12; i++) {
									sampleVec[i] = MonthlyEstimate(t, i)();
								}
								out.AddValue(t, df2(dlib::mat(sampleVec)));
							}
						}
					}
					return out + prevBest;
				};

				void LearnWeekday(cweeUnitPattern const& mask, cweeUnitPattern const& prevBest) {
					AUTO ToPredict = (Labels - prevBest).Blur(cweeUnitValues::year(LabelSummary.maxTime - LabelSummary.minTime)() * 365, mask);
					cweeThreadedList<float>							label_list;
					cweeThreadedList < cweeThreadedList<float> >	feature_list;

					for (int i = 0; i <= 6; i++) {
						feature_list.Alloc();
					}

					{
						int i; 
						for (auto& x : (ToPredict + mask).GetKnotSeries()) {
							if (mask.GetCurrentValue(x.first) > 0) continue;
							
							label_list.Append(ToPredict.GetCurrentValue(x.first)());

							for (int i = 0; i <= 6; i++) {
								feature_list[i].Append(WeekdayEstimate(x.first, i)());
							}
						}
					}

					weekday = cweeMachineLearning::LearnFast(label_list, feature_list, nullptr, 0);
				};
				cweeUnitPattern ForecastWeekday(cweeUnitPattern const& prevBest) const {
					auto out{ cweeUnitPattern(cweeUnitValues::second(), 1) };
					if (weekday.learned) {
						AUTO df2Ptr = GetDf2(weekday.svr_results.df2);
						if (df2Ptr) {
							AUTO df2 = *df2Ptr;
							std::vector<float> sampleVec = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

							auto step{ cweeUnitValues::day(1) };
							int i;
							for (cweeUnitValues::second t = LabelSummary.minTime; t < LabelSummary.maxTime; t += step) {
								for (i = 0; i <= 6; i++) {
									sampleVec[i] = WeekdayEstimate(t, i)();
								}
								out.AddValue(t, df2(dlib::mat(sampleVec)));
							}
						}
					}
					return out + prevBest;
				};

				void LearnDaily(cweeUnitPattern const& mask, cweeUnitPattern const& prevBest) {
					AUTO ToPredict = Labels - prevBest;
					cweeThreadedList<float>							label_list;
					cweeThreadedList < cweeThreadedList<float> >	feature_list;

					feature_list.Alloc(); // is weekday

					{
						int i; cweeUnitValues::unit_value est;
						auto step{ cweeUnitValues::hour(1) };
						for (cweeUnitValues::second t = LabelSummary.minTime; t <= LabelSummary.maxTime; t += step) {
							if (mask.GetCurrentValue(t) > 0) continue;

							label_list.Append(ToPredict.GetCurrentValue(t)());

							est = DailyEstimate(t);

							feature_list[0].Append(est());
						}
					}

					daily = cweeMachineLearning::LearnFast(label_list, feature_list, nullptr, 0);
				};
				cweeUnitPattern ForecastDaily(cweeUnitPattern const& prevBest) const {
					auto out{ cweeUnitPattern(cweeUnitValues::second(), 1) };
					if (daily.learned) {
						AUTO df2Ptr = GetDf2(daily.svr_results.df2);
						if (df2Ptr) {
							AUTO df2 = *df2Ptr;
							std::vector<float> sampleVec = { 0.0f };

							auto step{ cweeUnitValues::hour(1) };
							int i;
							for (cweeUnitValues::second t = LabelSummary.minTime; t <= LabelSummary.maxTime; t += step) {
								sampleVec[0] = DailyEstimate(t)();
								out.AddValue(t, df2(dlib::mat(sampleVec)));
							}
						}
					}
					return out + prevBest;
				};

			public:
				//cweeUnitValues::second LearnBasic(cweeUnitPattern const& mask) {
				//	
				//	this->LabelSummary = PatternSummary(Labels);

				//	Stopwatch sw; sw.Start();

				//	cweeUnitPattern finalLabels;
				//	//cweeUnitPattern finalMask;
				//	if (hourlyEffect) {
				//		//finalMask = mask.Blur((cweeUnitValues::hour(mask.GetMaxTime() - mask.GetMinTime()))());
				//		finalLabels = Labels.Blur((cweeUnitValues::hour(Labels.GetMaxTime() - Labels.GetMinTime()))(), mask);
				//	}
				//	else if (weekdayEffect) {
				//		//finalMask = mask.Blur((cweeUnitValues::day(mask.GetMaxTime() - mask.GetMinTime()))());
				//		finalLabels = Labels.Blur((cweeUnitValues::day(Labels.GetMaxTime() - Labels.GetMinTime()))(), mask);
				//	}
				//	else if (monthlyEffect) {
				//		//finalMask = mask.Blur((cweeUnitValues::month(mask.GetMaxTime() - mask.GetMinTime()))());
				//		finalLabels = Labels.Blur((cweeUnitValues::month(Labels.GetMaxTime() - Labels.GetMinTime()))(), mask);
				//	}
				//	else if (annualEffect) {
				//		//finalMask = mask.Blur((cweeUnitValues::year(mask.GetMaxTime() - mask.GetMinTime()))() * 2);
				//		finalLabels = Labels.Blur((cweeUnitValues::year(Labels.GetMaxTime() - Labels.GetMinTime()))() * 2, mask);
				//	}

				//	this->LineOfBestFit = finalLabels.LineOfBestFit(mask);

				//	sw.Stop();

				//	return cweeUnitValues::second(sw.Seconds_Passed());
				//};

				cweeUnitValues::second Learn(cweeUnitPattern const& mask) {
					this->LineOfBestFit = Labels.LineOfBestFit(mask);
					this->LabelSummary = PatternSummary(Labels);
					
					Stopwatch sw; sw.Start();
					if (hourlyEffect || weekdayEffect || monthlyEffect || annualEffect) {
						if (Labels.GetNumValues() <= 2) throw std::runtime_error("Complex machine learning features require the labels  (to be predicted) must have more values for learning.");
						if (!LabelSummary.minTime.AreConvertableTypes(cweeUnitValues::second())) throw std::runtime_error("Complex machine learning features require the labels (to be predicted) X-axis must be a time-type unit (seconds, minutes, etc) convertable to date-times.");
						LearnSeasonal(mask, LineOfBestFit);
						if (hourlyEffect || weekdayEffect || monthlyEffect) {
							AUTO seasonalFore = ForecastSeasonal(LineOfBestFit);
							LearnMonthly(mask, seasonalFore);

							if (hourlyEffect || weekdayEffect) {
								AUTO monthlyFore = ForecastMonthly(seasonalFore);
								LearnWeekday(mask, monthlyFore);

								if (hourlyEffect) {
									AUTO weekdayFore = ForecastWeekday(monthlyFore);
									LearnDaily(mask, weekdayFore);
								}
							}
						}
					}

					sw.Stop();

					return cweeUnitValues::second(sw.Seconds_Passed());
				};
				cweeUnitValues::second Learn() {
					// Create summaries
					LabelSummary = PatternSummary(Labels);

					// Create temporary mask
					auto mask = cweeUnitPattern(LabelSummary.minTime, 1); {
						mask.AddValue(LabelSummary.minTime, 0);
						mask.AddValue(LabelSummary.maxTime, 0);
					}

					// Perform 'learn'
					return Learn(mask);
				};

				cweeUnitPattern Forecast() const {
					if (hourlyEffect) return ForecastDaily(ForecastWeekday(ForecastMonthly(ForecastSeasonal(LineOfBestFit))));
					else if (weekdayEffect) return ForecastWeekday(ForecastMonthly(ForecastSeasonal(LineOfBestFit)));
					else if (monthlyEffect) return ForecastMonthly(ForecastSeasonal(LineOfBestFit));
					else if (annualEffect) return ForecastSeasonal(LineOfBestFit);
					else return LineOfBestFit;
				};

				/* Takes the average forecast, and attempts to augment it with additional data from the past or future of this timeseries */
				cweeUnitPattern ForecastAdvanced(cweeUnitPattern const& mask) const {
					AUTO averageForecast = Forecast();
					AUTO maxT = averageForecast.GetMaxTime();
					AUTO minT = averageForecast.GetMinTime();
					AUTO out = cweeUnitPattern(Labels.X_Type(), Labels.Y_Type());
					bool wasSuccessful;

					auto step{ cweeUnitValues::year(1) }; // { cweeUnitValues::day(7) };
					auto val{ Labels.Y_Type() }; int count = 0; cweeUnitValues::unit_value t2;
					for (cweeUnitValues::unit_value t = minT; t <= maxT; t += cweeUnitValues::hour(1)) {
						AUTO currAvg = averageForecast.GetCurrentValue(t);
						val = 0; // currAvg;
						count = 0; // 1;

						// try to get the value locally if we can, working backwards. 
						wasSuccessful = false;
						for (t2 = t - step; t2 >= minT && count < 1; t2 -= step) {
							if (mask.GetCurrentValue(t2) <= 0) {
								cweeMath::rollingAverageRef(val, (currAvg - averageForecast.GetCurrentValue(t2)) + Labels.GetCurrentValue(t2), count);
							}
						}
						for (t2 = t + step; t2 <= maxT && count < 2; t2 += step) {
							if (mask.GetCurrentValue(t2) <= 0) {
								cweeMath::rollingAverageRef(val, (currAvg - averageForecast.GetCurrentValue(t2)) + Labels.GetCurrentValue(t2), count);
							}
						}

						if (count > 0) out.AddValue(t, val);
						else out.AddValue(t, currAvg);
					}

					return out;
				};


            };

			lib->add(chaiscript::user_type<PatternLearner>(), "PatternLearner");
			lib->add(chaiscript::constructor<PatternLearner()>(), "PatternLearner");
			lib->add(chaiscript::constructor<PatternLearner(const PatternLearner&)>(), "PatternLearner");
			lib->add(chaiscript::fun([](PatternLearner& a, const PatternLearner& b)->PatternLearner& { a = b; return a; }), "=");
			lib->add(chaiscript::fun(&PatternLearner::annualEffect), "annualEffect");
			lib->add(chaiscript::fun(&PatternLearner::monthlyEffect), "monthlyEffect");
			lib->add(chaiscript::fun(&PatternLearner::weekdayEffect), "weekdayEffect");
			lib->add(chaiscript::fun(&PatternLearner::hourlyEffect), "hourlyEffect");
			lib->add(chaiscript::fun(&PatternLearner::Labels), "Labels");
			
			lib->AddFunction(, Learn, -> PatternLearner, SINGLE_ARG(
				PatternLearner learn;
				learn.Labels = pat;
				learn.annualEffect = true;
				learn.monthlyEffect = true;
				learn.weekdayEffect = true;
				learn.hourlyEffect = true;
				learn.Learn(pat.GetOutlierMask());
				return learn;
			), cweeUnitPattern const& pat);
			lib->AddFunction(, Learn, -> PatternLearner, SINGLE_ARG(
				PatternLearner learn;
				learn.Labels = pat;
				learn.annualEffect = true;
				learn.monthlyEffect = true;
				learn.weekdayEffect = true;
				learn.hourlyEffect = true;
				learn.Learn(mask);
				return learn;
			), cweeUnitPattern const& pat, cweeUnitPattern const& mask);
			lib->AddFunction(, Learn, -> cweeUnitValues::unit_value, return learned.Learn(), PatternLearner& learned);
			lib->AddFunction(, Learn, -> cweeUnitValues::unit_value, return learned.Learn(mask), PatternLearner& learned, cweeUnitPattern const& mask);
			lib->AddFunction(, Forecast, -> cweeUnitPattern, return learned.Forecast(), PatternLearner const& learned);
			lib->AddFunction(, ForecastAdvanced, -> cweeUnitPattern, return learned.ForecastAdvanced(mask), PatternLearner const& learned, cweeUnitPattern const& mask);
			
			AUTO todo = [](cweeUnitPattern const& pat) {
				AUTO mask{ pat.GetOutlierMask() };

				PatternLearner learn; {
					learn.Labels = pat;
					learn.annualEffect = true;
					learn.monthlyEffect = true;
					learn.weekdayEffect = true;
					learn.hourlyEffect = true;
					learn.Learn(mask);
				}
				return cweeUnitPattern::Lerp(learn.Forecast(), pat, mask);				
			};
			lib->AddFunction(todo, Clean, -> cweeUnitPattern, return todo(pat), cweeUnitPattern const& pat);

			AUTO todo2 = [](cweeUnitPattern const& pat, cweeUnitPattern const& mask) {
				PatternLearner learn; {
					learn.Labels = pat;
					learn.annualEffect = true;
					learn.monthlyEffect = true;
					learn.weekdayEffect = true;
					learn.hourlyEffect = true;
					learn.Learn(mask);
				}
				return cweeUnitPattern::Lerp(learn.Forecast(), pat, mask);
			};
			lib->AddFunction(todo2, Clean, -> cweeUnitPattern, return todo2(pat, mask), cweeUnitPattern const& pat, cweeUnitPattern const& mask);

			return lib;
		};
	};
}; // namespace chaiscript