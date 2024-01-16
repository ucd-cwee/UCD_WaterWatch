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
cweeStr MachineLearning_SVR_Results::Serialize() {
	cweeStr delim = ":cweeML_parameters_SVR_regression_DELIM:";
	cweeStr out;
#ifndef hideMachineLearning
	auto ptr = GetDf2(df2);
	if (ptr) {
		{
			cweeStr listOut;
			for (auto& x : ptr->alpha) {
				if (x == 0.0f) {
					if (listOut.IsEmpty()) {
						listOut = " ";
					}
					else {
						listOut.AddToDelimiter("", ",");
					}
				}
				else {
					listOut.AddToDelimiter(x, ",");
				}
			}
			if (listOut.IsEmpty()) {
				out.AddToDelimiter(" ", delim);
			}
			else {
				out.AddToDelimiter(listOut, delim);
			}
		}
		{
			// often nr = 2, nc = 1, size = 2;
			int nr = ptr->basis_vectors.nr();
			int nc = ptr->basis_vectors.nc();
			int nrTimesNc = nr * nc;
			int size = ptr->basis_vectors.size();
			int sizeDif = size - nrTimesNc;

			cweeStr matri; cweeStr listOut;
			int sizeOfVector = 0;
			for (auto& x : ptr->basis_vectors) {
				listOut.Clear();
				sizeOfVector = x.size();

				//if (sizeOfVector == nr) {
				//	int ijk = 0;
				//	// column major. i.e. 
				//	// for (auto& column : df2.basis)
				//	//		for (auto& row : column)...
				//	submitToast("Machine Learning Debug", cweeStr::printf("Column Major: nr%i nc%i size%i", nr, nc, size));
				//}
				//else if (sizeOfVector == nc) {
				//	int ijk = 0;
				//	// row major. i.e. 
				//	// for (auto& row : df2.basis)
				//	//		for (auto& column : row)...
				//	submitToast("Machine Learning Debug", cweeStr::printf("Row Major: nr%i nc%i size%i", nr, nc, size));
				//}
				//else {
				//	int ijk = 0;
				//	// no idea what is happening
				//	submitToast("Machine Learning Debug", cweeStr::printf("Unknown Major: nr%i nc%i size%i", nr, nc, size));
				//}

				// often n_subR = 192, nc = 1, size = 192

				//int n_subR = x.nr();
				//int n_subC = x.nc();
				//int subSize = x.size();
				//submitToast("Machine Learning Debug2", cweeStr::printf("Unknown Major: nr%i nc%i size%i", n_subR, n_subC, subSize));

				for (auto& y : x) {
					if (y == 0.0f) {
						if (listOut.IsEmpty()) {
							listOut = " ";
						}
						else {
							listOut.AddToDelimiter("", ",");
						}
					}
					else {
						listOut.AddToDelimiter(y, ",");
					}
				}

				if (1) {
					// reduce string size
					listOut.Replace(",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,", "_");
					listOut.Replace(",,,,,", "?");
				}

				matri.AddToDelimiter(listOut, ":SVR_basis_vector:");
			}
			if (out.IsEmpty()) out = delim + matri;
			else out.AddToDelimiter(matri, delim);
		}
		out.AddToDelimiter(ptr->b, delim);
		out.AddToDelimiter(ptr->kernel_function.gamma, delim);
	}
#endif
	return out;
};
void MachineLearning_SVR_Results::Deserialize(cweeStr& in) {
	cweeParser obj(in, ":cweeML_parameters_SVR_regression_DELIM:", true);
	in.Clear();

	std::vector<float> alpha_vec;
	std::vector < std::vector <float> > basis_vector;
	float b_scalar, gamma;
	cweeParser knots, vals;
	int i, j;
	std::vector <float> row;
	{
		if (!obj.getVar(0).IsEmpty()) {
			knots.Parse(obj.getVar(0), ",", true);
			if (knots.getNumVars() >= 2) {
				alpha_vec.reserve(cweeMath::max(1, knots.getNumVars()));
				for (i = 0; i < knots.getNumVars(); i++) {
					alpha_vec.push_back((float)knots.getVar(i));
				}
			}
		}
	}
	{
		if (!obj.getVar(1).IsEmpty()) {
			cweeParser rows(obj.getVar(1), ":SVR_basis_vector:", true);
			basis_vector.reserve(cweeMath::max(1, rows.getNumVars()));
			for (i = 0; i < rows.getNumVars(); i++) {
				row.clear(); row.reserve(cweeMath::max(1, vals.getNumVars()));

				if (1) {
					// increase string size
					rows.getVar(i).Replace("?", ",,,,,");
					rows.getVar(i).Replace("_", ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,");
				}

				vals.Parse(rows.getVar(i), ",", true);
				for (j = 0; j < vals.getNumVars(); j++) {
					if (vals[j].IsEmpty())
						row.push_back(0.0f);
					else
						row.push_back((float)vals[j]);
				}
				basis_vector.push_back(row);
			}
		}
	}
	b_scalar = (float)obj.getVar(2);
	gamma = obj.getVar(3).IsEmpty() ? 0.1f : (float)obj.getVar(3);
	{
#ifndef hideMachineLearning
		auto ptr = GetDf2(df2);
		if (ptr) {
			*ptr = dlib::decision_function<kernel_type>(); //  alpha_vec, b_scalar, kernel_type(), basis_vector);
			ptr->basis_vectors.set_size(basis_vector.size(), 1);
			ptr->alpha.set_size(alpha_vec.size(), 1);

			for (int index_i = 0; index_i < basis_vector.size(); index_i++) {
				auto& content = basis_vector[index_i];
				ptr->basis_vectors(index_i).set_size(content.size(), 1);
				for (int index_j = 0; index_j < content.size(); index_j++) {
					ptr->basis_vectors(index_i)(index_j) = content[index_j];
				}
			}

			for (int index_i = 0; index_i < alpha_vec.size(); index_i++) {
				ptr->alpha(index_i) = alpha_vec[index_i];
			}

			ptr->b = b_scalar;
			ptr->kernel_function = kernel_type(gamma);
		}
#endif
	}
};

cweeStr MachineLearning_Results::Serialize() {
	cweeStr delim = ":cweeML_learned_parameters_in_DELIM:";
	cweeStr out;

	if (learned) {
		out += " ";
		out.AddToDelimiter(svr_results.Serialize(), delim);
		out.AddToDelimiter(learned, delim);
		out.AddToDelimiter(performance.Serialize(), delim);
		return out;
	}
	else {
		return "";
	}

};
void MachineLearning_Results::Deserialize(cweeStr& in) {
	if (in.IsEmpty()) {
		*this = MachineLearning_Results(); // make new
	}
	else {
		cweeParser obj(in, ":cweeML_learned_parameters_in_DELIM:", true);
		in.Clear();

		svr_results.Deserialize(obj[1]); obj[1].Clear();
		learned = (bool)(int)obj[2]; obj[2].Clear();
		performance.Deserialize(obj[3]);
	}
};

MachineLearning_Results		MachineLearning_Math::Learn(const MachineLearningInput& input) {
	MachineLearning_Results output_parameters;
#ifndef hideMachineLearning
	typedef dlib::matrix<float, 0, 1> sample_type;
	typedef dlib::radial_basis_kernel<sample_type> kernel_type;
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
		trainer.set_c(input.c);																//	"1"		//	c value		// higher = higher accuracy, lower = reduced accuracy. 
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
			trainer.set_epsilon(diff / 1000.0f); // i.e. 0.1% of the range of the dataset	
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
	typedef dlib::matrix<float, 0, 1> sample_type;
	typedef dlib::radial_basis_kernel<sample_type> kernel_type;
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
		trainer.set_c(input.c);																//	"1"		//	c value		// higher = higher accuracy, lower = reduced accuracy. 
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
			trainer.set_epsilon(diff / 20.0f); // i.e. 5% of the range of the dataset
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

cweeMachineLearning::cweeLearnedParams cweeMachineLearning::Learn_Opt(cweeSharedPtr<cweeThreadedList<float>> trueLabels, cweeSharedPtr<cweeThreadedList<cweeThreadedList<float>>> trueFeatures) {
	cweeSharedPtr<cweeLearnedParams> out = new cweeLearnedParams();
	{
		// the constant is the average value
		out->labelSummary = SummarizeFeature(*trueLabels);
		for (auto& x : *trueFeatures) out->featureSummaries.Append(SummarizeFeature(x));

		// solve for the multiplier per feature.

		// predict the normalized labels using the normalized features
		{
			cweeThreadedList<float> lowerBound, upperBound;
			for (auto& bv : *trueFeatures) { lowerBound.push_back(-10000); }
			for (auto& bv : *trueFeatures) { upperBound.push_back(10000); }

			// MINIMIZE THE ERROR
			AUTO todo = [=](cweeThreadedList<float> const& x) -> float {
				cweeThreadedList<float> result;
				float prediction = out->labelSummary.min;
				int x_n;
				for (int obs_i = 0; obs_i < trueLabels->Num(); obs_i++) {
					prediction = out->labelSummary.min;
					x_n = 0;
					for (int feature_n = 0; feature_n < trueFeatures->Num(); feature_n++) {
						prediction += (
							x[x_n++] * (
								trueFeatures->at(feature_n)[obs_i] - (out->featureSummaries[feature_n].min)
								)
							);
					}
					result.push_back(prediction);
				}
				float err = 0;
				for (int i = 0; i < result.Num(); i++) { err += cweeMath::Pow(trueLabels->at(i) - result[i], 2.0); }
				err = cweeMath::Pow(err, 0.5);
				return err;
			};
			int numDimensions = cweeMath::min(lowerBound.Num(), upperBound.Num());

			constexpr int maxIterations = 1000;
			constexpr float eps = 0.000001f;
			static std::function objFunc = [=](cweeThreadedList<u64>& policy) -> u64 {
				return todo(policy);
			};
			static std::function isFinishedFunc = [=](OptimizationObj& shared, cweeThreadedList<u64>& bestPolicy, u64 bestPerformance) -> bool {
				auto& i = shared.results.Alloc();
				i.bestPolicy = bestPolicy;
				i.bestPerformance = bestPerformance;
				if (shared.results.Num() <= 10) return false; // guarrantee at least 10 iterations
				else {
					// ensure the most recent 5 iterations are better than the 5 before them.
					int i = shared.results.Num() - 1;
					float perf_new = 0;
					float perf_old = 0;
					int count = 0;
					for (; i >= (shared.results.Num() - 5); i--) {
						cweeMath::rollingAverageRef(perf_new, shared.results[i].bestPerformance, count);
					}
					count = 0;
					for (; i >= 0 && i >= (shared.results.Num() - 10); i--) {
						cweeMath::rollingAverageRef(perf_old, shared.results[i].bestPerformance, count);
					}

					// ensure that we are improving. 
					if (cweeMath::Fabs(perf_new - perf_old) < cweeMath::Fabs(bestPerformance * eps)) {
						shared.numIterationsFailedImprovement++;
						if (shared.numIterationsFailedImprovement > (maxIterations / 10)) {
							return true;
						}
						else {
							return false;
						}
					}
					shared.numIterationsFailedImprovement = 0;
					return false; // we have not flat-lined. continue.
				}
			};
			cweeSharedPtr<OptimizationObj> shared = make_cwee_shared<OptimizationObj>();

			cweeJob toAwait;
#define OPT_FUNC_DIM(NUMDIM, NUMPOLICIES, _min_) case (NUMDIM) : { \
		Genetic_OptimizationManagementTool<_min_> ramt = Genetic_OptimizationManagementTool<_min_>(NUMDIM, NUMPOLICIES); \
		ramt.lower_constraints() = lowerBound; \
		ramt.upper_constraints() = upperBound; \
		toAwait = cweeOptimizer::run_optimization(shared, ramt, objFunc, isFinishedFunc, maxIterations); \
		break; \
	}
			switch (numDimensions) {
				OPT_FUNC_DIM(1, 10, true)
				OPT_FUNC_DIM(2, 10, true)
				OPT_FUNC_DIM(3, 10, true)
				OPT_FUNC_DIM(4, 10, true)
				OPT_FUNC_DIM(5, 10, true)
				OPT_FUNC_DIM(6, 10, true)
				OPT_FUNC_DIM(7, 10, true)
				OPT_FUNC_DIM(8, 10, true)
				OPT_FUNC_DIM(9, 10, true)
				OPT_FUNC_DIM(10, 10, true)
				OPT_FUNC_DIM(11, 10, true)
				OPT_FUNC_DIM(12, 10, true)
				OPT_FUNC_DIM(13, 10, true)
				OPT_FUNC_DIM(14, 10, true)
				OPT_FUNC_DIM(15, 10, true)
				OPT_FUNC_DIM(16, 10, true)
				OPT_FUNC_DIM(17, 10, true)
				OPT_FUNC_DIM(18, 10, true)
				OPT_FUNC_DIM(19, 10, true)
				OPT_FUNC_DIM(20, 10, true)
			}
			toAwait.AwaitAll();

			auto& bestPolicy = shared->results[shared->results.Num() - 1].bestPolicy;
			out->fit = bestPolicy;
		}
	}
	return *out;
};
float cweeMachineLearning::Forecast_Opt(const cweeMachineLearning::cweeLearnedParams& out, const cweeThreadedList<float>& features) {
	float prediction = 0;
	int x_n;

	prediction = out.labelSummary.min;
	x_n = 0;
	for (int feature_n = 0; feature_n < features.Num(); feature_n++) {
		prediction += (
			out.fit[x_n++] * (
				features.at(feature_n) - (out.featureSummaries[feature_n].min)
				)
			);
	}
	return prediction;
};
cweeThreadedList<float> cweeMachineLearning::Forecast_Opt(const cweeMachineLearning::cweeLearnedParams& out, const cweeThreadedList<cweeThreadedList<float>>& features) {
	cweeThreadedList<float> result;
	float prediction = out.labelSummary.min;
	int x_n;
	if (features.Num() > 0 && features[0].Num() > 0) {
		for (int obs_i = 0; obs_i < features[0].Num(); obs_i++) {
			prediction = out.labelSummary.min;
			x_n = 0;
			for (int feature_n = 0; feature_n < features.Num(); feature_n++) {
				prediction += (
					out.fit[x_n++] * (
						features.at(feature_n)[obs_i] - (out.featureSummaries[feature_n].min)
						)
					);
			}
			result.push_back(prediction);
		}
	}
	return result;
};

cweeMachineLearning::dlibLearnedParams cweeMachineLearning::Learn_Dlib(cweeThreadedList<float> const& trueLabels, cweeThreadedList<cweeThreadedList<float>> const& trueFeatures) {
	dlibLearnedParams out;
	/* Summarize the inputs */
	{
		out.labelSummary = SummarizeFeature(trueLabels);
		for (auto& x : trueFeatures) out.featureSummaries.Append(SummarizeFeature(x));
	}
	/* Meta-optimization of the machine-learning */
	{
		MachineLearningInput input;
		input.labels = trueLabels;
		for (auto& feat : trueFeatures) { input.addFeature(feat); }
		if (input.NumObs() > 0 && input.NumFeatures() > 0) {
			/* Discover optimum selection for gamma and C */
			float gamma = 1.0f, C = 1.0f; {
				cweeThreadedList<float> lowerBound({ 0.1f, 0.1f }), upperBound({ 1250.0f, 1250.0f });

				// MINIMIZE THE ERROR
				AUTO todo = [=](cweeThreadedList<float> const& x) -> float {
					MachineLearningInput temp_input = input;
					temp_input.gamma = x[0];
					temp_input.c = x[1];
					AUTO result = MachineLearning_Math::LearnFast(temp_input);
					return result.performance.x;
				};
				int numDimensions = cweeMath::min(lowerBound.Num(), upperBound.Num());

				constexpr int maxIterations = 1000;
				constexpr float eps = 0.000001f;
				static std::function objFunc = [=](cweeThreadedList<u64>& policy) -> u64 {
					return todo(policy);
				};
				static std::function isFinishedFunc = [=](OptimizationObj& shared, cweeThreadedList<u64>& bestPolicy, u64 bestPerformance) -> bool {
					auto& i = shared.results.Alloc();
					i.bestPolicy = bestPolicy;
					i.bestPerformance = bestPerformance;
					if (shared.results.Num() <= 10) return false; // guarrantee at least 10 iterations
					else {
						// ensure the most recent 5 iterations are better than the 5 before them.
						int i = shared.results.Num() - 1;
						float perf_new = 0;
						float perf_old = 0;
						int count = 0;
						for (; i >= (shared.results.Num() - 5); i--) {
							cweeMath::rollingAverageRef(perf_new, shared.results[i].bestPerformance, count);
						}
						count = 0;
						for (; i >= 0 && i >= (shared.results.Num() - 10); i--) {
							cweeMath::rollingAverageRef(perf_old, shared.results[i].bestPerformance, count);
						}

						// ensure that we are improving. 
						if (cweeMath::Fabs(perf_new - perf_old) < cweeMath::Fabs(bestPerformance * eps)) {
							shared.numIterationsFailedImprovement++;
							if (shared.numIterationsFailedImprovement > (maxIterations / 10)) { // 10% processed and we saw no further improvement. Unlikely to see improvement with another 10%.
								return true;
							}
							else {
								return false;
							}
						}
						shared.numIterationsFailedImprovement = 0;
						return false; // we have not flat-lined. continue.
					}
				};
				cweeSharedPtr<OptimizationObj> shared = make_cwee_shared<OptimizationObj>();

				Genetic_OptimizationManagementTool<true> ramt(2,10); // 2 dimensions, 10 policies per iteration, minimize
				ramt.lower_constraints() = lowerBound; ramt.upper_constraints() = upperBound;
				cweeOptimizer::run_optimization(shared, ramt, objFunc, isFinishedFunc, maxIterations, cweeThreadedList<float>({ gamma, C })).AwaitAll();

				auto& bestPolicy = shared->results[shared->results.Num() - 1].bestPolicy;
				gamma = bestPolicy[0];
				C = bestPolicy[1];
			}
			input.gamma = gamma;
			input.c = C;
			AUTO result = MachineLearning_Math::Learn(input); // using the 'best' version, do the best learn we can do. 
			out.fit = result.svr_results;
		}
	}
	return out;
};
float cweeMachineLearning::Forecast_Dlib(const cweeMachineLearning::dlibLearnedParams& out, const cweeThreadedList<float>& features) {
#ifndef hideMachineLearning
	auto mat = dlib::mat(std::vector<float>(features));
	AUTO ptr = GetDf2(out.fit.df2);
	if (ptr) {
		return ptr->operator()(mat);
	}
#endif
	return 0;
};
cweeThreadedList<float> cweeMachineLearning::Forecast_Dlib(const cweeMachineLearning::dlibLearnedParams& out, const cweeThreadedList<cweeThreadedList<float>>& features) {
	cweeThreadedList<float> results;
	if (features.Num() > 0) {
		results.SetGranularity(features[0].Num() + 16);
#ifndef hideMachineLearning
		AUTO ptr = GetDf2(out.fit.df2);
		if (ptr) {
			for (auto& x : MachineLearningInput::GetInvertedFeatures(features)) results.Append(
				ptr->operator()(dlib::mat(std::vector<float>(x)))
			);
		}
#endif
	}
	return results;
};

MachineLearning_Results cweeMachineLearning::Learn(const cweeThreadedList<float>& trueLabels, const cweeThreadedList<cweeThreadedList<float>>& features, std::pair<vec2, vec2>* fit, const float percentSplit) {
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

		out = LearnSVR(TrueLabels, Features); // to-do, switch statement.
		auto ML_result = Forecast(out, Features);
		vec2 t1 = CalculateError(TrueLabels, ML_result);
		out.performance.x = t1.x;
		out.performance.y = t1.y;
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
		auto ML_result = Forecast(out, Features);
		vec2 t1 = CalculateError(TrueLabels, ML_result);
		out.performance.x = t1.x;
		out.performance.y = t1.y;
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
		// temp. Stupid hard-coded grid search - single elite from several attempts.  
		constexpr int max_i = 1;
		constexpr int max_j = 1;

		float minGamma = 0.1f;
		float maxGamma = 1250.0f;

		float minC = 0.1f;
		float maxC = 1250.0f;

		float previousPerformance = cweeMath::INF;

		float bestGamma;
		float bestC;
		cweeThreadedList<float> forecasted;
		MachineLearning_Results temp;
		float Gamma; float C; vec2 fit1; vec2 fit2; float perf;
		for (int k = 0; k < 2; k++) { // Gamma and C refinement over time
			for (int i = 0; i < (max_i + 1); i++) { // Gamma
				for (int j = 0; j < (max_j + 1); j++) { // C
					Gamma = minGamma + ((maxGamma - minGamma) / max_i) * (((float)i));
					C = minC + ((maxC - minC) / max_j) * (((float)j));

					input.gamma = Gamma;
					input.c = C;

					temp = MachineLearning_Math::Learn(input);

					forecasted = Forecast(temp, features);
					fit1 = CalculateError(trueLabels, forecasted);

					if (testLabels.Num() > 0) {
						forecasted = Forecast(temp, testfeatures);
						fit2 = CalculateError(testLabels, forecasted);
					}
					else {
						fit2 = vec2(1, 0);
					}

					perf = cweeMath::Fmax(fit1.y, fit2.y);

					if (perf < previousPerformance) {
						previousPerformance = perf;
						out = temp;
						bestGamma = Gamma;
						bestC = C;

						if (perf <= 0) return out; // early exit because... it's good enough. 
					}
				}
			}

			if (bestC >= ((maxC + minC) / 2.0f)) {
				minC += ((maxC + minC) / 2.0f);
				maxC *= 2.0f;
			}
			else {
				maxC -= ((maxC + minC) / 2.0f);
				minC *= 0.5f;
			}

			if (bestGamma >= ((maxGamma + minGamma) / 2.0f)) {
				minGamma += ((maxGamma + minGamma) / 2.0f);
				maxGamma *= 2.0f;
			}
			else {
				maxGamma -= ((maxGamma + minGamma) / 2.0f);
				minGamma *= 0.5f;
			}
		}

		input.gamma = bestGamma;
		input.c = bestC;
		out = MachineLearning_Math::Learn(input); // using the 'best' version, do the best learn we can do. 
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

#if 1
			class PatternLearner {
			public:
				bool annualEffect = true;
				bool monthlyEffect = true;
				bool weekdayEffect = false;
				bool hourlyEffect = false;
				bool autoregressive = false;
				cweeUnitPattern Labels;
				cweeList< cweeUnitPattern > AdditionalFeatures;
				double R;
				
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

				cweeAny DLIB;
				PatternSummary LabelSummary;
				cweeList<PatternSummary> AdditionalFeatureSummary;

				cweeUnitValues::unit_value AnnualEstimate(cweeUnitValues::unit_value time) const {
					return (time - LabelSummary.minTime) / (LabelSummary.maxTime - LabelSummary.minTime);
				};
				cweeUnitValues::unit_value MonthEstimate(cweeUnitValues::unit_value time, int month) const {
					cweeTime t(time);
					if ((t.tm_mon() + 1) == month) {
						return 1;
					}
					else {
						return 0;
					}					
				};
				cweeUnitValues::unit_value WeekdayEstimate(cweeUnitValues::unit_value time) const {
					cweeTime t(time);

					if (t.tm_wday() >= 1 && t.tm_wday() <= 5) {
						return 1;
					}
					else {
						return 0;
					}
				};
				cweeUnitValues::unit_value HourEstimate(cweeUnitValues::unit_value time, int hour) const {
					cweeTime t(time);
					if ((t.tm_hour()) == hour) {
						return 1;
					}
					else {
						return 0;
					}
				};
				cweeUnitValues::unit_value AutoregressiveEstimate(cweeUnitValues::unit_value time, cweeUnitPattern const& sampler) const {
					return (sampler.GetCurrentValue(time - LabelSummary.minTimestep) - LabelSummary.minimum) / (LabelSummary.maximum - LabelSummary.minimum);
				};
			
			public:
				cweeUnitValues::second Learn(cweeUnitPattern const& mask, cweeUnitValues::second const& learningBudget = cweeUnitValues::second(5)) {
					// Early exit for bad inputs
					if (Labels.GetNumValues() <= 2) throw std::runtime_error("Machine learning labels (to be predicted) must have more values for learning.");

					// Clear history
					AdditionalFeatureSummary.Clear();
					this->R = 0;
					this->DLIB.Clear();

					// Create summaries
					LabelSummary = PatternSummary(Labels);

					// Create the feature list to learn with
					cweeList<std::pair<PatternSummary*, int>> considerRemovalIndexes;
					cweeList<cweeUnitPattern> features_final; /* 0-1 normalized patterns */ {
						if (LabelSummary.minTime.AreConvertableTypes(cweeUnitValues::second())) { // X-units are time or unitless
							if (annualEffect) {
								auto feature_raw = cweeUnitPattern(LabelSummary.minTime, 1);
								for (auto& x : Labels.GetKnotSeries()) {
									feature_raw.AddUniqueValue(x.first, AnnualEstimate(x.first));
								}
								feature_raw.RemoveUnnecessaryKnots();

								features_final.Append(feature_raw);
							}
							if (monthlyEffect && (cweeUnitValues::month(LabelSummary.maxTime - LabelSummary.minTime) > cweeUnitValues::month(1))) {
								for (int month = 1; month <= 12; month++) {
									auto feature_raw = cweeUnitPattern(LabelSummary.minTime, 1);
									for (auto& x : Labels.GetTimeSeries(LabelSummary.minTime, LabelSummary.maxTime, LabelSummary.minTimestep)) {
										feature_raw.AddValue(x.first, MonthEstimate(x.first, month));
									}
									feature_raw.RemoveUnnecessaryKnots();
									features_final.Append(feature_raw);
								}
							}
							if (weekdayEffect && (cweeUnitValues::day(LabelSummary.maxTime - LabelSummary.minTime) > cweeUnitValues::day(7)) && (LabelSummary.minTimestep <= cweeUnitValues::day(7))) {
								{ // weekdays
									auto feature_raw = cweeUnitPattern(LabelSummary.minTime, 1);
									for (auto& x : Labels.GetTimeSeries(LabelSummary.minTime, LabelSummary.maxTime, LabelSummary.minTimestep)) {
										feature_raw.AddValue(x.first, WeekdayEstimate(x.first));
									}
									feature_raw.RemoveUnnecessaryKnots();
									features_final.Append(feature_raw);
								}
								{ // weekends
									auto feature_raw = cweeUnitPattern(LabelSummary.minTime, 1);
									for (auto& x : Labels.GetTimeSeries(LabelSummary.minTime, LabelSummary.maxTime, LabelSummary.minTimestep)) {
										feature_raw.AddValue(x.first, cweeUnitValues::unit_value(1.0) - WeekdayEstimate(x.first));
									}
									feature_raw.RemoveUnnecessaryKnots();
									features_final.Append(feature_raw);
								}
							}
							if (hourlyEffect && (cweeUnitValues::hour(LabelSummary.maxTime - LabelSummary.minTime) > cweeUnitValues::hour(24)) && (LabelSummary.minTimestep <= cweeUnitValues::hour(1))) {
								for (int hour = 0; hour <= 23; hour++) {
									auto feature_raw = cweeUnitPattern(LabelSummary.minTime, 1);
									for (auto& x : Labels.GetTimeSeries(LabelSummary.minTime, LabelSummary.maxTime, LabelSummary.minTimestep)) {
										feature_raw.AddValue(x.first, HourEstimate(x.first, hour));
									}
									feature_raw.RemoveUnnecessaryKnots();
									features_final.Append(feature_raw);
								}
							}
						}
						if (autoregressive) {
							auto feature_raw = cweeUnitPattern(LabelSummary.minTime, LabelSummary.average);
							for (auto& x : Labels.GetKnotSeries()) {
								feature_raw.AddUniqueValue(x.first, AutoregressiveEstimate(x.first, Labels));
							}
							feature_raw.RemoveUnnecessaryKnots();

							features_final.Append(feature_raw);
						}
						for (auto& AdditionalFeature : AdditionalFeatures) {
							PatternSummary summary{ PatternSummary(AdditionalFeature) };

							auto feature_raw = (AdditionalFeature - summary.minimum) / (summary.maximum - summary.minimum); // 0-1 range							
							feature_raw.RemoveUnnecessaryKnots();
							if (feature_raw.GetNumValues() > Labels.GetNumValues()) {
								feature_raw = feature_raw.Blur(Labels.GetNumValues());
							}

							// test for colinearity;
							bool shouldSkip = false;
							for (auto& y : features_final) {
								if (feature_raw.Collinear(y)) {
									summary.include = false;
									shouldSkip = true;
									break;
								}
							}
							AdditionalFeatureSummary.Append(summary);
							if (shouldSkip) {
								continue;
							}

							considerRemovalIndexes.Append(std::pair<PatternSummary*, int>(&AdditionalFeatureSummary[AdditionalFeatureSummary.Num() - 1], features_final.Num()));

							features_final.Append(feature_raw);
						}
						if (features_final.Num() <= 0) throw std::runtime_error("Features used for machine learning must include at least one (and nothing other than) Pattern objects.");
					}

					// Create a pattern that will be used to determine the "ideal" knot sample points; 
					cweeUnitPattern samplePointTracker; {
						/*
						the Y-axis unit and values don't matter -- what matters is
						that the pattern will automatically determine the intersections across
						each pattern where their values change. This is important for maximizing
						the performance of the machine learning while minimizing the necessary samples.
						*/
						samplePointTracker = Labels * features_final[0];
						for (int i = 1; i < features_final.Num(); i++) {
							cweeUnitPattern samplePointTracker2 = (samplePointTracker * features_final[i]);
							samplePointTracker.Clear();
							samplePointTracker = samplePointTracker2;
						}
					}

					// Matrix / Vectors for actual learning. 
					cweeThreadedList<float>							label_list;
					cweeThreadedList < cweeThreadedList<float> >	feature_list; {
						for (auto& x : features_final) { feature_list.Alloc(); }

						int i;
						for (auto& x : samplePointTracker.GetKnotSeries()) {
							if (mask.GetCurrentValue(x.first) >= 1) {
								continue;
							}
							label_list.Append(Labels.GetCurrentValue(x.first)());
							i = 0;
							for (auto& y : features_final) {
								feature_list[i++].Append(y.GetCurrentValue(x.first)());
							}
						}
					}

					Stopwatch sw; sw.Start();
					do {
						// Initial machine-learning effort
						std::pair<vec2, vec2> fit;
						this->DLIB = cweeMachineLearning::LearnFast(label_list, feature_list, &fit, 10.0);
						this->R = fit.second.x;

						cweeList<int> actualRemovalList_Best;
						while (true) {
							bool anyImprovement = false;

							int tryRemoveWho = 0;
							for (int removeWho = 0; removeWho < considerRemovalIndexes.Num(); removeWho++) {
								cweeList<int> removeList = actualRemovalList_Best;
								removeList.Append(considerRemovalIndexes[removeWho].second);

								cweeThreadedList<cweeThreadedList<float>> feature_list2 = feature_list;
								for (auto& x : removeList) feature_list2.RemoveIndex(x);

								AUTO newML = cweeMachineLearning::LearnFast(label_list, feature_list2, &fit, 1.0);
								if (this->R <= fit.first.x) {
									this->DLIB = newML;
									this->R = fit.first.x;
									actualRemovalList_Best = removeList;
									anyImprovement = true;
								}
							}
							if (!anyImprovement) break;
						}

						for (auto& index : actualRemovalList_Best) {
							for (auto& x : considerRemovalIndexes) {
								if (x.second == index) {
									x.first->include = false;
								}
							}
						}
						for (auto& x : actualRemovalList_Best) feature_list.RemoveIndex(x);
					
						AUTO ML = cweeMachineLearning::Learn(label_list, feature_list, &fit, 10.0);
						if (this->R <= fit.first.x) {
							this->DLIB = ML;
							this->R = fit.first.x;
						}
					
						ML = cweeMachineLearning::Learn(label_list, feature_list, &fit, 10.0);
						if (this->R <= fit.first.x) {
							this->DLIB = ML;
							this->R = fit.first.x;
						}
					} while (cweeUnitValues::second(sw.Stop() / 1000000000.0) < learningBudget);

					return cweeUnitValues::second(sw.Seconds_Passed());
				};
				cweeUnitValues::second Learn() {
					// Early exit for bad inputs
					if (Labels.GetNumValues() <= 2) throw std::runtime_error("Machine learning labels (to be predicted) must have more values for learning.");

					// Clear history
					AdditionalFeatureSummary.Clear();

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
				cweeUnitValues::unit_value Forecast(cweeUnitValues::unit_value time, const cweeUnitPattern* builder = nullptr) const {
					cweeUnitValues::unit_value out = LabelSummary.average;

					MachineLearning_Results& ML_param = this->DLIB.cast();
					AUTO df2 = *GetDf2(ML_param.svr_results.df2);

					std::vector<float> sample; {
						if (LabelSummary.minTime.AreConvertableTypes(cweeUnitValues::second())) { // X-units are time or unitless
							if (annualEffect) {
								sample.push_back(AnnualEstimate(time)());
							}
							if (monthlyEffect && (cweeUnitValues::month(LabelSummary.maxTime - LabelSummary.minTime) > cweeUnitValues::month(1))) {
								for (int month = 1; month <= 12; month++) {
									sample.push_back(MonthEstimate(time, month)());
								}
							}
							if (weekdayEffect && (cweeUnitValues::day(LabelSummary.maxTime - LabelSummary.minTime) > cweeUnitValues::day(7)) && (LabelSummary.minTimestep <= cweeUnitValues::day(7))) {
								{ // weekdays
									sample.push_back(WeekdayEstimate(time)());
								}
								{ // weekends
									sample.push_back((cweeUnitValues::unit_value(1.0) - WeekdayEstimate(time))());
								}
							}
							if (hourlyEffect && (cweeUnitValues::hour(LabelSummary.maxTime - LabelSummary.minTime) > cweeUnitValues::hour(24)) && (LabelSummary.minTimestep <= cweeUnitValues::hour(1))) {
								for (int hour = 0; hour <= 23; hour++) {
									sample.push_back(HourEstimate(time, hour)());
								}
							}
						}
						if (autoregressive) {
							if (builder && builder->GetNumValues() > 0) {
								sample.push_back(AutoregressiveEstimate(time, *builder)());
							}
							else {
								sample.push_back(AutoregressiveEstimate(time, Labels)());
							}
						}
						for (int i = 0; i < AdditionalFeatures.Num(); i++) {
							auto& AdditionalFeature = AdditionalFeatures[i];
							auto& summary = AdditionalFeatureSummary[i];
							if (summary.include) {
								sample.push_back(((AdditionalFeature.GetCurrentValue(time) - summary.minimum) / (summary.maximum - summary.minimum))() /* 0-1 range */);
							}
						}
					}
					out = df2(dlib::mat(sample));
					return out;
				};
				cweeUnitPattern Forecast(cweeUnitValues::unit_value time1, cweeUnitValues::unit_value time2, cweeUnitValues::unit_value timestep) const {
					auto out{ cweeUnitPattern(LabelSummary.minTime, LabelSummary.average) };

					for (cweeUnitValues::unit_value time = time1; time <= time2; time += timestep) {
						out.AddValue(time, Forecast(time, &out));
					}

					return out;
				};
				cweeUnitPattern Forecast() const {
					return Forecast(LabelSummary.minTime, LabelSummary.maxTime, LabelSummary.minTimestep);
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
			lib->add(chaiscript::fun(&PatternLearner::autoregressive), "autoregressive");
			lib->add(chaiscript::fun(&PatternLearner::Labels), "Labels");
			DEF_DECLARE_VECTOR_WITH_SCRIPT_ENGINE_AND_MODULE(cweeUnitPattern);
			lib->add(chaiscript::fun(&PatternLearner::AdditionalFeatures), "AdditionalFeatures");
			lib->add(chaiscript::fun(&PatternLearner::R), "R");
			
			lib->AddFunction(, Learn, -> PatternLearner, SINGLE_ARG(
				PatternLearner learn;
				learn.Labels = pat;
				learn.annualEffect = true;
				learn.monthlyEffect = true;
				learn.weekdayEffect = false;
				learn.hourlyEffect = false;
				learn.autoregressive = false;
				learn.Learn();
				return learn;
			), cweeUnitPattern const& pat);
			lib->AddFunction(, Learn, -> PatternLearner, SINGLE_ARG(
				PatternLearner learn;
				learn.Labels = pat;
				learn.annualEffect = true;
				learn.monthlyEffect = true;
				learn.weekdayEffect = false;
				learn.hourlyEffect = false;
				learn.autoregressive = false;
				for (auto& x : features) {
					auto* p = chaiscript::boxed_cast<cweeUnitPattern*>(x);
					if (p) learn.AdditionalFeatures.Append(*p);
				}
				learn.Learn();
				return learn;
			), cweeUnitPattern const& pat, std::vector<Boxed_Value> const& features);
			lib->AddFunction(, Learn, -> PatternLearner, SINGLE_ARG(
				PatternLearner learn;
				learn.Labels = pat;
				learn.annualEffect = true;
				learn.monthlyEffect = true;
				learn.weekdayEffect = false;
				learn.hourlyEffect = false;
				learn.autoregressive = false;
				for (auto& x : features) {
					auto* p = chaiscript::boxed_cast<cweeUnitPattern*>(x);
					if (p) learn.AdditionalFeatures.Append(*p);
				}
				learn.Learn(mask);
				return learn;
			), cweeUnitPattern const& pat, std::vector<Boxed_Value> const& features, cweeUnitPattern const& mask);
			lib->AddFunction(, Learn, -> PatternLearner, SINGLE_ARG(
				PatternLearner learn;
				learn.Labels = pat;
				learn.annualEffect = true;
				learn.monthlyEffect = true;
				learn.weekdayEffect = false;
				learn.hourlyEffect = false;
				learn.autoregressive = false;
				for (auto& x : features) {
					auto* p = chaiscript::boxed_cast<cweeUnitPattern*>(x);
					if (p) learn.AdditionalFeatures.Append(*p);
				}
				learn.Learn(mask, learningBudget);
				return learn;
			), cweeUnitPattern const& pat, std::vector<Boxed_Value> const& features, cweeUnitPattern const& mask, cweeUnitValues::unit_value const& learningBudget);
			lib->AddFunction(, Learn, -> cweeUnitValues::unit_value, return learned.Learn(), PatternLearner& learned);
			lib->AddFunction(, Learn, -> cweeUnitValues::unit_value, return learned.Learn(mask), PatternLearner& learned, cweeUnitPattern const& mask);
			lib->AddFunction(, Forecast, -> cweeUnitPattern, return learned.Forecast(), PatternLearner const& learned);
			lib->AddFunction(, Forecast, -> cweeUnitPattern, return learned.Forecast(time1, time2, timestep), PatternLearner const& learned, cweeUnitValues::unit_value const& time1, cweeUnitValues::unit_value const& time2, cweeUnitValues::unit_value const& timestep);
			lib->AddFunction(, Forecast, -> cweeUnitValues::unit_value, return learned.Forecast(sampleTime), PatternLearner const& learned, cweeUnitValues::unit_value const& sampleTime);

#else
			class PatternLearner {
			public:
				cweeAny DLIB;
				Boxed_Value Features;
				cweeUnitValues::unit_value Units;
				std::vector<Boxed_Value> Fit;
				double R;

				cweeUnitPattern Forecast() const {
					cweeUnitPattern out;
					std::vector<Boxed_Value>* features = chaiscript::boxed_cast<std::vector<Boxed_Value>*>(Features);
					if (features) {
						cweeUnitPattern* pat = chaiscript::boxed_cast<cweeUnitPattern*>(features->operator[](0));
						if (pat) {
							out = cweeUnitPattern(pat->X_Type(), Units);
						}
					}
					int i = 0;
					for (auto& xPtr : *features) {
						cweeUnitPattern* x = chaiscript::boxed_cast<cweeUnitPattern*>(xPtr);
						if (x) {
							double* FitRatio = chaiscript::boxed_cast<double*>(Fit[++i]);
							if (FitRatio) {
								out += ((*x) * (*FitRatio));
							}
						}
					}
					double* FitRatioFinal = chaiscript::boxed_cast<double*>(Fit[0]);
					if (FitRatioFinal) out += *FitRatioFinal;
					
					return out;
				};
				cweeUnitPattern ForecastDLib() const {
					MachineLearning_Results& ML_param = DLIB.cast();
					AUTO df2 = *GetDf2(ML_param.svr_results.df2);
					
					cweeUnitPattern out;
					cweeUnitValues::unit_value minT = 0, maxT = 0, step = 1;
					std::vector<Boxed_Value>* features = chaiscript::boxed_cast<std::vector<Boxed_Value>*>(Features);
					if (features) {
						cweeUnitPattern* pat = chaiscript::boxed_cast<cweeUnitPattern*>(features->operator[](0));
						if (pat) {
							out = cweeUnitPattern(pat->X_Type(), Units);
							minT = pat->GetMinTime();
							maxT = pat->GetMaxTime();
							step = pat->GetMinimumTimeStep();
						}
					}
					
					std::vector<float> sample;
					AUTO type = out.Y_Type();
					for (cweeUnitValues::unit_value t = minT; t <= maxT; t += step) {
						sample.clear();
						for (auto& xPtr : *features) {
							cweeUnitPattern* x = chaiscript::boxed_cast<cweeUnitPattern*>(xPtr);
							if (x) {
								sample.push_back(x->GetCurrentValue(t)());
							}
						}
						out.AddValue(t, (type = df2(dlib::mat(sample))));
					}

					return out;
				};
			};
			lib->add(chaiscript::user_type<PatternLearner>(), "PatternLearner");
			lib->add(chaiscript::constructor<PatternLearner()>(), "PatternLearner");
			lib->add(chaiscript::constructor<PatternLearner(const PatternLearner&)>(), "PatternLearner");
			lib->add(chaiscript::fun([](PatternLearner& a, const PatternLearner& b)->PatternLearner& { a = b; return a; }), "=");
			lib->add(chaiscript::fun(&PatternLearner::Features), "Features");
			lib->add(chaiscript::fun(&PatternLearner::Units), "Units");
			lib->add(chaiscript::fun(&PatternLearner::Fit), "Fit");
			lib->add(chaiscript::fun(&PatternLearner::R), "R");
			lib->add(chaiscript::fun(&PatternLearner::Forecast), "Forecast");

			auto LearnPatternsDLib = [](cweeUnitPattern const& labels, std::vector<Boxed_Value> const& features) {
				if (features.size() <= 0) { throw std::runtime_error("Features used for machine learning must include at least one (and nothing other than) Pattern objects."); }
				AUTO Range{ labels.GetMaxValue() - labels.GetMinValue() };
				AUTO avg{ labels.GetAvgValue() };
				if (avg < 0) { avg *= -1; }
				std::vector<double> minVector{ { -1.0 * cweeUnitValues::math::abs(avg + Range)() } };
				std::vector<double> maxVector{ { cweeUnitValues::math::abs(avg + Range)() } };
				cweeList<cweeUnitPattern> features_final;
				bool shouldSkip;
				for (auto& x : features) {
					AUTO VPtr{ chaiscript::boxed_cast<cweeUnitPattern*>(x) };
					if (VPtr) {
						AUTO V{ cweeUnitPattern(*VPtr) };

						AUTO rangeV{ V.GetMaxValue() - V.GetMinValue() };
						V -= V.GetMinValue();
						V /= rangeV;
						V.RemoveUnnecessaryKnots();

						// test for colinearity;
						shouldSkip = false;
						for (auto& y : features_final) {
							if (V.Collinear(y)) {
								shouldSkip = true;
								break;
							}
						}
						if (shouldSkip) { continue; }

						minVector.push_back(-1.0 * cweeUnitValues::math::abs(avg + Range)());
						maxVector.push_back(cweeUnitValues::math::abs(avg + Range)());
						features_final.push_back(V);
					}
					else {
						throw std::runtime_error("Features used for machine learning must include at least one (and nothing other than) Pattern objects.");
					}
				}

				auto maxT = labels.GetMaxTime();
				auto minT = labels.GetMinTime();

				for (auto& x : features_final) {
					maxT = cweeUnitValues::math::min(maxT, x.GetMaxTime());
					minT = cweeUnitValues::math::max(minT, x.GetMinTime());
				}

				cweeUnitValues::second timestep = cweeUnitValues::math::max(labels.GetMinimumTimeStep(), (maxT - minT) / 5000);

				double numSteps = ((maxT - minT) / timestep)();

				cweeThreadedList<float>							label_list;
				cweeThreadedList < cweeThreadedList<float> >	feature_list;

#if 0
				{
					AUTO labels_blurred = labels.Blur(numSteps);
					cweeThreadedList < cweeUnitPattern > features_blurred;
					for (auto& x : features_final) {
						features_blurred.Append(x.Blur(numSteps));
						feature_list.Alloc();
					}

					int i;
					for (cweeUnitValues::unit_value t = minT; t <= maxT; t += timestep) {
						label_list.Append(labels_blurred.GetCurrentValue(t)());
						i = 0;
						for (auto& x : features_blurred) {
							feature_list[i++].Append(x.GetCurrentValue(t)());
						}
					}
				}
#else
				{
					for (auto& x : features_final) {
						feature_list.Alloc();
					}

					int i;
					for (auto& x : labels.GetTimeSeries(minT, maxT, timestep)) {
						label_list.Append(x.second());
						i = 0;
						for (auto& y : features_final) {
							feature_list[i++].Append(y.GetCurrentValue(x.first)());
						}
					}
				}
#endif
				double R_SQR_PERF = 0;
				
				std::vector<int> includeList_Best(feature_list.Num(), 1);

				std::pair<vec2, vec2> fit;
				MachineLearning_Results ML_param = cweeMachineLearning::Learn(label_list, feature_list, &fit, 10);
				R_SQR_PERF = fit.second.x;

				while (true) {
					bool anyImprovement = false;
					int tryRemoveWho = 0;
					for (int removeWho = 0; removeWho < feature_list.Num(); removeWho++) {
						std::vector<int> includeList = includeList_Best;
						includeList[removeWho] = 0;

						cweeThreadedList < cweeThreadedList<float> >	feature_list2;
						for (int i = 0; i < feature_list.Num(); i++) {
							if (includeList[i] > 0) {
								feature_list2.push_back(feature_list[i]);
							}
						}
						ML_param = cweeMachineLearning::LearnFast(label_list, feature_list2, &fit, 10);
						if (R_SQR_PERF <= fit.second.x) {
							R_SQR_PERF = fit.second.x;
							includeList_Best = includeList;
							anyImprovement = true;
						}
					}
					if (!anyImprovement) break;
				}

				for (int i = includeList_Best.size() - 1; i >= 0; i--) {
					if (includeList_Best[i] <= 0) {
						feature_list.RemoveIndex(i);
						features_final.RemoveIndex(i);
					}
				}
				
				ML_param = cweeMachineLearning::Learn(label_list, feature_list, &fit, 0.05);

				AUTO ptr = GetDf2(ML_param.svr_results.df2);

				PatternLearner out;
				out.DLIB = ML_param;
				out.Fit.push_back(var((double)(ptr->b * -1)));
				for (auto& x : ptr->alpha) {
					out.Fit.push_back(var((double)(x)));
				}
				std::vector<Boxed_Value> FeaturesBV; for (auto& x : features_final) { FeaturesBV.push_back(var((cweeUnitPattern)x)); }; out.Features = var(std::move(FeaturesBV));
				out.Units = avg;
				out.R = labels.R_Squared(out.ForecastDLib())();

				return out;
			};


			auto LearnPatterns = [](cweeUnitPattern const& labels, std::vector<Boxed_Value> const& features)->PatternLearner {
				if (features.size() <= 0) {
					throw std::runtime_error("Features used for machine learning must include at least one (and nothing other than) Pattern objects.");
				}

				AUTO Label{ cweeUnitPattern(labels) };
				Label.RemoveUnnecessaryKnots();

				AUTO Range{ Label.GetMaxValue() - Label.GetMinValue() };

				AUTO avg{ Label.GetAvgValue() };
				if (avg < 0) { avg *= -1; }

				std::vector<double> minVector{ { -1.0 * cweeUnitValues::math::abs(avg + Range)() } };
				std::vector<double> maxVector{ { cweeUnitValues::math::abs(avg + Range)() } };
				std::vector<cweeUnitPattern> features_final;
				bool shouldSkip;
				for (auto& x : features) {
					AUTO VPtr{ chaiscript::boxed_cast<cweeUnitPattern*>(x) };
					if (VPtr) {
						AUTO V{ cweeUnitPattern(*VPtr) };

						AUTO rangeV{ V.GetMaxValue() - V.GetMinValue() };
						V -= V.GetMinValue();
						V /= rangeV;
						V.RemoveUnnecessaryKnots();


						// test for colinearity;
						shouldSkip = false;
						for (auto& y : features_final) {
							if (V.Collinear(y)) {
								shouldSkip = true;
								break;
							}
						}
						if (shouldSkip) { continue; }

						minVector.push_back(-1.0 * cweeUnitValues::math::abs(avg + Range)());
						maxVector.push_back(cweeUnitValues::math::abs(avg + Range)());
						features_final.push_back(V);
					}
					else {
						throw std::runtime_error("Features used for machine learning must include at least one (and nothing other than) Pattern objects.");
					}
				}

				auto maxT = Label.GetMaxTime();
				auto minT = Label.GetMinTime();
				AUTO labelIntegration{ Label.RombergIntegral(minT, maxT) };

				double N_steps = cweeMath::max(10, Label.GetNumValues());
				AUTO realLabels = Label.GetValueTimeSeries(minT, maxT, (maxT - minT) * (1.0 / N_steps));

				AUTO GetRSQR = [&labelIntegration, &features_final, &Label, &minT, &maxT, &avg, &N_steps, &realLabels](std::vector<double> const& params)->double {
					AUTO v{ cweeUnitPattern(minT, avg) };
					int i = 0;
					std::vector<std::pair<cweeUnitPattern*, double>> features_ref; {
						for (i = 0; i < features_final.size(); i++) { features_ref.push_back({ &features_final[i] , params[i + 1] }); };
						std::shuffle(features_ref.begin(), features_ref.end(), std::mt19937{ std::random_device{}() }); // order of addition does not matter, so the shuffle helps reduce competition between threads.
					}

					for (auto& feature : features_ref) { v += *feature.first * feature.second; } 
					v += params[0];
					v *= (labelIntegration / v.RombergIntegral(minT, maxT));

					// return Label.R_Squared(v, minT, maxT)() * 1000000.0; 
					return v.R_Squared(N_steps, realLabels, minT, maxT)() * 1000000.0;
				};

				AUTO bestFit = ChaiscriptOptimizations::OptimizeFunction_Internal(false, GetRSQR, minVector, maxVector, "Genetic");

				PatternLearner out;
				for (auto& x : bestFit) { out.Fit.push_back(var((double)x)); };
				std::vector<Boxed_Value> FeaturesBV; for (auto& x : features_final) { FeaturesBV.push_back(var((cweeUnitPattern)x)); }; out.Features = var(std::move(FeaturesBV));
				out.Units = avg;
				out.R = GetRSQR(bestFit) / 1000000.0;

				// adjust the fit by multiplying all the fit parameters by the ratio
				AUTO ratio{ labelIntegration / (out.Forecast().RombergIntegral()) };
				for (auto& x : out.Fit) {
					auto* xBV = chaiscript::boxed_cast<double*>(x);
					if (xBV) {
						*xBV *= ratio();
					}					
				}
				return out;
			};
			lib->add(chaiscript::fun(LearnPatterns), "Learn");
			lib->add(chaiscript::fun(LearnPatternsDLib), "LearnDLib");
			lib->add(chaiscript::fun(&PatternLearner::ForecastDLib), "ForecastDLib");

			lib->eval(
R"chaiscript(
			def Learn(Pattern labels, Vector Features, bool AddTimeFeatures) {
				Vector features; for (int i = 0; i < Features.size; ++i){ features.push_back(Features[i]); };			
				if (AddTimeFeatures) {
					if (1){ // month-specific factors
						for (int i = 0; i < 12; ++i){ 
							var& x = Pattern(1_s, 1);
							x.SetInterpolationType("LEFT");
							cweeTime minT = labels.GetMinTime.second;
							cweeTime maxT = labels.GetMaxTime.second;
							for (cweeTime t = minT; t <= maxT; t += 1_d){
								x.AddValue(t, t.tm_mon == i ? 1 : 0);
							}	
							x.RemoveUnnecessaryKnots();
							features.push_back(x);
						}
					}
					if (1) { // over-time gradual growth or decline
						var& x = Pattern(1_s, 1);
						x.SetInterpolationType("LINEAR");		
						cweeTime minT = labels.GetMinTime.second;
						cweeTime maxT = labels.GetMaxTime.second;						
						for (cweeTime t = minT; t <= maxT; t += ((maxT - minT) / 10)){
							x.AddValue(t.second, (t.second - minT).double);
						}
						features.push_back(x);
					}
				}					
				return Learn(labels, features);
			};
			def Learn(Pattern labels, bool AddTimeFeatures) {				
				return Learn(labels, Vector(), AddTimeFeatures);
			};
			def Learn(Pattern labels) {				
				return Learn(labels, Vector(), true);
			};
			def PatternLearner::Forecast(value time) {
				double out;
				int i = 0;
				var& fit = this.Fit;
				for (x : this.Features){
					out += double(x.GetCurrentValue(time) * fit[++i]);
				}
				out += fit[0];
				value x = this.Units;
				x = out;
				return x;
			};

            def LearnDLib(Pattern labels, Vector Features, bool AddTimeFeatures) {
				Vector features; for (int i = 0; i < Features.size; ++i){ features.push_back(Features[i]); };			
				if (AddTimeFeatures) {
					if (1){ // month-specific factors
						for (int i = 0; i < 12; ++i){ 
							var& x = Pattern(1_s, 1);
							x.SetInterpolationType("LEFT");
							cweeTime minT = labels.GetMinTime.second;
							cweeTime maxT = labels.GetMaxTime.second;
							for (cweeTime t = minT; t <= maxT; t += 1_d){
								x.AddValue(t, t.tm_mon == i ? 1 : 0);
							}	
							x.RemoveUnnecessaryKnots();
							features.push_back(x);
						}
					}
					if (1) { // over-time gradual growth or decline
						var& x = Pattern(1_s, 1);
						x.SetInterpolationType("LINEAR");		
						cweeTime minT = labels.GetMinTime.second;
						cweeTime maxT = labels.GetMaxTime.second;						
						for (cweeTime t = minT; t <= maxT; t += ((maxT - minT) / 10)){
							x.AddValue(t.second, (t.second - minT).double);
						}
						features.push_back(x);
					}
				}					
				return LearnDLib(labels, features);
			};
			def LearnDLib(Pattern labels, bool AddTimeFeatures) {				
				return LearnDLib(labels, Vector(), AddTimeFeatures);
			};
			def LearnDLib(Pattern labels) {				
				return LearnDLib(labels, Vector(), true);
			};



)chaiscript");
#endif
			return lib;
		};
	};
}; // namespace chaiscript